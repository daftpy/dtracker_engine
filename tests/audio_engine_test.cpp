#include <gtest/gtest.h>

#include "mocks/mock_playback_unit.hpp"
#include "mocks/mock_stereo_unit.hpp"
#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>
#include <dtracker/audio/sample_manager.hpp>
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
TEST(PlaybackManager, StartsTonePlayback)
{
    RtAudio audio;
    dtracker::audio::DeviceManager manager(&audio);
    auto infoOpt = manager.currentDeviceInfo();
    ASSERT_TRUE(infoOpt.has_value());

    dtracker::audio::Engine engine;
    engine.setOutputDevice(infoOpt->ID);

    engine.start();

    dtracker::audio::PlaybackManager pm(&engine, nullptr);
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

    dtracker::audio::PlaybackManager pm(&engine, nullptr);
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

TEST(SampleManager, AddReturnsUniqueIds)
{
    dtracker::audio::SampleManager manager;
    int id1 = manager.addSample({0.1f, 0.2f, 0.3f}, 44100);
    int id2 = manager.addSample({0.4f, 0.5f}, 44100);
    EXPECT_NE(id1, id2);
}

// Disabled: This test expects SampleManager::getSample() to return distinct
// copies. Currently, it returns raw pointers to owned instances. I'll revisit
// this later if I decide to make SampleManager produce clones.
// TEST(SampleManager, GetSampleReturnsCopy)
// {
//     dtracker::audio::SampleManager manager;
//     std::vector<float> pcm = {0.1f, 0.2f, 0.3f};
//     int id = manager.addSample(pcm, 44100);

//     auto sample1 = manager.getSample(id);
//     auto sample2 = manager.getSample(id);

//     EXPECT_NE(sample1, sample2); // Distinct objects
//     EXPECT_TRUE(sample1 != nullptr);
//     EXPECT_TRUE(sample2 != nullptr);

//     // Ensure deep copy by comparing internal data
//     auto *sp1 =
//         dynamic_cast<dtracker::audio::playback::SamplePlayback *>(sample1);
//     auto *sp2 =
//         dynamic_cast<dtracker::audio::playback::SamplePlayback *>(sample2);

//     ASSERT_NE(sp1, nullptr);
//     ASSERT_NE(sp2, nullptr);

//     EXPECT_EQ(sp1->sampleRate(), sp2->sampleRate());
//     EXPECT_EQ(sp1->data().size(), sp2->data().size());
//     EXPECT_EQ(sp1->data(), sp2->data());
// }

TEST(SampleManager, GetSampleReturnsNullOnInvalidId)
{
    dtracker::audio::SampleManager manager;
    auto sample = manager.getSample(999); // Never added
    EXPECT_EQ(sample, nullptr);
}

TEST(SampleManager, RemoveSampleDeletesIt)
{
    dtracker::audio::SampleManager manager;
    int id = manager.addSample({0.1f, 0.2f}, 44100);
    EXPECT_TRUE(manager.removeSample(id));
    EXPECT_FALSE(manager.removeSample(id)); // Already removed
    EXPECT_EQ(manager.getSample(id), nullptr);
}

TEST(SampleManager, AllSampleIdsReflectsContents)
{
    dtracker::audio::SampleManager manager;
    int id1 = manager.addSample({0.1f}, 44100);
    int id2 = manager.addSample({0.2f}, 44100);

    std::vector<int> ids = manager.allSampleIds();
    EXPECT_EQ(ids.size(), 2);
    EXPECT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());

    manager.removeSample(id1);
    ids = manager.allSampleIds();
    EXPECT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], id2);
}

TEST(MixerPlaybackUnit, RendersSilenceWhenEmpty)
{
    dtracker::audio::playback::MixerPlaybackUnit mixer;

    float buffer[64];
    std::fill_n(buffer, 64, 1.0f); // Pre-fill with non-zero
    mixer.render(buffer, 32, 2);   // 32 frames, 2 channels

    for (float sample : buffer)
    {
        EXPECT_FLOAT_EQ(sample, 0.0f) << "Expected silence when empty";
    }
}

