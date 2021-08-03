#ifndef GET_HPP_INCLUDED
#define GET_HPP_INCLUDED

#include <string>
#include <tuple>

namespace web_server
{
  auto get_get_content(
    std::ifstream& view,
    std::vector<std::tuple<std::string, std::string>> const& items)
    -> std::tuple<cpp_web_server::Content_type, std::string>
  {
    std::string base_template;
    view.seekg(0, std::ios::end);
    base_template.resize(view.tellg());
    view.seekg(0, std::ios::beg);
    view.read(&base_template[0], base_template.size());

    std::vector<fmt::internal::named_arg<std::string, char>> format_items;
    format_items.reserve(items.size());

    using ctx = fmt::format_context;
    std::vector<fmt::basic_format_arg<ctx>> args;

    for (auto& item : items)
    {
      format_items.push_back(fmt::arg(std::string_view(std::get<0>(item)), std::get<1>(item)));
      args.emplace_back(fmt::internal::make_arg<ctx>(format_items.back()));
    }
    args.emplace_back(fmt::basic_format_arg<ctx>());

    return { cpp_web_server::Content_type::view, fmt::vformat(base_template, fmt::basic_format_args<ctx>(args.data(), static_cast<unsigned int>(args.size()))) };
  }
}

#endif // GET_HPP_INCLUDED