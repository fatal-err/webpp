// Created by moisrex on 1/27/20.
#include "../core/include/webpp/utils/strings.hpp"

#include <gtest/gtest.h>

using namespace webpp;

TEST(Strings, StartsEndsWith) {
    EXPECT_TRUE(starts_with("_one", '_'));
    EXPECT_TRUE(starts_with("_one", "_"));

    EXPECT_FALSE(starts_with("_one", 'o'));
    EXPECT_FALSE(starts_with("_one", "o"));

    EXPECT_TRUE(ends_with("_one!", "!"));
    EXPECT_TRUE(ends_with("_one!", '!'));

    EXPECT_FALSE(ends_with("_one! ", "!"));
    EXPECT_FALSE(ends_with("_one! ", '!'));

    EXPECT_TRUE(starts_with("_one! ", "_one!"));
    EXPECT_FALSE(starts_with("_one! ", "__one!"));
}


TEST(Strings, ToLowerToUpper) {
    EXPECT_EQ(to_lower_copy('A'), 'a');
    EXPECT_EQ(to_lower_copy('-'), '-');
    EXPECT_EQ(to_lower_copy('1'), '1');
    EXPECT_EQ(to_upper_copy('-'), '-');
    EXPECT_EQ(to_upper_copy('A'), 'A');
    EXPECT_EQ(to_upper_copy('a'), 'A');
    EXPECT_EQ(to_upper_copy('1'), '1');
    EXPECT_EQ(to_upper_copy("ABC"), "ABC");
    EXPECT_EQ(to_upper_copy("ABC"), "ABC");

    std::string str = "this is [1] STRING";
    to_lower(str);
    EXPECT_EQ(str, "this is [1] string");
    to_upper(str);
    EXPECT_EQ(str, "THIS IS [1] STRING");
}

#ifdef WEBPP_EVE
TEST(Strings, EveToLowerToUpper) {
    std::string str = "this [is] 1 STRING";
    algo::eve_to_lower(str);
    EXPECT_EQ(str, "this [is] 1 string");
    algo::eve_to_upper(str);
    EXPECT_EQ(str, "THIS [IS] 1 STRING");

    // a long string
    constexpr auto times = 1023;
    constexpr auto long_string = "1 LONG [STRING] is a short string WHICH IS REPEATED MANY TIMES";
    std::string long_str = long_string;
    long_str.reserve(long_str.size() * times);
    for (auto i = 0; i < times; i++)
        long_str.append(long_string);
    std::string long_str2 = long_str;
    algo::simple_to_lower(long_str2);
    algo::eve_to_lower(long_str);
    EXPECT_EQ(long_str, long_str2) << "eve tolower doesn't work";

    algo::eve_to_upper(long_str);
    algo::simple_to_upper(long_str2);
    EXPECT_EQ(long_str, long_str2) << "eve toupper doesn't work";
}
#endif