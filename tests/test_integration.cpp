#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ezec.hpp"

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

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    void TearDown() override {
        if (ioc_pid_ > 0) {
            kill(ioc_pid_, SIGTERM);
            int status;
            waitpid(ioc_pid_, &status, 0);
        }
    }
};

void wait_for_connect(ezec::CAChannel& channel, int timout_sec = 5) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(SoftIocFixture, AoBindSyncInitialValue) {
    ezec::CAChannel channel("EZECTEST:ao");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within 5 seconds";

    double value = 0.0;
    channel.bind(value);

    bool synced = false;
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!synced && std::chrono::steady_clock::now() < deadline) {
        synced = channel.sync();
        if (!synced)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(synced) << "No monitor update received within 5 seconds";
    EXPECT_DOUBLE_EQ(value, 3.14);
}

TEST_F(SoftIocFixture, AoMultipleBind) {
    ezec::CAChannel channel("EZECTEST:ao");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within 5 seconds";

    double value1 = 0.0;
    double value2 = 0.0;
    channel.bind(value1);
    channel.bind(value2);

    bool synced = false;
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!synced && std::chrono::steady_clock::now() < deadline) {
        synced = channel.sync();
        if (!synced)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(synced) << "No monitor update received within 5 seconds";
    EXPECT_DOUBLE_EQ(value1, 3.14);
    EXPECT_DOUBLE_EQ(value2, 3.14);
}

TEST_F(SoftIocFixture, AoBindToString) {
    ezec::CAChannel channel("EZECTEST:ao");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within 5 seconds";

    std::string value;
    channel.bind(value);

    bool synced = false;
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!synced && std::chrono::steady_clock::now() < deadline) {
        synced = channel.sync();
        if (!synced)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ASSERT_TRUE(synced) << "No monitor update received within 5 seconds";
    EXPECT_STREQ(value.c_str(), "3.14");
}
