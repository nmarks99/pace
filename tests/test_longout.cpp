#include "test_helpers.hpp"

TEST_F(SoftIocFixture, longout_BindInt) {
    ezec::CAChannel channel("ezec:test:longout");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value = 0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 42);

    dbr_long_t new_value = 99;
    ca_put(DBR_LONG, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 99);
}

TEST_F(SoftIocFixture, longout_BindDouble) {
    ezec::CAChannel channel("ezec:test:longout");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = 0.0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 42.0);

    dbr_long_t new_value = 99;
    ca_put(DBR_LONG, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 99.0);
}

TEST_F(SoftIocFixture, longout_BindString) {
    ezec::CAChannel channel("ezec:test:longout");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "42");

    dbr_long_t new_value = 99;
    ca_put(DBR_LONG, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "99");
}

TEST_F(SoftIocFixture, longout_BindMultipleTypes) {
    ezec::CAChannel channel("ezec:test:longout");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value_int = 0;
    double value_double = 0.0;
    std::string value_str;
    channel.bind(value_int);
    channel.bind(value_double);
    channel.bind(value_str);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value_int, 42);
    EXPECT_DOUBLE_EQ(value_double, 42.0);
    EXPECT_EQ(value_str, "42");

    dbr_long_t new_value = 99;
    ca_put(DBR_LONG, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value_int, 99);
    EXPECT_DOUBLE_EQ(value_double, 99.0);
    EXPECT_EQ(value_str, "99");
}
