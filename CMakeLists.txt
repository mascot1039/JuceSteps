cmake_minimum_required(VERSION 3.22)
project(JuceSteps VERSION 0.1.0 LANGUAGES CXX)

find_package(JUCE CONFIG REQUIRED)

# Linux用のlibcurlパッケージを検索する設定
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(CURL REQUIRED)
endif()

juce_add_gui_app(JuceSteps
    PRODUCT_NAME "JuceSteps"
)

target_sources(JuceSteps PRIVATE
    Source/Main.cpp
    Source/MC500_InputComponent.cpp
    Source/MC500_InputComponent.h
    Source/InfiniteRotarySliderComponent.h
    Source/InfiniteRotarySliderComponent.cpp
    Source/LcdComponent.cpp
    Source/LcdComponent.h
)

target_include_directories(JuceSteps PRIVATE
    Source/
)

target_link_libraries(JuceSteps PRIVATE
    juce::juce_core
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_events
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Linux環境の場合、明示的にCURL（libcurl）をリンクする
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(JuceSteps PRIVATE CURL::libcurl)
endif()

target_compile_definitions(JuceSteps PRIVATE
    JUCE_APPLICATION_NAME="${PROJECT_NAME}"
    JUCE_APPLICATION_VERSION="${PROJECT_VERSION}"
)
