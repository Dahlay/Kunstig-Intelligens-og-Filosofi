cmake_minimum_required(VERSION 3.24)
project(ModularAudioPatcher VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# JUCE
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.3
)
FetchContent_MakeAvailable(juce)

# JSON
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

juce_add_gui_app(ModularAudioPatcher
  PRODUCT_NAME "Modular Audio Patcher"
)

juce_generate_juce_header(ModularAudioPatcher)

target_sources(ModularAudioPatcher
  PRIVATE
    src/main.cpp
    src/app/MainComponent.h
    src/app/MainComponent.cpp
    src/ui/CanvasView.h
    src/ui/CanvasView.cpp
    src/graph/PatchGraph.h
    src/graph/PatchGraph.cpp
    src/graph/GraphValidator.h
    src/graph/GraphValidator.cpp
    src/graph/GraphCompiler.h
    src/graph/GraphCompiler.cpp
    src/audio/RuntimePlan.h
    src/audio/AudioEngine.h
    src/audio/AudioEngine.cpp
    src/audio/processors/IProcessor.h
    src/persistence/PatchSerializer.h
    src/persistence/PatchSerializer.cpp
)

target_include_directories(ModularAudioPatcher
  PRIVATE
    src
)

target_compile_definitions(ModularAudioPatcher
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(ModularAudioPatcher
  PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    nlohmann_json::nlohmann_json
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  enable_testing()

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.6.0
  )
  FetchContent_MakeAvailable(Catch2)

  add_executable(graph_tests
    tests/GraphTests.cpp
    src/graph/PatchGraph.cpp
    src/graph/GraphValidator.cpp
    src/graph/GraphCompiler.cpp
    src/audio/AudioEngine.cpp
    src/persistence/PatchSerializer.cpp
  )

  target_include_directories(graph_tests PRIVATE src)
  target_link_libraries(graph_tests PRIVATE Catch2::Catch2WithMain juce::juce_audio_basics nlohmann_json::nlohmann_json)
  target_compile_definitions(graph_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

  include(CTest)
  include(Catch)
  catch_discover_tests(graph_tests)
endif()