TEST(MixerPlaybackUnit, MixesSingleUnitCorrectly)
{
    using namespace dtracker::audio::playback;

    auto mock = std::make_unique<MockPlaybackUnit>();
    mock->fillValue = 0.5f;
    mock->renderCallCount = 1;

    MixerPlaybackUnit mixer;
    mixer.addUnit(mock.get());

    float buffer[64];
    mixer.render(buffer, 32, 2);

    for (float sample : buffer)
    {
        EXPECT_FLOAT_EQ(sample, 0.5f);
    }
}

TEST(MixerPlaybackUnit, RemovesFinishedUnits)
{
    auto mock = std::make_unique<MockPlaybackUnit>();
    mock->fillValue = 0.1f;
    mock->finishedAfterRender = true;

    dtracker::audio::playback::MixerPlaybackUnit mixer;
    mixer.addUnit(mock.get());

    float buffer[64];
    mixer.render(buffer, 32, 2); // First render, removes finished unit

    EXPECT_TRUE(mixer.isFinished());
}

// TrackPlaybackUnit Tests
TEST(TrackPlaybackUnit, RendersSilenceWhenEmpty)
{
    dtracker::audio::playback::TrackPlaybackUnit track;

    float buffer[64];
    std::fill_n(buffer, 64, 1.0f); // Pre-fill to detect changes
    track.render(buffer, 32, 2);   // 32 frames, 2 channels

    for (float sample : buffer)
        EXPECT_FLOAT_EQ(sample, 0.0f);
}

TEST(TrackPlaybackUnit, AppliesVolumeAndPanCorrectly)
{
    auto unit = std::make_unique<MockStereoUnit>();
    unit->leftValue = 1.0f;
    unit->rightValue = 1.0f;

    dtracker::audio::playback::TrackPlaybackUnit track;
    track.setVolume(0.5f); // Halve output
    track.setPan(-1.0f);   // Full left

    track.addSample(unit.get());

    float buffer[64] = {};
    track.render(buffer, 32, 2);

    for (unsigned int i = 0; i < 32; ++i)
    {
        EXPECT_FLOAT_EQ(buffer[i * 2], 0.5f);     // Left
        EXPECT_FLOAT_EQ(buffer[i * 2 + 1], 0.0f); // Right (panned away)
    }
}

TEST(TrackPlaybackUnit, PanBiasesLeftOrRight)
{
    using namespace dtracker::audio::playback;

    auto mockLeft = std::make_unique<MockPlaybackUnit>();
    mockLeft->fillValue = 1.0f;

    TrackPlaybackUnit trackLeft;
    trackLeft.setVolume(1.0f);
    trackLeft.setPan(-1.0f); // Fully left
    trackLeft.addSample(mockLeft.get());

    float leftBuffer[64] = {};
    trackLeft.render(leftBuffer, 32, 2);

    for (unsigned int i = 0; i < 64; i += 2)
    {
        EXPECT_FLOAT_EQ(leftBuffer[i], 1.0f);     // Left channel
        EXPECT_FLOAT_EQ(leftBuffer[i + 1], 0.0f); // Right channel
    }

    auto mockRight = std::make_unique<MockPlaybackUnit>();
    mockRight->fillValue = 1.0f;

    TrackPlaybackUnit trackRight;
    trackRight.setVolume(1.0f);
    trackRight.setPan(1.0f); // Fully right
    trackRight.addSample(mockRight.get());

    float rightBuffer[64] = {};
    trackRight.render(rightBuffer, 32, 2);

    for (unsigned int i = 0; i < 64; i += 2)
    {
        EXPECT_FLOAT_EQ(rightBuffer[i], 0.0f);     // Left channel
        EXPECT_FLOAT_EQ(rightBuffer[i + 1], 1.0f); // Right channel
    }
}
