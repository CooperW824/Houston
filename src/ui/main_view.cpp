#include "main_view.hpp"
#include "process_view/processes_view.hpp"
#include "status_view/status_view.hpp"
#include "status_view/cpu_info_view.hpp"
#include <chrono>
#include <atomic>

void start_ui(double refresh_rate_seconds)
{
    std::vector<std::string> function_tabs = {"System Status", "Running Processes"};
    int selected_function = 0;
    auto function_select = Toggle(&function_tabs, &selected_function);

    auto processes = get_processes_list();
    std::mutex processes_mutex;

    auto processes_renderer = create_processes_view(processes, processes_mutex, refresh_rate_seconds);

    std::shared_ptr<StatusMonitor> status_monitor = std::make_shared<StatusMonitor>();
    std::vector<std::string> *hardware_resources = status_monitor->get_hardware_resources();
    std::vector<Component> status_tab_contents;

    status_tab_contents.push_back(create_cpu_info_view(
        status_monitor->get_logical_core_utilizations(),
        status_monitor->get_cpu_max_clock_speed_mhz(),
        status_monitor->get_cpu_logical_core_count(),
        status_monitor->get_cpu_process_count(),
        status_monitor->get_cpu_thread_count(),
        status_monitor->get_cpu_model_public()));

    for (int i = 1; i < hardware_resources->size(); ++i)
    {
        status_tab_contents.push_back(Renderer([i, hardware_resources]
                                               { return text(hardware_resources->at(i)) | bold; }) |
                                      border);
    }

    auto status_renderer = create_status_view(*hardware_resources, status_tab_contents);

    auto tab_container = Container::Tab(
        {status_renderer,
         processes_renderer},
        &selected_function);

    auto main_container_base = Container::Vertical({
        function_select,
        tab_container,
    });

    auto main_container = CatchEvent(main_container_base, [&](Event event)
                                     {
        if (selected_function == 1)
        {
            if (event == Event::ArrowUp || event == Event::ArrowDown ||
                event == Event::PageUp || event == Event::PageDown ||
                event == Event::Character('/') || event == Event::Escape ||
                event == Event::Backspace || event == Event::Delete ||
                event == Event::Return || event.is_character() || event.is_mouse())
            {
                return processes_renderer->OnEvent(event);
            }
        }
        return false; });

    auto main_view = Renderer(main_container, [&]
                              { return vbox({
                                           function_select->Render(),
                                           separator(),
                                           tab_container->Render(),
                                       }) |
                                       border; });

    auto screen = ScreenInteractive::Fullscreen();

    std::atomic<bool> should_exit(false);
    std::thread refresh_thread([&]()
                               {
        while (!should_exit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(refresh_rate_seconds * 1000)));
            if (!should_exit)
            {
                auto new_processes = get_processes_list();
                {
                    std::lock_guard<std::mutex> lock(processes_mutex);
                    processes = new_processes;
                }
                screen.PostEvent(Event::Custom);
            }
        } });

    screen.Loop(main_view);

    should_exit = true;
    if (refresh_thread.joinable())
    {
        refresh_thread.join();
    }
}
