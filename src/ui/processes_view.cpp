#include "processes_view.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include "ftxui/screen/box.hpp"

Component create_processes_view(std::vector<Process>& processes, std::mutex& processes_mutex, double& refresh_rate_seconds)
{
    auto selected_index = std::make_shared<int>(0);
    auto hover_index = std::make_shared<int>(-1);
    auto search_mode = std::make_shared<bool>(false);
    auto search_phrase = std::make_shared<std::string>("");
    auto boxes = std::make_shared<std::vector<Box>>();

    auto base_component = Renderer([&, selected_index, hover_index, search_mode, search_phrase, boxes]
    {
        boxes->clear();
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

        if (*selected_index >= static_cast<int>(processes_copy.size())) {
            *selected_index = std::max(0, static_cast<int>(processes_copy.size()) - 1);
        }
        if (*selected_index < 0) {
            *selected_index = 0;
        }

        std::vector<Element> rows;

        std::stringstream refresh_info;
        refresh_info << "Refresh rate: " << std::fixed << std::setprecision(1) << refresh_rate_seconds << "s";
        rows.push_back(text(refresh_info.str()) | color(Color::GreenLight));
        rows.push_back(separator());

        rows.push_back(hbox({
            text("PID") | size(WIDTH, EQUAL, 10),
            separator(),
            text("Process Name") | size(WIDTH, EQUAL, 25),
            separator(),
            text("Memory (KB)") | size(WIDTH, EQUAL, 15),
            separator(),
            text("CPU (%)") | size(WIDTH, EQUAL, 12),
            separator(),
            text("Network (B)") | size(WIDTH, EQUAL, 15),
        }) | bold);

        rows.push_back(separator());

        boxes->resize(processes_copy.size());
        for (size_t i = 0; i < processes_copy.size(); i++)
        {
            const auto& proc = processes_copy[i];
            std::stringstream pid_ss, mem_ss, cpu_ss, net_ss;
            pid_ss << proc.get_pid();
            mem_ss << proc.get_memory_usage();
            cpu_ss << std::fixed << std::setprecision(2) << proc.get_cpu_usage();
            net_ss << proc.get_network_usage();

            auto row = hbox({
                text(pid_ss.str()) | size(WIDTH, EQUAL, 10),
                separator(),
                text(proc.get_process_name()) | size(WIDTH, EQUAL, 25),
                separator(),
                text(mem_ss.str()) | size(WIDTH, EQUAL, 15),
                separator(),
                text(cpu_ss.str()) | size(WIDTH, EQUAL, 12),
                separator(),
                text(net_ss.str()) | size(WIDTH, EQUAL, 15),
            });

            if (static_cast<int>(i) == *selected_index) {
                row = row | bgcolor(Color::Blue) | bold | focus;
            } else if (static_cast<int>(i) == *hover_index) {
                row = row | bgcolor(Color::GrayDark);
            }

            row = row | reflect((*boxes)[i]);
            rows.push_back(row);
        }

        auto process_list = vbox(rows) | vscroll_indicator | yframe | flex;

        if (*search_mode || !search_phrase->empty()) {
            std::string search_display = "/" + *search_phrase;
            if (*search_mode) {
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
    });

    return CatchEvent(base_component, [selected_index, hover_index, search_mode, search_phrase, boxes](Event event) {
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
            return true;
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

        if (event == Event::ArrowUp) {
            (*selected_index)--;
            if (*selected_index < 0) {
                *selected_index = 0;
            }
            return true;
        }
        if (event == Event::ArrowDown) {
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

            *hover_index = -1;
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
    });
}