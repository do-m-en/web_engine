#ifndef POST_HPP_INCLUDED
#define POST_HPP_INCLUDED

#include <string>
#include <tuple>

namespace web_server
{
  template<class Body, class Allocator>
  auto get_post_content(
    Page_root_path const& paths,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
    std::vector<std::tuple<std::string, std::string>> const& items)
    -> std::tuple<cpp_web_server::Content_type, std::string>
  {
    std::string redirect_success;
    std::string redirect_error;
    for (auto const& i : items)
    {
      if (std::get<0>(i) == "redirect_success")
      {
        redirect_success = std::get<1>(i);
      }
      else if (std::get<0>(i) == "redirect_error")
      {
        redirect_error = std::get<1>(i);
      }
    } // TODO check that redirects exist...

    std::string session{ web_engine::create_user(paths.dynamic_root, "abc", "def", "admin") }; // TODO

    if (session.empty())
      return { cpp_web_server::Content_type::redirect_uri, redirect_error };


    //session_cookie = "theme=light; HttpOnly; SameSite=Strict"; // TODO use session variable
    return { cpp_web_server::Content_type::redirect_uri, redirect_success };
  }
}

#endif // POST_HPP_INCLUDED