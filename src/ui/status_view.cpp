#include "status_view.hpp"

Component create_status_view(const std::vector<std::string> &hardware_resources, const std::vector<Component> &tab_contents)
{
    // 1. State Variables
    // Stores the index of the currently selected menu item (and thus the current tab)
    auto menu_selected = std::make_shared<int>(0);

    // Stores the state of the resizable split (position of the separator)
    auto split_state = std::make_shared<int>(50); // Initial width for the menu pane (e.g., 50 columns)

    // 2. The Menu Component
    auto menu_component =
        Menu(&hardware_resources, menu_selected.get(),
             MenuOption::Vertical()); // Add a border around the menu

    // 3. The Tabs Component
    // The Tab component needs a container of all the content for each tab
    auto tab_container = Container::Tab(tab_contents, menu_selected.get());

    // 4. The Resizable Split Component
    // This splits the available space horizontally between the menu and the tabs.
    auto resizable_split = ResizableSplit(
                               ResizableSplitOption{
                                   .main = menu_component,
                                   .back = tab_container,
                                   .direction = Direction::Left,
                                   .main_size = split_state.get(),
                                   .separator_func = []
                                   { return ::ftxui::separator(); },
                               }) |
                           yflex | yflex_grow;

    // 5. Return the combined component
    return resizable_split;
}