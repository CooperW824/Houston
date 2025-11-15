#include "status_monitor.hpp"
#include <fstream>
#include <string>
#include <sys/sysinfo.h>
#include <pci/pci.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <iostream>
#include <ifaddrs.h>
#include <net/if.h>

StatusMonitor::StatusMonitor()
{
}

StatusMonitor::~StatusMonitor()
{
    for (auto copy : this->network_adapters)
    {
        if (copy->ifa_name)
            free(copy->ifa_name);
        if (copy->ifa_addr)
            free(copy->ifa_addr);
        if (copy->ifa_netmask)
            free(copy->ifa_netmask);
        delete copy;
    }

    network_adapters.clear();
}

std::string get_cpu_model()
{
    std::ifstream f("/proc/cpuinfo");
    std::string line;
    while (std::getline(f, line))
    {
        if (line.rfind("model name", 0) == 0)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos)
                return line.substr(pos + 2);
        }
    }
    return "Unknown CPU";
}

std::string read_file(const std::string &path)
{
    std::ifstream f(path);
    if (!f)
        return "";
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

std::string get_gpu_model()
{
    std::string vendor_name;
    std::string device_name;
    namespace fs = std::filesystem;

    for (auto &entry : fs::directory_iterator("/sys/bus/pci/devices"))
    {
        std::string class_code = read_file(entry.path().string() + "/class");
        if (class_code.size() >= 4)
        {
            // class code is "0x030000" for VGA, "0x030200" for 3D
            if (class_code.rfind("0x0300", 0) == 0 || class_code.rfind("0x0302", 0) == 0)
            {
                vendor_name = read_file(entry.path().string() + "/vendor_name");
                device_name = read_file(entry.path().string() + "/device_name");
            }
        }
    }

    return vendor_name + " " + device_name;
}

std::vector<ifaddrs *> get_network_adapters()
{
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        _exit(254);
    }

    std::vector<ifaddrs *> copies;

    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        // allocate new ifaddrs
        ifaddrs *copy = new ifaddrs;
        std::memset(copy, 0, sizeof(ifaddrs));

        // copy primitive fields
        copy->ifa_flags = ifa->ifa_flags;
        copy->ifa_addr = nullptr; // we will copy addr separately
        copy->ifa_netmask = nullptr;
        copy->ifa_ifu = ifa->ifa_ifu; // union, optional shallow copy

        // copy name string
        copy->ifa_name = strdup(ifa->ifa_name);

        // copy sockaddr if present
        if (ifa->ifa_addr)
        {
            socklen_t sa_len = sizeof(sockaddr);
            if (ifa->ifa_addr->sa_family == AF_INET)
                sa_len = sizeof(sockaddr_in);
            else if (ifa->ifa_addr->sa_family == AF_INET6)
                sa_len = sizeof(sockaddr_in6);
            copy->ifa_addr = (sockaddr *)malloc(sa_len);
            std::memcpy(copy->ifa_addr, ifa->ifa_addr, sa_len);
        }

        if (ifa->ifa_netmask)
        {
            socklen_t sa_len = sizeof(sockaddr);
            if (ifa->ifa_netmask->sa_family == AF_INET)
                sa_len = sizeof(sockaddr_in);
            else if (ifa->ifa_netmask->sa_family == AF_INET6)
                sa_len = sizeof(sockaddr_in6);
            copy->ifa_netmask = (sockaddr *)malloc(sa_len);
            std::memcpy(copy->ifa_netmask, ifa->ifa_netmask, sa_len);
        }

        copies.push_back(copy);
    }

    freeifaddrs(ifaddr);
    return copies;
}

void StatusMonitor::determine_hardware_resources()
{
    this->hardware_resources.push_back("CPU: " + get_cpu_model());

    struct sysinfo memory_info;

    if (sysinfo(&memory_info))
    {
        _exit(255);
    }

    this->hardware_resources.push_back("RAM: " + std::to_string(memory_info.totalram - memory_info.freeram) + " / " + std::to_string(memory_info.totalram));
    this->hardware_resources.push_back("GPU: " + get_gpu_model());

    this->network_adapters = get_network_adapters();
    for (int i = 0; i < network_adapters.size(); i++)
    {
        this->hardware_resources.push_back("Wireless" + std::to_string(i) + ": " + network_adapters[i]->ifa_name);
    }

    std::string path = "/sys/block";
    int storage_device_number = 0;

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        std::string dev = entry.path().filename();
        this->hardware_resources.push_back("Drive " + std::to_string(storage_device_number) + ": " + dev);
        storage_device_number++;
    }
}
