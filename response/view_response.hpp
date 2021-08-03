#ifndef VIEW_RESPONSE_HPP
#define VIEW_RESPONSE_HPP

#include "view_response/get.hpp"
#include "view_response/post.hpp"
#include "view_response/shared_lib.hpp"

namespace web_server
{

  template<class Body, class Allocator,class Send>
  boost::asio::awaitable<void> view_response(
    Page_root_path const& paths,
    View_paths const& view_paths,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
    Send&& send)
  {
    // Returns a server error response
    auto const server_error =
      [&req](boost::beast::string_view what)
      {
        return bad_response(req, boost::beast::http::status::internal_server_error, "An error occurred: '" + std::string{what} + "'");
      };

    auto items = template_to_items(view_paths.view_path);
    std::string shared_lib_name;
    std::string base_template_name;
    std::string redirect_uri;
    bool allow_unsecured = false;

    for (auto const& i : items)
    {
      if (std::get<0>(i) == "shared_lib_name")
        shared_lib_name = std::get<1>(i);
      else if (std::get<0>(i) == "base_template_name")
        base_template_name = std::get<1>(i);
      else if (std::get<0>(i) == "redirect_uri")
        redirect_uri = std::get<1>(i);
      else if (std::get<0>(i) == "allow_unsecured")
        allow_unsecured = true;
    }

    std::tuple<cpp_web_server::Content_type, std::string> content;
    bool redirect = false;

    std::string session_cookie{ /*req[http::field::cookie].empty() ? req[http::field::cookie].data() : */"" };

    if (!redirect)
    {
      if (shared_lib_name.empty())
      {
        if (redirect_uri.empty())
        {
          if (req.method() == boost::beast::http::verb::post)
          {
            if (view_paths.file_path.filename().c_str() != std::string{ "create_admin_user" })
              co_return co_await send(server_error("internal error"));

            content = get_post_content(paths, req, items);
          }
          else
          {
            if (base_template_name.empty())
              co_return co_await send(server_error("internal error"));

            std::filesystem::path base_template_path = paths.dynamic_root / std::string{ base_template_name.begin(), base_template_name.end() };

            std::ifstream in{ base_template_path.c_str() };

            if (!in)
              co_return co_await send(server_error("internal error"));

            content = get_get_content(in, items);
          }
        }
        else
          content = { cpp_web_server::Content_type::redirect_uri, redirect_uri };
      }
      else
      {
        auto resp = get_shared_lib_content(paths, shared_lib_name, req, items);
        if (!resp.has_value())
          co_return co_await send(server_error("internal error"));

        content = std::move(resp.value());
      }
    }

    boost::beast::http::response<boost::beast::http::string_body> res{ std::get<0>(content) == cpp_web_server::Content_type::view ? boost::beast::http::status::ok : boost::beast::http::status::found, req.version() };
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");

    if (!session_cookie.empty())
      res.set(boost::beast::http::field::set_cookie, session_cookie);

    res.keep_alive(req.keep_alive());

    if (std::get<0>(content) == cpp_web_server::Content_type::view)
      res.body() = std::get<1>(content);
    else
      res.set(boost::beast::http::field::location, std::get<1>(content));

    res.prepare_payload();
    co_return co_await send(std::move(res));
  }

} // ns

#endif // VIEW_RESPONSE_HPP