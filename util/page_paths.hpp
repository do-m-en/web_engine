#ifndef PAGE_PATHS_HPP_INCLUDED
#define PAGE_PATHS_HPP_INCLUDED

#include <filesystem>

namespace web_server
{
  struct Page_root_path
  {
    Page_root_path(std::filesystem::path const& request_to_host)
      : dynamic_root{request_to_host}
      , static_root{request_to_host}
    {
      dynamic_root += "/private";
      static_root += "/public";
    }

    std::filesystem::path dynamic_root;
    std::filesystem::path static_root;
  };

  struct View_paths
  {
    View_paths(
      std::filesystem::path const& view_path_,
      std::filesystem::path const& file_path_)
      : view_path{view_path_}
      , file_path{file_path_}
    {}

    std::filesystem::path view_path;
    std::filesystem::path file_path;
  };

} // ns

#endif // PAGE_PATHS_HPP_INCLUDED