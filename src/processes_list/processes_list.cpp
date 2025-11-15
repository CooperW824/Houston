#include "processes_list.hpp"
#include "process.hpp"
#include "network_tracker.hpp"
#include <vector>
#include <statgrab.h>
#include <mutex>
#include <ctime>

static std::once_flag init_flag;

static void init_statgrab()
{
    sg_init(1);
    sg_drop_privileges();
}

std::vector<Process> get_processes_list()
{
    std::vector<Process> processes;

    std::call_once(init_flag, init_statgrab);

    size_t num_processes;
    sg_process_stats *process_stats = sg_get_process_stats(&num_processes);

    if (process_stats == nullptr)
    {
        return processes;
    }

    time_t current_time = time(nullptr);

    NetworkTracker& tracker = NetworkTracker::getInstance();

    for (size_t i = 0; i < num_processes; i++)
    {
        pid_t pid = process_stats[i].pid;
        std::string name = process_stats[i].process_name;
        unsigned long memory = process_stats[i].proc_resident / 1024;
        double cpu = process_stats[i].cpu_percent;
        unsigned long network = tracker.getProcessNetworkUsage(pid);
        unsigned long uptime = current_time - process_stats[i].start_time;
        std::string command = process_stats[i].proctitle ? process_stats[i].proctitle : "";

        Process proc(pid, name, memory, cpu, network, uptime, command);
        processes.push_back(proc);
    }

    return processes;
}
