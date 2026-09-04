cmake_minimum_required(VERSION 3.22)
project(JuceMinimum VERSION 0.1.0 LANGUAGES CXX)

find_package(JUCE CONFIG REQUIRED)
#find_package(PkgConfig REQUIRED)

# Linux用のlibcurlパッケージを検索する設定
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(CURL REQUIRED)
endif()

juce_add_gui_app(JuceMinimum
    PRODUCT_NAME "JuceMinimum"
)

target_sources(JuceMinimum PRIVATE
    Main.cpp
)

target_link_libraries(JuceMinimum PRIVATE
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
    target_link_libraries(JuceMinimum PRIVATE CURL::libcurl)
endif()

target_compile_definitions(JuceMinimum PRIVATE
    JUCE_APPLICATION_NAME="${PROJECT_NAME}"
    JUCE_APPLICATION_VERSION="${PROJECT_VERSION}"
)
