#pragma once

#include <gtest/gtest.h>

#include <dtracker/audio/playback/pattern_playback_unit.hpp>


// A mock for the PatternPlaybackUnit. This allows us to test the
// TrackPlaybackUnit's sequencing logic in isolation.
class MockPatternPlaybackUnit
    : public dtracker::audio::playback::PatternPlaybackUnit
{
  public:
    // We must call the base class constructor. We can just pass it empty
    // arguments since we will be overriding all the important behavior.
    MockPatternPlaybackUnit()
        : dtracker::audio::playback::PatternPlaybackUnit({}, {}, nullptr, 44100)
    {
    }

    // --- Public variables for test control and verification ---
    int renderCallCount = 0;
    int resetCallCount = 0;
    bool isFinished_flag =
        false; // A flag to control the return value of isFinished()

    // --- Overridden Methods ---

    void render(float *buffer, unsigned int nFrames,
                unsigned int channels) override
    {
        // For the test, we don't need to render real audio. We just
        // need to record that this method was called.
        renderCallCount++;
    }

    void reset() override
    {
        // Record that reset() was called.
        resetCallCount++;
    }

    bool isFinished() const override
    {
        // Return the value of our controllable flag.
        return isFinished_flag;
    }
};