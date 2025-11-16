#ifndef __MAIN_VIEW_HPP
#define __MAIN_VIEW_HPP

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "../processes_list/processes_list.hpp"
#include <vector>
#include <mutex>
#include <thread>

using namespace ftxui;

void start_ui(double refresh_rate_seconds);

#endif
