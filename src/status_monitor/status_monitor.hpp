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
#include <thread>

// A map to store the device names associated with a vendor
using DeviceMap = std::map<std::string, std::string>;
// The main map: Vendor ID -> (Device ID -> Device Name)
using PciIdDatabase = std::map<std::string, DeviceMap>;

#include <statgrab.h>

struct CpuTime
{
    long long user = 0;
    long long nice = 0;
    long long system = 0;
    long long idle = 0;
    long long iowait = 0;
    long long irq = 0;
    long long softirq = 0;
    long long steal = 0;
    long long guest = 0;
    long long guest_nice = 0;

    /**
     * @brief Calculates the total non-idle time for the CPU core.
     * @return The sum of all time fields except idle and iowait.
     */
    long long getTotalTime() const
    {
        return user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
    }

    /**
     * @brief Calculates the total idle time (idle + iowait).
     * @return The sum of the idle and iowait fields.
     */
    long long getIdleTime() const
    {
        return idle + iowait;
    }
};

class StatusMonitor
{
private:
    std::vector<std::string> hardware_resources;
    std::vector<ifaddrs *> network_adapters;

    double cpu_max_clock_speed_mhz = 0.0;
    double overall_cpu_utilization_percent = 0.0;
    int cpu_logical_core_count = 0;
    int process_count = 0;
    int thread_count = 0;
    std::vector<double> logical_core_utilizations;

    PciIdDatabase pci_id_database;
    bool pci_database_loaded = false;

    std::string pci_file_location = "./src/assets/pci.ids";

    void load_pci_id_database(std::string pci_id_filepath);

    std::string lookup_pci_names(const std::string &vendor_id, const std::string &device_id);

    std::string get_cpu_model();
    std::string get_gpu_model();
    std::vector<ifaddrs *> get_network_adapters();
    bool is_physical_drive(const std::string &device_name);
    std::string read_file(const std::string &path);
    void determine_hardware_resources();

    void compute_cpu_utilization();
    void compute_process_and_thread_counts();
    void compute_max_cpu_clock_speeds();

public:
    StatusMonitor(/* args */);
    ~StatusMonitor();
    void update();
    std::vector<std::string> *get_hardware_resources()
    {
        return &this->hardware_resources;
    }

    double *get_overall_cpu_utilization()
    {
        return &this->overall_cpu_utilization_percent;
    }
    const std::vector<double> &get_logical_core_utilizations()
    {
        return this->logical_core_utilizations;
    }
    double *get_cpu_max_clock_speed_mhz()
    {
        return &this->cpu_max_clock_speed_mhz;
    }
    int get_cpu_logical_core_count()
    {
        return this->cpu_logical_core_count;
    }
    int *get_cpu_process_count()
    {
        return &this->process_count;
    }
    int *get_cpu_thread_count()
    {
        return &this->thread_count;
    }
    std::string get_cpu_model_public()
    {
        return this->get_cpu_model();
    }
};

#endif /* __STATUS_MONITOR_HPP */
