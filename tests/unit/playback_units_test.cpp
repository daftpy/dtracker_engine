// #include <gtest/gtest.h>

// #include "mocks/mock_pattern_playback_unit.hpp"
// #include "mocks/mock_playback_unit.hpp"
// #include "mocks/mock_stereo_unit.hpp"
// #include "mocks/mock_unit_pool.hpp"
// #include <algorithm>
// #include <atomic>
// #include <dtracker/audio/playback/mixer_playback.hpp>
// #include <dtracker/audio/playback/pattern_playback_unit.hpp>
// #include <dtracker/audio/playback/proxy_playback_unit.hpp>
// #include <dtracker/audio/playback/sample_playback_unit.hpp>
// #include <dtracker/audio/playback/track_playback_unit.hpp>
// #include <dtracker/sample/types.hpp>
// #include <memory>
// #include <thread>
// #include <vector>

// using namespace dtracker::audio;

// class PatternPlaybackTest : public ::testing::Test
// {
//   protected:
//     void SetUp() override
//     {
//         pattern.steps = {{1}, {}, {2}}; // Kick, Rest, Snare
//         pattern.stepIntervalMs = 100.0f;
//         blueprint[1] =
//             dtracker::sample::types::SampleDescriptor(1, nullptr, {});
//         blueprint[2] =
//             dtracker::sample::types::SampleDescriptor(2, nullptr, {});
//     }

//     dtracker::tracker::types::ActivePattern pattern;
//     playback::SampleBlueprint blueprint;
//     MockUnitPool pool;
// };

// // -------------------------
// // ProxyPlaybackUnit Tests
// // -------------------------

// // Verifies that render() calls are correctly forwarded to the delegate.
// TEST(ProxyPlaybackUnit, DelegatesRenderCall)
// {
//     playback::ProxyPlaybackUnit proxy;
//     MockPlaybackUnit mock;
//     proxy.setDelegate(&mock);

//     float dummyBuffer[64] = {0};
//     proxy.render(dummyBuffer, 32, 2);

//     EXPECT_EQ(mock.renderCallCount, 1)
//         << "ProxyPlaybackUnit did not forward render() call to delegate";
// }

// // Stress-tests the proxy by setting the delegate and rendering concurrently.
// TEST(ProxyPlaybackUnit, ThreadSafeDelegateAccess)
// {
//     playback::ProxyPlaybackUnit proxy;
//     MockPlaybackUnit mockA;
//     MockPlaybackUnit mockB;

//     // Writer thread repeatedly flips between two delegates.
//     std::atomic<bool> done = false;
//     std::thread writer(
//         [&]
//         {
//             for (int i = 0; i < 1000; ++i)
//             {
//                 proxy.setDelegate(i % 2 == 0 ? &mockA : &mockB);
//             }
//             done = true;
//         });

//     // Reader thread repeatedly calls render while the delegate is changing.
//     std::thread reader(
//         [&]
//         {
//             float dummy[128];
//             while (!done)
//             {
//                 proxy.render(dummy, 64, 2);
//             }
//         });

//     writer.join();
//     reader.join();

//     // The primary goal is to ensure this concurrent access does not crash.
//     SUCCEED();
// }

// // Verifies that the proxy outputs silence when its delegate is null.
// TEST(ProxyPlaybackUnit, NullDelegateRendersSilence)
// {
//     playback::ProxyPlaybackUnit proxy;
//     float buffer[64];
//     std::fill_n(buffer, 64, 1.0f); // Pre-fill with non-zero.
//     proxy.setDelegate(nullptr);

//     proxy.render(buffer, 32, 2);

//     for (float sample : buffer)
//         EXPECT_EQ(sample, 0.0f) << "Expected silence from null delegate";
// }

// // -------------------------
// // SamplePlaybackUnit Tests
// // -------------------------

