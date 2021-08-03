#ifndef TEMPLATE_PARSER_HPP_INCLUDED
#define TEMPLATE_PARSER_HPP_INCLUDED

// #define BOOST_SPIRIT_X3_DEBUG 1

#include <filesystem>
#include <fstream>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include <string>
#include <vector>
#include <tuple>
#include <iostream>

namespace web_server
{

std::vector<std::tuple<std::string, std::string>> template_to_items(std::filesystem::path const& path)
{
  using namespace boost::spirit::x3;

  std::ifstream in{ path.c_str() };
  in.unsetf( std::ios::skipws );

  boost::spirit::istream_iterator begin{ in };
  boost::spirit::istream_iterator end;

  auto const name = '{' >> +(char_ - '{') >> '{';
  auto const value = lexeme[*(char_ - "}}")] >> "}}";
  auto const item = name >> value;
  auto const comment = lexeme[ "//" >> *(char_ - eol) >> eol ];
  auto const grammar = +(*omit[comment] >> item);

  std::vector<std::tuple<std::string, std::string>> data;
  bool r = phrase_parse(begin, end, grammar, space, data); // TODO exception

  if (!r || begin != end)
  {
    std::cerr << "Template parsing failed: " << path.c_str() << std::endl;
    exit(1);
  }

  return data;
}

} // ns

#endif // TEMPLATE_PARSER_HPP_INCLUDED