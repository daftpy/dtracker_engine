#include <algorithm> // For std::fill
#include <dtracker/audio/playback/pattern_playback_unit.hpp>
#include <vector>

namespace dtracker::audio::playback
{
    // Constructs the pattern player with all necessary data and dependencies.
    PatternPlaybackUnit::PatternPlaybackUnit(
        const dtracker::tracker::types::ActivePattern &pattern,
        const SampleBlueprint &blueprint, UnitPool *sampleUnitPool,
        unsigned int sampleRate)
        : m_pattern(pattern), m_blueprint(blueprint),
          m_sampleUnitPool(sampleUnitPool), m_sampleRate(sampleRate)
    {
        // Pre-allocate memory for active notes to prevent allocation on the
        // audio thread.
        m_activeNotes.reserve(64);
    }

    // Performs one block of processing. This is the heart of the sequencer.
    void PatternPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                     unsigned int channels)
    {
        // Ensure we have the tools we need to work.
        if (!m_sampleUnitPool || m_pattern.steps.empty())
        {
            std::fill(buffer, buffer + nFrames * channels, 0.0f);
            return;
        }

        // --- 1. SCHEDULING LOGIC ---
        // Only schedule new notes if the pattern hasn't completed its first
        // full loop.
        if (!m_hasFinishedOneLoop)
        {
            // Use the render call as a high-precision timer.
            float deltaTimeMs =
                (static_cast<float>(nFrames) / m_sampleRate) * 1000.0f;
            m_pattern.elapsedMs += deltaTimeMs;

            // Trigger all steps that fall within the current time window.
            while (m_pattern.elapsedMs >= m_pattern.stepIntervalMs)
            {
                int sampleIdToPlay = m_pattern.steps[m_pattern.currentStep];

                if (sampleIdToPlay >=
                    0) // A non-negative ID is a note, not a rest.
                {
                    // Look up the sample's data in our pre-built blueprint.
                    auto blueprintIt = m_blueprint.find(sampleIdToPlay);
                    if (blueprintIt != m_blueprint.end())
                    {
                        // Acquire a recycled player from the pool (fast and
                        // allocation-free).
                        auto unitPtr = m_sampleUnitPool->acquire();
                        if (unitPtr)
                        {
                            // Configure the recycled player with the correct
                            // sample data.
                            unitPtr->reinitialize(blueprintIt->second);
                            // Add it to our internal list of notes that are
                            // currently playing.
                            m_activeNotes.push_back(std::move(unitPtr));
                        }
                    }
                }

                // Advance the sequencer state for the next step.
                m_pattern.currentStep++;
                if (m_pattern.currentStep >= m_pattern.steps.size())
                {
                    m_pattern.currentStep = 0;
                    m_hasFinishedOneLoop =
                        true; // Mark that a full cycle is complete.

                    // FIXES THE CLICK, but doesnt seem to fix tail ring issue
                    m_activeNotes.clear();
                }
                m_pattern.elapsedMs -= m_pattern.stepIntervalMs;
            }
        }

        // --- 2. MIXING LOGIC ---
        // Always render and mix the currently active notes. This allows note
        // tails to continue playing even after the pattern has finished
        // scheduling.
        std::fill(buffer, buffer + nFrames * channels, 0.0f);
        if (m_activeNotes.empty())
        {
            return;
        }

        // TODO: pre-allocate this as a private member to remove the last
        // allocation
        std::vector<float> temp(nFrames * channels, 0.0f);

        for (auto it = m_activeNotes.begin(); it != m_activeNotes.end();)
        {
            // Render the note into the temporary buffer.
            (*it)->render(temp.data(), nFrames, channels);

            // Additively mix the note's audio into our main output buffer.
            for (unsigned int i = 0; i < nFrames * channels; ++i)
            {
                buffer[i] += temp[i];
            }

            // If a note has finished playing, remove it from the active list.
            if ((*it)->isFinished())
            {
                // Erasing the shared_ptr triggers its custom deleter, which
                // automatically returns the object to the UnitPool.
                it = m_activeNotes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Resets the pattern to its initial state.
    void PatternPlaybackUnit::reset()
    {
        m_pattern.currentStep = 0;
        m_pattern.elapsedMs = 0.0f;
        m_hasFinishedOneLoop = false;
        // Clear any notes that were still playing out.
        m_activeNotes.clear();
    }

    // The pattern is only truly finished after it has completed its sequence
    // AND all the notes it triggered have finished their playback (the "tail").
    bool PatternPlaybackUnit::isFinished() const
    {
        // Loops, but sustains tails
        // return m_hasFinishedOneLoop && m_activeNotes.empty();

        // Loops, better, but clicks bad between loops
        return m_hasFinishedOneLoop;
    }

} // namespace dtracker::audio::playback