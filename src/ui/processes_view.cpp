#include "processes_view.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>

Component create_processes_view(std::vector<Process>& processes, std::mutex& processes_mutex, double& refresh_rate_seconds)
{
    auto selected_index = std::make_shared<int>(0);

    auto base_component = Renderer([&, selected_index]
    {
        std::vector<Process> processes_copy;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            processes_copy = processes;
        }

        std::sort(processes_copy.begin(), processes_copy.end(), [](const Process& a, const Process& b) {
            return a.get_memory_usage() > b.get_memory_usage();
        });

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
            }

            rows.push_back(row);
        }

        return vbox(rows) | vscroll_indicator | yframe | flex;
    });

    return CatchEvent(base_component, [selected_index](Event event) {
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
            if (event.mouse().button == Mouse::WheelUp) {
                (*selected_index)--;
                if (*selected_index < 0) {
                    *selected_index = 0;
                }
                return true;
            }
            if (event.mouse().button == Mouse::WheelDown) {
                (*selected_index)++;
                return true;
            }
        }
        return false;
    });
}
