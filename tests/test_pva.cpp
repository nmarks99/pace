#include "test_helpers.hpp"
#include <pvxs/nt.h>

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

TEST_F(SoftIocPVAFixture, pva_BindRawValue) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:Position");

    pvxs::Value root;
    pvxs::Value x;
    ctx.bind(root, "pace:test:Position");
    ctx.bind(x, "pace:test:Position", "X");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    ASSERT_TRUE(root);
    ASSERT_TRUE(x);
    EXPECT_DOUBLE_EQ(root["X.value"].as<double>(), 1.11);
    EXPECT_DOUBLE_EQ(root["Y.value"].as<double>(), 2.22);
    EXPECT_DOUBLE_EQ(root["Z.value"].as<double>(), 3.33);
    EXPECT_DOUBLE_EQ(x["value"].as<double>(), 1.11);
}

TEST_F(SoftIocPVAFixture, pva_MonitorReconnectsAfterIocRestart) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao");

    double value = 0.0;
    ctx.bind(value, "pace:test:ao");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No initial monitor update received";
    EXPECT_DOUBLE_EQ(value, 3.14);

    ASSERT_TRUE(ctx.put("pace:test:ao", 6.28));
    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received after put";
    EXPECT_DOUBLE_EQ(value, 6.28);

    stop_ioc();
    auto disconnect_deadline = std::chrono::steady_clock::now() + 5s;
    while (ctx.all_connected() && std::chrono::steady_clock::now() < disconnect_deadline) {
        std::this_thread::sleep_for(50ms);
    }
    ASSERT_FALSE(ctx.all_connected()) << "PVA channel did not disconnect";

    start_ioc();
    wait_all_connected(ctx, 10);
    ASSERT_TRUE(ctx.all_connected()) << "PVA channel did not reconnect";
    ASSERT_TRUE(wait_context_sync(ctx, 10)) << "No monitor update received after IOC restart";
    EXPECT_DOUBLE_EQ(value, 3.14);
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

TEST_F(SoftIocPVAFixture, pva_BindVectorDouble) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:wfdouble");

    std::vector<double> val;
    ctx.bind(val, "pace:test:wfdouble");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    std::vector<double> truth = {1.11, 2.22, 3.33};
    EXPECT_EQ(val, truth);
}

TEST_F(SoftIocPVAFixture, pva_BindVectorInt) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:wflong");

    std::vector<int> val;
    ctx.bind(val, "pace:test:wflong");

    ASSERT_TRUE(wait_context_sync(ctx)) << "No monitor update received within timeout";
    std::vector<int> truth = {10, 20, 30};
    EXPECT_EQ(val, truth);
}

static double wait_count(pace::Context& ctx, double expected, int timeout_sec = 5) {
    double count = -1.0;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (std::chrono::steady_clock::now() < deadline) {
        count = ctx.get<double>("pace:test:count").value_or(-1.0);
        if (count >= expected) {
            break;
        }
        std::this_thread::sleep_for(50ms);
    }
    return count;
}

TEST_F(SoftIocPVAFixture, pva_ManyPuts) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:sink", 5.0);
    ctx.connect("pace:test:count", 5.0);

    for (int i = 0; i < 100; i++) {
        ASSERT_TRUE(ctx.put("pace:test:sink", static_cast<double>(i)));
    }

    EXPECT_DOUBLE_EQ(wait_count(ctx, 100.0), 100.0);
}

TEST_F(SoftIocPVAFixture, pva_PutBeforeDestroy) {
    {
        pace::Context ctx("pva");
        ctx.connect("pace:test:sink", 5.0);
        ASSERT_TRUE(ctx.put("pace:test:sink", 1.0));
    }

    pace::Context ctx("pva");
    ctx.connect("pace:test:count", 5.0);
    EXPECT_DOUBLE_EQ(wait_count(ctx, 1.0, 2), 1.0);
}

TEST_F(SoftIocPVAFixture, pva_DestroyWithStalledPut) {
    auto ctx = std::make_unique<pace::Context>("pva");
    ctx->connect("pace:test:sink", 5.0);

    kill(ioc_pid_, SIGSTOP);
    int status;
    waitpid(ioc_pid_, &status, WUNTRACED);

    for (int i = 0; i < 10; i++) {
        ctx->put("pace:test:sink", static_cast<double>(i));
    }
    auto start = std::chrono::steady_clock::now();
    ctx.reset();
    auto elapsed = std::chrono::steady_clock::now() - start;

    kill(ioc_pid_, SIGCONT);
    EXPECT_LT(elapsed, 2s);
}

