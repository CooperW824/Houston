#include "src/status_monitor/status_monitor.hpp"
#include <iostream>
#include <ifaddrs.h>
#include <net/if.h>

int main()
{

#if 1
    StatusMonitor status_monitor;
    status_monitor.determine_hardware_resources();
    auto resources = status_monitor.get_hardware_resources();
    for (const auto &res : resources)
    {
        std::cout << res << std::endl;
    }
    return 0;
#endif

    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return 1;
    }

    std::cout << "Network adapters:\n";
    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        std::cout << "- " << ifa->ifa_name;
        if (ifa->ifa_flags & IFF_LOOPBACK)
            std::cout << " (loopback)";
        std::cout << "\n";
    }

    freeifaddrs(ifaddr);
    return 0;
}