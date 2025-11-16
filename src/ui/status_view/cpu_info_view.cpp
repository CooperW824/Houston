#include "cpu_info_view.hpp"

Component create_cpu_info_view(
    const std::vector<double *> &core_utilizations,
    const double *max_clock_speed_mhz,
    const double *overall_utilization,
    const int logical_core_count,
    const int *process_count,
    const int *thread_count,
    const std::string &cpu_model)
{
    // Plain bars component (no yframe / vscroll here)
    std::vector<Component> per_core_components;
    for (int i = 0; i < core_utilizations.size(); i++)
    {
        // Capture a *local copy* of the double pointer (util_ptr) and the core index (i).
        // The pointer itself won't change, but the value it points to will.
        double *util_ptr = core_utilizations[i];
        per_core_components.push_back(Renderer([i, util_ptr]
                                               {
                                                     // Dereference the pointer inside the lambda.
                                                     double current_utilization = *util_ptr;
                                                     char buffer[32];
                                                     snprintf(buffer, sizeof(buffer), "CPU%d: %.2f%%", i, current_utilization);
                                                     return text(std::string(buffer)); }));
    }

    auto core_outputs = Container::Vertical(per_core_components);

    // ... info component unchanged (keep it flexible)
    auto info = Container::Vertical({Renderer([cpu_model]
                                              { return text("CPU Model: " + cpu_model) | bold; }),
                                     Renderer([max_clock_speed_mhz]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", *max_clock_speed_mhz); 
            return text(std::string("Max Clock Speed: ") + s + " MHz") | bold; }),
                                     Renderer([overall_utilization]
                                              {
            char s[32]; snprintf(s, sizeof(s), "%.2f", *overall_utilization); 
            return text(std::string("Processor Utilization: ") + s + " %") | bold; }),
                                     Renderer([logical_core_count]
                                              { return text("Logical Cores: " + std::to_string(logical_core_count)) | bold; }),
                                     Renderer([process_count]
                                              { return text("Process Count: " + std::to_string(*process_count)) | bold; }),
                                     Renderer([thread_count]
                                              { return text("Thread Count: Not Available") | bold; })}) |
                flex | flex_grow;

    auto final_layout = ResizableSplit(
                            ResizableSplitOption{
                                .main = core_outputs,
                                .back = info,
                                .direction = Direction::Right,
                                .main_size = 70,
                                .separator_func = []
                                { return ::ftxui::separator(); }}) |
                        yflex | yflex_grow;

    return final_layout;
}
