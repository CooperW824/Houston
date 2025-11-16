#include "process_sorter.hpp"
#include <algorithm>
#include <unordered_set>

std::vector<Process> get_top_processes()
{
    std::vector<Process> all = get_processes_list();
    if (all.empty())
        return {};

//cpu descending 
    std::vector<Process> by_cpu = all;
    std::sort(by_cpu.begin(), by_cpu.end(),
        [](const Process &a, const Process &b) {
            return a.get_cpu_usage() > b.get_cpu_usage();
        });

    if (by_cpu.size() > 20)
        by_cpu.erase(by_cpu.begin() + 20, by_cpu.end());

//mem decending 
    std::vector<Process> by_mem = all;
    std::sort(by_mem.begin(), by_mem.end(),
        [](const Process &a, const Process &b) {
            return a.get_memory_usage() > b.get_memory_usage();
        });

    if (by_cpu.size() > 20)
        by_cpu.erase(by_cpu.begin() + 20, by_cpu.end());

//kill duplicates
    std::unordered_set<pid_t> seen;
    std::vector<Process> merged;
    merged.reserve(40);

//cpu list
    for (const auto &p : by_cpu)
    {
        if (seen.insert(p.get_pid()).second)
            merged.push_back(p);
    }
//mem list
    for (const auto &p : by_mem)
    {
        if (seen.insert(p.get_pid()).second)
            merged.push_back(p);
    }

    return merged;
}
