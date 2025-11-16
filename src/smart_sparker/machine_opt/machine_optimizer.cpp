#include "machine_optimizer.hpp"
#include "../get_https.hpp"
#include <future>
#include <iostream>
#include <string>

std::future<std::string> MachineOptimizer::run_async() {
    //get_https() on a separate thread
    return std::async(std::launch::async, []() -> std::string {
        std::string pid_to_kill = get_https();  // Calls your AI function
        return pid_to_kill;
    });
}
