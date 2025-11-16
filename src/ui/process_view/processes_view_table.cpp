#include "processes_view_table.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ProcessesView {

std::vector<Process> prepare_process_list(
    const std::vector<Process>& processes,
    const ViewState& state
) {
    std::vector<Process> processes_copy = processes;

    if (processes_copy.size() > 0 && processes_copy.size() < 10000) {
        SortColumn current_sort = *state.sort_column;
        bool ascending = *state.sort_ascending;

        std::stable_sort(processes_copy.begin(), processes_copy.end(),
            [current_sort, ascending](const Process& a, const Process& b) {
                switch (current_sort) {
                    case SortColumn::PID:
                        return ascending ? (a.get_pid() < b.get_pid()) : (a.get_pid() > b.get_pid());
                    case SortColumn::NAME:
                        return ascending ? (a.get_process_name() < b.get_process_name()) : (a.get_process_name() > b.get_process_name());
                    case SortColumn::MEMORY:
                        return ascending ? (a.get_memory_usage() < b.get_memory_usage()) : (a.get_memory_usage() > b.get_memory_usage());
                    case SortColumn::CPU:
                        return ascending ? (a.get_cpu_usage() < b.get_cpu_usage()) : (a.get_cpu_usage() > b.get_cpu_usage());
                    case SortColumn::NETWORK:
                        return ascending ? (a.get_network_usage() < b.get_network_usage()) : (a.get_network_usage() > b.get_network_usage());
                    case SortColumn::TIME:
                        return ascending ? (a.get_cpu_time() < b.get_cpu_time()) : (a.get_cpu_time() > b.get_cpu_time());
                    case SortColumn::COMMAND:
                        return ascending ? (a.get_command() < b.get_command()) : (a.get_command() > b.get_command());
                    default:
                        return false;
                }
            });
    }

    if (!state.search_phrase->empty()) {
        std::string search_lower = *state.search_phrase;
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

        processes_copy.erase(
            std::remove_if(processes_copy.begin(), processes_copy.end(),
                [&search_lower](const Process& proc) {
                    std::string name_lower = proc.get_process_name();
                    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

                    std::string pid_str = std::to_string(proc.get_pid());

                    return name_lower.find(search_lower) == std::string::npos &&
                           pid_str.find(search_lower) == std::string::npos;
                }
            ),
            processes_copy.end()
        );
    }

    return processes_copy;
}

