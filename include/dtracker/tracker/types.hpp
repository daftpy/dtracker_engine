#pragma once

#include <dtracker/tracker/types/active_pattern.hpp>
#include <memory>
#include <string>
#include <vector>

namespace dtracker::tracker::types
{
    struct ActivePattern
    {
        std::vector<int> steps;
        float stepIntervalMs;

        // ADD: The musical timing of the pattern.
        // e.g., 4 = 16th notes, 8 = 32nd notes.
        float stepsPerBeat = 4.0f;

        float elapsedMs = 0.0f;
        size_t currentStep = 0;
    };

    struct Track
    {
        int id{-1};
        std::string name{"New Track"};
        float volume = 1.0f;
        float pan = 0.0f;

        // A sequence of patterns
        std::vector<ActivePattern> patterns;

        explicit Track(const std::string &name) : name(name) {}
    };
} // namespace dtracker::tracker::types