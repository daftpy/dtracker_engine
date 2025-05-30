cmake_minimum_required(VERSION 3.27)
project(dtracker_engine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include engine headers
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Optional testing support ---
include(CTest)  # defines BUILD_TESTING and calls enable_testing()

# --- Fetch RtAudio ---
include(FetchContent)

# Only statically link RtAudio if building tests
if(BUILD_TESTING)
    set(BUILD_SHARED_LIBS OFF)
    set(gtest_force_shared_crt OFF)

    set(RTAUDIO_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_TESTING OFF CACHE BOOL "" FORCE)
else()
    # Let the consumer decide or use shared by default in non-test mode
    set(RTAUDIO_BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    RtAudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG        6.0.1
)
FetchContent_MakeAvailable(RtAudio)

# --- dtracker_engine static lib ---
add_library(dtracker_engine STATIC
    src/audio/engine.cpp
    src/audio/device_manager.cpp
    src/audio/playback_manager.cpp
    src/audio/sample_manager.cpp
    src/audio/playback/proxy_playback_unit.cpp
    src/audio/playback/tone_playback.cpp
    src/audio/playback/sample_playback.cpp
    src/audio/playback/mixer_playback.cpp
    src/audio/playback/track_playback_unit.cpp
    src/tracker/track_manager.cpp
)

target_include_directories(dtracker_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>        # For consumers: #include <dtracker/audio/...>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dtracker                  # For internal engine .cpp/.hpp: #include "audio/engine.hpp"
)

target_link_libraries(dtracker_engine PUBLIC rtaudio)

# --- Add tests only when enabled ---
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
