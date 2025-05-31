#include <algorithm>
#include <cmath>
#include <cstring>
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{

    // void TrackPlaybackUnit::addSample(std::unique_ptr<PlaybackUnit> unit)
    // {
    //     m_units.push_back(std::move(unit));
    // }

    void TrackPlaybackUnit::addSample(PlaybackUnit *unit)
    {
        m_units.push_back(unit);
    }

    void TrackPlaybackUnit::setVolume(float v)
    {
        m_volume = std::clamp(v, 0.0f, 1.0f);
    }

    void TrackPlaybackUnit::setPan(float p)
    {
        m_pan = std::clamp(p, -1.0f, 1.0f);
    }

    void TrackPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                   unsigned int channels)
    {
        if (channels != 2)
        {
            std::cerr << "TrackPlaybackUnit only supports stereo output\n";
            return;
        }

        if (m_units.empty())
        {
            std::fill(buffer, buffer + nFrames * channels, 0.0f);
            return;
        }

        std::vector<float> temp(nFrames * channels, 0.0f);

        for (auto it = m_units.begin(); it != m_units.end();)
        {
            std::fill(temp.begin(), temp.end(), 0.0f);
            (*it)->render(temp.data(), nFrames, channels);

            float leftGain = m_volume * (1.0f - std::max(0.0f, m_pan));
            float rightGain = m_volume * (1.0f + std::min(0.0f, m_pan));

            for (unsigned int i = 0; i < nFrames; ++i)
            {
                buffer[i * 2] += temp[i * 2] * leftGain;
                buffer[i * 2 + 1] += temp[i * 2 + 1] * rightGain;
            }

            if ((*it)->isFinished())
                it = m_units.erase(it);
            else
                ++it;
        }
    }

    bool TrackPlaybackUnit::isFinished() const
    {
        return m_units.empty();
    }

} // namespace dtracker::audio::playback
