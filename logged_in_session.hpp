#ifndef web_engine_LOGGED_IN_SESSION_HPP_INCLUDED
#define web_engine_LOGGED_IN_SESSION_HPP_INCLUDED
/*
#include "cryptlib.h"
#include "secblock.h"
#include "scrypt.h"
#include "osrng.h"
#include "files.h"
#include "hex.h"
*/
#include <filesystem>
#include <fstream>

#include <iostream>

namespace web_engine
{
  std::string create_user(std::filesystem::path const& base_dir, std::string_view user_name, std::string_view password, std::string_view session_type)
  {
    auto user_file{ base_dir / ("users/" + std::string{ user_name.begin(), user_name.end() }) };

    if (std::filesystem::exists(user_file))
      return ""; // ERROR user already exists

    std::string salt_data{ "NaCl" };

    /*using namespace CryptoPP;

    SecByteBlock derived(64);
    const byte* salt = reinterpret_cast<const byte*>(salt_data.data());
    const byte* pass = reinterpret_cast<const byte*>(password.data());

    Scrypt scrypt;
    scrypt.DeriveKey(derived, derived.size(), pass, 8, salt, 4, 1024, 8, 16);

    std::ofstream out_user_file{ user_file.c_str() };

    StringSource(derived, derived.size(), true, new HexEncoder(new FileSink(out_user_file)));
    out_user_file << '\n' << salt_data << '\n' << session_type;
    out_user_file.close();

    auto user_session_file{ base_dir / ("sessions/" + std::string{ user_name.begin(), user_name.end() }) };
    std::string session_token{ "abc" };

    std::ofstream out_session_file{ user_session_file.c_str() };
    out_session_file << session_token; // TODO add expiration date

    return session_token;*/
return "abc";
  }

  std::tuple<std::string, std::string> get_session_token_and_type(std::filesystem::path const& base_dir, std::string_view user_name, std::string_view password)
  {
    return { "", "" };
  }
}

#endif // web_engine_LOGGED_IN_SESSION_HPP_INCLUDED
