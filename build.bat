@echo off
REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Navigate to build directory
cd build

REM Configure with CMake
cmake ..

REM Build the project
cmake --build . --config Release

REM Return to the root directory
cd ..

echo Build complete. Executable is in build\bin\
