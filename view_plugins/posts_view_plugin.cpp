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

class Posts_view_plugin final : public cpp_web_server::View_plugin_api
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

        std::string content;
        for(auto& p: fs::directory_iterator(dynamic_root.c_str() + std::string{"/posts"}))
        {
          content += fmt::format(R"content(<article>
          <header>
            <h1>{title}</h1>
            <p>Posted on {date}</p>
          </header>
          <p>{content_part} <a href="{post_url}">Continue reading &rarr;</a></p>
          <footer>
            <p>(Posted in) Categories/Tags ...posted in tags/categories...</p>
          </footer>
        </article>)content",
          fmt::arg("date", p.path().stem().c_str()),
          fmt::arg("post_url", "tmp"),
          fmt::arg("title", "Blog Post Title"),
          fmt::arg("content_part", "Part of the blog post"));
        }

        return { cpp_web_server::Content_type::view, fmt::format(content_template, fmt::arg("content", content)) };
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

extern "C" BOOST_SYMBOL_EXPORT Posts_view_plugin view_plugin;
Posts_view_plugin view_plugin;

} // ns
