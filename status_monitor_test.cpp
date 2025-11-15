#include "src/status_monitor/status_monitor.hpp"
#include <iostream>


int main()
{
    StatusMonitor status_monitor;
    status_monitor.determine_hardware_resources();
    auto resources = status_monitor.get_hardware_resources();
    for (const auto &res : resources)
    {
        std::cout << res << std::endl;
    }
    return 0;
}