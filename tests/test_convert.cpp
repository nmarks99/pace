#include <gtest/gtest.h>
#include "ezec.hpp"

using ezec::detail::convert;
using ezec::detail::ValueVariant;

TEST(Convert, DoubleToDouble) {
    ValueVariant v{3.14};
    auto result = convert<double>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 3.14);
}

TEST(Convert, IntToInt) {
    ValueVariant v{42};
    auto result = convert<int>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(Convert, StringToString) {
    ValueVariant v{std::string("hello")};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST(Convert, DoubleToInt) {
    ValueVariant v{3.7};
    auto result = convert<int>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3);
}

TEST(Convert, IntToDouble) {
    ValueVariant v{42};
    auto result = convert<double>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 42.0);
}

TEST(Convert, DoubleToString) {
    ValueVariant v{3.14};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "3.1400");
}

TEST(Convert, IntToString) {
    ValueVariant v{42};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "42");
}

TEST(Convert, MonostateReturnsNullopt) {
    ValueVariant v{std::monostate{}};
    EXPECT_FALSE(convert<double>(v).has_value());
    EXPECT_FALSE(convert<int>(v).has_value());
    EXPECT_FALSE(convert<std::string>(v).has_value());
}

TEST(Convert, StringToDoubleReturnsNullopt) {
    ValueVariant v{std::string("hello")};
    EXPECT_FALSE(convert<double>(v).has_value());
}

TEST(Convert, StringToIntReturnsNullopt) {
    ValueVariant v{std::string("hello")};
    EXPECT_FALSE(convert<int>(v).has_value());
}
