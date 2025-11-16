#pragma once
#include <future>
#include <string>

class MachineOptimizer {
public:
    std::future<std::string> run_async();
};

