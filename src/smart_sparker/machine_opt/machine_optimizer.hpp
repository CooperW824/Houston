#pragma once
#include <future>
#include <string>
#include <vector>
#include <utility>
#include <sys/types.h>
#include "../../processes_list/process.hpp"

class MachineOptimizer {
public:
    // Returns a pair: <display_string, pid>
    std::future<std::pair<std::string, pid_t>> run_async(const std::vector<Process>& processes);
};

