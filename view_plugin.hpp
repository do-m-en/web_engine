#ifndef VIEW_PLUGIN_HPP_INCLUDED
#define VIEW_PLUGIN_HPP_INCLUDED

#include <string_view>
#include <vector>
#include <tuple>
#include <string_view>
#include <filesystem>

namespace cpp_web_server
{

enum class Resuorce_type
{
  css,
  javascript
};

class Resource
{
public:
  Resource(Resuorce_type type, std::filesystem::path&& file_path, bool prefer_footer)
    : type_{type}
    , file_path_{std::forward<std::filesystem::path>(file_path)}
    , prefer_footer_{prefer_footer}
  {
  }

  Resuorce_type type() const {return type_;}
  std::filesystem::path const& file_path() const {return file_path_;}
  bool prefer_footer() const {return prefer_footer_;}

private:
  Resuorce_type type_;
  std::filesystem::path file_path_;
  bool prefer_footer_;
};

enum class Content_type
{
  view,
  redirect_uri
};

class View_plugin_api
{
public:
  virtual ~View_plugin_api() {}

  virtual std::string_view name() const = 0;
  virtual std::tuple<Content_type, std::string> content(
    std::filesystem::path const& static_root,
    std::filesystem::path const& dynamic_root,
    std::vector<std::tuple<std::string, std::string>> const& items) = 0;
  virtual std::vector<Resource> resources() const = 0;
};

} // ns

#endif // VIEW_PLUGIN_HPP_INCLUDED
