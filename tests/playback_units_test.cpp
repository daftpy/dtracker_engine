#include <gtest/gtest.h>

#include <dtracker/audio/playback/sample_playback.hpp>

TEST(SamplePlaybackTest, CompletesPlaybackAndResets)
{
    std::vector<float> pcm(100, 0.5f); // 50 stereo frames (100 samples)
    dtracker::audio::playback::SamplePlaybackUnit unit(pcm, 44100);

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
    dtracker::audio::playback::SamplePlaybackUnit unit(pcm, 44100);

    std::vector<float> buffer(6, -1.0f); // Request 3 frames
    unit.render(buffer.data(), 3, 2);

    // Expect: [1.0, 1.0, 0.0, 0.0, 0.0, 0.0]
    EXPECT_EQ(buffer[0], 1.0f);
    EXPECT_EQ(buffer[1], 1.0f);
    for (size_t i = 2; i < buffer.size(); ++i)
        EXPECT_EQ(buffer[i], 0.0f);
}