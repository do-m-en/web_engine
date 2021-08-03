// Copyright (c) 2020 Domen Vrankar (domen dot vrankar at gmail dot com)
//
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef CERT_DATA_PROVIDER_HPP
#define CERT_DATA_PROVIDER_HPP

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <streambuf>
#include <optional>

#include "tls_sni_context.hpp"

namespace tls_sni_sample
{
  std::string file_to_string(std::string_view file)
  {
    std::ifstream t{file.data()};
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());

    return str;
  }

  struct Basic_cert_data_provider
  {
    std::string resolve_name(std::string_view name) const
    {
      auto count = std::count(name.begin(), name.end(), '.');

      if(count > 2 || count < 1)
        return {};

      if(count == 2)
      {
        // remove sub domain prefix (www.)
        size_t found = name.find('.');

        return std::string{name.substr(found + 1)};
      }

      return std::string{name};
    }

    bool requires_reload() const
    {
      return false;
    }

    std::optional<tls_sni::Cert_data> load(std::string_view resolved_name)
    {
      std::string name{resolved_name};

      if(!std::filesystem::exists("/etc/letsencrypt/live/" + name + "/cert.pem") ||
        !std::filesystem::exists("/etc/letsencrypt/live/" + name + "/privkey.pem"))
      {
        return {};
      }

      return
        tls_sni::Cert_data{
          file_to_string("/etc/letsencrypt/live/" + name + "/cert.pem"),
          file_to_string("/etc/letsencrypt/live/" + name + "/privkey.pem")};
    }
  };
}


#endif // CERT_DATA_PROVIDER_HPP
