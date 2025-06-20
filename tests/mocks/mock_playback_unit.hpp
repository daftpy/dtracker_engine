#pragma once

#include <algorithm>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/types.hpp>
#include <vector>

class MockPlaybackUnit : public dtracker::audio::playback::PlaybackUnit
{
  public:
    int renderCallCount = 0;
    float fillValue = 0.0f;
    bool finishedAfterRender = false;

    void render(float *buffer, unsigned int frames, unsigned int channels,
                const dtracker::audio::types::RenderContext &context) override
    {
        renderCallCount++;
        std::fill(buffer, buffer + frames * channels, fillValue);
        hasRendered = true;
    }

    void reset() override
    {
        hasRendered = false;
        renderCallCount = 0;
    }

    bool isFinished() const override
    {
        return finishedAfterRender && hasRendered;
    }

  private:
    mutable bool hasRendered = false;
};
