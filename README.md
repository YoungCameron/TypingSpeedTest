# TypingSpeedTest

A desktop typing speed test application built with C++ and SFML 3. Features a state machine architecture, real-time WPM and accuracy tracking, configurable play time, and a phrase management system.

## Features

- **Real-time feedback** — Characters highlight green/red as you type
- **WPM & Accuracy** — Calculated at the end of each session
- **Configurable timer** — 15, 30, 60, or 90 second sessions
- **Phrase cycling** — Loads prompts from an external text file and cycles through them
- **Phrase browser** — Card-style menu to browse all loaded phrases
- **State machine** — 7 game states: Main Menu, Waiting, Playing, Results, Options, Stats, Phrase Menu

## Tech Stack

- **C++20** (std::format, std::optional)
- **SFML 3.0.2** — Window, Graphics, System
- **CMake** — Build system

## Building

### Prerequisites
- CMake 3.16+
- SFML 3.0.2
- MinGW-w64 or compatible C++20 compiler

### Steps
```bash
git clone https://github.com/YoungCameron/TypingSpeedTest.git
cd TypingSpeedTest
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
```

## Usage

- **Arrow keys** — Navigate menus
- **Enter** — Select / start game
- **Escape** — Return to previous menu
- **Left/Right arrows** — Browse phrases in Phrase Menu
- Start typing to begin the timer — no button press needed

## Project Structure

    src/
    ├── main.cpp           # Game class, state machine, input handling, rendering
    ├── textPrompts.txt    # Typing prompts (one per line)
    ├── SNPro.ttf          # Font
    └── STT.jpeg           # Window icon
## License

MIT