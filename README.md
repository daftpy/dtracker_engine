# dtracker_engine

A modular C++ audio engine library built with [RtAudio](https://github.com/thestk/rtaudio). Designed to be used as the backend for a Qt-based audio tracker GUI.

## Core Concepts and Architecture
The engine is built with a layered architecture that emphasizes testability and separation of concerns:

- **Interface-based Design:** Core components like `Engine` and `PlaybackManager` are defined by abstract interfaces (`IEngine` and `IPlaybackManager`). This allows for easy mocking and testing.

- **Sample Instancing:** The `SampleManager` separates the concept of raw audio samples stored in the cache and the instances stored in the manager. This allows hundreds of soounds in a project to efficient share thee same underlying audio data safely.

- **Component Layers:** The system is organized into clear service layers:
  - `Engine`: The low-level service that communicates with `RtAudio`.

  - `SampleManager`: Provides caching and management for all audio samples.

  - `PlaybackManager`: The high-level API for controlling all playback (previews, patterns, tracks).

## Features

- **C++17 Design** Utilizes smart pointers and move semantics when possible to prevent memory errors and improve efficiency.
- **Decoupled Architecture:** Core components are abstracted behind interfaces for improved flexibility and testability.
- **Thread-Safe Sample Managemeent:** A central manager with LRU cache provides efficient and shared access to audio data.
- **Comprehensive Test Suite:** Built with GoogleTest and a clean separation between unit and integration tests.

## Requirements

- CMake 3.27+
- C++17-compatible compiler (e.g., MSVC, Clang, GCC)
- [RtAudio](https://github.com/thestk/rtaudio) (fetched automatically)
- GoogleTest for testing (fetched automatically)

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
