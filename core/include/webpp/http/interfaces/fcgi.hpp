#ifndef WEBPP_INTERFACE_FCGI
#define WEBPP_INTERFACE_FCGI

#include "../../server/server_concepts.hpp"
#include "../../std/internet.hpp"
#include "../../std/map.hpp"
#include "../../std/set.hpp"
#include "../../std/vector.hpp"
#include "../request.hpp"
#include "./fastcgi/fcgi_manager.hpp"

namespace webpp::fastcgi {

    template <Traits TraitsType, typename /* fixme: RequestExtensionList */ REL, Interface IfaceType>
    struct fcgi_request : public REL {
        using traits_type            = TraitsType;
        using interface_type         = IfaceType;
        using request_extension_list = REL;
        using allocator_type   = typename traits_type::template allocator<typename traits_type::char_type>;
        using application_type = typename interface_type::application_type;
        using logger_type      = typename traits_type::logger;
        using logger_ref       = typename traits_type::logger::logger_ref;
        using string_type      = typename traits_type::string_type;

        [[no_unique_address]] logger_ref                 logger;
        istl::map<traits_type, string_type, string_type> data;


        fcgi_request(logger_ref logger = logger_type{}, auto const& alloc = allocator_type{}) noexcept
          : logger{logger},
            data{alloc} {}

        auto const& get_allocator() const noexcept {
            return data.get_allocator();
        }
    };


    template <ServerTraits ServerType, typename App, typename EList = empty_extension_pack>
    struct fcgi {
        using server_type      = ServerType;
        using traits_type      = typename server_type::traits_type;
        using endpoint_type    = stl::net::ip::tcp::endpoint;
        using application_type = stl::remove_cvref_t<App>;
        using extension_list   = stl::remove_cvref_t<EList>;
        using interface_type   = fcgi<server_type, application_type, extension_list>;
        using request_type     = simple_request<traits_type, fcgi_request, interface_type, extension_list>;
        using logger_type      = typename traits_type::logger_type;
        using logger_ref       = typename logger_type::logger_ref;

        static constexpr auto default_listen_address = "0.0.0.0";
        static constexpr auto default_listen_port    = 8080u;
        static constexpr auto logging_category       = "FastCGI";

        stl::set<traits_type, endpoint_type> endpoints;
        application_type                     app;
        server_type                          server;
        [[no_unique_address]] logger_ref     logger;

        template <typename AllocType>
        requires(ConstructibleWithLoggerAndAllocator<application_type, logger_ref, AllocType>)
          fcgi(logger_ref logger = logger_type{}, AllocType const& alloc = AllocType{})
          : logger{logger},
            endpoints{alloc},
            server{logger, alloc},
            app{logger, alloc} {};

        template <typename AllocType>
        requires(ConstructibleWithLogger<application_type, logger_ref>)
          fcgi(logger_ref logger = logger_type{}, AllocType const& alloc = AllocType{})
          : logger{logger},
            endpoints{alloc},
            server{logger, alloc},
            app{logger} {};

        template <typename AllocType>
        requires(ConstructibleWithAllocator<application_type, AllocType>)
          fcgi(logger_ref logger = logger_type{}, AllocType const& alloc = AllocType{})
          : logger{logger},
            endpoints{alloc},
            server{logger, alloc},
            app{alloc} {};

        template <typename AllocType>
        requires(stl::is_default_constructible<application_type>)
          fcgi(logger_ref logger = logger_type{}, AllocType const& alloc = AllocType{})
          : logger{logger},
            endpoints{alloc},
            server{logger, alloc},
            app{} {};

        void operator()() noexcept {
            if (endpoints.empty()) {
                stl::net::error_code ec;
                endpoints.emplace(stl::net::ip::make_address(default_listen_address, ec),
                                  default_listen_port);
                if (!ec) {
                    logger.critical(logging_category,
                                    stl::format("We're not able to listen to {}:{}", default_listen_address,
                                                default_listen_port),
                                    ec);
                    return;
                }
            }
            server();
        }
    };


} // namespace webpp::fastcgi

#endif
