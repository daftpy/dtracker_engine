#include <gtest/gtest.h>

#include <engine/audio_engine.hpp>

TEST(AudioEngine, StartsSuccessfully)
{
    dtracker::engine::AudioEngine engine;
    EXPECT_TRUE(engine.start()) << "No usable output device found";
}
