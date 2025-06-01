#include <gtest/gtest.h>

#include "mocks/mock_playback_unit.hpp"
#include "mocks/mock_stereo_unit.hpp"
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <dtracker/audio/playback/proxy_playback_unit.hpp>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback/track_playback_unit.hpp>

using namespace dtracker::audio;

// -------------------------
// ProxyPlaylayback Tests
// -------------------------
TEST(ProxyPlaybackUnit, DelegatesRenderCall)
{
    playback::ProxyPlaybackUnit proxy;

    MockPlaybackUnit mock;
    proxy.setDelegate(&mock);

    float dummyBuffer[64] = {0};
    proxy.render(dummyBuffer, 32, 2); // Render one block

    EXPECT_EQ(mock.renderCallCount, 1)
        << "ProxyPlaybackUnit did not forward render() call to delegate";
}

TEST(ProxyPlaybackUnit, ThreadSafeDelegateAccess)
{
    playback::ProxyPlaybackUnit proxy;
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
    playback::ProxyPlaybackUnit proxy;

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

// -------------------------
// SamplePlaylayback Tests
// -------------------------
TEST(SamplePlaybackTest, CompletesPlaybackAndResets)
{
    std::vector<float> pcm(100, 0.5f); // 50 stereo frames (100 samples)
    playback::SamplePlaybackUnit unit(pcm, 44100);

    std::vector<float> buffer(100, 0.0f);
    EXPECT_FALSE(unit.isFinished());

    // First render: full buffer
    unit.render(buffer.data(), 50, 2); // 50 frames, 2 channels
    EXPECT_TRUE(unit.isFinished());

    // Reset and check it's ready again
    unit.reset();
    EXPECT_FALSE(unit.isFinished());

    // Second render: still works after reset
    std::fill(buffer.begin(), buffer.end(), 0.0f); // clear buffer
    unit.render(buffer.data(), 50, 2);
    EXPECT_TRUE(unit.isFinished());

    // Spot-check audio content (optional)
    for (float sample : buffer)
        EXPECT_NEAR(sample, 0.5f, 0.0001f);
}

TEST(SamplePlaybackTest, PadsWithSilenceIfBufferLargerThanSample)
{
    std::vector<float> pcm = {1.0f, 1.0f}; // 1 stereo frame
    playback::SamplePlaybackUnit unit(pcm, 44100);

    std::vector<float> buffer(6, -1.0f); // Request 3 frames
    unit.render(buffer.data(), 3, 2);

    // Expect: [1.0, 1.0, 0.0, 0.0, 0.0, 0.0]
    EXPECT_EQ(buffer[0], 1.0f);
    EXPECT_EQ(buffer[1], 1.0f);
    for (size_t i = 2; i < buffer.size(); ++i)
        EXPECT_EQ(buffer[i], 0.0f);
}

TEST(SamplePlaybackUnit, ResetRestartsPlaybackFromBeginning)
{
    std::vector<float> pcm(20, 0.25f); // 10 stereo frames
    playback::SamplePlaybackUnit unit(pcm, 44100);

    std::vector<float> buffer(20, 0.0f);
    unit.render(buffer.data(), 10, 2);
    EXPECT_TRUE(unit.isFinished());

    unit.reset();
    EXPECT_FALSE(unit.isFinished());

    std::fill(buffer.begin(), buffer.end(), 0.0f);
    unit.render(buffer.data(), 10, 2);

    for (float sample : buffer)
        EXPECT_NEAR(sample, 0.25f, 0.0001f);
}

// -------------------------
// MixerPlaylayback Tests
// -------------------------
TEST(MixerPlaybackUnit, RendersSilenceWhenEmpty)
{
    playback::MixerPlaybackUnit mixer;

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
    auto mock = std::make_unique<MockPlaybackUnit>();
    mock->fillValue = 0.5f;
    mock->renderCallCount = 1;

    playback::MixerPlaybackUnit mixer;
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

    playback::MixerPlaybackUnit mixer;
    mixer.addUnit(mock.get());

    float buffer[64];
    mixer.render(buffer, 32, 2); // First render, removes finished unit

    EXPECT_TRUE(mixer.isFinished());
}

TEST(MixerPlaybackUnit, ClearRemovesAllUnits)
{
    auto unit = std::make_unique<MockPlaybackUnit>();
    unit->fillValue = 1.0f;

    playback::MixerPlaybackUnit mixer;
    mixer.addUnit(unit.get());

    EXPECT_FALSE(mixer.isFinished());

    mixer.clear(); // Explicit clear

    EXPECT_TRUE(mixer.isFinished());

    float buffer[64];
    std::fill_n(buffer, 64, -1.0f); // ensure it's affected
    mixer.render(buffer, 32, 2);

    for (float sample : buffer)
        EXPECT_FLOAT_EQ(sample, 0.0f); // Confirm silence
}

// -------------------------
// TrackPlaylayback Tests
// -------------------------
TEST(TrackPlaybackUnit, RendersSilenceWhenEmpty)
{
    playback::TrackPlaybackUnit track;

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

    playback::TrackPlaybackUnit track;
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
    auto mockLeft = std::make_unique<MockPlaybackUnit>();
    mockLeft->fillValue = 1.0f;

    playback::TrackPlaybackUnit trackLeft;
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

    playback::TrackPlaybackUnit trackRight;
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

TEST(TrackPlaybackUnit, ResetResetsAllSamples)
{
    std::vector<float> pcm(20, 0.75f);
    auto sample = std::make_unique<playback::SamplePlaybackUnit>(pcm, 44100);

    playback::TrackPlaybackUnit track;
    track.addSample(sample.get());

    float buffer[20];
    track.render(buffer, 5, 2);
    EXPECT_FALSE(track.isFinished());

    track.render(buffer, 5, 2);
    EXPECT_TRUE(track.isFinished());

    track.reset(); // should reset sample too
    EXPECT_FALSE(track.isFinished());
}