// // Verifies that a unit plays its full sample, reports as finished, and can be
// // reset.
// TEST(SamplePlaybackTest, CompletesPlaybackAndResets)
// {
//     dtracker::sample::types::SampleDescriptor descriptor{
//         -1,
//         std::make_shared<const dtracker::audio::types::PCMData>(
//             dtracker::audio::types::PCMData(100, 0.5f)),
//         {44100, 16}};
//     auto unit = playback::makePlaybackUnit(std::move(descriptor));

//     std::vector<float> buffer(100, 0.0f);
//     EXPECT_FALSE(unit->isFinished());

//     // Render the full sample (50 stereo frames).
//     unit->render(buffer.data(), 50, 2);
//     EXPECT_TRUE(unit->isFinished());

//     // Reset and confirm it's ready to play again.
//     unit->reset();
//     EXPECT_FALSE(unit->isFinished());

//     // Render again after reset.
//     std::fill(buffer.begin(), buffer.end(), 0.0f);
//     unit->render(buffer.data(), 50, 2);
//     EXPECT_TRUE(unit->isFinished());

//     // Spot-check that the audio content is correct.
//     for (float sample : buffer)
//         EXPECT_NEAR(sample, 0.5f, 0.0001f);
// }

// // Verifies the unit pads the buffer with silence if it has no more data to
// // play.
// TEST(SamplePlaybackTest, PadsWithSilenceIfBufferLargerThanSample)
// {
//     dtracker::sample::types::SampleDescriptor descriptor{
//         -1,
//         std::make_shared<const dtracker::audio::types::PCMData>(
//             dtracker::audio::types::PCMData{1.0f, -1.0f}), // 1 stereo frame
//         {44100, 16}};
//     auto unit = playback::makePlaybackUnit(std::move(descriptor));

//     std::vector<float> buffer(6, 99.0f); // Request 3 frames.

//     unit->render(buffer.data(), 3, 2);

//     // Expect: [1.0, -1.0, 0.0, 0.0, 0.0, 0.0]
//     EXPECT_FLOAT_EQ(buffer[0], 1.0f);
//     EXPECT_FLOAT_EQ(buffer[1], -1.0f);
//     for (size_t i = 2; i < buffer.size(); ++i)
//         EXPECT_FLOAT_EQ(buffer[i], 0.0f);
// }

// // Verifies that reset() correctly restarts playback from the beginning.
// TEST(SamplePlaybackUnit, ResetRestartsPlaybackFromBeginning)
// {
//     dtracker::sample::types::SampleDescriptor descriptor{
//         -1,
//         std::make_shared<const dtracker::audio::types::PCMData>(
//             dtracker::audio::types::PCMData(20, 0.25f)), // 10 stereo frames
//         {44100, 16}};
//     auto unit = playback::makePlaybackUnit(std::move(descriptor));

//     std::vector<float> buffer(20, 0.0f);
//     unit->render(buffer.data(), 10, 2);
//     EXPECT_TRUE(unit->isFinished());

//     unit->reset();
//     EXPECT_FALSE(unit->isFinished());

//     // Render again and verify the content is from the beginning.
//     std::fill(buffer.begin(), buffer.end(), 0.0f);
//     unit->render(buffer.data(), 10, 2);

//     for (float sample : buffer)
//         EXPECT_NEAR(sample, 0.25f, 0.0001f);
// }

// // -------------------------
// // MixerPlaybackUnit Tests
// // -------------------------

// // Verifies the mixer produces silence when it contains no playback units.
// TEST(MixerPlaybackUnit, RendersSilenceWhenEmpty)
// {
//     playback::MixerPlaybackUnit mixer;
//     float buffer[64];
//     std::fill_n(buffer, 64, 1.0f); // Pre-fill to detect change.

//     mixer.render(buffer, 32, 2);

//     for (float sample : buffer)
//         EXPECT_FLOAT_EQ(sample, 0.0f) << "Expected silence when empty";
// }

// // Verifies that audio from a single unit passes through the mixer correctly.
// TEST(MixerPlaybackUnit, MixesSingleUnitCorrectly)
// {
//     auto mock = std::make_unique<MockPlaybackUnit>();
//     mock->fillValue = 0.5f;

