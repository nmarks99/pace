#include <chrono>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ezec.hpp"

using namespace std::chrono_literals;

class SoftIocPVAFixture : public ::testing::Test {
  protected:
    pid_t ioc_pid_ = -1;

    void SetUp() override {
        std::string softIoc = std::string(EPICS_BASE) + "/bin/" + EPICS_HOST_ARCH + "/softIocPVA";
        std::string db = std::string(TEST_DB_DIR) + "/test.db";

        ioc_pid_ = fork();
        ASSERT_NE(ioc_pid_, -1) << "fork() failed";

        if (ioc_pid_ == 0) {
            close(STDIN_FILENO);
            int devnull = open("/dev/null", O_WRONLY);
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
            execlp(softIoc.c_str(), "softIocPVA", "-d", db.c_str(), nullptr);
            _exit(1);
        }

        std::this_thread::sleep_for(3s);
    }

    void TearDown() override {
        if (ioc_pid_ > 0) {
            kill(ioc_pid_, SIGTERM);
            int status;
            waitpid(ioc_pid_, &status, 0);
        }
    }
};

bool wait_pva_sync(ezec::Context& ctx, int timeout_sec = 5) {
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

TEST_F(SoftIocPVAFixture, pva_BindDouble) {
    ezec::Context ctx("pva");

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao");

    ASSERT_TRUE(wait_pva_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST_F(SoftIocPVAFixture, pva_BindInt) {
    ezec::Context ctx("pva");

    int val = 0;
    ctx.bind(val, "ezec:test:longout");

    ASSERT_TRUE(wait_pva_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_EQ(val, 42);
}

TEST_F(SoftIocPVAFixture, pva_BindString) {
    ezec::Context ctx("pva");

    std::string val;
    ctx.bind(val, "ezec:test:stringout");

    ASSERT_TRUE(wait_pva_sync(ctx)) << "No monitor update received within timeout";
    EXPECT_EQ(val, "Hello World");
}

TEST_F(SoftIocPVAFixture, pva_BindMultipleTypes) {
    ezec::Context ctx("pva");

    double ao_double = 0.0;
    int ao_int = 0;
    std::string ao_str;

    ctx.bind(ao_double, "ezec:test:ao");
    ctx.bind(ao_int, "ezec:test:ao");
    ctx.bind(ao_str, "ezec:test:ao");

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
    ezec::Context ctx("pva");

    double ao_val = 0.0;
    int longout_val = 0;
    std::string stringout_val;

    ctx.bind(ao_val, "ezec:test:ao");
    ctx.bind(longout_val, "ezec:test:longout");
    ctx.bind(stringout_val, "ezec:test:stringout");

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

TEST_F(SoftIocPVAFixture, pva_NoMonitorUntilBind) {
    ezec::Context ctx("pva");

    // PVA connection state is only tracked after a subscription is created,
    // so we can't wait for connected() here. Instead, wait a bit for the
    // channel to have time to connect, then verify no data arrives without bind.
    ctx.get_channel("ezec:test:ao");
    std::this_thread::sleep_for(3s);
    EXPECT_FALSE(ctx.sync()) << "sync() returned true before bind() was called";

    double val = 0.0;
    ctx.bind(val, "ezec:test:ao");

    ASSERT_TRUE(wait_pva_sync(ctx)) << "No monitor update after bind()";
    EXPECT_DOUBLE_EQ(val, 3.14);
}
