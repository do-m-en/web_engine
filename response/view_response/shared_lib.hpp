#ifndef SHARED_LIB_HPP_INCLUDED
#define SHARED_LIB_HPP_INCLUDED

#include <tuple>
#include <filesystem>

namespace web_server
{
  template<class Body, class Allocator>
  auto get_shared_lib_content(
    Page_root_path const& paths,
    std::string_view shared_lib_name,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> const& req,
    std::vector<std::tuple<std::string, std::string>> const& items)
    -> std::optional<std::tuple<cpp_web_server::Content_type, std::string>>
  {
    try
    {
      std::filesystem::path shared_lib_path = paths.dynamic_root / shared_lib_name;

      auto view_plugin = boost::dll::import<cpp_web_server::View_plugin_api>(
        shared_lib_path.c_str(), // conversion to boost::filesystem
        "view_plugin",
        boost::dll::load_mode::append_decorations
        );

      // FIXME additional...
      std::vector<std::tuple<std::string, std::string>> items2{items};

      if (req.method() == boost::beast::http::verb::post) // TODO send extra params to content
      {
        /*boost::system::error_code ec;
        boost::beast::http::request_parser<boost::beast::http::string_body> p;
        std::cout << req.body() << std::endl;
        auto const& body = req.body();
        p.put(boost::asio::buffer(req.body()), ec);
        auto const& got = p.get()["password"];*/

        // TODO body_format handling...
        items2.emplace_back("data", req.body());
      }

      return view_plugin->content(paths.static_root, paths.dynamic_root, items2);
    }
    catch (boost::system::system_error e)
    {
      std::cerr << e.what() << std::endl;
    }
    catch (std::bad_alloc e)
    {
      std::cerr << "bad alloc: " << e.what() << std::endl;
    }
    catch (std::exception e)
    {
      std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
      std::cerr << "odd!!" << std::endl;
    }

    return {};
  }
}

#endif // SHARED_LIB_HPP_INCLUDED