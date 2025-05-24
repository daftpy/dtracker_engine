// proxy_playback_unit.cpp
#include "audio/playback/proxy_playback_unit.hpp"

#include <algorithm>

namespace dtracker::audio::playback
{

    // Delegates rendering to the attached playback unit, or fills silence if
    // none is set
    void ProxyPlaybackUnit::render(float *buffer, unsigned int frames,
                                   unsigned int channels)
    {
        if (m_delegate)
        {
            // Forward render call to the current delegate
            m_delegate->render(buffer, frames, channels);
        }
        else
        {
            // No delegate means silence/fill buffer with zeros
            std::fill(buffer, buffer + (frames * channels), 0.0f);
        }
    }

    // Sets the current delegate playback unit (non-owning)
    void ProxyPlaybackUnit::setDelegate(PlaybackUnit *unit)
    {
        m_delegate = unit;
    }

    // Returns the currently assigned delegate, or nullptr if none
    PlaybackUnit *ProxyPlaybackUnit::delegate() const
    {
        return m_delegate;
    }

    // Returns whether playback is finished. Returns true if no delegate
    bool ProxyPlaybackUnit::isFinished() const
    {
        return m_delegate ? m_delegate->isFinished() : true;
    }

} // namespace dtracker::audio::playback
