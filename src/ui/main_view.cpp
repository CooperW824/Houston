#include "main_view.hpp"
#include "process_view/processes_view.hpp"
#include "status_view/status_view.hpp"
#include "status_view/cpu_info_view.hpp"
#include "status_view/mem_info_view.hpp"
#include <chrono>
#include <atomic>
#include <condition_variable>

void start_ui(double refresh_rate_seconds)
{
    std::vector<std::string> function_tabs = {"System Status", "Running Processes", "Machine Optimize"};
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
        status_monitor->get_overall_cpu_utilization(),
        status_monitor->get_cpu_logical_core_count(),
        status_monitor->get_cpu_process_count(),
        status_monitor->get_cpu_thread_count(),
        status_monitor->get_cpu_model_public()));

    status_tab_contents.push_back(create_mem_info_view(
        status_monitor->get_memory_total_mb(),
        status_monitor->get_memory_used_mb(),
        status_monitor->get_memory_free_mb(),
        status_monitor->get_memory_cached_mb(),
        status_monitor->get_memory_swap_total_mb(),
        status_monitor->get_memory_swap_used_mb(),
        status_monitor->get_history_memory_used_mb()));

    for (int i = 2; i < hardware_resources->size(); ++i)
    {
        status_tab_contents.push_back(Renderer([i, hardware_resources]
                                               { return text(hardware_resources->at(i)) | bold; }) |
                                      border);
    }

    auto status_renderer = create_status_view(*hardware_resources, status_tab_contents);

    // Machine Optimize tab
    auto optimize_clicked = std::make_shared<bool>(false);
    auto optimize_button = Button("Optimize resource allocation with artificial intelligence", [optimize_clicked]
                                  { *optimize_clicked = true; });

    auto optimize_renderer = Renderer(optimize_button, [optimize_button, optimize_clicked]
                                      {
        auto elements = vbox({
            text("") | center,
            optimize_button->Render() | center,
            text("") | center,
        });
        
        if (*optimize_clicked)
        {
            elements = vbox({
                text("") | center,
                optimize_button->Render() | center,
                text("") | center,
                text("Program Optimizing") | center | bold,
            });
        }
        
        return elements; });

    auto tab_container = Container::Tab(
        {status_renderer,
         processes_renderer,
         optimize_renderer},
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
    std::mutex refresh_mutex;
    std::condition_variable refresh_cv;

    std::thread refresh_thread([&]()
                               {
        while (!should_exit)
        {
            std::unique_lock<std::mutex> lock(refresh_mutex);
            if (refresh_cv.wait_for(lock, std::chrono::milliseconds(static_cast<int>(refresh_rate_seconds * 1000)),
                                   [&]() { return should_exit.load(); }))
            {
                break; // Exit if should_exit was set
            }
            
            if (should_exit) break;
            
            // Update status monitor
            status_monitor->update();
            
            // Update processes with timeout protection
            auto new_processes = get_processes_list();
            {
                std::unique_lock<std::mutex> proc_lock(processes_mutex, std::try_to_lock);
                if (proc_lock.owns_lock() && !should_exit)
                {
                    processes = new_processes;
                }
            }
            
            if (!should_exit)
            {
                screen.PostEvent(Event::Custom);
            }
        } });

    screen.Loop(main_view);

    // cleanup sequence
    should_exit = true;
    refresh_cv.notify_all(); // Wake up refresh thread immediately

    if (refresh_thread.joinable())
    {
        refresh_thread.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}