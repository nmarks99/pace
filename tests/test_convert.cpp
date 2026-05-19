#include <gtest/gtest.h>
#include "ezec.hpp"

using ezec::detail::convert;
using ezec::detail::MonitorVariant;

TEST(Convert, DoubleToDouble) {
    MonitorVariant v{3.14};
    auto result = convert<double>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 3.14);
}

TEST(Convert, IntToInt) {
    MonitorVariant v{42};
    auto result = convert<int>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(Convert, StringToString) {
    MonitorVariant v{std::string("hello")};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST(Convert, DoubleToInt) {
    MonitorVariant v{3.7};
    auto result = convert<int>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3);
}

TEST(Convert, IntToDouble) {
    MonitorVariant v{42};
    auto result = convert<double>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 42.0);
}

TEST(Convert, DoubleToString) {
    MonitorVariant v{3.14};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::to_string(3.14));
}

TEST(Convert, IntToString) {
    MonitorVariant v{42};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "42");
}

TEST(Convert, MonostateReturnsNullopt) {
    MonitorVariant v{std::monostate{}};
    EXPECT_FALSE(convert<double>(v).has_value());
    EXPECT_FALSE(convert<int>(v).has_value());
    EXPECT_FALSE(convert<std::string>(v).has_value());
}

TEST(Convert, StringToDoubleReturnsNullopt) {
    MonitorVariant v{std::string("hello")};
    EXPECT_FALSE(convert<double>(v).has_value());
}

TEST(Convert, StringToIntReturnsNullopt) {
    MonitorVariant v{std::string("hello")};
    EXPECT_FALSE(convert<int>(v).has_value());
}
