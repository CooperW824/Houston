#include "process_detail_view.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

Element create_process_detail_view(const Process& process,
                                   const std::vector<float>& cpu_history,
                                   const std::vector<float>& memory_history,
                                   const std::vector<float>& network_history,
                                   int history_size)
{
    unsigned long uptime_seconds = process.get_cpu_time();
    unsigned long days = uptime_seconds / 86400;
    unsigned long hours = (uptime_seconds % 86400) / 3600;
    unsigned long minutes = (uptime_seconds % 3600) / 60;
    unsigned long seconds = uptime_seconds % 60;

    std::stringstream uptime_ss;
    if (days > 0) {
        uptime_ss << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
    } else if (hours > 0) {
        uptime_ss << hours << "h " << minutes << "m " << seconds << "s";
    } else if (minutes > 0) {
        uptime_ss << minutes << "m " << seconds << "s";
    } else {
        uptime_ss << seconds << "s";
    }


    auto cpu_func = [cpu_history, history_size](int width, int height) {
        std::vector<int> output;
        if (width <= 0 || height <= 0) {
            return output;
        }

        output.reserve(width);
        for (int x = 0; x < width; ++x) {
            int history_pos = (x * history_size) / width;
            int data_start = history_size - static_cast<int>(cpu_history.size());
            if (history_pos >= data_start && history_pos < history_size) {
                size_t data_index = history_pos - data_start;
                if (data_index < cpu_history.size()) {
                    if (cpu_history[data_index] < 0.01) {
                        output.push_back(-1);
                    } else {
                        int value = static_cast<int>((cpu_history[data_index] * height) / 100.0);
                        output.push_back(std::min(std::max(0, value), height));
                    }
                } else {
                    output.push_back(-1);
                }
            } else {
                output.push_back(-1);
            }
        }
        return output;
    };

    auto mem_func = [memory_history, history_size](int width, int height) {
        std::vector<int> output;
        if (width <= 0 || height <= 0) {
            return output;
        }

        float max_mem = memory_history.empty() ? 1.0f : *std::max_element(memory_history.begin(), memory_history.end());
        if (max_mem <= 0) max_mem = 1.0f;
        max_mem *= 1.5f;

        output.reserve(width);
        for (int x = 0; x < width; ++x) {
            int history_pos = (x * history_size) / width;
            int data_start = history_size - static_cast<int>(memory_history.size());
            if (history_pos >= data_start && history_pos < history_size) {
                size_t data_index = history_pos - data_start;
                if (data_index < memory_history.size()) {
                    if (memory_history[data_index] < 0.01) {
                        output.push_back(-1);
                    } else {
                        int value = static_cast<int>((memory_history[data_index] * height) / max_mem);
                        output.push_back(std::min(std::max(0, value), height));
                    }
                } else {
                    output.push_back(-1);
                }
            } else {
                output.push_back(-1);
            }
        }
        return output;
    };

    auto net_func = [network_history, history_size](int width, int height) {
        std::vector<int> output;
        if (width <= 0 || height <= 0) {
            return output;
        }

        float max_net = network_history.empty() ? 1.0f : *std::max_element(network_history.begin(), network_history.end());
        if (max_net <= 0) max_net = 1.0f;
        max_net *= 1.5f;

        output.reserve(width);
        for (int x = 0; x < width; ++x) {
            int history_pos = (x * history_size) / width;
            int data_start = history_size - static_cast<int>(network_history.size());
            if (history_pos >= data_start && history_pos < history_size) {
                size_t data_index = history_pos - data_start;
                if (data_index < network_history.size()) {
                    if (network_history[data_index] < 0.01) {
                        output.push_back(-1);
                    } else {
                        int value = static_cast<int>((network_history[data_index] * height) / max_net);
                        output.push_back(std::min(std::max(0, value), height));
                    }
                } else {
                    output.push_back(-1);
                }
            } else {
                output.push_back(-1);
            }
        }
        return output;
    };

    float max_mem = memory_history.empty() ? 0.0f : *std::max_element(memory_history.begin(), memory_history.end());
    float max_net = network_history.empty() ? 0.0f : *std::max_element(network_history.begin(), network_history.end());

    if (max_mem <= 0) max_mem = 100.0f;
    if (max_net <= 0) max_net = 100.0f;
    max_mem *= 1.5f;
    max_net *= 1.5f;

    std::stringstream max_mem_ss, max_net_ss;
    max_mem_ss << std::fixed << std::setprecision(0) << max_mem;
    max_net_ss << std::fixed << std::setprecision(0) << max_net;

    Element cpu_graph = vbox({
        text("CPU (%)") | bold | center,
        hbox({
            vbox({
                text("100") | dim,
                filler(),
                text("50") | dim,
                filler(),
                text("0") | dim,
            }),
            separator(),
            vbox({
                graph(cpu_func) | color(Color::Green) | flex,
                hbox({
                    text("60s") | dim,
                    filler(),
                    text("30s") | dim,
                    filler(),
                    text("now") | dim,
                }) | size(HEIGHT, EQUAL, 1),
            }) | flex,
        }) | flex,
    }) | border | flex;

    Element memory_graph = vbox({
        text("Memory (KB)") | bold | center,
        hbox({
            vbox({
                text(max_mem_ss.str()) | dim,
                filler(),
                text(std::to_string(static_cast<int>(max_mem / 2))) | dim,
                filler(),
                text("0") | dim,
            }),
            separator(),
            vbox({
                graph(mem_func) | color(Color::Blue) | flex,
                hbox({
                    text("60s") | dim,
                    filler(),
                    text("30s") | dim,
                    filler(),
                    text("now") | dim,
                }) | size(HEIGHT, EQUAL, 1),
            }) | flex,
        }) | flex,
    }) | border | flex;

    Element network_graph = vbox({
        text("Network (B)") | bold | center,
        hbox({
            vbox({
                text(max_net_ss.str()) | dim,
                filler(),
                text(std::to_string(static_cast<int>(max_net / 2))) | dim,
                filler(),
                text("0") | dim,
            }),
            separator(),
            vbox({
                graph(net_func) | color(Color::Cyan) | flex,
                hbox({
                    text("60s") | dim,
                    filler(),
                    text("30s") | dim,
                    filler(),
                    text("now") | dim,
                }) | size(HEIGHT, EQUAL, 1),
            }) | flex,
        }) | flex,
    }) | border | flex;

    return vbox({
        hbox({
            vbox({
                text("Process Details") | bold,
                separator(),
                hbox({text("PID: ") | bold, text(std::to_string(process.get_pid()))}),
                hbox({text("Name: ") | bold, text(process.get_process_name())}),
                hbox({text("Uptime: ") | bold, text(uptime_ss.str())}),
                text(""),
                hbox({text("Command: ") | bold, text(process.get_command())}),
            }) | border | size(WIDTH, GREATER_THAN, 40),
            filler(),
        }),
        separator(),
        hbox({
            cpu_graph,
            memory_graph,
            network_graph,
        }) | flex,
        separator(),
        hbox({
            text("ESC: Return") | dim,
            text(" | ") | dim,
            text("Backspace: SIGTERM") | dim,
            text(" | ") | dim,
            text("Delete: SIGKILL") | dim,
        }) | center,
    }) | flex;
}

