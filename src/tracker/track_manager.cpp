#include "dtracker/tracker/track_manager.hpp"

#include <iostream>

namespace dtracker::tracker
{
    int TrackManager::createTrack(const std::string &name)
    {
        // Acquire a unique (exclusive) lock for this write operation.
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // Create the new track data object, managed by a shared_ptr.
        auto track = std::make_shared<types::Track>(name);

        // Generate and assign the new unique ID.
        int id = m_nextId++;
        track->id = id;

        // Move the new track into our map.
        m_tracks[id] = std::move(track);

        return id;
    }

    std::shared_ptr<types::Track> TrackManager::getTrack(int id)
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        if (auto it = m_tracks.find(id); it != m_tracks.end())
        {
            return it->second;
        }

        return nullptr;
    }

    bool TrackManager::addPatternToTrack(int trackId,
                                         const types::ActivePattern &pattern)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_tracks.find(trackId);
        if (it != m_tracks.end())
        {
            // Get the shared_ptr to the track.
            auto &trackPtr = it->second;

            // Add a copy of the pattern to the track's pattern sequence.
            trackPtr->patterns.push_back(pattern);

            return true;
        }

        return false; // Track with the given ID was not found.
    }

    bool TrackManager::removeTrack(int id)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        return m_tracks.erase(id) > 0;
    }

    bool dtracker::tracker::TrackManager::updateTrackPatterns(
        int trackId,
        const std::vector<dtracker::tracker::types::ActivePattern> &patterns)
    {
        // Use a unique_lock as this is a write operation.
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_tracks.find(trackId);
        if (it != m_tracks.end())
        {
            std::cout << "Updating track pattern\n";
            // Get the shared_ptr to the track and update its patterns.
            it->second->patterns = patterns;
            return true;
        }
        return false; // Track not found.
    }

    std::vector<int> TrackManager::getAllTrackIds() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        std::vector<int> ids;
        // Pre-allocate vector memory.
        ids.reserve(m_tracks.size());

        // Get all the IDs.
        for (const auto &[id, _] : m_tracks)
        {
            ids.push_back(id);
        }

        return ids;
    }

} // namespace dtracker::tracker