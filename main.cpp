#include "listener.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <string_view>

#include <boost/asio/co_spawn.hpp>

#include <boost/spirit/home/x3.hpp>

using boost::asio::co_spawn;
using boost::asio::detached;

template <typename T>
std::vector<unsigned short> parse_numbers(T range)
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = boost::spirit::x3::ascii;

    using x3::double_;
    using x3::phrase_parse;
    using x3::_attr;
    using ascii::space;

    std::vector<unsigned short> v;

    auto begin = range.begin();
    auto end = range.end();

    bool r = phrase_parse(begin, end, ( double_ % ',' ), space, v);

    if (begin != end) // fail if we did not get a full match
        return {};
    return v;
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    std::cerr <<
      "Usage: web_engine <address> <ports> <threads>\n" <<
      "Example:\n" <<
      "    web_engine 0.0.0.0 8080 1\n";

    return 1;
  }

  auto const address = boost::asio::ip::make_address(argv[1]);
  auto const ports = parse_numbers(std::string_view(argv[2]));
  auto const threads = std::max<int>(1, std::atoi(argv[5]));

  if(ports.empty())
  {
    std::cerr << "No ports set!\n";

    return 1;
  }

  // The io_context is required for all I/O
  boost::asio::io_context ioc{threads};

  tls_sni_sample::Basic_cert_data_provider crt_provider;
  tls_sni::Basic_context_provider ctx_provider{crt_provider};

  for(unsigned short port : ports)
  {
    co_spawn(ioc,
      [&address, port, &ctx_provider]() -> boost::asio::awaitable<void>
      {
        co_await
          web_server::listen(
            boost::asio::ip::tcp::endpoint{address, port},
            ctx_provider);
      },
      detached);
  }

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for(auto i = threads - 1; i > 0; --i)
    v.emplace_back(
    [&ioc]
    {
      ioc.run();
    });
  ioc.run();

  return 0;
}
