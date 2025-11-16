#include "processes_view_event_handler.hpp"
#include "processes_view_inputs.hpp"
#include "processes_view_table.hpp"
#include <algorithm>

namespace ProcessesView {

bool handle_detail_view_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
) {
    if (event == Event::Escape) {
        *state.show_detail_view = false;
        *state.detail_process_pid = 0;
        state.cpu_history->clear();
        state.memory_history->clear();
        state.network_history->clear();
        *state.last_sample_time = std::chrono::steady_clock::now();
        return true;
    }

    if (event == Event::Backspace) {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        auto it = std::find_if(processes_copy.begin(), processes_copy.end(),
            [&](const Process& p) { return p.get_pid() == *state.detail_process_pid; });

        if (it != processes_copy.end()) {
            it->kill(15);
        }

        *state.show_detail_view = false;
        *state.detail_process_pid = 0;
        state.cpu_history->clear();
        state.memory_history->clear();
        state.network_history->clear();
        *state.last_sample_time = std::chrono::steady_clock::now();
        return true;
    }

    if (event == Event::Delete) {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        auto it = std::find_if(processes_copy.begin(), processes_copy.end(),
            [&](const Process& p) { return p.get_pid() == *state.detail_process_pid; });

        if (it != processes_copy.end()) {
            it->kill(9);
        }

        *state.show_detail_view = false;
        *state.detail_process_pid = 0;
        state.cpu_history->clear();
        state.memory_history->clear();
        state.network_history->clear();
        *state.last_sample_time = std::chrono::steady_clock::now();
        return true;
    }

    return true;
}

bool handle_header_click_events(
    Event& event,
    ViewState& state
) {
    if (!event.is_mouse() || event.mouse().button != Mouse::Left || event.mouse().motion != Mouse::Released) {
        return false;
    }

    auto handle_column_click = [&](Box& box, SortColumn col, bool default_ascending = true) {
        if (box.Contain(event.mouse().x, event.mouse().y)) {
            if (*state.sort_column == col) {
                *state.sort_ascending = !*state.sort_ascending;
            } else {
                *state.sort_column = col;
                *state.sort_ascending = default_ascending;
            }
            return true;
        }
        return false;
    };

    if (handle_column_click(*state.header_pid_box, SortColumn::PID, true)) return true;
    if (handle_column_click(*state.header_name_box, SortColumn::NAME, true)) return true;
    if (handle_column_click(*state.header_memory_box, SortColumn::MEMORY, false)) return true;
    if (handle_column_click(*state.header_cpu_box, SortColumn::CPU, false)) return true;
    if (handle_column_click(*state.header_network_box, SortColumn::NETWORK, false)) return true;
    if (handle_column_click(*state.header_time_box, SortColumn::TIME, false)) return true;
    if (handle_column_click(*state.header_command_box, SortColumn::COMMAND, true)) return true;

    return false;
}

bool handle_process_list_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
) {
    if (event == Event::Return && !*state.search_mode) {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        processes_copy = prepare_process_list(processes_copy, state);

        if (*state.selected_index >= 0 && *state.selected_index < static_cast<int>(processes_copy.size())) {
            *state.detail_process_pid = processes_copy[*state.selected_index].get_pid();
            *state.show_detail_view = true;
            return true;
        }
    }

    return false;
}

bool handle_all_events(
    Event& event,
    ViewState& state,
    std::vector<Process>& processes,
    std::mutex& processes_mutex
) {
    if (*state.show_detail_view) {
        return handle_detail_view_events(event, state, processes, processes_mutex);
    }

    if (handle_process_list_events(event, state, processes, processes_mutex)) {
        return true;
    }

    if (handle_header_click_events(event, state)) {
        return true;
    }

    return handle_processes_view_event(
        event,
        state.selected_index,
        state.hover_index,
        state.hover_sigterm,
        state.hover_sigkill,
        state.search_mode,
        state.search_phrase,
        state.boxes,
        state.sigterm_boxes,
        state.sigkill_boxes,
        processes,
        processes_mutex,
        state.show_detail_view,
        state.detail_process_pid,
        state.last_click_time,
        state.last_clicked_index,
        state.displayed_pids
    );
}

} // namespace ProcessesView

