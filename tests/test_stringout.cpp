#include "test_helpers.hpp"

TEST_F(SoftIocFixture, stringout_BindString) {
    ezec::CAChannel channel("ezec:test:stringout.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "Hello World");

    dbr_string_t new_value;
    strncpy(new_value, "Goodbye", sizeof(new_value) - 1);
    new_value[sizeof(new_value) - 1] = '\0';
    ca_put(DBR_STRING, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "Goodbye");
}

TEST_F(SoftIocFixture, stringout_BindMultipleStrings) {
    ezec::CAChannel channel("ezec:test:stringout.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value1;
    std::string value2;
    channel.bind(value1);
    channel.bind(value2);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value1, "Hello World");
    EXPECT_EQ(value2, "Hello World");

    dbr_string_t new_value;
    strncpy(new_value, "Goodbye", sizeof(new_value) - 1);
    new_value[sizeof(new_value) - 1] = '\0';
    ca_put(DBR_STRING, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value1, "Goodbye");
    EXPECT_EQ(value2, "Goodbye");
}

TEST_F(SoftIocFixture, stringout_BindInt) {
    ezec::CAChannel channel("ezec:test:stringout.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value = 0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    // string -> int conversion is not supported, value should remain 0
    EXPECT_EQ(value, 0);
}

TEST_F(SoftIocFixture, stringout_BindDouble) {
    ezec::CAChannel channel("ezec:test:stringout.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = 0.0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    // string -> double conversion is not supported, value should remain 0.0
    EXPECT_DOUBLE_EQ(value, 0.0);
}
