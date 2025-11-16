#include "get_https.hpp"
#include <future>
#include <iostream>
#include <string>

class MachineOptimizer {
public:
    std::future<std::string> run_async() {
        //get_https() on a separate thread
        return std::async(std::launch::async, []() -> std::string {
            std::cout << "[MachineOptimizer] Running AI optimization in background...\n";
            std::string pid_to_kill = get_https();  // Calls your AI function
            std::cout << "[MachineOptimizer] AI returned PID: " << pid_to_kill << "\n";
            return pid_to_kill;
        });
    }
};
