#pragma once
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <dtracker/audio/sample_manager.hpp>
#include <memory>
#include <unordered_map>

namespace dtracker::tracker
{
    class TrackManager
    {
      public:
        explicit TrackManager(audio::SampleManager *sampleManager);

        // Creates and stores a new track, returns its ID
        int createTrack(std::vector<int> sampleIds, float volume = 1.0f,
                        float pan = 0.0f);

        // Creates and stores a new empty track, returns its ID
        int createTrack(float volume = 1.0f, float pan = 0.0f);

        // Retrieves a raw pointer to a track by ID (or nullptr if not found)
        audio::playback::TrackPlaybackUnit *getTrack(int id);

        // Adds one or more samples to an existing track by ID
        bool addSamplesToTrack(int trackId, const std::vector<int> &sampleIds);

        // Optional: remove a track
        bool removeTrack(int id);

        // Returns all track IDs
        std::vector<int> allTrackIds() const;

      private:
        audio::SampleManager *m_sampleManager = nullptr;
        std::unordered_map<int,
                           std::unique_ptr<audio::playback::TrackPlaybackUnit>>
            m_tracks;
        int m_nextId = 0;
    };

} // namespace dtracker::tracker
