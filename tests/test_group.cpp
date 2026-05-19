#include "test_helpers.hpp"

/// Waits for group sync to return true or times out
bool wait_group_sync(ezec::ChannelGroup& group, int timeout_sec = 5) {
    bool new_data = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (!new_data && std::chrono::steady_clock::now() < deadline) {
        new_data = group.sync();
        if (!new_data) {
            std::this_thread::sleep_for(50ms);
        }
    }
    return new_data;
}

TEST_F(SoftIocFixture, group_BindAllRecords) {
    ezec::ChannelGroup group;
    group.add("ezec:test:ao.VAL");
    group.add("ezec:test:longout.VAL");
    group.add("ezec:test:stringout.VAL");
    group.add("ezec:test:mbbo.VAL");

    double ao_val = 0.0;
    int longout_val = 0;
    std::string stringout_val;
    int mbbo_val = -1;

    group.bind(ao_val, "ezec:test:ao.VAL");
    group.bind(longout_val, "ezec:test:longout.VAL");
    group.bind(stringout_val, "ezec:test:stringout.VAL");
    group.bind(mbbo_val, "ezec:test:mbbo.VAL");

    // Wait for all channels to deliver initial values
    // Multiple syncs may be needed since channels connect independently
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        group.sync();
        if (ao_val != 0.0 && longout_val != 0 && !stringout_val.empty() && mbbo_val != -1) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(ao_val, 3.14);
    EXPECT_EQ(longout_val, 42);
    EXPECT_EQ(stringout_val, "Hello World");
    EXPECT_EQ(mbbo_val, 0);
}

TEST_F(SoftIocFixture, group_BindMultipleTypesToSameChannel) {
    ezec::ChannelGroup group;
    group.add("ezec:test:ao.VAL");

    double ao_double = 0.0;
    int ao_int = 0;
    std::string ao_str;

    group.bind(ao_double, "ezec:test:ao.VAL");
    group.bind(ao_int, "ezec:test:ao.VAL");
    group.bind(ao_str, "ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        group.sync();
        if (ao_double != 0.0) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(ao_double, 3.14);
    EXPECT_EQ(ao_int, 3);
    EXPECT_EQ(ao_str, "3.14");
}

TEST_F(SoftIocFixture, group_BindBeforeAdd) {
    ezec::ChannelGroup group;

    double val = 0.0;
    EXPECT_THROW(group.bind(val, "ezec:test:ao.VAL"), std::runtime_error);
}

TEST_F(SoftIocFixture, group_DuplicateAddIsNoop) {
    ezec::ChannelGroup group;
    group.add("ezec:test:ao.VAL");
    group.add("ezec:test:ao.VAL");

    double val = 0.0;
    group.bind(val, "ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        group.sync();
        if (val != 0.0) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST_F(SoftIocFixture, group_GetChannel) {
    ezec::ChannelGroup group;
    group.add("ezec:test:ao.VAL");

    auto& channel = group.get_channel("ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(50ms);
    }
    EXPECT_TRUE(channel.connected());
}

TEST_F(SoftIocFixture, group_GetChannelNotRegistered) {
    ezec::ChannelGroup group;
    EXPECT_THROW(group.get_channel("nonexistent.VAL"), std::runtime_error);
}
