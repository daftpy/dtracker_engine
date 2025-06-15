#pragma once
#include <dtracker/tracker/types.hpp>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace dtracker::tracker
{
    /// Defines the public contract for a service that manages a project's
    /// tracks. Its role is to be a thread-safe container for track data
    /// objects.
    class ITrackManager
    {
      public:
        /// Enables safe destruction of derived classes through a base pointer.
        virtual ~ITrackManager() = default;

        /// Creates a new, empty track with a given name.
        /// @return The unique integer ID assigned to the new track.
        virtual int createTrack(const std::string &name = "New Track") = 0;

        /// Retrieves a shared, thread-safe handle to a track's data.
        /// @param id The unique ID of the track to retrieve.
        /// @return A shared_ptr to the track, or nullptr if not found.
        virtual std::shared_ptr<types::Track> getTrack(int id) = 0;

        /// Adds a pattern to the end of a specific track's sequence.
        /// @param trackId The ID of the track to modify.
        /// @param pattern The pattern data to add. Passed by const reference
        /// for efficiency.
        /// @return True if the track was found and the pattern was added.
        virtual bool addPatternToTrack(int trackId,
                                       const types::ActivePattern &pattern) = 0;

        /// Removes a track from the manager.
        /// @return True if the track existed and was removed.
        virtual bool removeTrack(int id) = 0;

        /// Returns a copy of all currently registered track IDs.
        virtual std::vector<int> getAllTrackIds() const = 0;
    };
} // namespace dtracker::tracker