//     playback::MixerPlaybackUnit mixer;
//     mixer.addUnit(std::move(mock));

//     float buffer[64];
//     mixer.render(buffer, 32, 2);

//     for (float sample : buffer)
//         EXPECT_FLOAT_EQ(sample, 0.5f);
// }

// // Verifies the mixer automatically detects and removes units that have finished
// // playing.
// TEST(MixerPlaybackUnit, RemovesFinishedUnits)
// {
//     auto mock = std::make_unique<MockPlaybackUnit>();
//     mock->finishedAfterRender =
//         true; // This mock will be finished after one render.

//     playback::MixerPlaybackUnit mixer;
//     mixer.addUnit(std::move(mock));

//     float buffer[64];
//     mixer.render(buffer, 32,
//                  2); // This render call should remove the finished unit.

//     EXPECT_TRUE(mixer.isFinished()); // The mixer should now be empty.
// }

// // Verifies that clear() removes all units and results in silence.
// TEST(MixerPlaybackUnit, ClearRemovesAllUnits)
// {
//     auto unit = std::make_unique<MockPlaybackUnit>();
//     playback::MixerPlaybackUnit mixer;
//     mixer.addUnit(std::move(unit));
//     ASSERT_FALSE(mixer.isFinished());

//     mixer.clear();
//     EXPECT_TRUE(mixer.isFinished());

//     // Confirm it now renders silence.
//     float buffer[64];
//     std::fill_n(buffer, 64, -1.0f);
//     mixer.render(buffer, 32, 2);

//     for (float sample : buffer)
//         EXPECT_FLOAT_EQ(sample, 0.0f);
// }

// // -------------------------
// // TrackPlaybackUnit Tests
// // -------------------------

// // Verifies the track produces silence when it contains no samples.
// TEST(TrackPlaybackUnit, RendersSilenceWhenEmpty)
// {
//     playback::TrackPlaybackUnit track;
//     float buffer[64];
//     std::fill_n(buffer, 64, 1.0f); // Pre-fill to detect changes.

//     track.render(buffer, 32, 2);

//     for (float sample : buffer)
//         EXPECT_FLOAT_EQ(sample, 0.0f);
// }

// // Verifies the track's volume and pan DSP logic is applied correctly.
// TEST(TrackPlaybackUnit, AppliesVolumeAndPanCorrectly)
// {
//     auto unit = std::make_unique<MockStereoUnit>();
//     unit->leftValue = 1.0f;
//     unit->rightValue = 1.0f;

//     playback::TrackPlaybackUnit track;
//     track.setVolume(0.5f); // Halve output.
//     track.setPan(-1.0f);   // Pan fully left.

//     track.addUnit(std::move(unit));

//     float buffer[64] = {};
//     track.render(buffer, 32, 2);

//     // Expect left channel at 50% volume and right channel silent.
//     for (unsigned int i = 0; i < 32; ++i)
//     {
//         EXPECT_FLOAT_EQ(buffer[i * 2], 0.5f);
//         EXPECT_FLOAT_EQ(buffer[i * 2 + 1], 0.0f);
//     }
// }

// // Verifies the pan logic correctly isolates audio to the left or right
// // channels.
// TEST(TrackPlaybackUnit, PanBiasesLeftOrRight)
// {
//     // Test full left pan
//     auto mockLeft = std::make_unique<MockPlaybackUnit>();
//     mockLeft->fillValue = 1.0f;
//     playback::TrackPlaybackUnit trackLeft;
//     trackLeft.setPan(-1.0f);
//     trackLeft.addUnit(std::move(mockLeft));
//     float leftBuffer[64] = {};
//     trackLeft.render(leftBuffer, 32, 2);
//     for (unsigned int i = 0; i < 64; i += 2)
//     {
//         EXPECT_FLOAT_EQ(leftBuffer[i], 1.0f);
//         EXPECT_FLOAT_EQ(leftBuffer[i + 1], 0.0f);
//     }

