#ifndef __CPU_INFO_VIEW_HPP
#define __CPU_INFO_VIEW_HPP

#include "ftxui/component/component.hpp"

using namespace ftxui;

Component create_cpu_info_view(
    const std::vector<double> &core_utilizations,
    const double *max_clock_speed_mhz,
    const int logical_core_count,
    const int *process_count,
    const int *thread_count,
    const std::string &cpu_model);

#endif /* __CPU_INFO_VIEW_HPP */
