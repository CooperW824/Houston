#ifndef __PROCESSES_VIEW_EVENT_HANDLER_HPP
#define __PROCESSES_VIEW_EVENT_HANDLER_HPP

#include "ftxui/component/event.hpp"
#include "processes_view_state.hpp"
#include "../../processes_list/process.hpp"
#include <vector>
#include <mutex>

using namespace ftxui;

namespace ProcessesView {

// Handle events when in detail view mode
bool handle_detail_view_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
);

// Handle header click events for sorting
bool handle_header_click_events(
    Event& event,
    ViewState& state
);

// Handle main process list events (Enter key for detail view)
bool handle_process_list_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
);

// Main event handler that coordinates all event handling
bool handle_all_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
);

} // namespace ProcessesView

#endif

