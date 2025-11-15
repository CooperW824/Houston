cmake_minimum_required (VERSION 3.22)

project(houston
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------


add_executable(houston
  src/main.cpp
  src/status_monitor/status_monitor.cpp
  src/processes_list/process.cpp
  src/processes_list/processes_list.cpp
  src/processes_list/network_tracker.cpp
  src/ui/main_view.cpp
  src/ui/processes_view.cpp
  src/ui/processes_view_inputs.cpp
  src/ui/process_detail_view.cpp
  src/ui/status_view.cpp
)
target_include_directories(houston PRIVATE src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(STATGRAB REQUIRED libstatgrab)
find_package(Threads REQUIRED)

# Find libpci
pkg_check_modules(LIBPCI REQUIRED libpci)


target_link_libraries(houston
PRIVATE ftxui::screen
PRIVATE ftxui::dom
PRIVATE ftxui::component
PRIVATE ${STATGRAB_LIBRARIES}
PRIVATE Threads::Threads
PRIVATE ${LIBPCI_LIBRARIES}
)

target_include_directories(houston PRIVATE ${LIBPCI_INCLUDE_DIRS})
target_compile_options(houston PRIVATE ${LIBPCI_CFLAGS_OTHER})
target_include_directories(houston PRIVATE ${STATGRAB_INCLUDE_DIRS})
target_compile_options(houston PRIVATE ${STATGRAB_CFLAGS_OTHER})

# --- Testing with Google Test -------------------------------------------------
option(BUILD_TESTS "Build the tests" ON)

if(BUILD_TESTS)
  enable_testing()

  # Fetch Google Test
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
  )

  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  # Create test executable
  add_executable(houston_tests
    tests/test_process.cpp
    tests/test_processes_view.cpp
    tests/test_network_tracker.cpp
    src/processes_list/process.cpp
    src/processes_list/network_tracker.cpp
    src/ui/processes_view_inputs.cpp
  )

  target_include_directories(houston_tests PRIVATE src)
  target_include_directories(houston_tests PRIVATE ${STATGRAB_INCLUDE_DIRS})
  target_include_directories(houston_tests PRIVATE ${LIBPCI_INCLUDE_DIRS})

  target_link_libraries(houston_tests
    PRIVATE GTest::gtest_main
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
    PRIVATE ${STATGRAB_LIBRARIES}
    PRIVATE Threads::Threads
    PRIVATE ${LIBPCI_LIBRARIES}
  )

  target_compile_options(houston_tests PRIVATE ${STATGRAB_CFLAGS_OTHER})
  target_compile_options(houston_tests PRIVATE ${LIBPCI_CFLAGS_OTHER})

  include(GoogleTest)
  gtest_discover_tests(houston_tests)
endif()
# ------------------------------------------------------------------------------
