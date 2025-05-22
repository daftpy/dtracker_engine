#include <gtest/gtest.h>

#include <engine/audio_engine.hpp>

TEST(AudioEngine, StartsSuccessfully)
{
    dtracker::engine::AudioEngine engine;
    EXPECT_TRUE(engine.start()) << "No usable output device found";

    EXPECT_TRUE(engine.isStreamOpen()) << "Stream failed to open";

    EXPECT_TRUE(engine.isStreamRunning()) << "Stream stopped running";
}

TEST(AudioEngine, StopsAndClosesSteam)
{
    dtracker::engine::AudioEngine engine;
    ASSERT_TRUE(engine.start());

    engine.stop();

    EXPECT_FALSE(engine.isStreamRunning()) << "Stream should be stopped";
    EXPECT_FALSE(engine.isStreamOpen()) << "Stream should be closed";
}

TEST(AudioEngineDeviceTest, DeviceInfoHasOutputChannels)
{
    dtracker::engine::AudioEngine engine;
    ASSERT_TRUE(engine.start());

    const auto &info = engine.currentDeviceInfo();
    EXPECT_GT(info.outputChannels, 0)
        << "Expected devicee to have output channels";
}
