#include "processes_view.hpp"
#include "processes_view_state.hpp"
#include "processes_view_table.hpp"
#include "processes_view_event_handler.hpp"
#include "process_detail_view.hpp"
#include <algorithm>
#include <chrono>

using namespace ProcessesView;

Component create_processes_view(std::vector<Process>& processes, std::mutex& processes_mutex, double& refresh_rate_seconds)
{
    auto state = std::make_shared<ViewState>();

    auto base_component = Renderer([&processes, &processes_mutex, &refresh_rate_seconds, state] {
        if (*state->show_detail_view) {
            std::vector<Process> processes_copy;
            {
                std::lock_guard<std::mutex> lock(processes_mutex);
                processes_copy = processes;
            }

            auto it = std::find_if(processes_copy.begin(), processes_copy.end(),
                [&](const Process& p) { return p.get_pid() == *state->detail_process_pid; });

            if (it != processes_copy.end()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - *state->last_sample_time).count();
                auto sample_interval_ms = static_cast<long long>(refresh_rate_seconds * 1000);

                if (elapsed >= sample_interval_ms) {
                    state->cpu_history->push_back(static_cast<float>(it->get_cpu_usage()));
                    state->memory_history->push_back(static_cast<float>(it->get_memory_usage()));
                    state->network_history->push_back(static_cast<float>(it->get_network_usage()));

                    if (state->cpu_history->size() > ViewState::HISTORY_SIZE) {
                        state->cpu_history->erase(state->cpu_history->begin());
                    }
                    if (state->memory_history->size() > ViewState::HISTORY_SIZE) {
                        state->memory_history->erase(state->memory_history->begin());
                    }
                    if (state->network_history->size() > ViewState::HISTORY_SIZE) {
                        state->network_history->erase(state->network_history->begin());
                    }

                    *state->last_sample_time = now;
                }

                return create_process_detail_view(*it, *state->cpu_history, *state->memory_history,
                                                  *state->network_history, ViewState::HISTORY_SIZE);
            } else {
                return vbox({
                    text("Process Not Found") | bold | center,
                    separator(),
                    text(""),
                    text("The process (PID: " + std::to_string(*state->detail_process_pid) + ") no longer exists.") | center,
                    text(""),
                    text("Press ESC to return to process list") | dim | center,
                }) | border | flex;
            }
        }

        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        return create_process_table(processes_copy, *state);
    });

    return CatchEvent(base_component, [&processes, &processes_mutex, state](Event event) {
        return handle_all_events(event, *state, processes, processes_mutex);
    });
}
