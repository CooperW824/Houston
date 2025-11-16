#ifndef __PROCESSES_VIEW_STATE_HPP
#define __PROCESSES_VIEW_STATE_HPP

#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "ftxui/screen/box.hpp"
#include "../processes_list/process.hpp"

using namespace ftxui;

namespace ProcessesView {

enum class SortColumn { PID, NAME, MEMORY, CPU, NETWORK, TIME, COMMAND };

// Shared state structure for the processes view
struct ViewState {
    // Constants
    static constexpr int COL_SIGTERM_WIDTH = 1;
    static constexpr int COL_SIGKILL_WIDTH = 2;
    static constexpr int COL_KILL_WIDTH = COL_SIGTERM_WIDTH + 1 + COL_SIGKILL_WIDTH;
    static constexpr int COL_PID_WIDTH = 10;
    static constexpr int COL_NAME_WIDTH = 20;
    static constexpr int COL_MEMORY_WIDTH = 12;
    static constexpr int COL_CPU_WIDTH = 10;
    static constexpr int COL_NETWORK_WIDTH = 12;
    static constexpr int COL_TIME_WIDTH = 12;
    static constexpr int HISTORY_SIZE = 60;

    // Selection and interaction state
    std::shared_ptr<int> selected_index;
    std::shared_ptr<int> hover_index;
    std::shared_ptr<int> hover_sigterm;
    std::shared_ptr<int> hover_sigkill;
    
    // Search state
    std::shared_ptr<bool> search_mode;
    std::shared_ptr<std::string> search_phrase;
    
    // Sort state
    std::shared_ptr<SortColumn> sort_column;
    std::shared_ptr<bool> sort_ascending;
    
    // Detail view state
    std::shared_ptr<bool> show_detail_view;
    std::shared_ptr<pid_t> detail_process_pid;
    std::shared_ptr<std::vector<float>> cpu_history;
    std::shared_ptr<std::vector<float>> memory_history;
    std::shared_ptr<std::vector<float>> network_history;
    std::shared_ptr<std::chrono::steady_clock::time_point> last_sample_time;
    
    // Click tracking for double-click detection
    std::shared_ptr<std::chrono::steady_clock::time_point> last_click_time;
    std::shared_ptr<int> last_clicked_index;
    
    // Display tracking
    std::shared_ptr<std::vector<pid_t>> displayed_pids;
    
    // Bounding boxes for mouse interaction
    std::shared_ptr<std::vector<Box>> boxes;
    std::shared_ptr<std::vector<Box>> sigterm_boxes;
    std::shared_ptr<std::vector<Box>> sigkill_boxes;
    std::shared_ptr<Box> header_pid_box;
    std::shared_ptr<Box> header_name_box;
    std::shared_ptr<Box> header_memory_box;
    std::shared_ptr<Box> header_cpu_box;
    std::shared_ptr<Box> header_network_box;
    std::shared_ptr<Box> header_time_box;
    std::shared_ptr<Box> header_command_box;

    ViewState() {
        selected_index = std::make_shared<int>(0);
        hover_index = std::make_shared<int>(-1);
        hover_sigterm = std::make_shared<int>(-1);
        hover_sigkill = std::make_shared<int>(-1);
        search_mode = std::make_shared<bool>(false);
        search_phrase = std::make_shared<std::string>("");
        sort_column = std::make_shared<SortColumn>(SortColumn::CPU);
        sort_ascending = std::make_shared<bool>(false);
        show_detail_view = std::make_shared<bool>(false);
        detail_process_pid = std::make_shared<pid_t>(0);
        cpu_history = std::make_shared<std::vector<float>>();
        memory_history = std::make_shared<std::vector<float>>();
        network_history = std::make_shared<std::vector<float>>();
        last_sample_time = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
        last_click_time = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
        last_clicked_index = std::make_shared<int>(-1);
        displayed_pids = std::make_shared<std::vector<pid_t>>();
        boxes = std::make_shared<std::vector<Box>>();
        sigterm_boxes = std::make_shared<std::vector<Box>>();
        sigkill_boxes = std::make_shared<std::vector<Box>>();
        header_pid_box = std::make_shared<Box>();
        header_name_box = std::make_shared<Box>();
        header_memory_box = std::make_shared<Box>();
        header_cpu_box = std::make_shared<Box>();
        header_network_box = std::make_shared<Box>();
        header_time_box = std::make_shared<Box>();
        header_command_box = std::make_shared<Box>();
    }
};

} // namespace ProcessesView

#endif

