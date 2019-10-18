#ifndef WEBPP_IP_H
#define WEBPP_IP_H

#include "../validators/validators.h"
#include "casts.h"
#include <array>
#include <sstream>
#include <string>
#include <string_view>

namespace webpp {

    /**
     * @brief considers this ip as a subnet and converts it into a int
     * prefix
     */
    constexpr uint8_t to_prefix(uint32_t octets) noexcept {
        uint8_t prefix = 0u;
        for (; octets != 0u; prefix++)
            octets <<= 1u;
        return prefix;
    }

    constexpr uint8_t to_prefix(std::array<uint32_t, 4> octets) noexcept {}

    /**
     * Convert a prefix to a subnet
     * @param prefix
     * @return bool
     */
    constexpr std::array<uint8_t, 4> to_subnet_array(uint8_t prefix) noexcept {}

    /**
     * Convert a prefix to a subnet
     * @param prefix
     * @return bool
     */
    constexpr uint32_t to_subnet(uint8_t prefix) noexcept {}

    constexpr uint32_t to_ipv4_int(std::string_view) noexcept {}

    class ipv4 {
      private:
        mutable uint32_t data = 0u;   // all bits are used
        mutable uint8_t _prefix = 0u; // use 255 as the false value
        mutable bool _valid = false;
        // FIXME: there's a padding, you might wanna use it

        constexpr void parse(std::string_view const& _data) const noexcept {
            if (_data.size() > 15 || _data.size() < 7) {
                _valid = false;
                return;
            }
            std::size_t first_dot = 0u;
            std::size_t len = _data.size();
            while (_data[first_dot] != '.' && first_dot != len)
                first_dot++;

            auto octet_1 = _data.substr(0u, first_dot);
            if (first_dot == len || !is::digit(octet_1)) {
                _valid = false;
                return;
            }

            std::size_t second_dot = first_dot + 1;
            while (_data[second_dot] != '.' && second_dot != len)
                second_dot++;

            auto octet_2 =
                _data.substr(first_dot + 1u, second_dot - (first_dot + 1));
            if (second_dot == len || !is::digit(octet_2)) {
                _valid = false;
                return;
            }

            std::size_t third_dot = second_dot + 1;
            while (_data[third_dot] != '.' && third_dot != len)
                third_dot++;

            auto octet_3 =
                _data.substr(second_dot + 1u, third_dot - (second_dot + 1));
            if (third_dot == len || !is::digit(octet_3)) {
                _valid = false;
                return; // parsing failed.
            }

            std::size_t slash = third_dot + 1;
            while (_data[slash] != '/' && slash != len)
                slash++;

            auto octet_4 =
                _data.substr(third_dot + 1u, slash - (third_dot + 1));

            if (!is::digit(octet_4)) {
                _valid = false;
                return;
            }

            if (slash != len) {
                auto prefix_str = _data.substr(slash);
                if (!is::digit(prefix_str)) {
                    _valid = false;
                    return;
                }
                _prefix = to_uint8(prefix_str);
            }

            data = parse({to_uint8(octet_1), to_uint8(octet_2),
                          to_uint8(octet_3), to_uint8(octet_4)});
            _valid = true;
        }

        constexpr uint32_t parse(std::array<uint8_t, 4u> const& ip) const
            noexcept {
            return static_cast<uint32_t>(ip[0] << 24u) |
                   static_cast<uint32_t>(ip[1] << 16u) |
                   static_cast<uint32_t>(ip[2] << 8u) |
                   static_cast<uint32_t>(ip[3]);
        }

      public:
        constexpr ipv4(ipv4 const& ip) = default;

        constexpr ipv4(ipv4&& ip) = default;

        constexpr explicit ipv4(std::string_view const& ip) noexcept {
            parse(ip);
        }

        constexpr ipv4(std::string_view const& ip,
                       std::string_view const& subnet) noexcept {
            parse(ip);
            if (is::subnet(subnet)) {
                _prefix = to_prefix(subnet);
            }
        }

        constexpr ipv4(std::string_view const& ip,
                       std::array<uint8_t, 4> const& subnet) noexcept {
            parse(ip);
            _prefix = to_prefix(subnet);
        }

        constexpr ipv4(std::string_view const& ip, uint8_t __prefix) noexcept
            : _prefix(__prefix) {
            parse(ip);
        }

        constexpr ipv4(uint8_t octet1, uint8_t octet2, uint8_t octet3,
                       uint8_t octet4, uint8_t prefix = 255) noexcept
            : data(parse({octet1, octet2, octet3, octet4})), _valid(true),
              _prefix(prefix > 32 ? 255 : prefix) {}

        constexpr ipv4(uint8_t octet1, uint8_t octet2, uint8_t octet3,
                       uint8_t octet4, std::string_view const& subnet) noexcept
            : data(parse({octet1, octet2, octet3, octet4})),
              _valid(is::subnet(subnet)),
              _prefix(_valid ? to_prefix(subnet) : 255) {}

        constexpr ipv4(uint32_t const& ip, uint8_t prefix = 255) noexcept
            : data(ip), _valid(true), _prefix(prefix > 32 ? 255 : prefix) {}

