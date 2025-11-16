#ifndef __MEM_INFO_VIEW_HPP
#define __MEM_INFO_VIEW_HPP

#include "ftxui/component/component.hpp"

using namespace ftxui;

Component create_mem_info_view(
    const double memory_total_mb,
    const double *memory_used_mb,
    const double *memory_free_mb,
    const double *memory_cached_mb,
    const double memory_swap_total_mb,
    const double *memory_swap_used_mb,
    const std::vector<double> *history_memory_used_mb);

#endif /* __MEM_INFO_VIEW_HPP */
