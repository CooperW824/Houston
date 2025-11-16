#include "status_view.hpp"

Component create_status_view(const std::vector<std::string> &hardware_resources, const std::vector<Component> &tab_contents, std::shared_ptr<int> split_state)
{
    // 1. State Variables
    // Stores the index of the currently selected menu item (and thus the current tab)
    auto menu_selected = std::make_shared<int>(0);

    // 2. The Menu Component
    auto menu_component =
        Menu(&hardware_resources, menu_selected.get(),
             MenuOption::Vertical()); // Add a border around the menu

    // 3. The Tabs Component
    // The Tab component needs a container of all the content for each tab
    auto tab_container = Container::Tab(tab_contents, menu_selected.get());

    auto tab_pane = Renderer(tab_container, [tab_container]
                             { return tab_container->Render(); });

    // 4. The Resizable Split Component
    // This splits the available space horizontally between the menu and the tabs.
    auto resizable_split = ResizableSplit(
                               ResizableSplitOption{
                                   .main = menu_component,
                                   .back = tab_pane,
                                   .direction = Direction::Left,
                                   .main_size = split_state.get(),
                                   .separator_func = []
                                   { return ::ftxui::separator(); },
                               }) |
                           yflex | yflex_grow;

    // 5. Return the combined component
    return resizable_split;
}