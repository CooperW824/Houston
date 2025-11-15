#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

int main()
{
    std::vector<std::string> function_tabs = {"System Status", "Running Processes"};
    int selected_function = 0;
    auto function_select = Toggle(&function_tabs, &selected_function);

    auto tab_container = Container::Tab(
        {Renderer([]
                  { return text("System Status"); }),
         Renderer([]
                  { return text("Running Processes"); })},
        &selected_function);

    auto main_view = Container::Vertical({
        function_select,
        tab_container,
    });

    auto renderer = Renderer(main_view, [&]
                             { return vbox({
                                          function_select->Render(),
                                          separator(),
                                          tab_container->Render(),
                                      }) |
                                      border; });

    ScreenInteractive::Fullscreen().Loop(renderer);
}