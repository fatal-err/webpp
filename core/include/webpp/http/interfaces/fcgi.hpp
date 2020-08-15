#ifndef WEBPP_INTERFACE_FCGI
#define WEBPP_INTERFACE_FCGI

#include "../../std/internet.hpp"
#include "../../std/set.hpp"
#include "../../std/vector.hpp"
#include "../request.hpp"
#include "./common/server.hpp"


namespace webpp {

    template <Traits TraitsType>
    struct fcgi_request {
        using traits_type = TraitsType;


    };


    template <Traits TraitsType>
    class fcgi {
      public:
        using traits_type = TraitsType;
        using endpoint_type = stl::net::ip::tcp::endpoint;

        stl::set<traits_type, endpoint_type> _endpoints;

      private:
        common::server                    _server;

        auto get_endpoints() noexcept {
            stl::net::ip::tcp::resolver resolver(_server.io);
            stl::error_code             ec;
            //        std::net::ip::tcp::resolver::results_type _endpoints;
            //        if (_fcgi->endpoints().empty()) {
            //            _endpoints = resolver.resolve(
            //                default_fcgi_listen_addr,
            //                std::to_string(default_fcgi_listen_port));
            //        } else {
            //            _endpoints = std::net::ip::tcp::v4();
            //        }
            return stl::vector<traits_type, stl::net::ip::tcp::endpoint>{};
        }


      public:
        fcgi() noexcept;

        void operator()() noexcept;

        /**
         * This will only work before you run the operator()
         */
        void add_endpoint(stl::string_view const& addr, uint_fast8_t port) noexcept {
            _endpoints.emplace(stl::net::ip::make_address(addr), port);
        }

    };


} // namespace webpp

#endif
