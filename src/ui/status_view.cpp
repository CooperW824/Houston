#include "status_view.hpp"

Component create_status_view()
{
    auto status_monitor = std::make_shared<StatusMonitor>();

    auto menu_tabs = status_monitor->determine_hardware_resources();

    int selected_tab = 0;

    auto menu = Menu(&menu_tabs, &selected_tab);

    auto tabs = std::vector<Component>{};
    for (const auto &tab_name : menu_tabs)
    {
        tabs.push_back(Renderer([status_monitor, tab_name]
                                { return text(tab_name); }));
    }

    auto tab_content = Container::Tab(tabs, &selected_tab);

    int left_size = 20;
    Component system_monitor_layout =
        ResizableSplitLeft(menu, tab_content, &left_size);

    return system_monitor_layout;
}