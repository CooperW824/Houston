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
#include <statgrab.h>
#include <thread>
#include <unordered_map>
#include <algorithm>


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
    this->hardware_resources = std::vector<std::string>{};
    this->determine_hardware_resources();

    int logical_cores = std::thread::hardware_concurrency();
    this->cpu_logical_core_count = logical_cores;
    this->logical_core_utilizations.resize(logical_cores);

    for (int i = 0; i < logical_core_utilizations.size(); i++)
    {
        logical_core_utilizations[i] = new double;
    }

    this->compute_cpu_utilization();
    this->compute_process_and_thread_counts();
    this->compute_max_cpu_clock_speeds();
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

void StatusMonitor::update()
{
    this->hardware_resources.clear();
    this->determine_hardware_resources();
    this->compute_cpu_utilization();
    this->compute_process_and_thread_counts();
    this->compute_max_cpu_clock_speeds();
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
    if (device_name.rfind("ram", 0) == 0)
        return false;
    if (device_name.rfind("dm-", 0) == 0)
        return false;

    return true;
}

void StatusMonitor::determine_hardware_resources()
{
    this->hardware_resources.push_back("CPU: " + get_cpu_model());

    struct sysinfo memory_info;

    if (!sysinfo(&memory_info))
    {
        this->hardware_resources.push_back("RAM: " + std::to_string(memory_info.totalram / (1000 * 1000)) + " MB");
    }

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
}

double calculate_utilization(const CpuTime &prev_times, const CpuTime &current_times)
{
    long long prev_total = prev_times.getTotalTime();
    long long current_total = current_times.getTotalTime();

    long long prev_idle = prev_times.getIdleTime();
    long long current_idle = current_times.getIdleTime();

    // Calculate the difference in total time and idle time
    long long total_diff = current_total - prev_total;
    long long idle_diff = current_idle - prev_idle;

    if (total_diff <= 0)
    {
        // Handle case where total time hasn't increased (e.g., if the time interval was too short)
        return -1.0;
    }

    // Non-idle time difference is the time the CPU spent working
    long long work_diff = total_diff - idle_diff;

    // Utilization = (Work Time / Total Time) * 100
    double utilization = (static_cast<double>(work_diff) / total_diff) * 100.0;

    return utilization;
}

void StatusMonitor::compute_cpu_utilization()
{
    std::unordered_map<std::string, CpuTime> cpu_data_1;
    std::ifstream file("/proc/stat");
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open /proc/stat" << std::endl;
    }

    // Read the file line by line
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string cpu_label;
        ss >> cpu_label;

        // Check if the line starts with "cpu" (total) or "cpuN" (individual core)
        if (cpu_label.rfind("cpu", 0) == 0)
        {
            if (cpu_label.length() > 3 && !isdigit(cpu_label[3]))
            {
                // Ignore non-standard cpu labels like cpuidle, cpusets etc if they exist
                continue;
            }

            CpuTime times;
            // Extract Jiffy values
            ss >> times.user >> times.nice >> times.system >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal >> times.guest >> times.guest_nice;

            cpu_data_1[cpu_label] = times;
        }
        // Stop reading after all "cpu" lines are processed to save time
        if (cpu_label.rfind("cpu", 0) != 0 && !cpu_data_1.empty())
        {
            break;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::unordered_map<std::string, CpuTime> cpu_data_2;
    file.close();
    file.open("/proc/stat");

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open /proc/stat" << std::endl;
        return;
    }

    // Read the file line by line again
    // Read the file line by line
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string cpu_label;
        ss >> cpu_label;

        // Check if the line starts with "cpu" (total) or "cpuN" (individual core)
        if (cpu_label.rfind("cpu", 0) == 0)
        {
            if (cpu_label.length() > 3 && !isdigit(cpu_label[3]))
            {
                // Ignore non-standard cpu labels like cpuidle, cpusets etc if they exist
                continue;
            }

            CpuTime times;
            // Extract Jiffy values
            ss >> times.user >> times.nice >> times.system >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal >> times.guest >> times.guest_nice;

            cpu_data_2[cpu_label] = times;
        }
        // Stop reading after all "cpu" lines are processed to save time
        if (cpu_label.rfind("cpu", 0) != 0 && !cpu_data_2.empty())
        {
            break;
        }
    }

    // Calculate overall CPU utilization
    this->overall_cpu_utilization_percent = calculate_utilization(cpu_data_1["cpu"], cpu_data_2["cpu"]);
    // Calculate per-core utilizations
    for (int i = 0; i < this->cpu_logical_core_count; ++i)
    {
        std::string core_label = "cpu" + std::to_string(i);
        *this->logical_core_utilizations[i] = calculate_utilization(cpu_data_1[core_label], cpu_data_2[core_label]);
    }
}

void StatusMonitor::compute_process_and_thread_counts()
{
    this->process_count = 0;
    std::string proc_path = "/proc";

    for (const auto &entry : std::filesystem::directory_iterator(proc_path))
    {
        if (entry.is_directory())
        {
            std::string dir_name = entry.path().filename().string();
            // Check if the directory name is a number (PID)
            if (std::all_of(dir_name.begin(), dir_name.end(), ::isdigit))
            {
                // This is a process directory
                int pid = std::stoi(dir_name);

                // Optionally, read /proc/<PID>/status to check process state
                std::string status_file_path = proc_path + "/" + dir_name + "/status";
                std::ifstream status_file(status_file_path);
                if (status_file.is_open())
                {
                    std::string line;
                    while (std::getline(status_file, line))
                    {
                        if (line.rfind("State:", 0) == 0)
                        { // Check if line starts with "State:"
                            // Extract state (e.g., "S (sleeping)", "R (running)")
                            // You can parse this to count only specific states if needed
                            // For a simple count of "running" processes (not zombies/stopped),
                            // you can check for 'R' or 'S' (running/sleeping)
                            // A more robust check would involve parsing the full state string.
                            this->process_count++;
                            break; // Move to next process after finding state
                        }
                    }
                    status_file.close();
                }
            }
        }
    }

    // TODO: Thread count computation (statgrab does not provide this directly)
    this->thread_count = 0; // Placeholder
}

void StatusMonitor::compute_max_cpu_clock_speeds()
{
    std::ifstream f("/proc/cpuinfo");
    std::string line;
    std::vector<double> mhz;

    while (std::getline(f, line))
    {
        if (line.find("cpu MHz") != std::string::npos)
        {
            double val;
            if (sscanf(line.c_str(), "cpu MHz\t: %lf", &val) == 1)
                mhz.push_back(val);
        }
    }

    if (!mhz.empty())
    {
        this->cpu_logical_core_count = mhz.size();
        this->cpu_max_clock_speed_mhz = *std::max_element(mhz.begin(), mhz.end());
    }
}
