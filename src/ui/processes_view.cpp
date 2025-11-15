#include "processes_view.hpp"
#include "processes_view_inputs.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include "ftxui/screen/box.hpp"

Component create_processes_view(std::vector<Process>& processes, std::mutex& processes_mutex, double& refresh_rate_seconds)
{
    const int COL_SIGTERM_WIDTH = 1;
    const int COL_SIGKILL_WIDTH = 2;
    const int COL_KILL_WIDTH = COL_SIGTERM_WIDTH + 1 + COL_SIGKILL_WIDTH;
    const int COL_PID_WIDTH = 10;
    const int COL_NAME_WIDTH = 25;
    const int COL_MEMORY_WIDTH = 15;
    const int COL_CPU_WIDTH = 12;
    const int COL_NETWORK_WIDTH = 15;

    auto selected_index = std::make_shared<int>(0);
    auto hover_index = std::make_shared<int>(-1);
    auto hover_sigterm = std::make_shared<int>(-1);
    auto hover_sigkill = std::make_shared<int>(-1);
    auto search_mode = std::make_shared<bool>(false);
    auto search_phrase = std::make_shared<std::string>("");
    auto boxes = std::make_shared<std::vector<Box>>();
    auto sigterm_boxes = std::make_shared<std::vector<Box>>();
    auto sigkill_boxes = std::make_shared<std::vector<Box>>();

    auto base_component = Renderer([&, selected_index, hover_index, hover_sigterm, hover_sigkill, search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
                                     COL_SIGTERM_WIDTH, COL_SIGKILL_WIDTH, COL_KILL_WIDTH, COL_PID_WIDTH, COL_NAME_WIDTH, COL_MEMORY_WIDTH, COL_CPU_WIDTH, COL_NETWORK_WIDTH]
    {
        boxes->clear();
        sigterm_boxes->clear();
        sigkill_boxes->clear();
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

        rows.push_back(hbox({
            text("Kill") | size(WIDTH, EQUAL, COL_KILL_WIDTH),
            separator(),
            text("PID") | size(WIDTH, EQUAL, COL_PID_WIDTH),
            separator(),
            text("Process Name") | size(WIDTH, EQUAL, COL_NAME_WIDTH),
            separator(),
            text("Memory (KB)") | size(WIDTH, EQUAL, COL_MEMORY_WIDTH),
            separator(),
            text("CPU (%)") | size(WIDTH, EQUAL, COL_CPU_WIDTH),
            separator(),
            text("Network (B)") | size(WIDTH, EQUAL, COL_NETWORK_WIDTH),
        }) | bold);

        rows.push_back(separator());

        boxes->resize(processes_copy.size());
        sigterm_boxes->resize(processes_copy.size());
        sigkill_boxes->resize(processes_copy.size());
        for (size_t i = 0; i < processes_copy.size(); i++)
        {
            const auto& proc = processes_copy[i];
            std::stringstream pid_ss, mem_ss, cpu_ss, net_ss;
            pid_ss << proc.get_pid();
            mem_ss << proc.get_memory_usage();
            cpu_ss << std::fixed << std::setprecision(2) << proc.get_cpu_usage();
            net_ss << proc.get_network_usage();

            Element sigterm_btn = text("x") | color(Color::Red);
            if (static_cast<int>(i) == *hover_sigterm) {
                sigterm_btn = sigterm_btn | bgcolor(Color::White) | bold;
            }
            sigterm_btn = sigterm_btn | size(WIDTH, EQUAL, COL_SIGTERM_WIDTH) | reflect((*sigterm_boxes)[i]);

            Element sigkill_btn = text("☠ ") | color(Color::RedLight);
            if (static_cast<int>(i) == *hover_sigkill) {
                sigkill_btn = sigkill_btn | bgcolor(Color::White) | bold;
            }
            sigkill_btn = sigkill_btn | size(WIDTH, EQUAL, COL_SIGKILL_WIDTH) | reflect((*sigkill_boxes)[i]);

            auto row = hbox({
                sigterm_btn,
                text(" "),
                sigkill_btn,
                separator(),
                text(pid_ss.str()) | size(WIDTH, EQUAL, COL_PID_WIDTH),
                separator(),
                text(proc.get_process_name()) | size(WIDTH, EQUAL, COL_NAME_WIDTH),
                separator(),
                text(mem_ss.str()) | size(WIDTH, EQUAL, COL_MEMORY_WIDTH),
                separator(),
                text(cpu_ss.str()) | size(WIDTH, EQUAL, COL_CPU_WIDTH),
                separator(),
                text(net_ss.str()) | size(WIDTH, EQUAL, COL_NETWORK_WIDTH),
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

    return CatchEvent(base_component, [&, selected_index, hover_index, hover_sigterm, hover_sigkill, search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes](Event event) {
        return handle_processes_view_event(
            event,
            selected_index,
            hover_index,
            hover_sigterm,
            hover_sigkill,
            search_mode,
            search_phrase,
            boxes,
            sigterm_boxes,
            sigkill_boxes,
            processes,
            processes_mutex
        );
    });
}