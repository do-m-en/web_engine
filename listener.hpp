#ifndef LISTENER_HPP_INCLUDED
#define LISTENER_HPP_INCLUDED

#include "session.hpp"
#include "tls_sni_context.hpp"
#include "cert_data_provider.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/redirect_error.hpp>

#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core/detect_ssl.hpp>

#include "cert_data_provider.hpp"

#include <filesystem>

namespace web_server
{

auto tls_sln_handshake(
  boost::asio::ip::tcp::socket&& socket,
  tls_sni::Basic_context_provider<tls_sni_sample::Basic_cert_data_provider>& ctx_provider,
  boost::beast::flat_buffer& buffer)
  -> boost::asio::awaitable<boost::beast::ssl_stream<boost::beast::tcp_stream>>
{
    tls_sni::Context ctx{ctx_provider};
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream{
      std::move(socket),
      ctx};
    std::size_t bytes_used =
      co_await stream.async_handshake(
        boost::asio::ssl::stream_base::server,
        buffer.data(),
        boost::asio::use_awaitable);

    // Consume the portion of the buffer used by the handshake
    buffer.consume(bytes_used);

    co_return stream;
}

// Accepts incoming connections and launches the sessions
boost::asio::awaitable<void> listen(
  boost::asio::ip::tcp::endpoint endpoint,
  tls_sni::Basic_context_provider<tls_sni_sample::Basic_cert_data_provider>& ctx_provider)
{
  auto executor = co_await boost::asio::this_coro::executor;
  boost::asio::ip::tcp::acceptor acceptor(executor, endpoint); // TODO system_error exception handling

  for (;;)
  {
    boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
    co_spawn(executor,
        [socket = std::move(socket), &ctx_provider]() mutable -> boost::asio::awaitable<void>
        {
          // TODO system_error exception handling
          // This buffer is required to persist across reads
          boost::beast::flat_buffer buffer;

          bool is_ssl =
            co_await
              boost::beast::async_detect_ssl(
                socket,
                buffer,
                boost::asio::use_awaitable);

          if(is_ssl)
          {
            auto ssl_stream =
              co_await tls_sln_handshake(
                std::move(socket),
                ctx_provider,
                buffer);
            co_await do_session(std::move(ssl_stream), std::move(buffer));
          }
          else
            co_await do_session(std::move(socket), std::move(buffer));
        },
        boost::asio::detached);
  }
}

} // ns

#endif // LISTENER_HPP_INCLUDED
