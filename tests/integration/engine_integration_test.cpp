#include <gtest/gtest.h>

#include <chrono>
#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/engine.hpp>
#include <thread>

// This test suite is for integration tests that use an Engine instance.
// It will interact with the actual audio hardware on the machine.
TEST(EngineIntegration, StartsAndStopsSuccessfully)
{
    dtracker::audio::Engine engine;

    // Use the engine's DeviceManager to find a valid output device.
    auto deviceManager = engine.createDeviceManager();
    auto deviceInfoOpt = deviceManager.currentDeviceInfo();

    // If this fails, there's no audio device on the system, so we can't test.
    // SUCCEED() tells gtest the test passed because the condition isn't met.
    if (!deviceInfoOpt)
    {
        std::cout << "Skipping test: No audio output device found."
                  << std::endl;
        SUCCEED();
        return;
    }

    // Tell the engine which device to use.
    engine.setOutputDevice(deviceInfoOpt->ID);

    // Check the audio stream is not running
    ASSERT_FALSE(engine.isStreamRunning());

    // Start the engine.
    bool success = engine.start();
    ASSERT_TRUE(success) << "Engine failed to start with a valid device.";

    // The stream should now be running.
    EXPECT_TRUE(engine.isStreamRunning());

    // Let the stream run for a brief moment.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop the stream.
    engine.stop();

    // The stream should no longer be running.
    EXPECT_FALSE(engine.isStreamRunning());
}

TEST(EngineIntegration, FailsToStartWithInvalidDevice)
{
    dtracker::audio::Engine engine;

    // Attempt to start with an invalid device ID.
    engine.setOutputDevice(99999); // An ID that should not exist.
    bool success = engine.start();

    // The engine should fail to start and return false.
    EXPECT_FALSE(success);
    EXPECT_FALSE(engine.isStreamRunning());
}