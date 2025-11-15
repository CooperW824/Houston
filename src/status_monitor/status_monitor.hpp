#ifndef __STATUS_MONITOR_HPP
#define __STATUS_MONITOR_HPP

#include "ftxui/component/component.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

class StatusMonitor : public ComponentBase
{
private:
    std::vector<std::string> hardware_resources;
    void determine_hardware_resources();
public:
    StatusMonitor(/* args */);
    ~StatusMonitor();
};


#endif /* __STATUS_MONITOR_HPP */
