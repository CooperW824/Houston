#include "processes_view_inputs.hpp"
#include <algorithm>

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
)
{
    if (*search_mode) {
        if (event == Event::Escape) {
            *search_mode = false;
            *search_phrase = "";
            *selected_index = 0;
            return true;
        }
        if (event == Event::Backspace) {
            if (!search_phrase->empty()) {
                search_phrase->pop_back();
                *selected_index = 0;
            }
            return true;
        }
        if (event == Event::Return) {
            *search_mode = false;
            return true;
        }
        if (event.is_character()) {
            *search_phrase += event.character();
            *selected_index = 0;
            return true;
        }
        if (!event.is_mouse()) {
            return true;
        }
    }

    if (event == Event::Character('/')) {
        *search_mode = true;
        *search_phrase = "";
        *selected_index = 0;
        return true;
    }

    if (event == Event::Escape && !search_phrase->empty()) {
        *search_phrase = "";
        *selected_index = 0;
        return true;
    }

    if (event == Event::Backspace) {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        std::sort(processes_copy.begin(), processes_copy.end(), [](const Process& a, const Process& b) {
            return a.get_memory_usage() > b.get_memory_usage();
        });

        if (!search_phrase->empty()) {
            std::string search_lower = *search_phrase;
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

        if (*selected_index >= 0 && *selected_index < static_cast<int>(processes_copy.size())) {
            Process proc = processes_copy[*selected_index];
            proc.kill(15);
            return true;
        }
    }

    if (event == Event::Delete) {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        std::sort(processes_copy.begin(), processes_copy.end(), [](const Process& a, const Process& b) {
            return a.get_memory_usage() > b.get_memory_usage();
        });

        if (!search_phrase->empty()) {
            std::string search_lower = *search_phrase;
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

        if (*selected_index >= 0 && *selected_index < static_cast<int>(processes_copy.size())) {
            Process proc = processes_copy[*selected_index];
            proc.kill(9);
            return true;
        }
    }

    if (event == Event::ArrowUp || event == Event::Character('k')) {
        (*selected_index)--;
        if (*selected_index < 0) {
            *selected_index = 0;
        }
        return true;
    }
    if (event == Event::ArrowDown || event == Event::Character('j')) {
        (*selected_index)++;
        return true;
    }
    if (event == Event::PageUp) {
        (*selected_index) -= 10;
        if (*selected_index < 0) {
            *selected_index = 0;
        }
        return true;
    }
    if (event == Event::PageDown) {
        (*selected_index) += 10;
        return true;
    }
    if (event.is_mouse()) {
        auto mouse = event.mouse();

        if (mouse.button == Mouse::Left && mouse.motion == Mouse::Released) {
            std::vector<Process> processes_copy;
            {
                std::lock_guard<std::mutex> lock(processes_mutex);
                processes_copy = processes;
            }

            std::sort(processes_copy.begin(), processes_copy.end(), [](const Process& a, const Process& b) {
                return a.get_memory_usage() > b.get_memory_usage();
            });

            if (!search_phrase->empty()) {
                std::string search_lower = *search_phrase;
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

            for (int i = 0; i < static_cast<int>(sigterm_boxes->size()); ++i) {
                if ((*sigterm_boxes)[i].Contain(mouse.x, mouse.y)) {
                    Process proc = processes_copy[i];
                    proc.kill(15);
                    return true;
                }
                if ((*sigkill_boxes)[i].Contain(mouse.x, mouse.y)) {
                    Process proc = processes_copy[i];
                    proc.kill(9);
                    return true;
                }
            }
        }

        *hover_index = -1;
        *hover_sigterm = -1;
        *hover_sigkill = -1;

        for (int i = 0; i < static_cast<int>(sigterm_boxes->size()); ++i) {
            if ((*sigterm_boxes)[i].Contain(mouse.x, mouse.y)) {
                *hover_sigterm = i;
            }
            if ((*sigkill_boxes)[i].Contain(mouse.x, mouse.y)) {
                *hover_sigkill = i;
            }
        }

        for (int i = 0; i < static_cast<int>(boxes->size()); ++i) {
            if ((*boxes)[i].Contain(mouse.x, mouse.y)) {
                *hover_index = i;

                if (mouse.button == Mouse::Left && mouse.motion == Mouse::Released) {
                    *selected_index = i;
                    return true;
                }
                break;
            }
        }

        if (mouse.button == Mouse::WheelUp) {
            (*selected_index)--;
            if (*selected_index < 0) {
                *selected_index = 0;
            }
            return true;
        }
        if (mouse.button == Mouse::WheelDown) {
            (*selected_index)++;
            return true;
        }

        return false;
    }
    return false;
}

