#include "test_helpers.hpp"

TEST_F(SoftIocFixture, mbbo_BindInt) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value = -1;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 0);

    dbr_enum_t new_value = 2;
    ca_put(DBR_ENUM, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 2);
}

TEST_F(SoftIocFixture, mbbo_BindDouble) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = -1.0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 0.0);

    channel.put(1);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 1.0);
}

TEST_F(SoftIocFixture, mbbo_BindString) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "0");

    channel.put(2);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "2");
}
