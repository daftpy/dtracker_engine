# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
# This populates the GTest source
FetchContent_MakeAvailable(googletest)

# Define test executable
add_executable(audio_engine_test audio_engine_test.cpp)

target_link_libraries(audio_engine_test
  gtest_main
  dtracker_engine
)

include(GoogleTest)
gtest_discover_tests(audio_engine_test
    DISCOVERY_MODE PRE_TEST
)