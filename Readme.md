# Houston - System Process Monitor

A modern terminal-based process monitor built with C++ and FTXUI.

## Features

- Real-time process monitoring
- Interactive UI with keyboard and mouse support
- Search and filter processes
- Sort by PID, name, memory, CPU, or network usage
- Kill processes with SIGTERM or SIGKILL
- Vim-style navigation

## Installation

### Installing libstatgrab

    cd external/libstatgrab
    ./autogen.sh
    ./configure --prefix=$HOME/.local
    make
    make install

### Build and setup

    mkdir build
    cd build
    cmake configure ..
    cd ..
    cmake --build build

### Running the application

    ./build/houston

## Testing

Houston includes a comprehensive test suite with 42 tests covering:
- Process data model functionality
- UI input handling
- Navigation (keyboard and mouse)
- Search functionality
- Edge cases

### Running tests

Quick way:

    ./run_tests.sh

Manual way:

    cd build
    cmake .. -DBUILD_TESTS=ON
    make houston_tests
    ./houston_tests

For more information about the tests, see [tests/README.md](tests/README.md).

### Disable tests

To build without tests:

    cmake .. -DBUILD_TESTS=OFF
