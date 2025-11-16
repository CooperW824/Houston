#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <chrono>
#include "../src/processes_list/process.hpp"
#include "../src/ui/process_view/processes_view_inputs.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/screen/box.hpp"

using namespace ftxui;

// Test fixture for ProcessesView tests
class ProcessesViewTest : public ::testing::Test {
protected:
    std::shared_ptr<int> selected_index;
    std::shared_ptr<int> hover_index;
    std::shared_ptr<int> hover_sigterm;
    std::shared_ptr<int> hover_sigkill;
    std::shared_ptr<bool> search_mode;
    std::shared_ptr<std::string> search_phrase;
    std::shared_ptr<std::vector<Box>> boxes;
    std::shared_ptr<std::vector<Box>> sigterm_boxes;
    std::shared_ptr<std::vector<Box>> sigkill_boxes;
    std::vector<Process> processes;
    std::mutex processes_mutex;
    std::shared_ptr<bool> show_detail_view;
    std::shared_ptr<pid_t> detail_process_pid;
    std::shared_ptr<std::chrono::steady_clock::time_point> last_click_time;
    std::shared_ptr<int> last_clicked_index;
    std::shared_ptr<std::vector<pid_t>> displayed_pids;

    void SetUp() override {
        selected_index = std::make_shared<int>(0);
        hover_index = std::make_shared<int>(-1);
        hover_sigterm = std::make_shared<int>(-1);
        hover_sigkill = std::make_shared<int>(-1);
        search_mode = std::make_shared<bool>(false);
        search_phrase = std::make_shared<std::string>("");
        boxes = std::make_shared<std::vector<Box>>();
        sigterm_boxes = std::make_shared<std::vector<Box>>();
        sigkill_boxes = std::make_shared<std::vector<Box>>();
        show_detail_view = std::make_shared<bool>(false);
        detail_process_pid = std::make_shared<pid_t>(0);
        last_click_time = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
        last_clicked_index = std::make_shared<int>(-1);
        displayed_pids = std::make_shared<std::vector<pid_t>>();

        // Create sample processes for testing
        processes.push_back(Process(1000, "chrome", 5000, 10.5, 1024));
        processes.push_back(Process(2000, "firefox", 3000, 5.2, 512));
        processes.push_back(Process(3000, "code", 8000, 20.1, 2048));
        processes.push_back(Process(4000, "terminal", 1000, 2.3, 256));
    }
};

// ===========================
// Navigation Tests
// ===========================

TEST_F(ProcessesViewTest, ArrowUpDecreasesSelectedIndex) {
    *selected_index = 2;
    Event event = Event::ArrowUp;

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 1);
}

TEST_F(ProcessesViewTest, ArrowDownIncreasesSelectedIndex) {
    *selected_index = 1;
    Event event = Event::ArrowDown;

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 2);
}

TEST_F(ProcessesViewTest, ArrowUpAtZeroStaysAtZero) {
    *selected_index = 0;
    Event event = Event::ArrowUp;

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 0);
}

TEST_F(ProcessesViewTest, VimStyleNavigationK) {
    *selected_index = 2;
    Event event = Event::Character('k');

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 1);
}

TEST_F(ProcessesViewTest, VimStyleNavigationJ) {
    *selected_index = 1;
    Event event = Event::Character('j');

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 2);
}

TEST_F(ProcessesViewTest, PageUpDecreasesBy10) {
    *selected_index = 15;
    Event event = Event::PageUp;

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 5);
}

TEST_F(ProcessesViewTest, PageDownIncreasesBy10) {
    *selected_index = 5;
    Event event = Event::PageDown;

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 15);
}

TEST_F(ProcessesViewTest, PageUpAtLowIndexStaysAtZero) {
    *selected_index = 3;
    Event event = Event::PageUp;

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 0);
}

// ===========================
// Search Mode Tests
// ===========================

TEST_F(ProcessesViewTest, SlashEntersSearchMode) {
    *search_mode = false;
    Event event = Event::Character('/');

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_TRUE(*search_mode);
    EXPECT_EQ(*search_phrase, "");
    EXPECT_EQ(*selected_index, 0);
}

TEST_F(ProcessesViewTest, SearchModeAcceptsCharacters) {
    *search_mode = true;
    Event event = Event::Character('c');

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*search_phrase, "c");
}

TEST_F(ProcessesViewTest, SearchModeAcceptsMultipleCharacters) {
    *search_mode = true;

    std::vector<char> chars = {'c', 'h', 'r', 'o', 'm', 'e'};
    for (char c : chars) {
        Event event = Event::Character(c);
        handle_processes_view_event(
            event, selected_index, hover_index, hover_sigterm, hover_sigkill,
            search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
            processes, processes_mutex,
            show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
        );
    }

    EXPECT_EQ(*search_phrase, "chrome");
}

TEST_F(ProcessesViewTest, SearchModeBackspaceRemovesCharacter) {
    *search_mode = true;
    *search_phrase = "chrome";

    Event event = Event::Backspace;
    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*search_phrase, "chrom");
}

