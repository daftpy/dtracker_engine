#include <gtest/gtest.h>

#include <engine/audio_engine.hpp>

TEST(AudioEngine, StartsSuccessfully)
{
    dtracker::engine::AudioEngine engine;
    EXPECT_TRUE(engine.start()) << "No usable output device found";
}

TEST(AudioEngineDeviceTest, DeviceInfoHasOutputChannels)
{
    dtracker::engine::AudioEngine engine;
    ASSERT_TRUE(engine.start());

    const auto &info = engine.currentDeviceInfo();
    EXPECT_GT(info.outputChannels, 0)
        << "Expected devicee to have output channels";
}