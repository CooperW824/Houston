#include "machine_optimizer_view.hpp"
#include "../../smart_sparker/machine_opt/machine_optimizer.hpp"
#include "ftxui/component/component.hpp"
#include <chrono>
#include <future>
#include <sys/types.h>
#include <signal.h>
#include <utility>
#include <memory>

using namespace ftxui;

Component create_machine_optimizer_view(
    std::vector<Process>& processes,
    std::mutex& processes_mutex
) {
    auto machine_optimizer = std::make_shared<MachineOptimizer>();
    auto optimize_future = std::make_shared<std::future<std::pair<std::string, pid_t>>>();
    auto optimize_running = std::make_shared<bool>(false);
    auto optimize_result = std::make_shared<std::string>("");
    auto target_pid = std::make_shared<pid_t>(-1);
    auto process_killed = std::make_shared<bool>(false);
    auto kill_success = std::make_shared<bool>(false);

    auto optimize_button = Button("Optimize resource allocation with artificial intelligence",
                                  [machine_optimizer, optimize_future, optimize_running, optimize_result, target_pid, process_killed, &processes, &processes_mutex]
                                  {
                                      if (!*optimize_running)
                                      {
                                          *optimize_running = true;
                                          *optimize_result = "";
                                          *target_pid = -1;
                                          *process_killed = false;

                                          // Get a snapshot of current processes
                                          std::vector<Process> processes_snapshot;
                                          {
                                              std::lock_guard<std::mutex> lock(processes_mutex);
                                              processes_snapshot = processes;
                                          }

                                          *optimize_future = machine_optimizer->run_async(processes_snapshot);
                                      }
                                  });

    auto kill_button = Button("Kill Process",
                              [target_pid, process_killed, kill_success, &processes, &processes_mutex]
                              {
                                  if (*target_pid > 0 && !*process_killed)
                                  {
                                      std::lock_guard<std::mutex> lock(processes_mutex);
                                      for (auto& proc : processes)
                                      {
                                          if (proc.get_pid() == *target_pid)
                                          {
                                              *kill_success = proc.kill(SIGTERM);
                                              *process_killed = true;
                                              break;
                                          }
                                      }
                                  }
                              });

    auto optimize_container = Container::Vertical({optimize_button, kill_button});

    auto optimize_renderer = Renderer(optimize_container, [optimize_button, kill_button, optimize_running, optimize_result, optimize_future, target_pid, process_killed, kill_success]
                                      {
        // Check if the async operation completed
        if (*optimize_running && optimize_future->valid() &&
            optimize_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            auto result = optimize_future->get();
            *optimize_result = result.first;
            *target_pid = result.second;
            *optimize_running = false;
        }

        auto elements = vbox({
            text("") | center,
            optimize_button->Render() | center,
            text("") | center,
        });

        if (*optimize_running)
        {
            elements = vbox({
                text("") | center,
                optimize_button->Render() | center,
                text("") | center,
                text("AI Optimization Running...") | center | bold | color(Color::Yellow),
            });
        }
        else if (!optimize_result->empty())
        {
            if (*process_killed)
            {
                elements = vbox({
                    text("") | center,
                    optimize_button->Render() | center,
                    text("") | center,
                    text("Optimization Complete!") | center | bold | color(Color::Green),
                    text("AI Recommended Process to Kill: " + *optimize_result) | center,
                    text("") | center,
                    text(*kill_success ? "✓ Process killed successfully" : "✗ Failed to kill process") |
                        center | bold | color(*kill_success ? Color::Green : Color::Red),
                });
            }
            else
            {
                elements = vbox({
                    text("") | center,
                    optimize_button->Render() | center,
                    text("") | center,
                    text("Optimization Complete!") | center | bold | color(Color::Green),
                    text("AI Recommended Process to Kill: " + *optimize_result) | center,
                    text("") | center,
                    kill_button->Render() | center,
                });
            }
        }

        return elements;
    });

    return optimize_renderer;
}

