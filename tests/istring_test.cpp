// Created by moisrex on 9/28/20.

#include "./common_pch.hpp"

// clang-format off
#include webpp_include(strings/istring)
// clang-format on


using namespace webpp;

TEST(IString, Creation) {
    std_istring test = "test";
    EXPECT_EQ(test, "test");

    std_istring_view test_view = "test view";
    EXPECT_EQ(test_view, "test view");

    istring hello = "hello";
    EXPECT_EQ(hello, "hello");
}

TEST(IString, TOLowerToUpper) {
    std_istring test = "test";
    EXPECT_EQ(test.ascii_to_upper_copy(), "TEST");
    test.ascii_to_upper();
    EXPECT_EQ(test, "TEST");
    EXPECT_TRUE(test.is_ascii_upper());
    EXPECT_EQ(test.ascii_to_lower_copy(), "test");
    test.ascii_to_lower();
    EXPECT_TRUE(test.is_ascii_lower());
}

TEST(IString, Trim) {
    std_istring test = "test";
    EXPECT_EQ("test", test.trim_copy());
    test = "  ";
    EXPECT_TRUE(test.trim_copy().empty());
    test = " t";
    EXPECT_EQ("t", test.trim_copy());
    test = "t    \t";
    EXPECT_EQ("t", test.rtrim_copy());
}

