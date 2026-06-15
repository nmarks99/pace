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
    EXPECT_EQ(value, "Zero");

    channel.put(2);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, "Two");
}

TEST_F(SoftIocFixture, mbbo_BindEnum) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    pace::Enum value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value.index, 0);
    EXPECT_EQ(value.choice, "Zero");
    ASSERT_EQ(value.choices.size(), 3u);
    EXPECT_EQ(value.choices[0], "Zero");
    EXPECT_EQ(value.choices[1], "One");
    EXPECT_EQ(value.choices[2], "Two");

    channel.put(2);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value.index, 2);
    EXPECT_EQ(value.choice, "Two");
}

TEST_F(SoftIocFixture, mbbo_GetEnum) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    auto result = channel.get<pace::Enum>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->index, 0);
    EXPECT_EQ(result->choice, "Zero");
    ASSERT_EQ(result->choices.size(), 3u);
    EXPECT_EQ(result->choices[0], "Zero");
    EXPECT_EQ(result->choices[1], "One");
    EXPECT_EQ(result->choices[2], "Two");
}

TEST_F(SoftIocFixture, mbbo_GetString) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    auto result = channel.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Zero");
}

TEST_F(SoftIocFixture, mbbo_GetInt) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    auto result = channel.get<int>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0);
}

TEST_F(SoftIocFixture, mbbo_PutInt) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    ASSERT_TRUE(channel.put(2));
    std::this_thread::sleep_for(100ms);

    auto result = channel.get<pace::Enum>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->index, 2);
    EXPECT_EQ(result->choice, "Two");
}

TEST_F(SoftIocFixture, mbbo_PutString) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    ASSERT_TRUE(channel.put("One"));
    std::this_thread::sleep_for(100ms);

    auto result = channel.get<pace::Enum>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->index, 1);
    EXPECT_EQ(result->choice, "One");
}

TEST_F(SoftIocFixture, mbbo_PutEnum) {
    pace::Context ctxt;
    pace::CAChannel channel("pace:test:mbbo.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    pace::Enum e;
    e.index = 2;
    ASSERT_TRUE(channel.put(e));
    std::this_thread::sleep_for(100ms);

    auto result = channel.get<pace::Enum>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->index, 2);
    EXPECT_EQ(result->choice, "Two");
}
