#ifndef PROCESS_SORTER_HPP
#define PROCESS_SORTER_HPP
#include <iostream>
#include "json.hpp"  // nlohmann::json
#include "../processes_list/process.hpp"
#include <vector>

// Return full list of processes (CPU + memory + merged top 20)
std::vector<Process> get_top_processes();

// Return JSON ready to send to AI
nlohmann::json get_top_processes_json();

#endif
