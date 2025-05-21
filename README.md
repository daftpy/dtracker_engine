# dtracker_engine

A modular C++ audio engine library built with [RtAudio](https://github.com/thestk/rtaudio). Designed to be used as the backend for a Qt-based audio tracker GUI.

## Features

- Static library for clean integration
- Realtime audio support via RtAudio
- Optional unit tests with GoogleTest

## Requirements

- CMake 3.27+
- C++17-compatible compiler (e.g., MSVC, Clang, GCC)
- [RtAudio](https://github.com/thestk/rtaudio) (fetched automatically)
- (Optional) GoogleTest for testing

## Build Instructions

### 1. Clone the project

```bash
git clone https://github.com/yourname/dtracker_engine.git
cd dtracker_engine
```

### 2. Configure and build

#### 🔧 Debug Build (without tests)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

Builds the `dtracker_engine` static library in debug mode.

#### 🚦 Debug Build (with tests)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug
```

Includes and runs unit tests.

#### 🚀 Release Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Builds the optimized release version of the static library.
