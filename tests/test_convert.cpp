#include <gtest/gtest.h>
#include "pace.hpp"

using pace::detail::convert;
using pace::detail::ValueVariant;

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

TEST(Convert, EnumToEnum) {
    pace::Enum e;
    e.choices = {"Zero", "One", "Two"};
    e.index = 1;
    e.choice = "One";
    ValueVariant v{e};
    auto result = convert<pace::Enum>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->index, 1);
    EXPECT_EQ(result->choice, "One");
    EXPECT_EQ(result->choices.size(), 3u);
}

TEST(Convert, EnumToInt) {
    pace::Enum e;
    e.choices = {"Zero", "One", "Two"};
    e.index = 2;
    e.choice = "Two";
    ValueVariant v{e};
    auto result = convert<int>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 2);
}

TEST(Convert, EnumToDouble) {
    pace::Enum e;
    e.choices = {"Zero", "One", "Two"};
    e.index = 1;
    e.choice = "One";
    ValueVariant v{e};
    auto result = convert<double>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 1.0);
}

TEST(Convert, EnumToString) {
    pace::Enum e;
    e.choices = {"Zero", "One", "Two"};
    e.index = 0;
    e.choice = "Zero";
    ValueVariant v{e};
    auto result = convert<std::string>(v);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Zero");
}

TEST(Convert, EnumToVectorReturnsNullopt) {
    pace::Enum e;
    e.choices = {"Zero", "One"};
    e.index = 0;
    e.choice = "Zero";
    ValueVariant v{e};
    EXPECT_FALSE(convert<std::vector<double>>(v).has_value());
    EXPECT_FALSE(convert<std::vector<int>>(v).has_value());
}
