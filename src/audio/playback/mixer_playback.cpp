#include <cstring> // for std::memset
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <iostream>

namespace dtracker::audio::playback
{

    // Adds a new playback unit to the internal list
    void MixerPlaybackUnit::addUnit(PlaybackUnit *unit)
    {
        m_units.push_back(unit);
    }

    // Mixes all active units into the provided output buffer
    void MixerPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                   unsigned int channels)
    {
        // Start with silence
        std::fill(buffer, buffer + nFrames * channels, 0.0f);

        // Temporary buffer for each unit's output
        std::vector<float> temp(nFrames * channels, 0.0f);

        for (auto it = m_units.begin(); it != m_units.end();)
        {
            std::fill(temp.begin(), temp.end(), 0.0f);     // Zero the temp
            (*it)->render(temp.data(), nFrames, channels); // Render to temp

            // Mix into main buffer
            for (unsigned int i = 0; i < nFrames * channels; ++i)
                buffer[i] += temp[i];

            // Remove finished units
            if ((*it)->isFinished())
            {
                std::cout << "MixerPlaybackUnit: "
                          << "sample finished, erasing\n";
                it = m_units.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // True if no units are left to play
    bool MixerPlaybackUnit::isFinished() const
    {
        return m_units.empty();
    }

} // namespace dtracker::audio::playback
