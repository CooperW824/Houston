#include "process.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

Process::Process(pid_t pid, unsigned long memory, double cpu, unsigned long network)
    : pid(pid), memory_usage(memory), cpu_usage(cpu), network_usage(network)
{
}

pid_t Process::get_pid() const
{
    return pid;
}

unsigned long Process::get_memory_usage() const
{
    return memory_usage;
}

double Process::get_cpu_usage() const
{
    return cpu_usage;
}

unsigned long Process::get_network_usage() const
{
    return network_usage;
}

void Process::set_memory_usage(unsigned long memory)
{
    memory_usage = memory;
}

void Process::set_cpu_usage(double cpu)
{
    cpu_usage = cpu;
}

void Process::set_network_usage(unsigned long network)
{
    network_usage = network;
}

bool Process::kill(int signal_number)
{
    if (::kill(pid, signal_number) == 0)
    {
        return true;
    }
    else
    {
        std::cerr << "Failed to send signal " << signal_number
                  << " to process " << pid
                  << ": " << std::strerror(errno) << std::endl;
        return false;
    }
}

Process::~Process()
{
}

