#include <gtest/gtest.h>
#include "ezec.hpp"

TEST(PVA, unimplemented) {
    ezec::PVAChannel chan("dummy");
    EXPECT_FALSE(chan.connected());
}
