#include <gtest/gtest.h>

#include <engine/audio_engine.hpp>

TEST(AudioEngineTest, StartsSuccessfully)
{
    dtracker::engine::AudioEngine engine;
    EXPECT_TRUE(engine.start());
}