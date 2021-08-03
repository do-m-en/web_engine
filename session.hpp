#ifndef SESSION_HPP_INCLUDED
#define SESSION_HPP_INCLUDED

#include <boost/dll/import.hpp> // TODO move elsewhere
#include "view_plugin.hpp"
#include "template_parser.hpp"
#include "logged_in_session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/redirect_error.hpp>

#include <fmt/format.h>

#include <iostream>
#include <fstream>

#include <filesystem>
#include <algorithm>
#include <string_view>

#include "util/page_paths.hpp"
#include "util/mime_type.hpp"
#include "response/bad_response.hpp"
#include "response/view_response.hpp"
#include "response/file_response.hpp"

namespace web_server
{

// Report a failure
void fail(boost::system::error_code ec, char const* what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

template<class Body, class Allocator,class Send>
boost::asio::awaitable<std::optional<Page_root_path>> check_request(
  boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
  Send&& send)
{
  // Returns a bad request response
  auto const bad_request =
    [&req](boost::beast::string_view why)
    {
      return bad_response(req, boost::beast::http::status::bad_request, std::string{why});
    };

  std::filesystem::path request_to_host{"./pages/" + std::string{req.base()[boost::beast::http::field::host]}};

  if(!std::filesystem::exists(request_to_host))
  {
    co_await send(bad_request("Unknown host"));
    co_return std::optional<Page_root_path>{};
  }

  // Make sure we can handle the method
  if(req.method() != boost::beast::http::verb::get &&
    req.method() != boost::beast::http::verb::head &&
    req.method() != boost::beast::http::verb::post)
  {
    co_await send(bad_request("Unknown HTTP-method"));
    co_return std::optional<Page_root_path>{};
  }

  // Request path must be absolute and not contain "..".
  if( req.target().empty() ||
    req.target()[0] != '/' ||
    req.target().find("..") != boost::beast::string_view::npos)
  {
    co_await send(bad_request("Illegal request-target"));
    co_return std::optional<Page_root_path>{};
  }

  co_return std::optional<Page_root_path>{request_to_host};
}

template<typename Body, typename Allocator>
std::optional<View_paths> get_requested_view_paths(
  Page_root_path const& paths,
  boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req)
{
  std::filesystem::path file_path = paths.dynamic_root;
  file_path += std::filesystem::path{req.target()};
  if(req.target().back() == '/' ||
    file_path.filename() == "index.html")
  {
    file_path = paths.dynamic_root;
    file_path += "/index";
  }

  std::filesystem::path view_path =
      file_path.parent_path()
    / (file_path.filename().c_str() + std::string{ ".template" });

  if(std::filesystem::exists(view_path))
  {
    return View_paths{view_path, file_path};
  }

  return {};
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
template<class Stream>
struct send_lambda
{
  Stream& stream_;
  bool& close_;
  boost::system::error_code& ec_;

  explicit
  send_lambda(
    Stream& stream,
    bool& close,
    boost::system::error_code& ec)
    : stream_(stream)
    , close_(close)
    , ec_(ec)
  {
  }

  template<bool isRequest, class Body, class Fields>
  boost::asio::awaitable<void>
  operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const // FIXME on linux && can be used while on windows it causes an error because file copy constructor is deleted
  {
    auto executor = co_await boost::asio::this_coro::executor;

    // Determine if we should close the connection after
    close_ = msg.need_eof();

    // We need the serializer here because the serializer requires
    // a non-const file_body, and the message oriented version of
    // http::write only works with const messages.
    boost::beast::http::serializer<isRequest, Body, Fields> sr{msg};
    co_await boost::beast::http::async_write(stream_, sr, boost::asio::use_awaitable /*redirect_error(executor, ec_)*/);
  }
};

// Handles an HTTP server connection
template<typename Socket>
boost::asio::awaitable<void> do_session(
  Socket&& socket,
  boost::beast::flat_buffer&& buffer)
{
  bool close = false;
  boost::system::error_code ec;

  // This lambda is used to send messages
  send_lambda<Socket> lambda{socket, close, ec};

  for(;;)
  {
    // Read a request
    boost::beast::http::request<boost::beast::http::string_body> req;
    co_await boost::beast::http::async_read(socket, buffer, req, boost::asio::use_awaitable /*, redirect_error(executor, ec)*/);
    if(ec == boost::beast::http::error::end_of_stream)
      break;
    if(ec)
      co_return fail(ec, "read");

    // Send the response
    auto paths = co_await check_request(req, lambda);

    if(!paths.has_value())
      break;

    auto view_paths = get_requested_view_paths(paths.value(), req);

    if(view_paths.has_value())
      co_await view_response(paths.value(), view_paths.value(), req, lambda);
    else
      co_await file_response(paths.value(), req, lambda);

    /*if(ec)
      co_return fail(ec, "write");*/
    if(close)
    {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      break;
    }
  }

  if constexpr(std::is_same_v<boost::asio::ip::tcp::socket, Socket>)
  {
    // Send a TCP shutdown
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
  }
  else
  {
    using namespace std::chrono_literals;
    // Set the timeout.
    boost::beast::get_lowest_layer(socket).expires_after(30s);

    // Perform the SSL shutdown
    co_await socket.async_shutdown(boost::asio::use_awaitable/*redirect_error(executor, ec)*/);

    /*if(ec)
        co_return fail(ec, "shutdown");*/
  }

  // At this point the connection is closed gracefully
}

} // ns

#endif // SESSION_HPP_INCLUDED
