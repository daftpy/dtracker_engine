#include <gtest/gtest.h>

#include <dtracker/tracker/track_manager.hpp>


using namespace dtracker::tracker;

// Verifies that creating a track returns a valid ID and the track is
// retrievable.
TEST(TrackManager, CreateTrackSucceeds)
{
    TrackManager tm;
    int id = tm.createTrack("My Test Track");

    // The first track should have ID 0.
    EXPECT_EQ(id, 0);

    // We should be able to retrieve the track we just created.
    auto trackPtr = tm.getTrack(id);
    ASSERT_NE(trackPtr, nullptr);
    EXPECT_EQ(trackPtr->id, 0);
    EXPECT_EQ(trackPtr->name, "My Test Track");
}

// Verifies that getting a non-existent track returns a null pointer.
TEST(TrackManager, GetInvalidTrackReturnsNull)
{
    TrackManager tm;
    EXPECT_EQ(tm.getTrack(999), nullptr);
}

// Verifies that patterns can be successfully added to an existing track.
TEST(TrackManager, AddPatternToTrackSucceeds)
{
    TrackManager tm;
    int trackId = tm.createTrack();

    types::ActivePattern pattern;
    pattern.steps = {1, 2, 3, 4};

    // Act: Add the pattern to the track.
    bool success = tm.addPatternToTrack(trackId, pattern);
    ASSERT_TRUE(success);

    // Assert: The track should now contain one pattern.
    auto trackPtr = tm.getTrack(trackId);
    ASSERT_NE(trackPtr, nullptr);
    EXPECT_EQ(trackPtr->patterns.size(), 1);
    EXPECT_EQ(trackPtr->patterns[0].steps.size(), 4);
}

// Verifies that adding a pattern to a non-existent track fails.
TEST(TrackManager, AddPatternToInvalidTrackFails)
{
    TrackManager tm;
    types::ActivePattern pattern;

    bool success = tm.addPatternToTrack(999, pattern); // 999 is not a valid ID.
    EXPECT_FALSE(success);
}

// Verifies that removing a track works correctly.
TEST(TrackManager, RemoveTrackSucceeds)
{
    TrackManager tm;
    int id = tm.createTrack();
    ASSERT_NE(tm.getTrack(id), nullptr); // Verify it exists first.

    // Act & Assert
    EXPECT_TRUE(tm.removeTrack(id));
    EXPECT_EQ(tm.getTrack(id), nullptr); // Should be gone now.
    EXPECT_FALSE(tm.removeTrack(id));    // Removing it again should fail.
}

// Verifies that getAllTrackIds returns an accurate list of current track IDs.
TEST(TrackManager, GetAllTrackIdsReflectsContents)
{
    TrackManager tm;
    int id1 = tm.createTrack();
    int id2 = tm.createTrack();

    auto ids = tm.getAllTrackIds();
    ASSERT_EQ(ids.size(), 2);
    // Use find to check for presence regardless of order.
    EXPECT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());

    tm.removeTrack(id1);
    ids = tm.getAllTrackIds();
    ASSERT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], id2);
}