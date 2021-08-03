#include <boost/config.hpp>
#include "../view_plugin.hpp"
#include "../template_parser.hpp"

#include <fstream>
#include <string>

#include <fmt/format.h>

#include <iostream>
#include <filesystem>

namespace view_plugin
{

class About_view_plugin final : public cpp_web_server::View_plugin_api
{
public:
  std::string_view name() const override {return "Test";}

  std::tuple<cpp_web_server::Content_type, std::string> content(std::filesystem::path const& static_root, std::filesystem::path const& dynamic_root,
    std::vector<std::tuple<std::string, std::string>> const& items) override
  {
    try
    {
      std::ifstream in{dynamic_root.c_str() + std::string{"/test.template"}};
      if (in)
      {
        std::string content_template;
        in.seekg(0, std::ios::end);
        content_template.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&content_template[0], content_template.size());

        std::string about_template;
        std::ifstream about{dynamic_root.c_str() + std::string{"/about.template"}};
        if (about)
        {
          about.seekg(0, std::ios::end);
          about_template.resize(about.tellg());
          about.seekg(0, std::ios::beg);
          about.read(&about_template[0], about_template.size());
        }

        auto raw_items = web_server::template_to_items(dynamic_root.c_str() + std::string{"/about.template"});

        std::vector<fmt::internal::named_arg<std::string, char>> format_items;
        format_items.reserve(items.size());

        using ctx = fmt::format_context;
        std::vector<fmt::basic_format_arg<ctx>> args;

        for(auto& item : items)
        {
          format_items.push_back(fmt::arg(std::string_view(std::get<0>(item)), std::get<1>(item)));
          args.emplace_back(fmt::internal::make_arg<ctx>(format_items.back()));
        }
        args.emplace_back(fmt::basic_format_arg<ctx>());

        return { cpp_web_server::Content_type::view, fmt::vformat(content_template, fmt::basic_format_args<ctx>(args.data(), static_cast<unsigned int>(args.size()))) };
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

extern "C" BOOST_SYMBOL_EXPORT About_view_plugin view_plugin;
About_view_plugin view_plugin;

} // ns
