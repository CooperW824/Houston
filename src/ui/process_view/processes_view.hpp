#ifndef __PROCESSES_VIEW_HPP
#define __PROCESSES_VIEW_HPP

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "../../processes_list/process.hpp"
#include <vector>
#include <mutex>

using namespace ftxui;

Component create_processes_view(std::vector<Process>& processes, std::mutex& processes_mutex, double& refresh_rate_seconds);

#endif

