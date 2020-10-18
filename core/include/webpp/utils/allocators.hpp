// Created by moisrex on 10/14/20.

#ifndef WEBPP_ALLOCATORS_HPP
#define WEBPP_ALLOCATORS_HPP

#include "../std/type_traits.hpp"
#include <memory>

namespace webpp {

    template <typename AllocType, typename NewValueType>
    using rebind_allocator = typename stl::allocator_traits<AllocType>::template rebind_alloc<NewValueType>;


    namespace details {
        template <typename T>
        struct temp_alloc_holder {
            T alloc;

            [[nodiscard]] inline auto operator|(auto&&new_alloc) noexcept {
                // just ignore the new alloc, our order is from right to left
                return *this;
            }
        };

        template <typename AllocType = void>
        struct alloc_finder_type {
            auto operator()(auto &&arg) const noexcept {
                using U = stl::remove_cvref_t<decltype(arg)>;
                if constexpr (!stl::is_void_v<AllocType>) {
                    if constexpr (stl::uses_allocator_v<U, AllocType> &&
                                  requires(U n_alloc) { n_alloc.get_allocator(); }) {
                        return temp_alloc_holder{arg.get_allocator()};
                    } else {
                        return false;
                    }
                } else {
                    if constexpr (requires(U n_alloc) {
                                      typename U::allocator_type;
                                      n_alloc.get_allocator();
                                  }) {
                        return temp_alloc_holder{arg.get_allocator()};
                    } else {
                        return false;
                    }
                }
            }

            auto operator|(auto&& finder_res) const noexcept { // NOLINT(misc-unconventional-assign-operator)
                using U = stl::remove_cvref_t<decltype(finder_res)>;
                if constexpr (stl::is_integral_v<U>) {
                    return *this;
                } else {
                    return temp_alloc_holder{finder_res};
                }
            }
        };

    }

    /**
     * Extract an allocator from all the inputs. This is not safe if one of the inputs uses
     *   - allocator_type and
     *   - get_allocator()
     * in a different context as the standard library does.
     */
    template <typename ...T>
    [[nodiscard]] inline auto extract_allocator(T&&...args) noexcept {
        details::alloc_finder_type<> finder;
        const auto res = (finder | ... | finder(stl::forward<T>(args)));
        static_assert(requires {
          res.alloc;
        }, "We didn't find any allocator in the inputs.");
        return res.alloc;
    }


    /**
     * This version of allocator extractor will help you extract an allocator of an specific type.
     */
    template <typename AllocType, typename ...T>
    [[nodiscard]] inline auto extract_allocator_of(T&&...args) noexcept {
        details::alloc_finder_type<AllocType> finder;
        const auto res = (finder | ... | finder(stl::forward<T>(args)));
        static_assert(stl::is_same_v<stl::remove_cvref_t<decltype(res)>, details::temp_alloc_holder<AllocType>>,
                      "We didn't find any allocator in the inputs.");
        return res;
    }



    template <typename AllocType>
    struct allocator_holder {
        using allocator_type = stl::remove_cvref_t<AllocType>;
        using allocator_value_type = typename allocator_type::value_type;

        template <typename ...T>
        constexpr allocator_holder(T &&...alloc_holders) noexcept :
            alloc{extract_allocator_of<AllocType, T...>(stl::forward<T>(alloc_holders)...)}
        {}

        constexpr allocator_holder(allocator_type const& new_alloc = allocator_type{}) noexcept : alloc(new_alloc) {}

        template <typename T = allocator_value_type>
        [[nodiscard]] auto get_allocator_as() const noexcept {
            if constexpr (stl::is_same_v<T, allocator_value_type>) {
                return alloc;
            } else {
                return rebind_allocator<allocator_type, T>(alloc);
            }
        }

        [[nodiscard]] auto const& get_allocator() const noexcept {
            return alloc;
        }

      private:
        allocator_type alloc;
    };

    struct nothing_type{};

    template <typename AllocType, bool Mutable>
    using conditional_allocator_holder = stl::conditional<Mutable, allocator_holder<AllocType>, nothing_type>;

}

#endif // WEBPP_ALLOCATORS_HPP