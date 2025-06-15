#pragma once

#include "i_track_manager.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace dtracker::tracker
{
    /// A thread-safe, concrete implementation of the ITrackManager interface.
    class TrackManager : public ITrackManager
    {
      public:
        TrackManager() = default;
        ~TrackManager() override = default;

        /// Creates a new, empty track and returns its unique ID.
        int createTrack(const std::string &name = "New Track") override;

        /// Retrieves a shared, thread-safe handle to a track's data.
        std::shared_ptr<types::Track> getTrack(int id) override;

        /// Adds a pattern to the end of a specific track's sequence.
        bool addPatternToTrack(int trackId,
                               const types::ActivePattern &pattern) override;

        /// Removes a track from the manager.
        bool removeTrack(int id) override;

        /// Returns a copy of all currently registered track IDs.
        std::vector<int> getAllTrackIds() const override;

      private:
        // The container for all track data, keyed by unique ID.
        std::unordered_map<int, std::shared_ptr<types::Track>> m_tracks;

        // The counter for the next available track ID.
        int m_nextId{0};

        // A mutex to ensure thread-safe access to the tracks map.
        mutable std::shared_mutex m_mutex;
    };

} // namespace dtracker::tracker