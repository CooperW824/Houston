#ifndef __PROCESSES_VIEW_TABLE_HPP
#define __PROCESSES_VIEW_TABLE_HPP

#include "ftxui/dom/elements.hpp"
#include "../../processes_list/process.hpp"
#include "processes_view_state.hpp"
#include <vector>
#include <mutex>

using namespace ftxui;

namespace ProcessesView {

// Sort and filter processes according to the view state
std::vector<Process> prepare_process_list(
    const std::vector<Process>& processes,
    const ViewState& state
);

// Create the process table UI element
Element create_process_table(
    const std::vector<Process>& processes,
    ViewState& state
);

} // namespace ProcessesView

#endif

