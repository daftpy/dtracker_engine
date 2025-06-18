#include <gtest/gtest.h>

#include "mocks/mock_engine.hpp"
#include "mocks/mock_mixer_playback_unit.hpp"
#include "mocks/mock_sample_manager.hpp"
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>
#include <dtracker/sample/types.hpp>
#include <memory>

// --- Test Fixture ---
// This class sets up a fresh environment for each test case.
class PlaybackManagerTest : public ::testing::Test
{
  protected:
    // Runs before each TEST_F.
    void SetUp() override
    {
        // Start our mock engine to simulate a running audio stream.
        engine.start();

        // Create the PlaybackManager for testing and inject the MockEngine.
        pm = std::make_unique<dtracker::audio::PlaybackManager>(
            &engine, &m_mockSampleManager);
    }

    // Helper function to create a dummy sample unit for testing playback.
    std::unique_ptr<dtracker::audio::playback::SamplePlaybackUnit>
    createDummySample()
    {
        dtracker::sample::types::SampleDescriptor descriptor{
            -1,
            std::make_shared<const dtracker::audio::types::PCMData>(
                dtracker::audio::types::PCMData{0.1, 0.2}),
            {44100, 16}};
        return dtracker::audio::playback::makePlaybackUnit(descriptor);
    }

    MockEngine engine;
    MockSampleManager m_mockSampleManager;
    std::unique_ptr<dtracker::audio::PlaybackManager> pm;
};

// Test that a newly created manager correctly reports it is not playing.
TEST_F(PlaybackManagerTest, IsNotPlayingInitially)
{
    // A new PlaybackManager with an empty engine should not be playing.
    EXPECT_FALSE(pm->isPlaying());
}

// Test that calling playSample correctly starts playback.
TEST_F(PlaybackManagerTest, PlaySampleStartsPlayback)
{
    // Access the mock mixer inside the mock engine.
    auto *mockMixer = engine.getMockMixer();
    ASSERT_NE(mockMixer, nullptr);
    ASSERT_EQ(mockMixer->getUnitCount(), 0); // Should be empty initially.

    // Play a sample.
    pm->playSample(createDummySample());

    // isPlaying() status should be true.
    EXPECT_TRUE(pm->isPlaying());

    // The mock mixer should report the correct unit count.
    EXPECT_EQ(mockMixer->getUnitCount(), 1);
}

// Test that calling stopPlayback correctly stops all sound.
TEST_F(PlaybackManagerTest, StopPlaybackStopsPlayback)
{
    // Start by playing a sample.
    pm->playSample(createDummySample());
    ASSERT_TRUE(pm->isPlaying()); // Verify it's playing first.

    // Stop the playback
    pm->stopPlayback();

    // It should no longer report that it is playing.
    EXPECT_FALSE(pm->isPlaying());
}
