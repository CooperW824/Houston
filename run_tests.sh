#!/bin/bash

# Houston Test Runner Script
# This script builds and runs the test suite for the Houston process monitor

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "========================================"
echo "Houston Process Monitor - Test Runner"
echo "========================================"
echo ""

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring build with CMake..."
cmake .. -DBUILD_TESTS=ON

# Build the test executable
echo ""
echo "Building test suite..."
make houston_tests -j$(nproc)

# Run the tests
echo ""
echo "========================================"
echo "Running Tests"
echo "========================================"
echo ""
./houston_tests

# Display summary
echo ""
echo "========================================"
echo "Test run complete!"
echo "========================================"

