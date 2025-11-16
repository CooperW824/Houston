#include "machine_optimizer.hpp"
#include "../get_https.hpp"
#include <future>
#include <string>
#include <utility>

std::future<std::pair<std::string, pid_t>> MachineOptimizer::run_async(const std::vector<Process>& processes) {
    //get_https() on a separate thread
    return std::async(std::launch::async, [processes]() -> std::pair<std::string, pid_t> {
        std::string pid_str = get_https();  // Calls your AI function
        
        // Convert PID string to integer
        pid_t target_pid;
        try {
            target_pid = std::stoi(pid_str);
        } catch (...) {
            return {"Invalid PID: " + pid_str, -1};
        }
        
        // Find the process with this PID
        for (const auto& proc : processes) {
            if (proc.get_pid() == target_pid) {
                return {proc.get_process_name() + " (PID: " + pid_str + ")", target_pid};
            }
        }
        
        // If not found, return PID only
        return {"Unknown Process (PID: " + pid_str + ")", target_pid};
    });
}
