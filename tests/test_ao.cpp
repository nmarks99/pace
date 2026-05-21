#include "test_helpers.hpp"

TEST_F(SoftIocFixture, ao_BindDouble) {
    ezec::CAChannel channel("ezec:test:ao.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = 0.0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 3.14);

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 6.28);
}

TEST_F(SoftIocFixture, ao_BindMultipleDoubles) {
    ezec::CAChannel channel("ezec:test:ao.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value1 = 0.0;
    double value2 = 0.0;
    channel.bind(value1);
    channel.bind(value2);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value1, 3.14);
    EXPECT_DOUBLE_EQ(value2, 3.14);

    channel.put(6.28);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value1, 6.28);
    EXPECT_DOUBLE_EQ(value2, 6.28);
}

TEST_F(SoftIocFixture, ao_BindString) {
    ezec::CAChannel channel("ezec:test:ao.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_STREQ(value.c_str(), "3.14");

    channel.put(6.28);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value.c_str(), "6.28");
}

TEST_F(SoftIocFixture, ao_BindInt) {
    ezec::CAChannel channel("ezec:test:ao.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value = 0;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_EQ(value, 3);

    channel.put(6.28);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 6);
}

TEST_F(SoftIocFixture, ao_BindMultipleTypes) {
    ezec::CAChannel channel("ezec:test:ao.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value_str;
    double value_double;
    int value_int;
    channel.bind(value_str);
    channel.bind(value_double);
    channel.bind(value_int);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value_str.c_str(), "3.14");
    EXPECT_DOUBLE_EQ(value_double, 3.14);
    EXPECT_EQ(value_int, 3);

    channel.put(6.28);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value_str.c_str(), "6.28");
    EXPECT_DOUBLE_EQ(value_double, 6.28);
    EXPECT_EQ(value_int, 6);
}
