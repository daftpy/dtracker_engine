#include <gtest/gtest.h>

#include "mocks/mock_playback_unit.hpp"
#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/playback_manager.hpp>
#include <thread>

// -------------------------
// AudioEngine Integration Tests
// -------------------------

TEST(AudioEngine, StartsSuccessfully)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);

    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value()) << "No usable output device found";

    dtracker::audio::Engine engine;

    EXPECT_TRUE(engine.openStream(infoOpt->ID))
        << "AudioEngine failed to start with selected device";

    EXPECT_TRUE(engine.isStreamOpen()) << "Stream failed to open";
    EXPECT_TRUE(engine.isStreamRunning()) << "Stream not running";
}

TEST(AudioEngine, StopsAndClosesStream)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);

    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value()) << "No usable output device found";

    dtracker::audio::Engine engine;
    engine.setOutputDevice(infoOpt->ID);

    // Must be set for the engine to start
    engine.setPlaybackUnit(
        std::make_unique<dtracker::audio::playback::TonePlayback>(440.0f,
                                                                  44100.0f));

    ASSERT_TRUE(engine.start()) << "Failed to start engine";

    engine.stop();

    EXPECT_FALSE(engine.isStreamRunning()) << "Stream should be stopped";
    EXPECT_FALSE(engine.isStreamOpen()) << "Stream should be closed";
}

// -------------------------
// DeviceManager Unit Tests
// -------------------------

TEST(DeviceManager, ReturnsValidDefaultOutputDevice)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);

    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value())
        << "Expected DeviceManager to return a valid output device";

    const auto &info = *infoOpt;
    EXPECT_GT(info.outputChannels, 0)
        << "Expected output device to have output channels";
}

// -------------------------
// PlaybackManager Tests
// -------------------------
TEST(PlaybackManager, StartsTonePlayback)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);
    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value());

    dtracker::audio::Engine engine;
    engine.setOutputDevice(infoOpt->ID);

    engine.start();

    dtracker::audio::PlaybackManager pm(&engine);
    pm.playTestTone(220.0f); // A3

    EXPECT_TRUE(pm.isPlaying()) << "PlaybackManager failed to start playback";
    EXPECT_TRUE(engine.isStreamRunning())
        << "Engine stream not running after playTestTone";
}

TEST(PlaybackManager, StopsPlayback)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);
    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value());

    dtracker::audio::Engine engine;
    engine.setOutputDevice(infoOpt->ID);

    engine.start();

    dtracker::audio::PlaybackManager pm(&engine);
    pm.playTestTone(); // Start playback first
    ASSERT_TRUE(pm.isPlaying());

    pm.stopPlayback(); // Now stop it

    EXPECT_FALSE(pm.isPlaying())
        << "PlaybackManager still reports playing after stop";
}

// -------------------------
// Playback Tests
// -------------------------
TEST(ProxyPlaybackUnit, DelegatesRenderCall)
{
    dtracker::audio::playback::ProxyPlaybackUnit proxy;

    MockPlaybackUnit mock;
    proxy.setDelegate(&mock);

    float dummyBuffer[64] = {0};
    proxy.render(dummyBuffer, 32, 2); // Render one block

    EXPECT_EQ(mock.renderCallCount, 1)
        << "ProxyPlaybackUnit did not forward render() call to delegate";
}

TEST(ProxyPlaybackUnit, ThreadSafeDelegateAccess)
{
    dtracker::audio::playback::ProxyPlaybackUnit proxy;
    MockPlaybackUnit mockA;
    MockPlaybackUnit mockB;

    // Writer thread: flip between two delegates
    std::atomic<bool> done = false;
    std::thread writer(
        [&]
        {
            for (int i = 0; i < 1000; ++i)
            {
                proxy.setDelegate(i % 2 == 0 ? &mockA : &mockB);
            }
            done = true;
        });

    // Render repeatedly while delegate is changing
    std::thread reader(
        [&]
        {
            float dummy[128];
            while (!done)
            {
                proxy.render(dummy, 64, 2);
            }
        });

    writer.join();
    reader.join();

    // Ensure no crashes occurred
    SUCCEED();
}

TEST(ProxyPlaybackUnit, NullDelegateRendersSilence)
{
    dtracker::audio::playback::ProxyPlaybackUnit proxy;

    // Fill with non-zero so we can check
    float buffer[64];
    std::fill_n(buffer, 64, 1.0f);

    // No delegate to zero the buffer
    proxy.setDelegate(nullptr);

    // 32 frames, 2 channels
    proxy.render(buffer, 32, 2);

    for (float sample : buffer)
        EXPECT_EQ(sample, 0.0f) << "Expected silence from null delegate";
}