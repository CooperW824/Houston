#ifndef __STATUS_VIEW_HPP
#define __STATUS_VIEW_HPP

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "../../status_monitor/status_monitor.hpp"
#include <memory>

using namespace ftxui;

Component create_status_view(const std::vector<std::string> &hardware_resources, const std::vector<Component> &tab_contents, std::shared_ptr<int> split_state);

#endif /* __STATUS_VIEW_HPP */
