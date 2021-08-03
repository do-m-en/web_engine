// Copyright (c) 2020 Domen Vrankar (domen dot vrankar at gmail dot com)
//
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef TLS_SNI_CONTEXT_HPP_INCLUDED
#define TLS_SNI_CONTEXT_HPP_INCLUDED

#include <unordered_map>
#include <memory>
#include <string>
#include <string_view>
#include <shared_mutex>
#include <optional>

#include <boost/asio/ssl.hpp>

namespace tls_sni
{
  template<typename TLS_SNI_context_provider>
  class Context : public boost::asio::ssl::context
  {
  public:
    Context(TLS_SNI_context_provider& provider)
      : boost::asio::ssl::context{boost::asio::ssl::context::tlsv12}
      , provider_{provider}
    {
      set_options(
          boost::asio::ssl::context::default_workarounds |
          boost::asio::ssl::context::no_sslv2);

      SSL_CTX_set_tlsext_servername_arg(
        native_handle(),
        this);
      SSL_CTX_set_tlsext_servername_callback(
        native_handle(),
        +[](SSL* s, int* ad, void* arg)
        {
          const char* servername =
            SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);

          if(servername == nullptr)
            return SSL_TLSEXT_ERR_NOACK;

          auto& ctx = *reinterpret_cast<Context*>(arg);

          ctx.active_context_ = ctx.provider_(servername);

          if(!ctx.active_context_)
            return SSL_TLSEXT_ERR_NOACK;

          // switch context
          SSL_set_SSL_CTX(
            s,
            ctx.active_context_->native_handle());

          return SSL_TLSEXT_ERR_OK;
        });
    }

  private:
    TLS_SNI_context_provider& provider_;
    std::shared_ptr<boost::asio::ssl::context> active_context_;
  };

  struct Cert_data
  {
    std::string cert;
    std::string pubkey;
  };

  template<typename Cert_data_provider>
  class Basic_context_provider
  {
  public:
    Basic_context_provider(Cert_data_provider& provider)
    : provider_{provider}
    {/**/}

    std::shared_ptr<boost::asio::ssl::context> operator()(std::string_view name)
    {
      std::string resolved_name{provider_.resolve_name(name)};

      if(resolved_name.empty())
        return {};

      Contexts_container::const_iterator it;
      {
        std::shared_lock lock{mutex_};

        it = contexts_.find(resolved_name);

        if(it != contexts_.end() && !provider_.requires_reload())
        {
          return it->second;
        }
      }

      std::unique_lock lock{mutex_};

      if(!provider_.requires_reload())
      {
        it = contexts_.find(resolved_name);

        if(it != contexts_.end())
          return it->second;
      }

      auto data = provider_.load(resolved_name);

      auto ctx =
        contexts_
          .insert_or_assign(
            it,
            resolved_name,
            std::make_shared<boost::asio::ssl::context>(
              boost::asio::ssl::context::tlsv12))
          ->second;

      ctx->set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2);

      ctx->use_certificate_chain(
        boost::asio::buffer(data->cert.data(), data->cert.size()));

      ctx->use_private_key(
        boost::asio::buffer(data->pubkey.data(), data->pubkey.size()),
        boost::asio::ssl::context::file_format::pem);

      return ctx;
    }

  private:
    using Contexts_container =
      std::unordered_map<std::string, std::shared_ptr<boost::asio::ssl::context>>;

    Cert_data_provider& provider_;
    std::shared_mutex mutex_;
    Contexts_container contexts_;
  };
}

#endif // TLS_SNI_CONTEXT_HPP_INCLUDED