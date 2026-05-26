#include <vector>
#include "test_helpers.hpp"

TEST_F(SoftIocFixture, wfchar_BindVectorInt) {
    ezec::CAChannel channel("ezec:test:wfchar.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::vector<int> value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    std::vector<int> truth = {1,2,3};
    EXPECT_EQ(value, truth);
}

TEST_F(SoftIocFixture, wfdouble_BindVectorDouble) {
    ezec::CAChannel channel("ezec:test:wfdouble.VAL");

    wait_connect(channel);
    ASSERT_TRUE(channel.connected()) << "Channel did not connect within timeout";

    std::vector<double> value;
    channel.bind(value);

    ASSERT_TRUE(wait_sync(channel)) << "No monitor update received within timeout";
    std::vector<double> truth = {1.11,2.22,3.33};
    EXPECT_EQ(value, truth);

}
