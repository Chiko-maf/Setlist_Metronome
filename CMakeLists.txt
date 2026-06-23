cmake_minimum_required(VERSION 3.20)
project(SetlistMetronome VERSION 1.0.0)

add_subdirectory(JUCE)

juce_add_gui_app(SetlistMetronome
        PRODUCT_NAME "Setlist Metronome"
        VERSION 1.0.0
        ICON_BIG "icon.png")

juce_generate_juce_header(SetlistMetronome)

set_target_properties(SetlistMetronome PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_sources(SetlistMetronome
        PRIVATE
        Main.cpp
        MainComponent.h)

target_link_libraries(SetlistMetronome
        PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra)