#ifndef __PROCESS_HPP
#define __PROCESS_HPP

#include <string>
#include <sys/types.h>
#include <signal.h>

class Process
{
private:
    pid_t pid;
    unsigned long memory_usage;
    double cpu_usage;
    unsigned long network_usage;

public:
    Process(pid_t pid, unsigned long memory = 0, double cpu = 0.0, unsigned long network = 0);

    pid_t get_pid() const;
    unsigned long get_memory_usage() const;
    double get_cpu_usage() const;
    unsigned long get_network_usage() const;

    void set_memory_usage(unsigned long memory);
    void set_cpu_usage(double cpu);
    void set_network_usage(unsigned long network);

    bool kill(int signal_number);

    ~Process();
};

#endif