Element create_process_table(
    const std::vector<Process>& processes,
    ViewState& state
) {
    state.boxes->clear();
    state.sigterm_boxes->clear();
    state.sigkill_boxes->clear();
    std::vector<Process> processes_copy = prepare_process_list(processes, state);

    if (*state.selected_index >= static_cast<int>(processes_copy.size())) {
        *state.selected_index = std::max(0, static_cast<int>(processes_copy.size()) - 1);
    }
    if (*state.selected_index < 0) {
        *state.selected_index = 0;
    }

    std::vector<Element> rows;

    auto get_indicator = [&](SortColumn col) {
        if (*state.sort_column == col) {
            return *state.sort_ascending ? " ▲" : " ▼";
        }
        return "";
    };

    rows.push_back(hbox({
        text("Kill") | size(WIDTH, EQUAL, ViewState::COL_KILL_WIDTH),
        separator(),
        text("PID" + std::string(get_indicator(SortColumn::PID))) | size(WIDTH, EQUAL, ViewState::COL_PID_WIDTH) | reflect(*state.header_pid_box),
        separator(),
        text("Name" + std::string(get_indicator(SortColumn::NAME))) | size(WIDTH, EQUAL, ViewState::COL_NAME_WIDTH) | reflect(*state.header_name_box),
        separator(),
        text("MEM (KB)" + std::string(get_indicator(SortColumn::MEMORY))) | size(WIDTH, EQUAL, ViewState::COL_MEMORY_WIDTH) | reflect(*state.header_memory_box),
        separator(),
        text("CPU (%)" + std::string(get_indicator(SortColumn::CPU))) | size(WIDTH, EQUAL, ViewState::COL_CPU_WIDTH) | reflect(*state.header_cpu_box),
        separator(),
        text("NET (B)" + std::string(get_indicator(SortColumn::NETWORK))) | size(WIDTH, EQUAL, ViewState::COL_NETWORK_WIDTH) | reflect(*state.header_network_box),
        separator(),
        text("TIME+" + std::string(get_indicator(SortColumn::TIME))) | size(WIDTH, EQUAL, ViewState::COL_TIME_WIDTH) | reflect(*state.header_time_box),
        separator(),
        text("Command" + std::string(get_indicator(SortColumn::COMMAND))) | flex | reflect(*state.header_command_box),
    }) | bold);

    rows.push_back(separator());

    state.boxes->resize(processes_copy.size());
    state.sigterm_boxes->resize(processes_copy.size());
    state.sigkill_boxes->resize(processes_copy.size());
    state.displayed_pids->resize(processes_copy.size());

    for (size_t i = 0; i < processes_copy.size(); i++) {
        const auto& proc = processes_copy[i];
        (*state.displayed_pids)[i] = proc.get_pid();
        std::stringstream pid_ss, mem_ss, cpu_ss, net_ss, time_ss;
        pid_ss << proc.get_pid();
        mem_ss << proc.get_memory_usage();
        cpu_ss << std::fixed << std::setprecision(2) << proc.get_cpu_usage();
        net_ss << proc.get_network_usage();

        unsigned long time_seconds = proc.get_cpu_time();
        unsigned long hours = time_seconds / 3600;
        unsigned long minutes = (time_seconds % 3600) / 60;
        unsigned long seconds = time_seconds % 60;
        time_ss << hours << ":" << std::setfill('0') << std::setw(2) << minutes << ":" << std::setw(2) << seconds;

        Element sigterm_btn = text("x") | color(Color::Red);
        if (static_cast<int>(i) == *state.hover_sigterm) {
            sigterm_btn = sigterm_btn | bgcolor(Color::White) | bold;
        }
        sigterm_btn = sigterm_btn | size(WIDTH, EQUAL, ViewState::COL_SIGTERM_WIDTH) | reflect((*state.sigterm_boxes)[i]);

        Element sigkill_btn = text("☠ ") | color(Color::RedLight);
        if (static_cast<int>(i) == *state.hover_sigkill) {
            sigkill_btn = sigkill_btn | bgcolor(Color::White) | bold;
        }
        sigkill_btn = sigkill_btn | size(WIDTH, EQUAL, ViewState::COL_SIGKILL_WIDTH) | reflect((*state.sigkill_boxes)[i]);

        auto row = hbox({
            sigterm_btn,
            text(" "),
            sigkill_btn,
            separator(),
            text(pid_ss.str()) | size(WIDTH, EQUAL, ViewState::COL_PID_WIDTH),
            separator(),
            text(proc.get_process_name()) | size(WIDTH, EQUAL, ViewState::COL_NAME_WIDTH),
            separator(),
            text(mem_ss.str()) | size(WIDTH, EQUAL, ViewState::COL_MEMORY_WIDTH),
            separator(),
            text(cpu_ss.str()) | size(WIDTH, EQUAL, ViewState::COL_CPU_WIDTH),
            separator(),
            text(net_ss.str()) | size(WIDTH, EQUAL, ViewState::COL_NETWORK_WIDTH),
            separator(),
            text(time_ss.str()) | size(WIDTH, EQUAL, ViewState::COL_TIME_WIDTH),
            separator(),
            text(proc.get_command()) | flex,
        });

        if (static_cast<int>(i) == *state.selected_index) {
            row = row | bgcolor(Color::Blue) | bold | focus;
        } else if (static_cast<int>(i) == *state.hover_index) {
            row = row | bgcolor(Color::GrayDark);
        }

        row = row | reflect((*state.boxes)[i]);
        rows.push_back(row);
    }

    auto process_list = vbox(rows) | vscroll_indicator | yframe | flex;

    if (*state.search_mode || !state.search_phrase->empty()) {
        std::string search_display = "/" + *state.search_phrase;
        if (*state.search_mode) {
            search_display += "_";
        }
        auto search_bar = text(search_display) | color(Color::Yellow) | bold;
        return vbox({
            process_list,
            separator(),
            search_bar,
        });
    }

    return process_list;
}

}

