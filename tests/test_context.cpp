#include "test_helpers.hpp"

bool wait_context_sync(ezec::Context& ctx, int timeout_sec = 5) {
    bool new_data = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (!new_data && std::chrono::steady_clock::now() < deadline) {
        new_data = ctx.sync();
        if (!new_data) {
            std::this_thread::sleep_for(50ms);
        }
    }
    return new_data;
}

TEST_F(SoftIocFixture, context_BindAllRecords) {
    ezec::Context ctx;
    ctx.add("ezec:test:", {"ao.VAL", "longout.VAL", "stringout.VAL", "mbbo.VAL"});

    double ao_val = 0.0;
    int longout_val = 0;
    std::string stringout_val;
    int mbbo_val = -1;

    ctx.bind(ao_val, "ezec:test:ao.VAL");
    ctx.bind(longout_val, "ezec:test:longout.VAL");
    ctx.bind(stringout_val, "ezec:test:stringout.VAL");
    ctx.bind(mbbo_val, "ezec:test:mbbo.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        ctx.sync();
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

TEST_F(SoftIocFixture, context_BindMultipleTypesToSameChannel) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    double ao_double = 0.0;
    int ao_int = 0;
    std::string ao_str;

    ctx.bind(ao_double, "ezec:test:ao.VAL");
    ctx.bind(ao_int, "ezec:test:ao.VAL");
    ctx.bind(ao_str, "ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        ctx.sync();
        if (ao_double != 0.0) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(ao_double, 3.14);
    EXPECT_EQ(ao_int, 3);
    EXPECT_EQ(ao_str, "3.14");
}

TEST_F(SoftIocFixture, context_BindBeforeAdd) {
    ezec::Context ctx;

    double val = 0.0;
    EXPECT_THROW(ctx.bind(val, "ezec:test:ao.VAL"), std::runtime_error);
}

TEST_F(SoftIocFixture, context_GetChannel) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    auto& channel = ctx.get_channel("ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(50ms);
    }
    EXPECT_TRUE(channel.connected());
}

TEST_F(SoftIocFixture, context_GetChannelNotRegistered) {
    ezec::Context ctx;
    EXPECT_THROW(ctx.get_channel("nonexistent.VAL"), std::runtime_error);
}

TEST_F(SoftIocFixture, context_NoMonitorUntilBind) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    auto& channel = ctx.get_channel("ezec:test:ao.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(50ms);
    }
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::this_thread::sleep_for(500ms);
    EXPECT_FALSE(ctx.sync()) << "sync() returned true before bind() was called";

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after bind()";
    EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST_F(SoftIocFixture, context_PutDouble) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update";
    ASSERT_DOUBLE_EQ(val, 3.14);

    ctx["ezec:test:ao.VAL"].put(99.5);

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after put";
    EXPECT_DOUBLE_EQ(val, 99.5);
}

TEST_F(SoftIocFixture, context_PutInt) {
    ezec::Context ctx;
    ctx.add("ezec:test:longout.VAL");

    int val = 0;
    ctx.bind(val, "ezec:test:longout.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update";
    ASSERT_EQ(val, 42);

    ctx["ezec:test:longout.VAL"].put(99);

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after put";
    EXPECT_EQ(val, 99);
}

TEST_F(SoftIocFixture, context_PutString) {
    ezec::Context ctx;
    ctx.add("ezec:test:stringout.VAL");

    std::string val;
    ctx.bind(val, "ezec:test:stringout.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update";
    ASSERT_EQ(val, "Hello World");

    ctx["ezec:test:stringout.VAL"].put("Goodbye");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after put";
    EXPECT_EQ(val, "Goodbye");
}

TEST_F(SoftIocFixture, context_PutViaContextMethod) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update";
    ASSERT_DOUBLE_EQ(val, 3.14);

    ctx.put("ezec:test:ao.VAL", 99.5);

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after put";
    EXPECT_DOUBLE_EQ(val, 99.5);
}

TEST_F(SoftIocFixture, context_PutViaRawCA) {
    ezec::Context ctx;
    ctx.add("ezec:test:ao.VAL");

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update";
    ASSERT_DOUBLE_EQ(val, 3.14);

    auto* ca_chan = dynamic_cast<ezec::CAChannel*>(&ctx.get_channel("ezec:test:ao.VAL"));
    ASSERT_NE(ca_chan, nullptr) << "get_channel did not return a CAChannel";

    double new_val = 99.5;
    ca_put(DBR_DOUBLE, ca_chan->id(), &new_val);
    ca_flush_io();

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update after ca_put";
    EXPECT_DOUBLE_EQ(val, 99.5);
}

TEST_F(SoftIocFixture, context_AddWithProtocolPrefix) {
    ezec::Context ctx;
    ctx.add("ca://ezec:test:ao.VAL");

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao.VAL");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received";
    EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST_F(SoftIocFixture, context_AddWithPrefixAndProtocol) {
    ezec::Context ctx;
    ctx.add("ezec:test:", {"ca://ao.VAL", "ca://longout.VAL"});

    double ao_val = 0.0;
    int longout_val = 0;
    ctx.bind(ao_val, "ezec:test:ao.VAL");
    ctx.bind(longout_val, "ezec:test:longout.VAL");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        ctx.sync();
        if (ao_val != 0.0 && longout_val != 0) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(ao_val, 3.14);
    EXPECT_EQ(longout_val, 42);
}
