#pragma once

#include <algorithm>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <vector>


class MockStereoUnit : public dtracker::audio::playback::PlaybackUnit
{
  public:
    float leftValue = 1.0f;
    float rightValue = 1.0f;
    bool finishedAfterRender = false;

    void render(float *buffer, unsigned int frames,
                unsigned int channels) override
    {
        for (unsigned int i = 0; i < frames; ++i)
        {
            buffer[i * 2] = leftValue;
            buffer[i * 2 + 1] = rightValue;
        }
        rendered = true;
    }

    bool isFinished() const override
    {
        return finishedAfterRender && rendered;
    }

  private:
    mutable bool rendered = false;
};
