# Setlist Metronome

A rock-solid, sample-accurate setlist metronome built in C++ and JUCE for live stage performance. Features an Ableton-style dark interface, track color-coding, inline notes, and a lock-free audio thread architecture to ensure zero latency or dropouts, even when running in the background. Perfect for modern worship and live gig setups.

## Features
* **Lock-Free Audio Engine:** The audio callback runs on a dedicated high-priority thread, completely decoupled from the UI. It will never lag or accordion, even if the window is minimized.
* **Ableton-Style Click:** Synthesizes a pure sine-wave ping (1000Hz downbeat / 500Hz upbeat) with a 15ms exponential decay for an ultra-tight transient.
* **Stage View:** A massive, high-contrast, distraction-free performance view with visual pulse indicators.
* **Live Notes:** Write, edit, and read chord progressions directly from the main stage interface.
* **Color Coding:** Assign custom colors to specific tracks to track your setlist via peripheral vision.

## Building from Source

This project uses CMake and requires the JUCE framework. 

1. Clone this repository.
2. Clone the JUCE framework directly into the project directory:
   `git clone https://github.com/juce-framework/JUCE.git`
3. Open the project in CLion (or your preferred CMake-compatible IDE).
4. Reload the CMake project so it detects the JUCE subdirectory.
5. Build the `SetlistMetronome` target in **Release** mode for optimal audio performance.

## License
This project is licensed under the MIT License - free to use, modify, and distribute.
