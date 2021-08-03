#include <boost/config.hpp>
#include "../view_plugin.hpp"

#include <fstream>
#include <string>

#include <fmt/format.h>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace view_plugin
{

class Editor_project_save_view_plugin final : public cpp_web_server::View_plugin_api
{
public:
  std::string_view name() const override {return "Test";}

  std::tuple<cpp_web_server::Content_type, std::string> content(std::filesystem::path const& static_root, std::filesystem::path const& dynamic_root,
    std::vector<std::tuple<std::string, std::string>> const& items) override
  {
    try
    {
      std::ofstream out{
        dynamic_root.c_str() + std::string{"/editor_project/test.project"},
        std::ios::trunc};

      if(out)
      {
        out << std::get<1>(items.back());

        return { cpp_web_server::Content_type::view, "{result: \"OK\"}" };
      }
    }
    catch(...)
    {
      //
    }

    return { cpp_web_server::Content_type::redirect_uri, "/error" };
  }

  std::vector<cpp_web_server::Resource> resources() const override // TODO resources should be returned along side content
  {
    return {};
  }
};

extern "C" BOOST_SYMBOL_EXPORT Editor_project_save_view_plugin view_plugin;
Editor_project_save_view_plugin view_plugin;

} // ns
