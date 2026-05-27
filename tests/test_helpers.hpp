#pragma once

#include <chrono>
#include <cstdlib>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ezec.hpp"

using namespace std::chrono_literals;

class SoftIocFixture : public ::testing::Test {
  protected:
    pid_t ioc_pid_ = -1;
    // std::unique_ptr<ezec::Context> ctx_;

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

        // ctx_ = std::make_unique<ezec::Context>();
    }

    void TearDown() override {
        // ctx_.reset();
        if (ioc_pid_ > 0) {
            kill(ioc_pid_, SIGTERM);
            int status;
            waitpid(ioc_pid_, &status, 0);
        }
    }
};

inline void wait_connect(ezec::CAChannel& channel, int timeout_sec = 5) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (!channel.connected() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(50ms);
    }
}

inline bool wait_sync(ezec::CAChannel& channel, int timeout_sec = 5) {
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
