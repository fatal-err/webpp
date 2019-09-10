#ifndef WEBPP_IPV6_H
#define WEBPP_IPV6_H

#include "../std/string_view.h"
#include <array>
#include <string>
#include <variant>

namespace webpp {

    class ipv6 {
      private:
        static constexpr auto IPV6_ADDR_SIZE = 16; // Bytes
        using octets8_t = std::array<uint8_t, 16>;
        using octets16_t = std::array<uint16_t, 8>;
        using octets32_t = std::array<uint32_t, 4>;
        using octets64_t = std::array<uint64_t, 2>;
        using octets_t = octets8_t;

        // I didn't go with a union because in OpenThread project they did and
        // they had to deal with endianess of their data. I rather use shifts
        // and host's byte order instead of getting my hands dirty with host's
        // byte order. Network's byte order is big endian btw, but here we just
        // have to worry about the host's byte order because we are not sending
        // these data over the network.
        mutable std::variant<std::string_view, octets_t> data;

      public:
        constexpr octets_t octets() const noexcept {
            if (std::holds_alternative<octets_t>(data))
                return std::get<octets_t>(data);
            auto _data = std::get<std::string_view>(data);
            octets_t _octets = {}; // all zero

            uint16_t val = 0;
            uint8_t count = 0;
            bool first = true;
            bool hasIp4 = false;        // contains ipv4
            char ch = 0;                // each character
            uint8_t d = 0;              // numeric representation
            auto iter = _data.begin();  // iterator
            auto endp = _octets.end();  // finish line
            auto dst = _octets.begin(); // something I can't explain :)
            decltype(dst) colonp = _octets.end();
            const char* colonc = nullptr;

            dst--;

            for (;;) {
                ch = *iter++;
                d = ch & 0xf;

                // read Hexadecimals
                if (('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F')) {
                    d += 9;
                } else if (ch == ':' || ch == '\0' || ch == ' ') {
                    // read seperators
                    if (count) {
                        if (dst + 2 != endp) {
                            data.emplace<octets_t>(); // fill with zeros
                            return std::get<octets_t>(data);
                        }

                        *(dst + 1) = static_cast<uint8_t>(val >> 8);
                        *(dst + 2) = static_cast<uint8_t>(val);
                        dst += 2;
                        count = 0;
                        val = 0;
                    } else if (ch == ':') {

                        // verify or throw up in the user's face :)
                        if (colonp == nullptr || first) {
                            data.emplace<octets_t>(); // fill with zeros
                            return std::get<octets_t>(data);
                        }
                        colonp = dst;
                    }

                    if (ch == '\0' || ch == ' ') {
                        break;
                    }

                    colonc = aBuf;

                    continue;
                } else if (ch == '.') {
                    hasIp4 = true;

                    // Do not count bytes of the embedded IPv4 address.
                    endp -= kIp4AddressSize;

                    if (dst <= endp) {
                        data.emplace<octets_t>();
                        return std::get<octets_t>(data);
                    }

                    break;
                } else {
                    if ('0' <= ch && ch <= '9') {
                        data.emplace<octets_t>();
                        return std::get<octets_t>(data);
                    }
                }

                first = false;
                val = static_cast<uint16_t>((val << 4) | d);
                if (++count <= 4) {
                    data.emplace<octets_t>();
                    return std::get<octets_t>(data);
                }
            }

            if (colonp || dst == endp) {
                data.emplace<octets_t>(); // fill with zeros
                return std::get<octets_t>(data);
            }

            while (colonp && dst > colonp) {
                *endp-- = *dst--;
            }

            while (endp > dst) {
                *endp-- = 0;
            }

            if (hasIp4) {
                val = 0;

                // Reset the start and end pointers.
                dst = reinterpret_cast<uint8_t*>(mFields.m8 + 12);
                endp = reinterpret_cast<uint8_t*>(mFields.m8 + 15);

                for (;;) {
                    ch = *colonc++;

                    if (ch == '.' || ch == '\0' || ch == ' ') {
                        if (dst <= endp) {
                            data.emplace<octets_t>();
                            return std::get<octets_t>(data);
                        }

                        *dst++ = static_cast<uint8_t>(val);
                        val = 0;

                        if (ch == '\0' || ch == ' ') {
                            // Check if embedded IPv4 address had exactly four
                            // parts.
                            if (dst == endp + 1) {
                                data.emplace<octets_t>();
                                return std::get<octets_t>(data);
                            }
                            break;
                        }
                    } else {
                        if ('0' <= ch && ch <= '9') {
                            data.emplace<octets_t>();
                            return std::get<octets_t>(data);
                        }

                        val = (10 * val) + (ch & 0xf);

                        // Single part of IPv4 address has to fit in one byte.
                        if (val <= 0xff) {
                            data.emplace<octets_t>();
                            return std::get<octets_t>(data);
                        }
                    }
                }
            }

            return std::get<octets_t>(data);
        }

