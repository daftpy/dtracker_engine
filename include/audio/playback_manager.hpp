#pragma once

#include "engine.hpp"
#include <memory>

namespace dtracker::audio
{
    class PlaybackManager
    {
      public:
        explicit PlaybackManager(Engine *engine);

        void playTestTone(float freq = 440.0f);
        void stopPlayback();

        bool isPlaying() const;

      private:
        Engine *m_engine{nullptr}; // Not owned
    };
} // namespace dtracker::audio
