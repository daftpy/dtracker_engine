#pragma once

#include <gtest/gtest.h> // For ASSERT_TRUE in helper methods

#include <dtracker/sample/i_manager.hpp>
#include <string>
#include <unordered_map>

// A mock implementation of the ISampleManager interface for testing.
// It uses a simple map to store sample data, allowing tests to control
// what it contains and what its methods return.
class MockSampleManager : public dtracker::sample::IManager
{
  public:
    // Pre-loads a descriptor into the mock's internal map.
    void
    addSampleToMock(int id,
                    const dtracker::sample::types::SampleDescriptor &descriptor)
    {
        m_mockSamples[id] = descriptor;
    }

    // Implementations for all pure virtual functions.

    std::shared_ptr<const dtracker::audio::types::PCMData>
    cacheSample(const std::string &,
                std::shared_ptr<const dtracker::audio::types::PCMData> pcmData,
                const dtracker::sample::types::SampleMetadata &) override
    {
        // For now, just return the data that was passed in.
        return pcmData;
    }

    int addSample(const std::string &,
                  std::shared_ptr<const dtracker::audio::types::PCMData>,
                  const dtracker::sample::types::SampleMetadata &) override
    {
        // Return a dummy ID.
        return m_nextId++;
    }

    int addSample(const std::string &) override
    {
        return m_nextId++;
    }

    // It looks in our internal map for a pre-programmed sample.
    std::optional<dtracker::sample::types::SampleDescriptor>
    getSample(int id) override
    {
        auto it = m_mockSamples.find(id);
        if (it != m_mockSamples.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    // For the simple tests, these can just return default values.
    bool removeSample(int) override
    {
        return true;
    }
    std::vector<int> getAllSampleIds() const override
    {
        return {};
    }
    bool contains(const std::string &) const override
    {
        return false;
    }
    std::optional<dtracker::sample::types::CacheEntry>
    peekCache(const std::string &) override
    {
        return std::nullopt;
    }

  private:
    // The mock's internal "database" of samples.
    std::unordered_map<int, dtracker::sample::types::SampleDescriptor>
        m_mockSamples;
    int m_nextId = 0;
};