static bool wait_double(pace::Context& ctx, const std::string& pv, double expected, int timeout_sec = 5) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (std::chrono::steady_clock::now() < deadline) {
        if (ctx.get<double>(pv).value_or(0.0) == expected) {
            return true;
        }
        std::this_thread::sleep_for(50ms);
    }
    return false;
}

TEST_F(SoftIocPVAFixture, pva_PvaContextShared) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);

    auto a = ctx.pva_context();
    ASSERT_TRUE(a);
    auto n = a.use_count();
    auto b = ctx.pva_context();
    EXPECT_EQ(a.use_count(), n + 1);

    auto result = a.get("pace:test:ao").exec()->wait(5.0);
    EXPECT_DOUBLE_EQ(result["value"].as<double>(), 3.14);
}

TEST_F(SoftIocPVAFixture, pva_PvaContextFromCAContext) {
    pace::Context ctx("ca");
    auto pva = ctx.pva_context();
    ASSERT_TRUE(pva);

    auto result = pva.get("pace:test:longout").exec()->wait(5.0);
    EXPECT_EQ(result["value"].as<int>(), 42);

    ctx.connect("pva://pace:test:ao", 5.0);
    auto after = ctx.pva_context();
    auto n = pva.use_count();
    auto copy = after;
    EXPECT_EQ(pva.use_count(), n + 1);
}

TEST_F(SoftIocPVAFixture, pva_GetValueRoot) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);
    ctx.connect("pace:test:Position", 5.0);

    auto ao = ctx.get<pvxs::Value>("pace:test:ao");
    ASSERT_TRUE(ao);
    EXPECT_DOUBLE_EQ((*ao)["value"].as<double>(), 3.14);

    auto pos = ctx.get<pvxs::Value>("pace:test:Position");
    ASSERT_TRUE(pos);
    EXPECT_DOUBLE_EQ((*pos)["X.value"].as<double>(), 1.11);
    EXPECT_DOUBLE_EQ((*pos)["Y.value"].as<double>(), 2.22);
    EXPECT_DOUBLE_EQ((*pos)["Z.value"].as<double>(), 3.33);
}

TEST_F(SoftIocPVAFixture, pva_GetValueField) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:Position", 5.0);

    auto y = ctx.get<pvxs::Value>("pace:test:Position", "Y");
    ASSERT_TRUE(y);
    EXPECT_DOUBLE_EQ((*y)["value"].as<double>(), 2.22);

    auto z = ctx.get<pvxs::Value>("pace:test:Position", "Z.value");
    ASSERT_TRUE(z);
    EXPECT_DOUBLE_EQ(z->as<double>(), 3.33);

    EXPECT_FALSE(ctx.get<pvxs::Value>("pace:test:Position", "nonexistent"));
}

TEST_F(SoftIocPVAFixture, pva_PutValueRoot) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);

    auto val = pvxs::nt::NTScalar{pvxs::TypeCode::Float64}.create();
    val["value"] = 6.28;
    ASSERT_TRUE(ctx.put("pace:test:ao", val));
    EXPECT_TRUE(wait_double(ctx, "pace:test:ao", 6.28));
}

TEST_F(SoftIocPVAFixture, pva_PutValueField) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);

    auto val = pvxs::TypeDef(pvxs::TypeCode::Float64).create();
    val = 1.5;
    ASSERT_TRUE(ctx.put("pace:test:ao", val, "value"));
    EXPECT_TRUE(wait_double(ctx, "pace:test:ao", 1.5));
}

TEST_F(SoftIocPVAFixture, pva_PutValueIsCopied) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);

    auto val = pvxs::nt::NTScalar{pvxs::TypeCode::Float64}.create();
    val["value"] = 7.0;
    ASSERT_TRUE(ctx.put("pace:test:ao", val));
    val["value"] = 99.0;
    EXPECT_TRUE(wait_double(ctx, "pace:test:ao", 7.0));
}

TEST_F(SoftIocPVAFixture, pva_PutEmptyValueFails) {
    pace::Context ctx("pva");
    ctx.connect("pace:test:ao", 5.0);
    EXPECT_FALSE(ctx.put("pace:test:ao", pvxs::Value{}));
}

TEST_F(SoftIocPVAFixture, pva_ValueOverloadsFailOnCA) {
    pace::Context ctx("ca");
    ctx.connect("pace:test:ao", 5.0);

    EXPECT_FALSE(ctx.get<pvxs::Value>("pace:test:ao"));

    auto val = pvxs::nt::NTScalar{pvxs::TypeCode::Float64}.create();
    val["value"] = 6.28;
    EXPECT_FALSE(ctx.put("pace:test:ao", val));
    EXPECT_DOUBLE_EQ(ctx.get<double>("pace:test:ao").value_or(0.0), 3.14);
}
