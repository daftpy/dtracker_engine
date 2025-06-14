#pragma once

#include <dtracker/audio/playback/mixer_playback.hpp>
#include <memory>
#include <vector>

// Mocks the main mixer. Its only job is to track the number of
// active sound units so we can test add/clear operations.
class MockMixerPlaybackUnit
    : public dtracker::audio::playback::MixerPlaybackUnit
{
  public:
    void addUnit(
        std::unique_ptr<dtracker::audio::playback::PlaybackUnit> unit) override
    {
        m_units.push_back(std::move(unit));
    }

    void clear() override
    {
        m_units.clear();
    }

    // The mixer is "finished" if it has no units to play.
    bool isFinished() const override
    {
        return m_units.empty();
    }

    // A helper for our tests to check the state.
    size_t getUnitCount() const
    {
        return m_units.size();
    }

  private:
    std::vector<std::unique_ptr<dtracker::audio::playback::PlaybackUnit>>
        m_units;
};