#ifndef __PROCESS_HPP
#define __PROCESS_HPP

#include <string>
#include <sys/types.h>
#include <signal.h>

class Process
{
private:
    pid_t pid;
    std::string process_name;
    unsigned long memory_usage;
    double cpu_usage;
    unsigned long network_usage;
    unsigned long cpu_time;
    std::string command;

public:
    Process(pid_t pid, const std::string& name = "", unsigned long memory = 0, double cpu = 0.0, unsigned long network = 0, unsigned long time = 0, const std::string& cmd = "");

    pid_t get_pid() const;
    std::string get_process_name() const;
    unsigned long get_memory_usage() const;
    double get_cpu_usage() const;
    unsigned long get_network_usage() const;
    unsigned long get_cpu_time() const;
    std::string get_command() const;

    void set_process_name(const std::string& name);
    void set_memory_usage(unsigned long memory);
    void set_cpu_usage(double cpu);
    void set_network_usage(unsigned long network);
    void set_cpu_time(unsigned long time);
    void set_command(const std::string& cmd);

    bool kill(int signal_number);

    ~Process();
};

#endif

