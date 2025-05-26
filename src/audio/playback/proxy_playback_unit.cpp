// proxy_playback_unit.cpp
#include <algorithm>
#include <dtracker/audio/playback/proxy_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{

    // Renders audio from the currently active playback unit.
    // If no unit is set, fills the buffer with silence.
    void ProxyPlaybackUnit::render(float *buffer, unsigned int frames,
                                   unsigned int channels)
    {
        PlaybackUnit *unit = m_delegate.load(std::memory_order_acquire);
        if (unit)
            unit->render(buffer, frames, channels);
        else
            std::fill(buffer, buffer + (frames * channels), 0.0f);
    }

    // Atomically sets the current delegate playback unit.
    // This can be safely called from another thread (Qt gui).
    void ProxyPlaybackUnit::setDelegate(PlaybackUnit *unit)
    {
        m_delegate.store(unit, std::memory_order_release);
    }

    // Atomically retrieves the current delegate playback unit.
    // Intended for safe access on the audio thread.
    PlaybackUnit *ProxyPlaybackUnit::delegate() const
    {
        return m_delegate.load(std::memory_order_acquire);
    }

    // Returns true if playback is finished or if no delegate is set.
    // Safe for real-time thread polling.
    bool ProxyPlaybackUnit::isFinished() const
    {
        PlaybackUnit *unit = m_delegate.load(std::memory_order_acquire);
        return unit ? unit->isFinished() : true;
    }

} // namespace dtracker::audio::playback
