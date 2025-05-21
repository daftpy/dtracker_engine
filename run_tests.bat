@echo off
REM Configure the build with tests enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

REM Build the project
cmake --build build --config Debug

REM Run tests
ctest --test-dir build --output-on-failure -C Debug
