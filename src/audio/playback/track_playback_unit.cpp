#include <algorithm> // For std::fill
#include <cmath>     // For std::max/min
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{
    // Renamed from addSample to addUnit to be more generic.
    void TrackPlaybackUnit::addUnit(std::unique_ptr<PatternPlaybackUnit> unit)
    {
        if (unit)
            m_units.push_back(std::move(unit));
    }

    void TrackPlaybackUnit::setVolume(float v)
    {
        m_volume = std::clamp(v, 0.0f, 1.0f);
    }

    void TrackPlaybackUnit::setPan(float p)
    {
        m_pan = std::clamp(p, -1.0f, 1.0f);
    }

    // This is the new, sequential rendering logic.
    void TrackPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                   unsigned int channels,
                                   const types::RenderContext &context)
    {
        // First, check if we've already finished the entire sequence.
        if (isFinished() || m_units.size() == 0)
        {
            std::fill(buffer, buffer + nFrames * channels, 0.0f);
            return;
        }

        // Get the current pattern.
        auto &currentUnit = m_units[m_currentUnitIndex];
        currentUnit->render(buffer, nFrames, channels, context);

        // --- Apply this track's own volume and pan to the pattern's output ---
        if (channels == 2)
        {
            float leftGain = m_volume * (1.0f - std::max(0.0f, m_pan));
            float rightGain = m_volume * (1.0f + std::min(0.0f, m_pan));

            for (unsigned int i = 0; i < nFrames; ++i)
            {
                buffer[i * 2] *= leftGain;
                buffer[i * 2 + 1] *= rightGain;
            }
        }

        // --- Check if the current pattern has finished its loop ---
        if (currentUnit->hasFinishedLoop())
        {
            // ...and looping is on, reset the index back to the beginning.
            if (context.isLooping)
            {
                // Reset the current pattern unit
                currentUnit->reset();

                // Stick to the same/first pattern
                m_currentUnitIndex = 0;
            }
            // If we have more patterns to play
            else if (m_currentUnitIndex < m_units.size() - 1)
            {
                // Reset the current pattern unit
                currentUnit->reset();

                // Move to the next pattern unit
                m_currentUnitIndex++;
            }
        }
    }

    void TrackPlaybackUnit::reset()
    {
        // When the track is reset, go back to the first pattern in the
        // sequence.
        m_currentUnitIndex = 0;
        // Also reset all the patterns it contains.
        for (auto &unit : m_units)
        {
            if (unit)
                unit->reset();
        }
    }

    bool TrackPlaybackUnit::isFinished() const
    {
        // The track is finished once it has played all the patterns in its
        // sequence.
        return m_currentUnitIndex >= m_units.size() - 1 &&
               m_units[m_currentUnitIndex]
                   ->isFinished(); // ensure the current pattern is done
                                   // rendering before reporting finished
    }

    void TrackPlaybackUnit::setLooping(bool shouldLoop)
    {
        m_isLooping = shouldLoop;
    }

} // namespace dtracker::audio::playback