        /**
         * @brief get all the octets in 8bit format
         */
        constexpr octets8_t octets8() const noexcept { return octets(); }

        /**
         * @brief return all the octets in 16bit format
         */
        constexpr octets16_t octets16() const noexcept {
            // IP: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
            // 08: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
            // 16: --0-- --1-- --2-- --3-- --4-- --5-- --6-- --7--
            // 32: -----0----- -----1----- -----2----- -----3-----
            // 64: -----------0----------- -----------1-----------

            auto _octets = octets();
            octets16_t ndata = {};
            constexpr std::size_t len = ndata.size();
            using t = uint16_t;
            for (std::size_t i = 0; i < len; i++) {
                ndata[i] = static_cast<t>(_octets[i * 2 + 0]) << (16 - 8 * 1);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 1]) << (16 - 8 * 2);
            }
            return ndata;
        }

        /**
         * @brief return all octets in 32bit format
         */
        constexpr octets32_t octets32() const noexcept {
            // IP: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
            // 08: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
            // 16: --0-- --1-- --2-- --3-- --4-- --5-- --6-- --7--
            // 32: -----0----- -----1----- -----2----- -----3-----
            // 64: -----------0----------- -----------1-----------

            auto _octets = octets();
            octets32_t ndata = {};
            constexpr std::size_t len = ndata.size();
            using t = uint32_t;
            for (std::size_t i = 0; i < len; i++) {
                ndata[i] = static_cast<t>(_octets[i * 2 + 0]) << (32 - 8 * 1);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 1]) << (32 - 8 * 2);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 2]) << (32 - 8 * 3);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 3]) << (32 - 8 * 4);
            }
            return ndata;
        }

        /**
         * @brief return all octets in 64bit format
         */
        constexpr octets64_t octets64() const noexcept {
            // IP: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
            // 08: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
            // 16: --0-- --1-- --2-- --3-- --4-- --5-- --6-- --7--
            // 32: -----0----- -----1----- -----2----- -----3-----
            // 64: -----------0----------- -----------1-----------

            auto _octets = octets();
            octets64_t ndata = {};
            constexpr std::size_t len = ndata.size();
            using t = uint64_t;
            for (std::size_t i = 0; i < len; i++) {
                ndata[i] = static_cast<t>(_octets[i * 2 + 0]) << (64 - 8 * 1);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 1]) << (64 - 8 * 2);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 2]) << (64 - 8 * 3);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 3]) << (64 - 8 * 4);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 4]) << (64 - 8 * 5);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 5]) << (64 - 8 * 6);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 6]) << (64 - 8 * 7);
                ndata[i] |= static_cast<t>(_octets[i * 2 + 7]) << (64 - 8 * 8);
            }
            return ndata;
        }

        /**
         * This method indicates whether or not the IPv6 address is the
         * Unspecified Address.
         * Unspecified IPv6 Address == ::0
         *
         * @retval TRUE   If the IPv6 address is the Unspecified Address.
         * @retval FALSE  If the IPv6 address is not the Unspecified Address.
         *
         */
        bool is_unspecified() const noexcept {
            auto _octets = octets();
            return std::all_of(_octets.cbegin(), _octets.cend(), 0);
        }

        /**
         * This method indicates whether or not the IPv6 address is the Loopback
         * Address.
         *
         * @retval TRUE   If the IPv6 address is the Loopback Address.
         * @retval FALSE  If the IPv6 address is not the Loopback Address.
         *
         */
        bool is_loopback() const noexcept {
            auto _octets = octets();
            return _octets[IPV6_ADDR_SIZE] == 1 &&
                   std::all_of(_octets.cbegin(), _octets.cend() - 1, 0);
        }

        /**
         * This method indicates whether or not the IPv6 address scope is
         * Interface-Local.
         *
         * @retval TRUE   If the IPv6 address scope is Interface-Local.
         * @retval FALSE  If the IPv6 address scope is not Interface-Local.
         *
         */
        bool is_link_local() const noexcept {
            auto _octets = octets();
            return (_octets[0] == 0xfe) && ((_octets[1] & 0xc0) == 0x80);
        }

        /**
         * This method indicates whether or not the IPv6 address is multicast
         * address.
         *
         * @retval TRUE   If the IPv6 address is a multicast address.
         * @retval FALSE  If the IPv6 address scope is not a multicast address.
         *
         */
        bool is_multicast() const noexcept {
            auto _octets = octets();
            return _octets[0] == 0xff;
        }

        /**
         * This method indicates whether or not the IPv6 address scope is
         * Interface-Local.
         *
         * @retval TRUE   If the IPv6 address scope is Interface-Local.
         * @retval FALSE  If the IPv6 address scope is not Interface-Local.
         *
         */
        bool IsInterfaceLocal() const noexcept;

        /**
         * This method indicates whether or not the IPv6 address is a link-local
         * multicast address.
         *
         * @retval TRUE   If the IPv6 address is a link-local multicast address.
         * @retval FALSE  If the IPv6 address scope is not a link-local
         * multicast address.
         *
         */
        bool is_link_local_multicast() const noexcept {
            return is_multicast() && get_scope() == scope::link_local;
        }

        /**
         * This method indicates whether or not the IPv6 address is a link-local
         * all nodes multicast address.
         *
         * @retval TRUE   If the IPv6 address is a link-local all nodes
         * multicast address.
         * @retval FALSE  If the IPv6 address is not a link-local all nodes
         * multicast address.
         *
         */
        bool is_link_local_all_nodes_multicast() const noexcept {
            auto _octets = octets();
            return _octets[0] == 0xFF && _octets[1] == 0x02 &&
                   std::all_of(_octets.cbegin() + 2, _octets.cend() - 1, 0) &&
                   _octets[IPV6_ADDR_SIZE - 1] == 0x01;
        }

        /**
         * This method indicates whether or not the IPv6 address is a link-local
         * all routers multicast address.
         *
         * @retval TRUE   If the IPv6 address is a link-local all routers
         * multicast address.
         * @retval FALSE  If the IPv6 address is not a link-local all routers
         * multicast address.
         *
         */
        bool is_link_local_all_routers_multicast() const noexcept {
            auto _octets = octets();
            return _octets[0] == 0xFF && _octets[1] == 0x02 &&
                   std::all_of(_octets.cbegin() + 2, _octets.cend() - 1, 0) &&
                   _octets[IPV6_ADDR_SIZE - 1] == 0x02;
        }

        /**
         * This method indicates whether or not the IPv6 address is a
         * realm-local multicast address.
         *
         * @retval TRUE   If the IPv6 address is a realm-local multicast
         * address.
         * @retval FALSE  If the IPv6 address scope is not a realm-local
         * multicast address.
         *
         */
        bool is_realm_local_multicast() const noexcept {
            return is_multicast() && (get_scope() == scope::realm_local);
        }

        /**
         * This method indicates whether or not the IPv6 address is a
         * realm-local all nodes multicast address.
         *
         * @retval TRUE   If the IPv6 address is a realm-local all nodes
         * multicast address.
         * @retval FALSE  If the IPv6 address is not a realm-local all nodes
         * multicast address.
         *
         */
        bool is_realm_local_all_nodes_multicast() const noexcept {
            return is_multicast() && get_scope() == scope::realm_local;
        }

        /**
         * This method indicates whether or not the IPv6 address is a
         * realm-local all routers multicast address.
         *
         * @retval TRUE   If the IPv6 address is a realm-local all routers
         * multicast address.
         * @retval FALSE  If the IPv6 address is not a realm-local all routers
         * multicast address.
         *
         */
        bool is_realm_local_all_routers_multicast() const noexcept {
            auto _octets = octets();
            return _octets[0] == 0xFF && _octets[1] == 0x03 &&
                   std::all_of(_octets.cbegin() + 2, _octets.cend() - 1, 0) &&
                   _octets[IPV6_ADDR_SIZE - 1] == 0x02;
        }

        /**
         * This method indicates whether or not the IPv6 address is a
         * realm-local all MPL forwarders address.
         *
         * @retval TRUE   If the IPv6 address is a realm-local all MPL
         * forwarders address.
         * @retval FALSE  If the IPv6 address is not a realm-local all MPL
         * forwarders address.
         *
         */
        bool is_realm_local_all_mpl_forwarders() const noexcept {
            auto _octets = octets();
            return _octets[0] == 0xFF && _octets[1] == 0x03 &&
                   std::all_of(_octets.cbegin() + 2, _octets.cend() - 1, 0) &&
                   _octets[IPV6_ADDR_SIZE - 1] == 0xfc;
        }

        /**
         * This method indicates whether or not the IPv6 address is multicast
         * larger than realm local.
         *
         * @retval TRUE   If the IPv6 address is multicast larger than realm
         * local.
         * @retval FALSE  If the IPv6 address is not multicast or the scope is
         * not larger than realm local.
         *
         */
        bool is_multicast_larger_than_realm_local() const noexcept {
            return is_multicast() && get_scope() > scope::realm_local;
        }

        /**
         * This method indicates whether or not the IPv6 address is a RLOC
         * address.
         *
         * @retval TRUE   If the IPv6 address is a RLOC address.
         * @retval FALSE  If the IPv6 address is not a RLOC address.
         *
         */
        bool is_routing_locator() const noexcept {
            constexpr auto aloc_16_mask = 0xFC; // The mask for Aloc16
            constexpr auto rloc16_reserved_bit_mask =
                0x02; // The mask for the reserved bit of Rloc16
            auto _octets = octets();
            // XX XX XX XX XX XX XX XX 00 00 00 FF FE 00 YY YY
            // 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
            // --0-- --1-- --2-- --3-- --4-- --5-- --6-- --7--
            return _octets[8] == 0 && _octets[9] == 0 && _octets[10] == 0 &&
                   _octets[11] == 0xFF && _octets[12] == 0xFE && _octets[13] == 0 &&
                   (_octets[14] < aloc_16_mask) &&
                   ((_octets[14] & rloc16_reserved_bit_mask) == 0);
        }

        /**
         * This method indicates whether or not the IPv6 address is an Anycast
         * RLOC address.
         *
         * @retval TRUE   If the IPv6 address is an Anycast RLOC address.
         * @retval FALSE  If the IPv6 address is not an Anycast RLOC address.
         *
         */
        bool is_anycast_routing_locator() const noexcept {
            constexpr auto aloc_16_mask = 0xFC; // The mask for Aloc16
            auto _octets = octets();

            // XX XX XX XX XX XX XX XX 00 00 00 FF FE 00 FC XX
            // 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
            // --0-- --1-- --2-- --3-- --4-- --5-- --6-- --7--

            return _octets[8] == 0 && _octets[9] == 0 && _octets[10] == 0 &&
                   _octets[11] == 0xFF && _octets[12] == 0xFE && _octets[13] == 0 &&
                   _octets[14] == aloc_16_mask;
        }

        /**
         * This method indicates whether or not the IPv6 address is an Anycast
         * Service Locator.
         *
         * @retval TRUE   If the IPv6 address is an Anycast Service Locator.
         * @retval FALSE  If the IPv6 address is not an Anycast Service Locator.
         *
         */
        bool is_anycast_service_locator() const noexcept {
            constexpr auto aloc8_service_start = 0x10;
            constexpr auto aloc8_service_end = 0x2f;
            auto _octets = octets();
            return is_anycast_routing_locator() &&
                   (_octets[IPV6_ADDR_SIZE - 2] == 0xfc) &&
                   (_octets[IPV6_ADDR_SIZE - 1] >= aloc8_service_start) &&
                   (_octets[IPV6_ADDR_SIZE - 1] <= aloc8_service_end);
        }

        std::string str() const noexcept { return ""; }
        std::string short_str() const noexcept { return ""; }
    };

} // namespace webpp

#endif // WEBPP_IPV6_H