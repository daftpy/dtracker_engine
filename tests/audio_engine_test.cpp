#include <gtest/gtest.h>

#include <engine/audio_engine.hpp>
#include <engine/device_manager.hpp>


// -------------------------
// AudioEngine Integration Tests
// -------------------------

TEST(AudioEngine, StartsSuccessfully)
{
    dtracker::engine::AudioEngine engine;
    EXPECT_TRUE(engine.start())
        << "AudioEngine failed to start: No usable output device found";

    EXPECT_TRUE(engine.isStreamOpen()) << "Stream failed to open";
    EXPECT_TRUE(engine.isStreamRunning())
        << "Stream stopped running unexpectedly";
}

TEST(AudioEngine, StopsAndClosesStream)
{
    dtracker::engine::AudioEngine engine;
    ASSERT_TRUE(engine.start());

    engine.stop();

    EXPECT_FALSE(engine.isStreamRunning())
        << "Stream should be stopped after stop()";
    EXPECT_FALSE(engine.isStreamOpen())
        << "Stream should be closed after stop()";
}

TEST(AudioEngine, ProvidesValidDeviceInfo)
{
    dtracker::engine::AudioEngine engine;
    ASSERT_TRUE(engine.start());

    auto infoOpt = engine.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value())
        << "Expected a valid device info after start()";

    const auto &info = *infoOpt;
    EXPECT_GT(info.outputChannels, 0)
        << "Expected device to have output channels";
}

// -------------------------
// DeviceManager Unit Tests
// -------------------------

TEST(DeviceManager, InitializesWithValidDevice)
{
    RtAudio audio;
    dtracker::engine::DeviceManager manager(&audio);

    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value())
        << "Expected DeviceManager to select a valid output device";

    const auto &info = *infoOpt;
    EXPECT_GT(info.outputChannels, 0)
        << "Expected output device to have channels";
}
