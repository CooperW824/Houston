#pragma once

#include "ftxui/component/component_base.hpp"
#include "../../processes_list/process.hpp"
#include <vector>
#include <mutex>

namespace ftxui {
    class ComponentBase;
}

using ftxui::Component;

Component create_machine_optimizer_view(
    std::vector<Process>& processes,
    std::mutex& processes_mutex
);

