#include "processes_list.hpp"
#include "process.hpp"
#include <vector>
#include <statgrab.h>

std::vector<Process> get_processes_list()
{
    std::vector<Process> processes;

    sg_init(1);
    sg_drop_privileges();

    size_t num_processes;
    sg_process_stats *process_stats = sg_get_process_stats(&num_processes);

    if (process_stats == nullptr)
    {
        return processes;
    }

    for (size_t i = 0; i < num_processes; i++)
    {
        pid_t pid = process_stats[i].pid;
        unsigned long memory = process_stats[i].proc_resident / 1024;
        double cpu = process_stats[i].cpu_percent;
        unsigned long network = 0;

        Process proc(pid, memory, cpu, network);
        processes.push_back(proc);
    }

    return processes;
}