//     // Test full right pan
//     auto mockRight = std::make_unique<MockPlaybackUnit>();
//     mockRight->fillValue = 1.0f;
//     playback::TrackPlaybackUnit trackRight;
//     trackRight.setPan(1.0f);
//     trackRight.addUnit(std::move(mockRight));
//     float rightBuffer[64] = {};
//     trackRight.render(rightBuffer, 32, 2);
//     for (unsigned int i = 0; i < 64; i += 2)
//     {
//         EXPECT_FLOAT_EQ(rightBuffer[i], 0.0f);
//         EXPECT_FLOAT_EQ(rightBuffer[i + 1], 1.0f);
//     }
// }

// // Verifies that resetting a track also resets all contained sample units.
// TEST(TrackPlaybackUnit, ResetResetsAllSamples)
// {
//     dtracker::sample::types::SampleDescriptor descriptor{
//         -1,
//         std::make_shared<const dtracker::audio::types::PCMData>(
//             dtracker::audio::types::PCMData(20, 0.75f)), // 10 stereo frames
//         {44100, 16}};
//     auto unit = playback::makePlaybackUnit(std::move(descriptor));

//     playback::TrackPlaybackUnit track;
//     track.addUnit(std::move(unit));

//     // Render half the sample.
//     float buffer[20];
//     track.render(buffer, 5, 2);
//     ASSERT_FALSE(track.isFinished());

//     // Render the rest of the sample.
//     track.render(buffer, 5, 2);
//     ASSERT_TRUE(track.isFinished());

//     // Reset the track, which should also reset the contained unit.
//     track.reset();
//     EXPECT_FALSE(track.isFinished());
// }

// // Verifies that the pattern triggers notes by acquiring units from the pool.
// TEST_F(PatternPlaybackTest, TriggersNotesFromPool)
// {
//     // Arrange: Create the pattern player.
//     auto patternUnit =
//         playback::PatternPlaybackUnit(pattern, blueprint, &pool, 44100);

//     // Act: Render enough audio to play through the entire pattern.
//     std::vector<float> buffer(13230 * 2); // 3 steps * 100ms = 300ms
//     patternUnit.render(buffer.data(), 13230, 2);

//     // Assert: The pool's acquire() method should have been called twice, once
//     // for each non-rest note (ID 1 and 2).
//     EXPECT_EQ(pool.acquireCallCount, 2);
// }

// //==============================================================================
// // UPDATED: TrackPlaybackUnit Tests
// //==============================================================================

// // Verifies that the track plays its child patterns in the correct order.
// TEST(TrackPlaybackUnit, PlaysPatternsSequentially)
// {
//     playback::TrackPlaybackUnit track;

//     // Arrange: Create two mock patterns.
//     auto pattern1 = std::make_unique<MockPatternPlaybackUnit>();
//     auto pattern2 = std::make_unique<MockPatternPlaybackUnit>();

//     // We need raw pointers to them to check their state later.
//     MockPatternPlaybackUnit *p1_ptr = pattern1.get();
//     MockPatternPlaybackUnit *p2_ptr = pattern2.get();

//     track.addUnit(std::move(pattern1));
//     track.addUnit(std::move(pattern2));

//     // --- First Render Call ---
//     // Act: Render one block of audio.
//     float buffer[128];
//     track.render(buffer, 64, 2);

//     // Assert: Only the first pattern should have been rendered.
//     EXPECT_EQ(p1_ptr->renderCallCount, 1);
//     EXPECT_EQ(p2_ptr->renderCallCount, 0);

//     // --- Second Render Call ---
//     // Now, tell the first pattern it's "finished".
//     p1_ptr->isFinished_flag = true;

//     // Act: Render another block of audio.
//     track.render(buffer, 64, 2);

//     // Assert: The track should have moved to the second pattern.
//     // The first pattern was not rendered again, but the second one was.
//     EXPECT_EQ(p1_ptr->renderCallCount, 1);
//     EXPECT_EQ(p2_ptr->renderCallCount, 1);
//     EXPECT_EQ(p1_ptr->resetCallCount, 1); // Verify the finished unit was reset.
// }
