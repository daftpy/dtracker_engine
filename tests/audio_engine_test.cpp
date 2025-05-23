#include <gtest/gtest.h>

#include <audio/device_manager.hpp>
#include <audio/engine.hpp>
#include <audio/playback/tone_playback.hpp>
#include <audio/playback_manager.hpp>

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

    dtracker::audio::PlaybackManager pm(&engine);
    pm.playTestTone(); // Start playback first
    ASSERT_TRUE(pm.isPlaying());

    pm.stopPlayback(); // Now stop it

    EXPECT_FALSE(pm.isPlaying())
        << "PlaybackManager still reports playing after stop";
    EXPECT_FALSE(engine.isStreamRunning())
        << "Engine stream still running after stopPlayback";
}
