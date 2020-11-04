// Created by moisrex on 11/4/20.

#ifndef WEBPP_PATH_HPP
#define WEBPP_PATH_HPP

#include "../std/string.hpp"
#include "../std/string_view.hpp"
#include "../std/vector.hpp"

namespace webpp::uri {

    /**
     * Including normal string and string view types
     */
    template <typename T>
    concept Slug = requires (T slug) {
        typename T::value_type;
        slug.empty();
        T{""};
        slug.size();
    };

    template <Slug SlugType = stl::string, typename AllocType = typename SlugType::allocator_type>
    struct basic_path : public stl::vector<SlugType, AllocType> {
        using container_type = stl::vector<SlugType, AllocType>;
        using allocator_type = AllocType;
        using char_type = typename SlugType::value_type;
        using string_type = stl::conditional_t<istl::String<SlugType>, SlugType,
        stl::basic_string<char_type, allocator_type>>;

        template <typename ...T>
        constexpr basic_path(T&&...args) :
          container_type{stl::forward<T>(args)...} {}

        [[nodiscard]] bool is_absolute() const noexcept {
            return !this->empty() && this->front().empty();
        }

        [[nodiscard]] bool is_relative() const noexcept {
            return !is_absolute();
        }

        void normalize() {
            remove_dot_segments(is_absolute());
        }


        /**
         * Remove Dot Segments from https://tools.ietf.org/html/rfc3986#section-5.2.4
         * Refer to uri_normalize_benchmark for more related algorithms of this
         */
        void remove_dot_segments(bool remove_leading) {
            if (this->empty())
                return;

            auto it = this->begin();

            // handle the first part
            while (it < this->end()) {
                if (*it == current_dir) {
                    this->erase(it);
                    continue;
                } else if (*it == parent_dir) {
                    if (it != this->begin()) {
                        const auto p = std::prev(it);
                        if (p->empty()) {
                            // remove just this one
                            this->erase(it);
                            continue;
                        } else if (*p != parent_dir) {
                            // remove the previous one and this one
                            this->erase(p, std::next(it));
                            --it;
                            continue;
                        }
                    } else if (remove_leading) {
                        this->erase(it);
                        continue;
                    }
                }
                it++;
            }
        }

        string_type to_string() const {
            string_type str{this->get_allocator()};
            if (this->size() == 0)
                return str;

            auto it = this->cbegin();
            str.append(*it++);
            for (; it != this->cend(); ++it) {
                str.append("/");
                str.append(*it);
            }
            return str;
        }
    };


}

#endif // WEBPP_PATH_HPP
