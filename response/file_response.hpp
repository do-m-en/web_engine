#ifndef FILE_RESPONSE_HPP_INCLUDED
#define FILE_RESPONSE_HPP_INCLUDED

namespace web_server
{

  template<class Body, class Allocator,class Send>
  boost::asio::awaitable<void> file_response(
    Page_root_path const& paths,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
    Send&& send)
  {
    // Returns a not found response
    auto const not_found =
      [&req](boost::beast::string_view target)
      {
        return bad_response(req, boost::beast::http::status::not_found, "The resource '" + std::string{target} + "' was not found.");
      };

    // Returns a server error response
    auto const server_error =
      [&req](boost::beast::string_view what)
      {
        return bad_response(req, boost::beast::http::status::internal_server_error, "An error occurred: '" + std::string{what} + "'");
      };

    std::filesystem::path file_path = paths.static_root;
    file_path += req.target();
    if(req.target().back() == '/')
      file_path /= "index.html";

    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    //body.open(file_path.string().c_str(), boost::beast::file_mode::scan, ec);
    body.open(file_path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == boost::system::errc::no_such_file_or_directory)
      co_return co_await send(not_found(req.target()));

    // Handle an unknown error
    if(ec)
      co_return co_await send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == boost::beast::http::verb::head)
    {
      std::cout << "head???" << std::endl;

      boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
      res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(boost::beast::http::field::content_type, mime_type(file_path.extension()));
      res.content_length(size);
      res.keep_alive(req.keep_alive());
      co_return co_await send(std::move(res));
    }

    // Respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> res{
      std::piecewise_construct,
      std::make_tuple(std::move(body)),
      std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, mime_type(file_path.extension()));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    co_await send(std::move(res));
  }

} // ns

#endif // FILE_RESPONSE_HPP_INCLUDED