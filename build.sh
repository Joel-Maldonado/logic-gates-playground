#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release

# Return to the root directory
cd ..

echo "Build complete. Executable is in build/bin/"
