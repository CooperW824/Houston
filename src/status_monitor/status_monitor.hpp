#ifndef __STATUS_MONITOR_HPP
#define __STATUS_MONITOR_HPP
#include <ifaddrs.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <map>

// A map to store the device names associated with a vendor
using DeviceMap = std::map<std::string, std::string>;
// The main map: Vendor ID -> (Device ID -> Device Name)
using PciIdDatabase = std::map<std::string, DeviceMap>;

class StatusMonitor
{
private:
    std::vector<std::string> hardware_resources;
    std::vector<ifaddrs *> network_adapters;

    PciIdDatabase pci_id_database;
    bool pci_database_loaded = false;

    void load_pci_id_database(std::string pci_id_filepath);

    std::string lookup_pci_names(const std::string &vendor_id, const std::string &device_id);

    std::string get_cpu_model();
    std::string get_gpu_model();
    std::vector<ifaddrs *> get_network_adapters();
    std::string read_file(const std::string &path);

public:
    StatusMonitor(/* args */);
    ~StatusMonitor();
    std::vector<std::string> determine_hardware_resources();
    std::vector<std::string> get_hardware_resources()
    {
        return this->hardware_resources;
    }
};

#endif /* __STATUS_MONITOR_HPP */
