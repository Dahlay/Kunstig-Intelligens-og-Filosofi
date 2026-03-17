cmake_minimum_required(VERSION 3.24)
project(ModularAudioPatcher VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Version bookkeeping ───────────────────────────────────────────────────────
set(MAP_VERSION_MAJOR 1)
set(MAP_VERSION_MINOR 0)
set(MAP_VERSION_PATCH 0)
set(MAP_VERSION "${MAP_VERSION_MAJOR}.${MAP_VERSION_MINOR}.${MAP_VERSION_PATCH}")

# ── CPack common settings (filled in per-platform below) ─────────────────────
set(CPACK_PACKAGE_NAME        "ModularAudioPatcher")
set(CPACK_PACKAGE_VENDOR      "Even")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modular Audio Patcher – node-based audio workstation")
set(CPACK_PACKAGE_VERSION     "${MAP_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MAP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MAP_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README.md")

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
  PRODUCT_NAME         "Modular Audio Patcher"
  VERSION              "${MAP_VERSION}"
  COMPANY_NAME         "Even"
  COMPANY_WEBSITE      "https://example.com"
  BUNDLE_ID            "com.even.modularaudiopatcher"
  # macOS
  MICROPHONE_PERMISSION_ENABLED   TRUE
  MICROPHONE_PERMISSION_TEXT      "Modular Audio Patcher needs microphone access for audio input routing."
  # Windows resources
  ICON_BIG             ""
)

juce_generate_juce_header(ModularAudioPatcher)

target_sources(ModularAudioPatcher
  PRIVATE
    src/main.cpp
    src/app/MainComponent.h
    src/app/MainComponent.cpp
    src/ui/CanvasView.h
    src/ui/CanvasView.cpp
    src/ui/SynthMidiGrid.h
    src/ui/SynthMidiGrid.cpp
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
    src/diagnostics/CrashLogger.h
)

target_include_directories(ModularAudioPatcher
  PRIVATE
    src
)

target_compile_definitions(ModularAudioPatcher
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ModularAudioPatcher,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ModularAudioPatcher,JUCE_VERSION>"
    MAP_VERSION_STRING="${MAP_VERSION}"
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

# ── Packaging ─────────────────────────────────────────────────────────────────
if(APPLE)
  # DragNDrop .dmg via hdiutil (macOS native)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_VOLUME_NAME "Modular Audio Patcher ${MAP_VERSION}")
  set(CPACK_DMG_FORMAT "UDZO")          # compressed UDIF
  set(CPACK_BUNDLE_NAME "Modular Audio Patcher")
  set(CPACK_BUNDLE_PLIST
    "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in")
  # Signing (opt-in via cmake -DMAP_SIGN=ON -DMAP_TEAM_ID=XXXXXXXXXX)
  option(MAP_SIGN "Code-sign the macOS bundle" OFF)
  if(MAP_SIGN)
    set(MAP_TEAM_ID "" CACHE STRING "Apple Developer Team ID for code signing")
    set_target_properties(ModularAudioPatcher PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY       "Developer ID Application"
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM         "${MAP_TEAM_ID}"
      XCODE_ATTRIBUTE_CODE_SIGN_STYLE          "Manual"
      XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS    "--timestamp --options=runtime"
    )
  endif()
elseif(WIN32)
  # NSIS installer on Windows (requires NSIS installed)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_PACKAGE_NAME       "Modular Audio Patcher")
  set(CPACK_NSIS_DISPLAY_NAME       "Modular Audio Patcher ${MAP_VERSION}")
  set(CPACK_NSIS_INSTALL_ROOT       "$PROGRAMFILES64")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MUI_ICON
    "${CMAKE_SOURCE_DIR}/packaging/windows/installer.ico")
  set(CPACK_NSIS_MUI_UNIICON
    "${CMAKE_SOURCE_DIR}/packaging/windows/installer.ico")
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
    "CreateShortcut \\\"$DESKTOP\\\\Modular Audio Patcher.lnk\\\" \\\"$INSTDIR\\\\ModularAudioPatcher.exe\\\"")
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
    "Delete \\\"$DESKTOP\\\\Modular Audio Patcher.lnk\\\"")
  # Install the built executable
  install(TARGETS ModularAudioPatcher DESTINATION ".")
elseif(UNIX)
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
