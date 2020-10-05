// Created by moisrex on 9/27/20.

#ifndef WEBPP_TYPE_TRAITS_HPP
#define WEBPP_TYPE_TRAITS_HPP

#include "./std.hpp"
#include <type_traits>

namespace webpp::istl {

    template <typename T>
    struct lazy_type {
        template <bool Condition>
        struct evaluate {
            using type = T;
        };
    };

    namespace details {

        template <bool Condition, template<typename...> typename Extractor, typename...Args>
        struct templated_lazy_evaluate {
            using type = void;
        };

        template <template<typename...>typename Extractor, typename ...Args>
        struct templated_lazy_evaluate<true, Extractor, Args...> {
            using type = Extractor<Args...>;
        };
    }

    template <template <typename...> typename Extractor, typename ...Args>
    struct templated_lazy_type {
        // I know how it looks, but it's c++, what did you expect? :)
        template <bool Condition>
        using evaluate = details::templated_lazy_evaluate<Condition, Extractor, Args...>;
    };

    /*
    template <typename T>
    concept LazyType = requires {
      typename T::template evaluated<true>::type;
      typename T::template evaluated<false>::type;
    };
*/

    /**
     * Lazy Conditional is a class type that is designed to have the same usage as std::conditional but
     * the values are not going to be evaluated if the condition doesn't meet for that value.
     */
    template <bool Condition, typename T1, typename T2>
    struct lazy_conditional;

    template <typename T1, typename T2>
    struct lazy_conditional<true, T1, T2> {
        using type = typename T1::template evaluate<true>::type;
    };

    template <typename T1, typename T2>
    struct lazy_conditional<false, T1, T2> {
        using type = typename T2::template evaluate<true>::type;
    };

    template <bool Condition, typename T1, typename T2>
    using lazy_conditional_t = typename lazy_conditional<Condition, T1, T2>::type;

}

#endif // WEBPP_TYPE_TRAITS_HPP
