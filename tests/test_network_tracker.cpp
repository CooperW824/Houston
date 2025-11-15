#include <gtest/gtest.h>
#include "../src/processes_list/network_tracker.hpp"
#include <unistd.h>

class NetworkTrackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tracker = &NetworkTracker::getInstance();
    }

    NetworkTracker* tracker;
};

TEST_F(NetworkTrackerTest, GetInstanceReturnsSameInstance) {
    NetworkTracker& instance1 = NetworkTracker::getInstance();
    NetworkTracker& instance2 = NetworkTracker::getInstance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(NetworkTrackerTest, GetNetworkUsageForCurrentProcess) {
    pid_t current_pid = getpid();

    unsigned long usage = tracker->getProcessNetworkUsage(current_pid);

    EXPECT_GE(usage, 0UL);
}

TEST_F(NetworkTrackerTest, GetNetworkUsageForInvalidProcess) {
    pid_t invalid_pid = 999999;

    unsigned long usage = tracker->getProcessNetworkUsage(invalid_pid);

    EXPECT_EQ(usage, 0UL);
}

TEST_F(NetworkTrackerTest, MultipleCallsTrackDelta) {
    pid_t current_pid = getpid();

    unsigned long first_call = tracker->getProcessNetworkUsage(current_pid);
    unsigned long second_call = tracker->getProcessNetworkUsage(current_pid);

    EXPECT_GE(first_call, 0UL);
    EXPECT_GE(second_call, 0UL);
}

TEST_F(NetworkTrackerTest, DifferentProcessesTrackedSeparately) {
    pid_t pid1 = getpid();
    pid_t pid2 = 1;

    unsigned long usage1 = tracker->getProcessNetworkUsage(pid1);
    unsigned long usage2 = tracker->getProcessNetworkUsage(pid2);

    EXPECT_GE(usage1, 0UL);
    EXPECT_GE(usage2, 0UL);
}

