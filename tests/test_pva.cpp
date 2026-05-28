#include <gtest/gtest.h>
#include "ezec.hpp"

TEST(PVA, unimplemented) {
    auto ctxt = pvxs::client::Context::fromEnv();
    ezec::PVAChannel chan(ctxt, "dummy");
    EXPECT_FALSE(chan.connected());
}
