#include <gtest/gtest.h>

#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>
#include <dtracker/audio/types.hpp>
#include <dtracker/sample/manager.hpp>
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
// TEST(PlaybackManager, StartsTonePlayback)
// {
//     RtAudio audio;
//     dtracker::audio::DeviceManager manager(&audio);
//     auto infoOpt = manager.currentDeviceInfo();
//     ASSERT_TRUE(infoOpt.has_value());

//     dtracker::audio::Engine engine;
//     engine.setOutputDevice(infoOpt->ID);

//     engine.start();

//     dtracker::audio::PlaybackManager pm(&engine, nullptr);
//     pm.playTestTone(220.0f); // A3

//     EXPECT_TRUE(pm.isPlaying()) << "PlaybackManager failed to start
//     playback"; EXPECT_TRUE(engine.isStreamRunning())
//         << "Engine stream not running after playTestTone";
// }

TEST(PlaybackManager, StopsPlayback)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);
    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value());

    dtracker::audio::Engine engine;
    engine.setOutputDevice(infoOpt->ID);

    engine.start();

    // Create a sample descriptor with some dummy data.
    dtracker::sample::types::SampleDescriptor descriptor{
        -1,
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1, 0.2, 0.3, 0.4}),
        {44100, 16}};

    dtracker::audio::PlaybackManager pm(&engine);

    auto unit =
        dtracker::audio::playback::makePlaybackUnit(std::move(descriptor));

    pm.playSample(std::move(unit));
    ASSERT_TRUE(pm.isPlaying());

    pm.stopPlayback(); // Now stop it

    EXPECT_FALSE(pm.isPlaying())
        << "PlaybackManager still reports playing after stop";
}
