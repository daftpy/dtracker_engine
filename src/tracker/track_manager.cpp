#include <dtracker/tracker/track_manager.hpp>

namespace dtracker::tracker
{
    TrackManager::TrackManager(audio::SampleManager *sampleManager)
        : m_sampleManager(sampleManager)
    {
    }

    // Creates a new TrackPlaybackUnit with the given sample IDs, volume, and
    // pan. Returns a unique track ID that can be used to access the created
    // track later.
    int TrackManager::createTrack(std::vector<int> sampleIds, float volume,
                                  float pan)
    {
        // Allocate a new track
        auto track = std::make_unique<audio::playback::TrackPlaybackUnit>();
        track->setVolume(volume);
        track->setPan(pan);

        // Load and add all the requested sample playback units
        for (int id : sampleIds)
        {
            if (!m_sampleManager)
                continue;

            auto unit = m_sampleManager->getSample(id);
            if (unit)
                track->addSample(std::move(unit));
        }

        // Store the track using a unique track ID
        int id = m_nextId++;
        m_tracks[id] = std::move(track);
        return id;
    }

    int TrackManager::createTrack(float volume, float pan)
    {
        auto track = std::make_unique<audio::playback::TrackPlaybackUnit>();
        track->setVolume(volume);
        track->setPan(pan);

        int id = m_nextId++;
        m_tracks[id] = std::move(track);
        return id;
    }

    // Returns a pointer to the track with the given ID, or nullptr if not
    // found.
    audio::playback::TrackPlaybackUnit *TrackManager::getTrack(int id)
    {
        auto it = m_tracks.find(id);
        return (it != m_tracks.end()) ? it->second.get() : nullptr;
    }

    // Adds a list of samples to a specified track
    bool TrackManager::addSamplesToTrack(int trackId,
                                         const std::vector<int> &sampleIds)
    {
        // Attempt to get the track
        auto *track = getTrack(trackId);
        if (!track || !m_sampleManager)
            return false;

        // Add the samples to the track if they exist
        for (int id : sampleIds)
        {
            auto unit = m_sampleManager->getSample(id);
            if (unit)
                track->addSample(std::move(unit));
        }

        return true;
    }

    // Removes a track by ID. Returns true if the track was removed, false if it
    // didn't exist.
    bool TrackManager::removeTrack(int id)
    {
        return m_tracks.erase(id) > 0;
    }

    // Returns a list of all current track IDs managed by this TrackManager.
    std::vector<int> TrackManager::allTrackIds() const
    {
        std::vector<int> ids;
        ids.reserve(m_tracks.size());

        for (const auto &[id, _] : m_tracks)
            ids.push_back(id);

        return ids;
    }

} // namespace dtracker::tracker
