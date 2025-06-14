#pragma once

#include "mock_mixer_playback_unit.hpp" // Our new mock mixer
#include <dtracker/audio/i_engine.hpp>

// A mock engine that implements the IEngine interface
class MockEngine : public dtracker::audio::IEngine
{
  public:
    MockEngine()
    {
        m_mockMixer = std::make_unique<MockMixerPlaybackUnit>();
    }

    bool start() override
    {
        m_streamIsRunning = true;
        return true;
    }

    void stop() override
    {
        m_streamIsRunning = false;
    }

    bool isStreamRunning() const override
    {
        return m_streamIsRunning;
    }

    // Return controllable mock mixer.
    dtracker::audio::playback::MixerPlaybackUnit *mixerUnit() override
    {
        return m_mockMixer.get();
    }

    // Not currently tested
    void setOutputDevice(unsigned int /*deviceId*/) override {}

    dtracker::audio::DeviceManager createDeviceManager() const override
    {
        // Return a default-constructed object as we don't need it for these
        // tests.
        return dtracker::audio::DeviceManager(nullptr);
    }

    // Helper for tests to get the raw mixer pointer for assertions
    MockMixerPlaybackUnit *getMockMixer()
    {
        return m_mockMixer.get();
    }

  private:
    std::unique_ptr<MockMixerPlaybackUnit> m_mockMixer;
    bool m_streamIsRunning = false;
};