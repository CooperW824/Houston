#ifndef __PROCESSES_VIEW_INPUTS_HPP
#define __PROCESSES_VIEW_INPUTS_HPP

#include "ftxui/component/event.hpp"
#include "ftxui/screen/box.hpp"
#include "../processes_list/process.hpp"
#include <vector>
#include <memory>
#include <string>
#include <mutex>

using namespace ftxui;

bool handle_processes_view_event(
    Event event,
    std::shared_ptr<int> selected_index,
    std::shared_ptr<int> hover_index,
    std::shared_ptr<int> hover_sigterm,
    std::shared_ptr<int> hover_sigkill,
    std::shared_ptr<bool> search_mode,
    std::shared_ptr<std::string> search_phrase,
    std::shared_ptr<std::vector<Box>> boxes,
    std::shared_ptr<std::vector<Box>> sigterm_boxes,
    std::shared_ptr<std::vector<Box>> sigkill_boxes,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
);

#endif

