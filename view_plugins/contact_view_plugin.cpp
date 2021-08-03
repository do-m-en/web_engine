#include <boost/config.hpp>
#include "../view_plugin.hpp"

#include <fstream>
#include <string>

#include <fmt/format.h>

#include <iostream>
#include <filesystem>

#ifdef _MSC_VER
using str = std::wstring;
#else
using str = std::string;
#endif

namespace view_plugin
{

class Contact_view_plugin final : public cpp_web_server::View_plugin_api
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

        std::string contact_template;
        std::ifstream contact{dynamic_root.c_str() + std::string{"/contact.template"}};
        if (contact)
        {
          contact.seekg(0, std::ios::end);
          contact_template.resize(contact.tellg());
          contact.seekg(0, std::ios::beg);
          contact.read(&contact_template[0], contact_template.size());
        }

        return { cpp_web_server::Content_type::view, fmt::format(content_template, fmt::arg("content", contact_template)) };
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

extern "C" BOOST_SYMBOL_EXPORT Contact_view_plugin view_plugin;
Contact_view_plugin view_plugin;

} // ns
