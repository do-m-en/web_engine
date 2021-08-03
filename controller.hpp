#ifndef cpp_web_server_CONTROLLER_HPP_INCLUDED
#define cpp_web_server_CONTROLLER_HPP_INCLUDED

#include "view_plugin.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cpp_web_server
{
  using map_view = std::unordered_map<std::string, View_plugin_api const&>&; // TODO make it a real view

  class Controller
  {
  public:
    Controller(View_plugin_api const& default_view, map_view views)
      : default_view_{default_view}
      , views_{views}
    {
    }

    View_plugin_api const& get(std::string_view url) const
    {
      if(auto const& view = views_.find(std::string{url}); view != views_.end())
        return view->second;

      return default_view_;
    }

  private:
    View_plugin_api const& default_view_;
    map_view views_;
  };
}

#endif // cpp_web_server_CONTROLLER_HPP_INCLUDED