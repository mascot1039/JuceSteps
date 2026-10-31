cmake_minimum_required(VERSION 3.22)

project(JuceSteps VERSION 0.1.0 LANGUAGES CXX)

find_package(JUCE CONFIG REQUIRED)

# ============================================================
# VST3 プラグインの定義
# ============================================================
juce_add_plugin(JuceSteps
    COMPANY_NAME "JuceSteps"
    BUNDLE_ID "com.jucesteps.plugin"

    PLUGIN_MANUFACTURER_CODE JuSt
    PLUGIN_CODE JcSt

    PRODUCT_NAME "JuceSteps"

    FORMATS VST3

    # MIDIエフェクトとして登録
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE

    # MIDI処理は後の段階で実装する
    IS_SYNTH FALSE

    COPY_PLUGIN_AFTER_BUILD FALSE
)

# ============================================================
# ソースファイル
# ============================================================
target_sources(JuceSteps PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h

    Source/MC500_InputComponent.cpp
    Source/MC500_InputComponent.h

    Source/InfiniteRotarySliderComponent.cpp
    Source/InfiniteRotarySliderComponent.h

    Source/LcdComponent.cpp
    Source/LcdComponent.h

    Source/Multi10KeyButton.cpp
    Source/Multi10KeyButton.h

    Source/JuceStepsMidiManager.cpp
    Source/JuceStepsMidiManager.h
)

target_include_directories(JuceSteps PRIVATE
    Source/
)

# ============================================================
# JUCE モジュール
# ============================================================
target_link_libraries(JuceSteps PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_audio_basics
    juce::juce_audio_processors

    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# ============================================================
# プリプロセッサ定義
# ============================================================
target_compile_definitions(JuceSteps PRIVATE
    JUCE_APPLICATION_NAME="${PROJECT_NAME}"
    JUCE_APPLICATION_VERSION="${PROJECT_VERSION}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
