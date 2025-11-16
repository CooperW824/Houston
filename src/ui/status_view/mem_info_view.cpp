#include "mem_info_view.hpp"

Component create_mem_info_view(const double memory_total_mb, const double *memory_used_mb, const double *memory_free_mb, const double *memory_cached_mb, const double memory_swap_total_mb, const double *memory_swap_used_mb, const std::vector<double> *history_memory_used_mb)
{
    auto info = Container::Vertical({Renderer([memory_total_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", memory_total_mb); 
            return text(std::string("Total Memory: ") + s + " MB") | bold; }),
                                     Renderer([memory_used_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", *memory_used_mb); 
            return text(std::string("Used Memory: ") + s + " MB") | bold; }),
                                     Renderer([memory_free_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", *memory_free_mb); 
            return text(std::string("Free Memory: ") + s + " MB") | bold; }),
                                     Renderer([memory_cached_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", *memory_cached_mb); 
            return text(std::string("Cached Memory: ") + s + " MB") | bold; }),
                                     Renderer([memory_swap_total_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", memory_swap_total_mb); 
            return text(std::string("Total Swap: ") + s + " MB") | bold; }),
                                     Renderer([memory_swap_used_mb]
                                              { 
            char s[32]; snprintf(s, sizeof(s), "%.2f", *memory_swap_used_mb); 
            return text(std::string("Used Swap: ") + s + " MB") | bold; })}) |
                flex | flex_grow;

    auto mem_func = [history_memory_used_mb, memory_total_mb](int width, int height)
    {
        int history_size = std::min(60, (int)history_memory_used_mb->size());
        std::vector<int> output;
        if (width <= 0 || height <= 0)
        {
            return output;
        }

        const auto &hist = *history_memory_used_mb;
        int true_size = hist.size();
        int visible_size = std::min(true_size, 60);

        // Last chunk of history
        int offset = true_size - visible_size;
        output.reserve(width);
        for (int x = 0; x < width; ++x)
        {
            int history_pos = (x * visible_size) / width;

            if (history_pos >= offset && history_pos < history_size)
            {
                size_t data_index = history_pos - offset;
                if (data_index < history_memory_used_mb->size())
                {
                    if ((*history_memory_used_mb)[data_index] < 0.01)
                    {
                        output.push_back(-1);
                    }
                    else
                    {
                        double used = (*history_memory_used_mb)[data_index];
                        double pct = (used / memory_total_mb) * 100.0;
                        int value = (pct * height) / 100.0;
                        output.push_back(std::min(std::max(0, value), height));
                    }
                }
                else
                {
                    output.push_back(-1);
                }
            }
            else
            {
                output.push_back(-1);
            }
        }
        return output;
    };

    // History graph component
    auto history_graph = Renderer([mem_func]()
                                  { return vbox({graph(mem_func) | border | flex, text("Memory Usage") | bold | center}); });

    auto final_layout = ResizableSplit(
                            ResizableSplitOption{
                                .main = info,
                                .back = history_graph,
                                .direction = Direction::Right,
                                .main_size = 70,
                                .separator_func = []
                                { return ::ftxui::separator(); }}) |
                        yflex | yflex_grow;

    return final_layout;
}