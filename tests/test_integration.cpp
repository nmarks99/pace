#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ezec.hpp"

using namespace std::chrono_literals;

class SoftIocFixture : public ::testing::Test {
  protected:
    pid_t ioc_pid_ = -1;

    void SetUp() override {
        std::string softIoc = std::string(EPICS_BASE) + "/bin/" + EPICS_HOST_ARCH + "/softIoc";
        std::string db = std::string(TEST_DB_DIR) + "/test.db";

        ioc_pid_ = fork();
        ASSERT_NE(ioc_pid_, -1) << "fork() failed";

        if (ioc_pid_ == 0) {
            close(STDIN_FILENO);
            execlp(softIoc.c_str(), "softIoc", "-d", db.c_str(), nullptr);
            _exit(1);
        }

        std::this_thread::sleep_for(2s);
    }

    void TearDown() override {
        if (ioc_pid_ > 0) {
            kill(ioc_pid_, SIGTERM);
            int status;
            waitpid(ioc_pid_, &status, 0);
        }
    }
};

/// Waits for channel to connect or times out
void wait_connect(ezec::CAChannel& channel, int timeout_sec = 5) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(50ms);
    }
}

/// Waits for sync to return (true=new data available) or times out
bool wait_sync(ezec::CAChannel& channel, int timeout_sec = 5) {
    bool new_data = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (!new_data && std::chrono::steady_clock::now() < deadline) {
        new_data = channel.sync();
        if (!new_data) {
            std::this_thread::sleep_for(50ms);
        }
    }
    return new_data;
}


TEST_F(SoftIocFixture, AoBindSyncInitialValue) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = 0.0;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 3.14);
}

TEST_F(SoftIocFixture, AoMultipleBind) {
    ezec::CAChannel channel("EZECTEST:ao");

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
}

TEST_F(SoftIocFixture, AoBindToString) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_STREQ(value.c_str(), "3.14");
}
