#pragma once
#include <vector>

struct ActivePattern
{
    std::vector<int> steps;
    float stepIntervalMs;
    float elapsedMs = 0.0f;
    size_t currentStep = 0;
};
