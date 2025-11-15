#include "status_monitor.hpp"
#include <fstream>
#include <string>
#include <sys/sysinfo.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <iostream>
#include <ifaddrs.h>
#include <net/if.h>
#include <unordered_set>

// Trims leading and trailing whitespace from a string
std::string trim(const std::string &str)
{
    const std::string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
    {
        return ""; // Only whitespace or empty
    }
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

void StatusMonitor::load_pci_id_database(std::string pci_id_filepath)
{
    std::ifstream file(pci_id_filepath);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open pci.ids file at " << pci_id_filepath << std::endl;
        return;
    }

    std::string line;
    std::string current_vendor_id;

    while (std::getline(file, line))
    {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == 'C')
        {
            continue;
        }

        // --- Vendor Entry (Starts at column 0) ---
        if (line[0] != '\t')
        {
            // Find the first space to separate ID from Name
            size_t space_pos = line.find(' ');
            if (space_pos != std::string::npos && space_pos >= 4)
            {
                // Extract 4-character hex ID
                current_vendor_id = trim(line.substr(0, 4));
                std::string vendor_name = trim(line.substr(space_pos + 1));

                // Insert the new vendor into the map (DeviceMap is initially empty)
                this->pci_id_database[current_vendor_id] = DeviceMap();
                // Store the vendor name itself under a special key (or just rely on the map structure)
                // For simplicity here, we rely on the key being the ID.
            }
        }
        // --- Device Entry (Indented by a single TAB) ---
        else if (line[0] == '\t' && current_vendor_id.empty() == false)
        {
            // Check if there is a second character to skip
            if (line.size() > 1 && line[1] != '\t')
            {
                // Remove the leading tab (\t)
                std::string device_line = line.substr(1);

                size_t space_pos = device_line.find(' ');
                if (space_pos != std::string::npos && space_pos >= 4)
                {
                    // Extract 4-character hex ID
                    std::string device_id = trim(device_line.substr(0, 4));
                    std::string device_name = trim(device_line.substr(space_pos + 1));

                    // Insert into the current vendor's DeviceMap
                    this->pci_id_database[current_vendor_id][device_id] = device_name;
                }
            }
        }
    }
}

std::string StatusMonitor::lookup_pci_names(const std::string &vendor_id, const std::string &device_id)
{
    if (this->pci_database_loaded == false)
    {
        // Load the database from a standard location
        this->load_pci_id_database(this->pci_file_location);
        this->pci_database_loaded = true;
    }

    auto vendor_it = this->pci_id_database.find(vendor_id);
    if (vendor_it != this->pci_id_database.end())
    {
        auto device_it = vendor_it->second.find(device_id);
        if (device_it != vendor_it->second.end())
        {
            return device_it->second; // Return the device name
        }
        else
        {
            return vendor_it->first + "/" + device_id; // Vendor found, device not found
        }
    }
    else
    {
        return vendor_id + "/" + device_id; // Neither found
    }
}

StatusMonitor::StatusMonitor()
{
    this->network_adapters = std::vector<ifaddrs *>{};
    this->pci_database_loaded = false;
}

StatusMonitor::~StatusMonitor()
{
    for (auto copy : this->network_adapters)
    {
        if (copy == nullptr)
            continue;
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

std::string StatusMonitor::get_cpu_model()
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

std::string StatusMonitor::read_file(const std::string &path)
{
    std::ifstream f(path);
    if (!f)
        return "";
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

std::string StatusMonitor::get_gpu_model()
{
    namespace fs = std::filesystem;
    std::string vendor_id;
    std::string device_id;
    std::string model_name = "Unknown GPU";

    for (auto &entry : fs::directory_iterator("/sys/bus/pci/devices"))
    {
        // 1. Check if the device is a VGA (0x030000) or 3D Controller (0x030200)
        std::string class_code = read_file(entry.path().string() + "/class");
        // Ensure class_code is trimmed of whitespace/newline from read_file
        if (!class_code.empty() && class_code.back() == '\n')
        {
            class_code.pop_back();
        }

        if (class_code.rfind("0x0300", 0) == 0 || class_code.rfind("0x0302", 0) == 0)
        {
            // 2. Read the Vendor ID and Device ID
            vendor_id = this->read_file(entry.path().string() + "/vendor").substr(2); // Remove "0x" prefix
            device_id = this->read_file(entry.path().string() + "/device").substr(2); // Remove "0x" prefix

            // Trim newlines from IDs (critical for comparison)
            if (!vendor_id.empty() && vendor_id.back() == '\n')
                vendor_id.pop_back();
            if (!device_id.empty() && device_id.back() == '\n')
                device_id.pop_back();

            // 3. Lookup the name (You must implement this function)
            model_name = this->lookup_pci_names(vendor_id, device_id);

            // Break after finding the primary GPU
            break;
        }
    }
    return model_name;
}

std::vector<ifaddrs *> StatusMonitor::get_network_adapters()
{
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        throw std::runtime_error("getifaddrs failed");
    }

    std::vector<ifaddrs *> copies;
    std::unordered_set<std::string> seen_names;

    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        // Skip duplicates based on interface name
        if (seen_names.find(ifa->ifa_name) != seen_names.end())
            continue;

        seen_names.insert(ifa->ifa_name);

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

bool StatusMonitor::is_physical_drive(const std::string &device_name)
{
    // Ignore loop, ram, and device mapper
    if (device_name.rfind("loop", 0) == 0)
        return false;
    if (device_name.rfind("zram", 0) == 0)
        return false;
    if (device_name.rfind("dm-", 0) == 0)
        return false;

    return true;
}

std::vector<std::string> StatusMonitor::determine_hardware_resources()
{
    this->hardware_resources.push_back("CPU: " + get_cpu_model());

    struct sysinfo memory_info;

    if (sysinfo(&memory_info))
    {
        _exit(255);
    }

    this->hardware_resources.push_back("RAM: " + std::to_string((memory_info.totalram - memory_info.freeram) / (1000 * 1000)) + " / " + std::to_string(memory_info.totalram / (1000 * 1000)) + " MB");
    this->hardware_resources.push_back("GPU: " + get_gpu_model());

    // Free previous network adapters
    for (auto copy : this->network_adapters)
    {
        if (copy == nullptr)
            continue;
        if (copy->ifa_name)
            free(copy->ifa_name);
        if (copy->ifa_addr)
            free(copy->ifa_addr);
        if (copy->ifa_netmask)
            free(copy->ifa_netmask);
        delete copy;
    }

    network_adapters.clear();

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
        if (!is_physical_drive(dev))
            continue;
        this->hardware_resources.push_back("Drive " + std::to_string(storage_device_number) + ": " + dev);
        storage_device_number++;
    }

    return this->hardware_resources;
}