        constexpr ipv4(uint32_t const& ip, std::string_view subnet) noexcept
            : data(ip), _valid(is::subnet(subnet)),
              _prefix(_valid ? to_prefix(subnet) : 255) {}

        constexpr explicit ipv4(std::array<uint8_t, 4> const& ip,
                                uint8_t prefix = 255) noexcept
            : data(parse(ip)), _valid(true),
              _prefix(prefix > 32 ? 255 : prefix) {}

        constexpr explicit ipv4(std::array<uint8_t, 4> const& ip,
                                std::string_view const& subnet) noexcept
            : data(parse(ip)), _valid(is::subnet(subnet)),
              _prefix(_valid ? to_prefix(subnet) : 255) {}

        constexpr explicit ipv4(std::array<uint8_t, 4> const& ip,
                                std::array<uint8_t, 4> const& subnet) noexcept
            : data(parse(ip)), _valid(is::subnet(subnet)),
              _prefix(_valid ? to_prefix(subnet) : 255) {}

        ipv4& operator=(ipv4 const& ip) = default;
        ipv4& operator=(ipv4&& ip) = default;

        ipv4& operator=(std::string_view const& ip) noexcept {
            parse(ip);
            _prefix = 255;
            return *this;
        }

        ipv4& operator=(uint32_t ip) noexcept {
            data = ip;
            _valid = true;
            _prefix = 255;
            return *this;
        }

        constexpr bool operator==(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data == parse(other);
        }

        constexpr bool operator!=(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data != parse(other);
        }

        constexpr bool operator<(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data < parse(other);
        }

        constexpr bool operator>(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data > parse(other);
        }

        constexpr bool operator<=(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data <= parse(other);
        }

        constexpr bool operator>=(std::array<uint8_t, 4> const& other) const
            noexcept {
            return data >= parse(other);
        }

        constexpr bool operator==(ipv4 const& other) const noexcept {
            return data == other.data && _prefix == other._prefix;
        }

        constexpr bool operator!=(ipv4 const& other) const noexcept {
            return !operator==(other);
        }

        constexpr bool operator<(ipv4 const& other) const noexcept {
            return data < other.data;
        }

        constexpr bool operator>(ipv4 const& other) const noexcept {
            return data > other.data;
        }

        constexpr bool operator>=(ipv4 const& other) const noexcept {
            return data >= other.data;
        }

        constexpr bool operator<=(ipv4 const& other) const noexcept {
            return data <= other.data;
        }

        constexpr bool operator!=(std::string_view ip) const noexcept {
            return operator!=(ipv4(ip));
        }

        constexpr bool operator==(std::string_view ip) const noexcept {
            return operator==(ipv4(ip));
        }

        constexpr bool operator<(std::string_view ip) const noexcept {
            return operator<(ipv4(ip));
        }

        constexpr bool operator>(std::string_view ip) const noexcept {
            return operator>(ipv4(ip));
        }

        constexpr bool operator<=(std::string_view ip) const noexcept {
            return operator<=(ipv4(ip));
        }

        constexpr bool operator>=(std::string_view ip) const noexcept {
            return operator>=(ipv4(ip));
        }

        constexpr bool operator==(uint32_t const& ip) const noexcept {
            return integer() == ip;
        }

        constexpr bool operator!=(uint32_t const& ip) const noexcept {
            return !operator==(ip);
        }

        constexpr bool operator<(uint32_t const& ip) const noexcept {
            return integer() < ip;
        }

        constexpr bool operator>(uint32_t const& ip) const noexcept {
            return integer() > ip;
        }

        constexpr bool operator<=(uint32_t const& ip) const noexcept {
            return integer() <= ip;
        }

        constexpr bool operator>=(uint32_t const& ip) const noexcept {
            return integer() >= ip;
        }

        friend std::ostream& operator<<(std::ostream& stream, ipv4 const& ip) {
            stream << ip.str();
            return stream;
        }

        friend std::istream& operator>>(std::istream& stream, ipv4& ip) {
            std::string str;
            stream >> str;
            ip = str;
            return stream;
        }

        /**
         * @brief get string representation of the ip
         * @return
         */
        std::string str() const noexcept {
            auto _octets = octets();
            std::ostringstream s;
            s << static_cast<unsigned int>(_octets[0]) << '.'
              << static_cast<unsigned int>(_octets[1]) << '.'
              << static_cast<unsigned int>(_octets[2]) << '.'
              << static_cast<unsigned int>(_octets[3]);
            return s.str();
        }

        /**
         * @brief get the integer representation of the ip address
         * @return
         */
        constexpr uint32_t integer() const noexcept { return data; }

        /**
         * @brief get the 4 octets of the ip address
         * @return
         */
        constexpr std::array<uint8_t, 4u> octets() const noexcept {
            uint32_t _data = integer();
            return std::array<uint8_t, 4u>(
                {static_cast<uint8_t>(_data >> 24u),
                 static_cast<uint8_t>(_data >> 16u & 0x0FFu),
                 static_cast<uint8_t>(_data >> 8u & 0x0FFu),
                 static_cast<uint8_t>(_data & 0x0FFu)});
        }

