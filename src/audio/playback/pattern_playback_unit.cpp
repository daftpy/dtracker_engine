#include <algorithm> // For std::fill
#include <dtracker/audio/playback/pattern_playback_unit.hpp>
#include <iostream>
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
                                     unsigned int channels,
                                     const types::RenderContext &context)
    {
        // Ensure we have the tools we need to work.
        if (!m_sampleUnitPool || m_pattern.steps.empty())
        {
            std::fill(buffer, buffer + nFrames * channels, 0.0f);
            return;
        }

        // --- DYNAMIC TIMING CALCULATION ---
        // Calculate the duration of a single beat in milliseconds from the BPM.
        const float msPerBeat = 60000.0f / context.bpm;
        // Calculate the duration of a single step for this specific pattern.
        const float stepIntervalMs = msPerBeat / m_pattern.stepsPerBeat;

        // --- 1. SCHEDULING LOGIC ---
        // Only schedule new notes if the pattern hasn't completed its first
        // full loop.

        // Use the render call as a high-precision timer.
        float deltaTimeMs =
            (static_cast<float>(nFrames) / m_sampleRate) * 1000.0f;
        m_pattern.elapsedMs += deltaTimeMs;

        // Get all the steps that fall within the current time window, use the
        // blue print to recyce playback units and add them to active notes
        // Note: Only build notes when the currentStep reader is <
        // pattern.steps.size() When the playback manager is looping, it'll
        // reset the current step to start this process again
        while (m_pattern.elapsedMs >= stepIntervalMs &&
               m_pattern.currentStep < m_pattern.steps.size())
        {
            // Get the sample to play
            int sampleIdToPlay = m_pattern.steps[m_pattern.currentStep];

            if (sampleIdToPlay >= 0) // A non-negative ID is a note, not a rest.
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

            // If we process the last step, mark that we have looped once.
            if (m_pattern.currentStep >= m_pattern.steps.size() &&
                !m_hasFinishedOneLoop)
            {
                // Let the track playback unit reset the current step
                // m_pattern.currentStep = 0;
                m_hasFinishedOneLoop = true;

                std::cout << "elapsed time " << m_pattern.elapsedMs << "\n";
            }
            // Prevents jitter
            m_pattern.elapsedMs -= stepIntervalMs;
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
            (*it)->render(temp.data(), nFrames, channels, context);

            // Additively mix the note's audio into our main output buffer.
            for (unsigned int i = 0; i < nFrames * channels; ++i)
            {
                buffer[i] += temp[i];
            }

            // If a note has finished playing, remove it from the active list.
            if ((*it)->isFinished())
            {
                // Erasing the shared_ptr triggers its custom deleter, which
                // returns the object to the UnitPool.
                it = m_activeNotes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Resets the pattern to its initial state.
    // Do not clear the m_activeNotes to sustain tails from the end of the
    // pattern into the beginning on loops.
    void PatternPlaybackUnit::reset()
    {
        m_pattern.currentStep = 0;
        m_pattern.elapsedMs = 0.0f;
        m_hasFinishedOneLoop = false;
    }

    // The pattern is only truly finished after it has completed its sequence
    // AND all the notes it triggered have finished their playback (the "tail").
    bool PatternPlaybackUnit::isFinished() const
    {
        return m_hasFinishedOneLoop && m_activeNotes.empty();
    }

    // Informs whether the pattern has finished sequencing one loop
    bool dtracker::audio::playback::PatternPlaybackUnit::hasFinishedLoop() const
    {
        return m_hasFinishedOneLoop;
    }

} // namespace dtracker::audio::playback