#include <gtest/gtest.h>
#include "../src/processes_list/process.hpp"

// Test fixture for Process class tests
class ProcessTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
};

// ===========================
// Constructor and Getter Tests
// ===========================

TEST_F(ProcessTest, ConstructorSetsAllFields) {
    Process proc(1234, "test_process", 5000, 10.5, 1024);

    EXPECT_EQ(proc.get_pid(), 1234);
    EXPECT_EQ(proc.get_process_name(), "test_process");
    EXPECT_EQ(proc.get_memory_usage(), 5000);
    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 10.5);
    EXPECT_EQ(proc.get_network_usage(), 1024);
}

TEST_F(ProcessTest, ConstructorWithDefaultValues) {
    Process proc(5678);

    EXPECT_EQ(proc.get_pid(), 5678);
    EXPECT_EQ(proc.get_process_name(), "");
    EXPECT_EQ(proc.get_memory_usage(), 0);
    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 0.0);
    EXPECT_EQ(proc.get_network_usage(), 0);
}

// ===========================
// Setter Tests
// ===========================

TEST_F(ProcessTest, SetProcessName) {
    Process proc(1000);
    proc.set_process_name("chrome");

    EXPECT_EQ(proc.get_process_name(), "chrome");
}

TEST_F(ProcessTest, SetMemoryUsage) {
    Process proc(1000);
    proc.set_memory_usage(8192);

    EXPECT_EQ(proc.get_memory_usage(), 8192);
}

TEST_F(ProcessTest, SetCpuUsage) {
    Process proc(1000);
    proc.set_cpu_usage(25.7);

    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 25.7);
}

TEST_F(ProcessTest, SetNetworkUsage) {
    Process proc(1000);
    proc.set_network_usage(2048);

    EXPECT_EQ(proc.get_network_usage(), 2048);
}

// ===========================
// Multiple Updates Tests
// ===========================

TEST_F(ProcessTest, MultipleUpdates) {
    Process proc(1000, "initial_name", 1000, 5.0, 100);

    proc.set_process_name("updated_name");
    proc.set_memory_usage(2000);
    proc.set_cpu_usage(10.0);
    proc.set_network_usage(200);

    EXPECT_EQ(proc.get_pid(), 1000);  // PID should not change
    EXPECT_EQ(proc.get_process_name(), "updated_name");
    EXPECT_EQ(proc.get_memory_usage(), 2000);
    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 10.0);
    EXPECT_EQ(proc.get_network_usage(), 200);
}

// ===========================
// Edge Case Tests
// ===========================

TEST_F(ProcessTest, ZeroValues) {
    Process proc(0, "", 0, 0.0, 0);

    EXPECT_EQ(proc.get_pid(), 0);
    EXPECT_EQ(proc.get_process_name(), "");
    EXPECT_EQ(proc.get_memory_usage(), 0);
    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 0.0);
    EXPECT_EQ(proc.get_network_usage(), 0);
}

TEST_F(ProcessTest, LargeValues) {
    Process proc(999999, "large_process", 999999999UL, 999.99, 999999999UL);

    EXPECT_EQ(proc.get_pid(), 999999);
    EXPECT_EQ(proc.get_process_name(), "large_process");
    EXPECT_EQ(proc.get_memory_usage(), 999999999UL);
    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 999.99);
    EXPECT_EQ(proc.get_network_usage(), 999999999UL);
}

TEST_F(ProcessTest, NegativePID) {
    // Some systems use negative PIDs for process groups
    Process proc(-1);

    EXPECT_EQ(proc.get_pid(), -1);
}

TEST_F(ProcessTest, ProcessNameWithSpaces) {
    Process proc(1000, "Google Chrome");

    EXPECT_EQ(proc.get_process_name(), "Google Chrome");
}

TEST_F(ProcessTest, ProcessNameWithSpecialCharacters) {
    Process proc(1000, "[kernel/worker]");

    EXPECT_EQ(proc.get_process_name(), "[kernel/worker]");
}

TEST_F(ProcessTest, VeryLongProcessName) {
    std::string long_name(1000, 'a');
    Process proc(1000, long_name);

    EXPECT_EQ(proc.get_process_name(), long_name);
}

TEST_F(ProcessTest, EmptyProcessName) {
    Process proc(1000, "");
    EXPECT_EQ(proc.get_process_name(), "");

    proc.set_process_name("");
    EXPECT_EQ(proc.get_process_name(), "");
}

// ===========================
// Floating Point CPU Tests
// ===========================

TEST_F(ProcessTest, CpuUsagePrecision) {
    Process proc(1000);

    proc.set_cpu_usage(12.3456789);
    EXPECT_NEAR(proc.get_cpu_usage(), 12.3456789, 0.0000001);
}

TEST_F(ProcessTest, NegativeCpuUsage) {
    // Some systems might report negative values in edge cases
    Process proc(1000);
    proc.set_cpu_usage(-1.0);

    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), -1.0);
}

TEST_F(ProcessTest, CpuUsageOver100) {
    // Multi-core systems can report >100% CPU usage
    Process proc(1000);
    proc.set_cpu_usage(450.5);

    EXPECT_DOUBLE_EQ(proc.get_cpu_usage(), 450.5);
}

// ===========================
// Copy and Assignment Tests
// ===========================

TEST_F(ProcessTest, CopyConstructor) {
    Process proc1(1234, "test", 1000, 50.0, 512);
    Process proc2 = proc1;

    EXPECT_EQ(proc2.get_pid(), proc1.get_pid());
    EXPECT_EQ(proc2.get_process_name(), proc1.get_process_name());
    EXPECT_EQ(proc2.get_memory_usage(), proc1.get_memory_usage());
    EXPECT_DOUBLE_EQ(proc2.get_cpu_usage(), proc1.get_cpu_usage());
    EXPECT_EQ(proc2.get_network_usage(), proc1.get_network_usage());
}

TEST_F(ProcessTest, Assignment) {
    Process proc1(1234, "test", 1000, 50.0, 512);
    Process proc2(5678);

    proc2 = proc1;

    EXPECT_EQ(proc2.get_pid(), proc1.get_pid());
    EXPECT_EQ(proc2.get_process_name(), proc1.get_process_name());
    EXPECT_EQ(proc2.get_memory_usage(), proc1.get_memory_usage());
    EXPECT_DOUBLE_EQ(proc2.get_cpu_usage(), proc1.get_cpu_usage());
    EXPECT_EQ(proc2.get_network_usage(), proc1.get_network_usage());
}

TEST_F(ProcessTest, IndependenceAfterCopy) {
    Process proc1(1234, "test", 1000, 50.0, 512);
    Process proc2 = proc1;

    proc2.set_process_name("different");
    proc2.set_memory_usage(2000);
    proc2.set_cpu_usage(75.0);

    EXPECT_EQ(proc1.get_process_name(), "test");
    EXPECT_EQ(proc1.get_memory_usage(), 1000);
    EXPECT_DOUBLE_EQ(proc1.get_cpu_usage(), 50.0);
}

