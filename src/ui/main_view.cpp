#include "main_view.hpp"
#include "processes_view.hpp"
#include <chrono>

void start_ui(double refresh_rate_seconds)
{
    std::vector<std::string> function_tabs = {"System Status", "Running Processes"};
    int selected_function = 0;
    auto function_select = Toggle(&function_tabs, &selected_function);

    auto processes = get_processes_list();
    std::mutex processes_mutex;

    auto processes_renderer = create_processes_view(processes, processes_mutex, refresh_rate_seconds);

    auto tab_container = Container::Tab(
        {Renderer([]
                  { return text("System Status"); }),
         processes_renderer},
        &selected_function);

    auto main_container_base = Container::Vertical({
        function_select,
        tab_container,
    });

    auto main_container = CatchEvent(main_container_base, [&](Event event) {
        if (selected_function == 1) {
            if (event == Event::ArrowUp || event == Event::ArrowDown ||
                event == Event::PageUp || event == Event::PageDown) {
                return processes_renderer->OnEvent(event);
            }
        }
        return false;
    });

    auto main_view = Renderer(main_container, [&]
                             { return vbox({
                                          function_select->Render(),
                                          separator(),
                                          tab_container->Render(),
                                      }) |
                                      border; });

    auto screen = ScreenInteractive::Fullscreen();

    std::thread refresh_thread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(refresh_rate_seconds * 1000)));
            auto new_processes = get_processes_list();
            {
                std::lock_guard<std::mutex> lock(processes_mutex);
                processes = new_processes;
            }
            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(main_view);
    refresh_thread.detach();
}

