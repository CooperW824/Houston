#include "process.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

Process::Process(pid_t pid, const std::string& name, unsigned long memory, double cpu, unsigned long network, unsigned long time, const std::string& cmd)
    : pid(pid), process_name(name), memory_usage(memory), cpu_usage(cpu), network_usage(network), cpu_time(time), command(cmd)
{
}

pid_t Process::get_pid() const
{
    return pid;
}

std::string Process::get_process_name() const
{
    return process_name;
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

unsigned long Process::get_cpu_time() const
{
    return cpu_time;
}

std::string Process::get_command() const
{
    return command;
}

void Process::set_process_name(const std::string& name)
{
    process_name = name;
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

void Process::set_cpu_time(unsigned long time)
{
    cpu_time = time;
}

void Process::set_command(const std::string& cmd)
{
    command = cmd;
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

