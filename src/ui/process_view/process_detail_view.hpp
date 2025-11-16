#ifndef __PROCESS_DETAIL_VIEW_HPP
#define __PROCESS_DETAIL_VIEW_HPP

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "../../processes_list/process.hpp"
#include <vector>

using namespace ftxui;

Element create_process_detail_view(const Process& process,
                                   const std::vector<float>& cpu_history,
                                   const std::vector<float>& memory_history,
                                   const std::vector<float>& network_history,
                                   int history_size);

#endif

