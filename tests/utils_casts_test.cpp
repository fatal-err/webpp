// Created by moisrex on 1/27/20.
#include "../core/include/webpp/traits/std_traits.h"
#include "../core/include/webpp/utils/casts.h"

#include <gtest/gtest.h>
#include <limits>
#include <string>

using namespace webpp;
using namespace std;

TEST(Casts, ToInt) {
    EXPECT_EQ(to_uint<std_traits>("10"), 10);
    EXPECT_EQ(to_int<std_traits>("+10"), 10);
    EXPECT_EQ(to_int<std_traits>("-10"), -10);
    EXPECT_EQ(to_uint32<std_traits>("-10"),
              std::numeric_limits<uint32_t>::max() - 10 + 1);
    EXPECT_EQ((to<std_traits, int>("-10")), -10);
    EXPECT_EQ((to<std_traits, unsigned long long>("+1025153153")), 1025153153);

    auto invalids = {"one",     "+two",    "12a",     "a11",    "+a11",
                     "111-751", "123,321", "123.123", "123+123"};

    for (auto const& c : invalids) {
        // EXPECT_EQ(to<int>(c), 0);
        // EXPECT_EQ(to_uint64(c), 0);
        try {
            to<std_traits, int, true, true>(c);
            EXPECT_TRUE(false) << c;
        } catch (std::exception const& err) { EXPECT_TRUE(true); }
    }
}

TEST(Casts, ToStr) {
    EXPECT_EQ("12", to_str<std_traits>(12));
    EXPECT_EQ("-12", to_str<std_traits>(-12));
    EXPECT_EQ(to_str<std_traits>(1222).size(), 4);
}