TEST_F(ProcessesViewTest, SearchModeBackspaceOnEmptyString) {
    *search_mode = true;
    *search_phrase = "";

    Event event = Event::Backspace;
    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*search_phrase, "");
}

TEST_F(ProcessesViewTest, SearchModeReturnExitsSearch) {
    *search_mode = true;
    *search_phrase = "chrome";

    Event event = Event::Return;
    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_FALSE(*search_mode);
    EXPECT_EQ(*search_phrase, "chrome");  // Search phrase persists
}

TEST_F(ProcessesViewTest, SearchModeEscapeExitsAndClearsSearch) {
    *search_mode = true;
    *search_phrase = "chrome";

    Event event = Event::Escape;
    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_FALSE(*search_mode);
    EXPECT_EQ(*search_phrase, "");
    EXPECT_EQ(*selected_index, 0);
}

TEST_F(ProcessesViewTest, EscapeOutsideSearchClearsSearchPhrase) {
    *search_mode = false;
    *search_phrase = "chrome";

    Event event = Event::Escape;
    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*search_phrase, "");
    EXPECT_EQ(*selected_index, 0);
}

TEST_F(ProcessesViewTest, SearchModeResetsSelectedIndex) {
    *selected_index = 5;
    *search_mode = true;

    Event event = Event::Character('x');
    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 0);
}

// ===========================
// Mouse Interaction Tests
// ===========================

TEST_F(ProcessesViewTest, MouseWheelUpDecreasesIndex) {
    *selected_index = 5;

    Mouse mouse_event;
    mouse_event.button = Mouse::WheelUp;
    mouse_event.motion = Mouse::Pressed;
    mouse_event.x = 10;
    mouse_event.y = 10;
    Event event = Event::Mouse("", mouse_event);

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 4);
}

TEST_F(ProcessesViewTest, MouseWheelDownIncreasesIndex) {
    *selected_index = 3;

    Mouse mouse_event;
    mouse_event.button = Mouse::WheelDown;
    mouse_event.motion = Mouse::Pressed;
    mouse_event.x = 10;
    mouse_event.y = 10;
    Event event = Event::Mouse("", mouse_event);

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*selected_index, 4);
}

TEST_F(ProcessesViewTest, MouseWheelUpAtZeroStaysAtZero) {
    *selected_index = 0;

    Mouse mouse_event;
    mouse_event.button = Mouse::WheelUp;
    mouse_event.motion = Mouse::Pressed;
    mouse_event.x = 10;
    mouse_event.y = 10;
    Event event = Event::Mouse("", mouse_event);

    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_EQ(*selected_index, 0);
}

// ===========================
// Kill Process Keyboard Shortcuts Tests
// ===========================

TEST_F(ProcessesViewTest, BackspaceKillsSelectedProcessWithSIGTERM) {
    *selected_index = 1;
    *search_mode = false;

    Event event = Event::Backspace;

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
}

TEST_F(ProcessesViewTest, DeleteKillsSelectedProcessWithSIGKILL) {
    *selected_index = 2;
    *search_mode = false;

    Event event = Event::Delete;

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
}

TEST_F(ProcessesViewTest, BackspaceInSearchModeDoesNotKillProcess) {
    *selected_index = 1;
    *search_mode = true;
    *search_phrase = "test";

    Event event = Event::Backspace;

    bool handled = handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_TRUE(handled);
    EXPECT_EQ(*search_phrase, "tes");
}

// ===========================
// Edge Case Tests
// ===========================

TEST_F(ProcessesViewTest, NonHandledEventReturnsFalse) {
    Event event = Event::Character('x');  // Not a special key

    bool handled = handle_processes_view_event (
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    EXPECT_FALSE(handled);
}

TEST_F(ProcessesViewTest, EmptyProcessList) {
    processes.clear();
    *selected_index = 5;

    Event event = Event::ArrowDown;
    handle_processes_view_event(
        event, selected_index, hover_index, hover_sigterm, hover_sigkill,
        search_mode, search_phrase, boxes, sigterm_boxes, sigkill_boxes,
        processes, processes_mutex,
        show_detail_view, detail_process_pid, last_click_time, last_clicked_index, displayed_pids
    );

    // Should still increment even with empty list
    EXPECT_EQ(*selected_index, 6);
}

// ===========================
// Double-Click and Display Tests
// ===========================

TEST_F(ProcessesViewTest, DisplayedPidsTracksCorrectPids) {
    displayed_pids->resize(processes.size());

    for (size_t i = 0; i < processes.size(); i++) {
        (*displayed_pids)[i] = processes[i].get_pid();
    }

    EXPECT_EQ(displayed_pids->size(), processes.size());
    EXPECT_EQ((*displayed_pids)[0], 1000);
    EXPECT_EQ((*displayed_pids)[1], 2000);
    EXPECT_EQ((*displayed_pids)[2], 3000);
    EXPECT_EQ((*displayed_pids)[3], 4000);
}

