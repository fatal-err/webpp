#ifndef WEBPP_INTERFACE_FCGI
#define WEBPP_INTERFACE_FCGI

#include "../../server/server_concepts.hpp"
#include "../../std/internet.hpp"
#include "../../std/set.hpp"
#include "../../traits/enable_traits.hpp"
#include "../app_wrapper.hpp"
#include "../request.hpp"
#include "./fastcgi/fcgi_manager.hpp"
#include "./fcgi_request.hpp"

namespace webpp::fastcgi {


    template <ServerTraits ServerType, typename App, typename EList = empty_extension_pack>
    struct fcgi : public enable_traits<typename ServerType::traits_type> {
        using server_type      = ServerType;
        using traits_type      = typename server_type::traits_type;
        using endpoint_type    = stl::net::ip::tcp::endpoint;
        using application_type = stl::remove_cvref_t<App>;
        using extension_list   = stl::remove_cvref_t<EList>;
        using interface_type   = fcgi<server_type, application_type, extension_list>;
        using request_type     = simple_request<traits_type, fcgi_request, interface_type, extension_list>;
        using logger_type      = typename traits_type::logger_type;
        using logger_ref       = typename logger_type::logger_ref;
        using allocator_type   = typename app_wrapper_type::allocator_type;
        using etraits          = enable_traits<traits_type>;
        using app_wrapper_type = http_app_wrapper<traits_type, application_type>;

        static constexpr auto default_listen_address = "0.0.0.0";
        static constexpr auto default_listen_port    = 8080u;
        static constexpr auto logging_category       = "FastCGI";

        stl::set<traits_type, endpoint_type> endpoints;
        server_type                          server;
        app_wrapper_type                     app;


        template <typename AllocType = allocator_type>
        fcgi(logger_ref logger = logger_type{}, AllocType const& alloc = AllocType{})
          : etraits{logger, alloc},
            app{logger, alloc},
            endpoints{alloc},
            server{logger, alloc} {}

        static constexpr [[nodiscard]] bool is_ssl_available() const noexcept {
            return false; // it's not, it's FCGI, we just don't know
        }

        void operator()() noexcept {
            if (endpoints.empty()) {
                stl::net::error_code ec;
                endpoints.emplace(stl::net::ip::make_address(default_listen_address, ec),
                                  default_listen_port);
                if (!ec) {
                    etraits::logger.critical(logging_category,
                                             stl::format("We're not able to listen to {}:{}",
                                                         default_listen_address, default_listen_port),
                                             ec);
                    return;
                }
            }
            server();
        }
    };


} // namespace webpp::fastcgi

#endif
