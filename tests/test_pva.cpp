#include "test_helpers.hpp"

TEST_F(SoftIocPVAFixture, pva_BindDouble) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao");

    double val = 0.0;
    ctx.bind(val, "pace:test:ao");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST_F(SoftIocPVAFixture, pva_BindInt) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:longout");

    int val = 0;
    ctx.bind(val, "pace:test:longout");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_EQ(val, 42);
}

TEST_F(SoftIocPVAFixture, pva_BindString) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:stringout");

    std::string val;
    ctx.bind(val, "pace:test:stringout");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_EQ(val, "Hello World");
}

TEST_F(SoftIocPVAFixture, pva_BindMultipleTypes) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao");

    double ao_double = 0.0;
    int ao_int = 0;
    std::string ao_str;

    ctx.bind(ao_double, "pace:test:ao");
    ctx.bind(ao_int, "pace:test:ao");
    ctx.bind(ao_str, "pace:test:ao");

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
    EXPECT_EQ(ao_str, "3.1400");
}

TEST_F(SoftIocPVAFixture, pva_BindAllRecords) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao");
    ctx.connect("pace:test:longout");
    ctx.connect("pace:test:stringout");

    double ao_val = 0.0;
    int longout_val = 0;
    std::string stringout_val;

    ctx.bind(ao_val, "pace:test:ao");
    ctx.bind(longout_val, "pace:test:longout");
    ctx.bind(stringout_val, "pace:test:stringout");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        ctx.sync();
        if (ao_val != 0.0 && longout_val != 0 && !stringout_val.empty()) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(ao_val, 3.14);
    EXPECT_EQ(longout_val, 42);
    EXPECT_EQ(stringout_val, "Hello World");
}

TEST_F(SoftIocPVAFixture, pva_MonitorCustomStructure) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:Position");

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    ctx.bind(x, "pace:test:Position", "X.value");
    ctx.bind(y, "pace:test:Position", "Y.value");
    ctx.bind(z, "pace:test:Position", "Z.value");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        ctx.sync();
        if (x != 0.0 && y != 0.0 && z != 0.0) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }

    EXPECT_DOUBLE_EQ(x, 1.11);
    EXPECT_DOUBLE_EQ(y, 2.22);
    EXPECT_DOUBLE_EQ(z, 3.33);
}
