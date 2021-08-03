#ifndef BAD_RESPONSE_HPP_INCLUDED
#define BAD_RESPONSE_HPP_INCLUDED

namespace web_server
{

  template<class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::string_body> bad_response(
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
    boost::beast::http::status status, std::string const& body_text)
  {
    boost::beast::http::response<boost::beast::http::string_body> res{status, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = body_text;
    res.prepare_payload();

    return res;
  }

} // ns

#endif // BAD_RESPONSE_HPP_INCLUDED