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
// DeviceManager Tests
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

//     EXPECT_TRUE(pm.isPlaying()) << "PlaybackManager failed to start"
//     "playback"; EXPECT_TRUE(engine.isStreamRunning())
//         << "Engine stream not running after playTestTone";
// }

// TEST(PlaybackManager, StopsPlayback)
// {
//     RtAudio audio;
//     dtracker::audio::DeviceManager manager(&audio);
//     auto infoOpt = manager.currentDeviceInfo();
//     ASSERT_TRUE(infoOpt.has_value());

//     dtracker::audio::Engine engine;
//     engine.setOutputDevice(infoOpt->ID);

//     engine.start();

//     dtracker::audio::PlaybackManager pm(&engine, nullptr);
//     pm.playTestTone(); // Start playback first
//     ASSERT_TRUE(pm.isPlaying());

//     pm.stopPlayback(); // Now stop it

//     EXPECT_FALSE(pm.isPlaying())
//         << "PlaybackManager still reports playing after stop";
// }
