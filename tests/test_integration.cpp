#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "ezec.hpp"

using namespace std::chrono_literals;

// Handles setup and teardown of a softIoc for testing
// softIoc shell output is redirected to null
class SoftIocFixture : public ::testing::Test {
  protected:
    pid_t ioc_pid_ = -1;
    std::unique_ptr<ezec::Context> ctx_;

    void SetUp() override {
        std::string softIoc = std::string(EPICS_BASE) + "/bin/" + EPICS_HOST_ARCH + "/softIoc";
        std::string db = std::string(TEST_DB_DIR) + "/test.db";

        ioc_pid_ = fork();
        ASSERT_NE(ioc_pid_, -1) << "fork() failed";

        if (ioc_pid_ == 0) {
            close(STDIN_FILENO);
            int devnull = open("/dev/null", O_WRONLY);
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
            execlp(softIoc.c_str(), "softIoc", "-d", db.c_str(), nullptr);
            _exit(1);
        }

        std::this_thread::sleep_for(2s);

        ctx_ = std::make_unique<ezec::Context>();
    }

    void TearDown() override {
        ctx_.reset();
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

TEST_F(SoftIocFixture, ao_BindDouble) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    double value = 0.0;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 3.14);

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value, 6.28);
}

TEST_F(SoftIocFixture, ao_BindMultipleDoubles) {
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

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_DOUBLE_EQ(value1, 6.28);
    EXPECT_DOUBLE_EQ(value2, 6.28);
}

TEST_F(SoftIocFixture, ao_BindString) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_STREQ(value.c_str(), "3.14");

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value.c_str(), "6.28");
}

TEST_F(SoftIocFixture, ao_BindInt) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    int value = 0;
    channel.bind(value);

    auto new_data = wait_sync(channel);
    ASSERT_TRUE(new_data) << "No monitor update received within timeout";
    EXPECT_EQ(value, 3);

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_EQ(value, 6);
}

TEST_F(SoftIocFixture, ao_BindMultipleTypes) {
    ezec::CAChannel channel("EZECTEST:ao");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::string value_str;
    double value_double;
    int value_int;
    channel.bind(value_str);
    channel.bind(value_double);
    channel.bind(value_int);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value_str.c_str(), "3.14");
    EXPECT_DOUBLE_EQ(value_double, 3.14);
    EXPECT_EQ(value_int, 3);

    double new_value = 6.28;
    ca_put(DBR_DOUBLE, channel.id(), &new_value);
    ca_flush_io();

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    EXPECT_STREQ(value_str.c_str(), "6.28");
    EXPECT_DOUBLE_EQ(value_double, 6.28);
    EXPECT_EQ(value_int, 6);
}
