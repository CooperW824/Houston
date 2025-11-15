#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/dom/elements.hpp"
#include "processes_list/processes_list.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace ftxui;

int main()
{
    std::vector<std::string> function_tabs = {"System Status", "Running Processes"};
    int selected_function = 0;
    auto function_select = Toggle(&function_tabs, &selected_function);

    int refresh_rate_seconds = 0.5;
    auto processes = get_processes_list();

    auto processes_renderer = Renderer([&]
    {
        std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.get_memory_usage() > b.get_memory_usage();
        });

        std::vector<Element> rows;

        std::stringstream refresh_info;
        refresh_info << "Refresh rate: " << refresh_rate_seconds << "s";
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

        for (const auto& proc : processes)
        {
            std::stringstream pid_ss, mem_ss, cpu_ss, net_ss;
            pid_ss << proc.get_pid();
            mem_ss << proc.get_memory_usage();
            cpu_ss << std::fixed << std::setprecision(2) << proc.get_cpu_usage();
            net_ss << proc.get_network_usage();

            rows.push_back(hbox({
                text(pid_ss.str()) | size(WIDTH, EQUAL, 10),
                separator(),
                text(proc.get_process_name()) | size(WIDTH, EQUAL, 25),
                separator(),
                text(mem_ss.str()) | size(WIDTH, EQUAL, 15),
                separator(),
                text(cpu_ss.str()) | size(WIDTH, EQUAL, 12),
                separator(),
                text(net_ss.str()) | size(WIDTH, EQUAL, 15),
            }));
        }

        return vbox(rows) | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 40);
    });

    auto tab_container = Container::Tab(
        {Renderer([]
                  { return text("System Status"); }),
         processes_renderer},
        &selected_function);

    auto main_view = Container::Vertical({
        function_select,
        tab_container,
    });

    auto renderer = Renderer(main_view, [&]
                             { return vbox({
                                          function_select->Render(),
                                          separator(),
                                          tab_container->Render(),
                                      }) |
                                      border; });

    auto screen = ScreenInteractive::Fullscreen();

    std::thread refresh_thread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(refresh_rate_seconds));
            processes = get_processes_list();
            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(renderer);
    refresh_thread.detach();
}