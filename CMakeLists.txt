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


add_executable(houston src/main.cpp)
target_include_directories(houston PRIVATE src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(STATGRAB REQUIRED libstatgrab)

target_link_libraries(houston
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE ${STATGRAB_LIBRARIES}
)

target_include_directories(houston PRIVATE ${STATGRAB_INCLUDE_DIRS})
target_compile_options(houston PRIVATE ${STATGRAB_CFLAGS_OTHER})
