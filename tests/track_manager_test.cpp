#include <gtest/gtest.h>

#include <dtracker/sample/manager.hpp>
#include <dtracker/tracker/track_manager.hpp>

// Ensure TrackManager initializes properly with a sample manager
TEST(TrackManager, InitializesWithSampleManager)
{
    dtracker::sample::Manager sampleManager;
    dtracker::tracker::TrackManager tm;
    EXPECT_TRUE(tm.allTrackIds().empty());
}

// Create an empty track and verify it is retrievable
TEST(TrackManager, CreatesEmptyTrack)
{
    dtracker::sample::Manager sampleManager;
    dtracker::tracker::TrackManager tm;
    int id = tm.createTrack();
    EXPECT_TRUE(tm.getTrack(id) != nullptr);
}

// Create a track with two valid samples and verify it's not finished
TEST(TrackManager, CreatesTrackWithSamples)
{
    dtracker::sample::Manager sampleManager;
    int s1 = sampleManager.addSample(
        "sample1",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1f, 0.2f, 0.3f}),
        {44100, 32});

    int s2 = sampleManager.addSample(
        "sample2",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.4f, 0.5f}),
        {44100, 32});
    dtracker::tracker::TrackManager tm;
    int id = tm.createTrack({s1, s2});
    auto *track = tm.getTrack(id);
    EXPECT_TRUE(track != nullptr);
    EXPECT_FALSE(track->isFinished()); // Should contain valid sample units
}

// Ensure tracks can be removed and are no longer accessible
TEST(TrackManager, RemovesTrack)
{
    dtracker::sample::Manager sampleManager;
    dtracker::tracker::TrackManager tm;
    int id = tm.createTrack();
    EXPECT_TRUE(tm.removeTrack(id));
    EXPECT_EQ(tm.getTrack(id), nullptr);
}

// Querying a non-existent track should return nullptr
TEST(TrackManager, GetNonExistentTrackReturnsNull)
{
    dtracker::sample::Manager sampleManager;
    dtracker::tracker::TrackManager tm;
    EXPECT_EQ(tm.getTrack(9999), nullptr);
}

// Track IDs should accurately reflect currently stored tracks
TEST(TrackManager, AllTrackIdsReflectsContents)
{
    dtracker::sample::Manager sampleManager;
    dtracker::tracker::TrackManager tm;
    int a = tm.createTrack();
    int b = tm.createTrack();
    auto ids = tm.allTrackIds();
    EXPECT_EQ(ids.size(), 2);
    EXPECT_NE(std::find(ids.begin(), ids.end(), a), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), b), ids.end());
}

TEST(TrackManager, CanAddSamplesToExistingTrack)
{
    dtracker::sample::Manager sampleManager;
    int s1 = sampleManager.addSample(
        "sample1",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1f, 0.2f, 0.3f}),
        {44100, 32});
    dtracker::tracker::TrackManager tm;

    int trackId = tm.createTrack();
    EXPECT_TRUE(tm.addSamplesToTrack(trackId, {s1}));

    auto *track = tm.getTrack(trackId);
    ASSERT_NE(track, nullptr);
    EXPECT_FALSE(track->isFinished()); // It now has one sample
}
