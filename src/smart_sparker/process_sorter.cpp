#include "process_sorter.hpp"
#include <algorithm>
#include <unordered_set>
#include "json.hpp"
#include "../processes_list/processes_list.hpp"
using json = nlohmann::json;

json get_top_processes_json()
{
    std::vector<Process> all = get_processes_list();
    if (all.empty())
        return json{ {"processes", json::array()} };

    std::vector<Process> by_cpu = all;
    std::sort(by_cpu.begin(), by_cpu.end(),
        [](const Process &a, const Process &b){
            return a.get_cpu_usage() > b.get_cpu_usage();
        });
    if (by_cpu.size() > 20)
        by_cpu.erase(by_cpu.begin() + 20, by_cpu.end());

    std::vector<Process> by_mem = all;
    std::sort(by_mem.begin(), by_mem.end(),
        [](const Process &a, const Process &b){
            return a.get_memory_usage() > b.get_memory_usage();
        });
    if (by_mem.size() > 20)
        by_mem.erase(by_mem.begin() + 20, by_mem.end());

    std::unordered_set<pid_t> seen;
    std::vector<Process> merged;

    for (const auto& p : by_cpu)
        if (seen.insert(p.get_pid()).second)
            merged.push_back(p);

    for (const auto& p : by_mem)
        if (seen.insert(p.get_pid()).second)
            merged.push_back(p);

    json processes_json = json::array();
    for (const auto& p : merged) {
        processes_json.push_back({
            {"pid", p.get_pid()},
            {"name", p.get_process_name()},
            {"cpu_usage", p.get_cpu_usage()},
            {"memory_usage", p.get_memory_usage()},
            {"network_usage", p.get_network_usage()},
            {"cpu_time", p.get_cpu_time()}
        });
    }

    return json{
        {"processes", processes_json}
    };
}
