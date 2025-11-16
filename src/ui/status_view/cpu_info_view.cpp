#include "cpu_info_view.hpp"

Component create_cpu_utilization_bars(const std::vector<double> &core_utilizations)
{
    std::vector<Component> bars;
    for (size_t i = 0; i < core_utilizations.size(); ++i)
    {
        double utilization = core_utilizations[i];

        auto bar = Container::Vertical({Renderer([utilization, i]
                                                 { return text("Core " + std::to_string(i) + ": " + std::to_string(static_cast<int>(utilization)) + "%") | bold; }),
                                        Renderer([utilization]
                                                 { return gauge(utilization / 100.0) | flex | flex_grow | flex_shrink; })}) |
                   border;
        bars.push_back(std::move(bar));
    }

    return Container::Vertical(bars) | flex | flex_grow;
}

Component create_cpu_info_view(
    const std::vector<double> &core_utilizations,
    const double *max_clock_speed_mhz,
    const int logical_core_count,
    const int *process_count,
    const int *thread_count,
    const std::string &cpu_model)
{
    auto bars = create_cpu_utilization_bars(core_utilizations);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", *max_clock_speed_mhz);

    auto info = Container::Vertical({
                    Renderer([cpu_model]
                             { return text("CPU Model: " + cpu_model) | bold; }),
                    Renderer([buffer]
                             { return text("Max Clock Speed: " + std::string(buffer) + " MHz") | bold; }),
                    Renderer([logical_core_count]
                             { return text("Logical Cores: " + std::to_string(logical_core_count)) | bold; }),
                    Renderer([process_count]
                             { return text("Process Count: " + std::to_string(*process_count)) | bold; }),
                    Renderer([thread_count]
                             { return text("Thread Count: " + std::to_string(*thread_count)) | bold; }),
                }) |
                flex | flex_grow;

    auto final_layout = ResizableSplit(
                            ResizableSplitOption{
                                .main = bars,
                                .back = info,
                                .direction = Direction::Left,
                                .main_size = 70,
                                .separator_func = []
                                { return ::ftxui::separator(); },
                            }) |
                        yflex | yflex_grow;

    return final_layout;
}