        /**
         * @brief check if the ip is in the specified range or not
         * @param start
         * @param finish
         * @return
         */
        constexpr bool in_range(ipv4 const& start, ipv4 const& finish) const
            noexcept {
            return *this >= start && *this <= finish;
        }

        /**
         * @brief checks if the ip in this class is in the specified subnet or
         * not regardless of the the prefix that is specified in the ctor
         * @param ip
         * @param prefix
         * @return
         */
        constexpr bool is_in_subnet(ipv4 const& ip, unsigned int _prefix) const
            noexcept {
            auto uint_val = integer();
            auto uint_ip = ip.integer();
            uint_val &= 0xFFFFFFFFu << (32u - _prefix);
            uint_ip &= 0xFFFFFFFFu << (32u - _prefix);
            return uint_val == uint_ip;
        }

        /**
         * @brief checks if the ip in this class is in the specified subnet or
         * not regardless of the prefix that is specified in the ctor
         * @param ip
         * @param subnet_mask
         * @return
         */
        constexpr bool is_in_subnet(ipv4 const& ip,
                                    ipv4 const& subnet_mask) const noexcept {
            return is_in_subnet(ip, subnet_mask.to_prefix());
        }

        /**
         * Get the prefix you specified in the constructor
         * @return
         */
        constexpr auto prefix() const noexcept { return _prefix; }

        /**
         * @brief checks if the ip is in private range or not regardless of the
         * prefix
         * @return
         */
        constexpr bool is_private() const noexcept {
            constexpr ipv4 class_C(std::array<uint8_t, 4u>{192, 168, 0, 0});
            constexpr ipv4 class_B_start(
                std::array<uint8_t, 4u>{172, 16, 0, 0});
            constexpr ipv4 class_B_finish(
                std::array<uint8_t, 4u>{172, 31, 255, 255});
            constexpr ipv4 class_A(std::array<uint8_t, 4u>{10, 0, 0, 0});
            return is_in_subnet(class_C, 16) ||
                   in_range(class_B_start, class_B_finish) ||
                   is_in_subnet(class_A, 8);
        }

        /**
         * @brief checks if the ip address is in public range or not
         * @return
         */
        constexpr bool is_public() const noexcept { return !is_private(); }

        /**
         * @brief check if all the octets are zero or not
         * @return true if all the octets are zero
         */
        constexpr bool is_all_zero() const noexcept {
            auto _data = integer();
            return _data == 0;
        }

        /**
         * Check if the ip you specified is valid or not (the ctor will not
         * throw an error if the specified string is not a valid ipv4 address)
         * @return bool
         */
        constexpr bool is_valid() const noexcept { return _valid; }

        /**
         * Check if the ip has a prefix
         * @return bool
         */
        constexpr bool has_prefix() const noexcept { return _prefix == 255; }

        /**
         * Set prefix for the ip
         * @param prefix
         * @return
         */
        ipv4& prefix(uint8_t __prefix) noexcept {
            _prefix = __prefix > 32 ? 255 : __prefix;
            return *this;
        }

        ipv4& subnet(std::array<uint8_t> const& _subnet) noexcept {}

        /**
         * TODO: implement this thing
         * @brief get the geographical location of the ip address based on
         * predefined rules
         * @return coordinates or string location
         */
        std::string geographic_location() const noexcept;
    };

    bool operator==(uint32_t const& one, ipv4 const& two) { return two == one; }

    bool operator!=(uint32_t const& one, ipv4 const& two) { return two == one; }

    bool operator<(uint32_t const& one, ipv4 const& two) {
        return one < two.integer();
    }

    bool operator>(uint32_t const& one, ipv4 const& two) {
        return one > two.integer();
    }

    bool operator<=(uint32_t const& one, ipv4 const& two) {
        return one <= two.integer();
    }

    bool operator>=(uint32_t const& one, ipv4 const& two) {
        return one >= two.integer();
    }

    bool operator==(std::string_view const& one, ipv4 const& two) {
        return two == one;
    }

    bool operator!=(std::string_view const& one, ipv4 const& two) {
        return two != one;
    }

    bool operator<(std::string_view const& one, ipv4 const& two) {
        return ipv4(one) < two;
    }

    bool operator>(std::string_view const& one, ipv4 const& two) {
        return ipv4(one) > two;
    }

    bool operator<=(std::string_view const& one, ipv4 const& two) {
        return ipv4(one) <= two;
    }

    bool operator>=(std::string_view const& one, ipv4 const& two) {
        return ipv4(one) >= two;
    }

    bool operator==(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return two == one;
    }

    bool operator!=(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return two != one;
    }

    bool operator<(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return ipv4(one) < two;
    }

    bool operator>(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return ipv4(one) > two;
    }

    bool operator<=(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return ipv4(one) <= two;
    }

    bool operator>=(std::array<uint8_t, 4> const& one, ipv4 const& two) {
        return ipv4(one) >= two;
    }

} // namespace webpp

#endif // WEBPP_IP_H
