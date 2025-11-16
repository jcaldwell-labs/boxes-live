# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Boxes-live is a Unix-style terminal application written in C that creates an interactive visual workspace similar to Miro boards. The application manipulates the terminal to display boxes with pan, zoom, and side-scrolling capabilities, creating 3D-like effects within a 2D terminal buffer.

Key concepts:
- Uses box-drawing similar to `/usr/bin/boxes`
- Terminal manipulation for pan/zoom effects simulating 3D
- Side-scrolling viewport into a larger 2D buffer
- Interactive terminal-based canvas for visual organization

## Build System

The project uses GNU Make for building:
- `make` - Build the application (creates `boxes-live` executable)
- `make clean` - Remove build artifacts and executable
- `make run` - Build and run the application

Dependencies: ncurses library (`-lncurses`), math library (`-lm`)

## Development Commands

**Building:**
```bash
make              # Compile all source files
make clean        # Clean build artifacts
```

**Running:**
```bash
./boxes-live      # Run the application
make run          # Build and run in one command
```

**Testing:**
- Run the executable in a terminal that supports box-drawing characters
- Terminal emulators like xterm, gnome-terminal, alacritay, kitty work well
- Use arrow keys or WASD to pan, +/- to zoom, Q to quit

**Debugging:**
```bash
gdb ./boxes-live          # Debug with GDB
valgrind ./boxes-live     # Check for memory leaks
```
Note: Visual output requires running outside debugger or in a separate terminal

## Architecture

The codebase is organized into separate modules:

**Directory Structure:**
```
include/           # Header files
  types.h          # Core data structures (Box, Viewport, Canvas)
  terminal.h       # Terminal initialization/cleanup
  viewport.h       # Viewport and coordinate transformation
  render.h         # Rendering functions
  input.h          # Input handling
src/              # Implementation files
  main.c          # Main application loop and canvas initialization
  terminal.c      # Terminal control using ncurses
  viewport.c      # Camera control and coordinate mapping
  render.c        # Box rendering with ACS characters
  input.c         # Keyboard input processing
```

**Core Components:**

1. **Terminal Module** (terminal.h/c):
   - Initializes ncurses with proper settings (cbreak, noecho, keypad)
   - Handles terminal cleanup on exit
   - Provides terminal size updates for viewport

2. **Viewport Module** (viewport.h/c):
   - Manages camera position (cam_x, cam_y) in world space
   - Handles zoom level (0.1x to 10.0x range)
   - Coordinate transformation: world_to_screen_x/y, screen_to_world_x/y
   - Pan and zoom operations with smooth center-focused zooming

3. **Render Module** (render.h/c):
   - Draws boxes using ncurses ACS box-drawing characters
   - Renders only visible boxes (viewport culling)
   - Scales box dimensions based on zoom level
   - Status bar showing camera position and controls

4. **Input Module** (input.h/c):
   - Non-blocking input with getch()
   - Arrow keys / WASD for panning
   - +/- or Z/X for zoom in/out
   - R or 0 to reset view
   - Q or ESC to quit

5. **Main Application** (main.c):
   - Initializes sample canvas with demonstration boxes
   - Main loop: update → clear → render → refresh → input → sleep
   - ~60 FPS rendering using nanosleep()
   - Proper memory cleanup for dynamically allocated box content

**Data Flow:**
1. User input → Input handler updates Viewport
2. Viewport transforms world coordinates to screen coordinates
3. Renderer draws boxes visible in current viewport
4. ncurses refreshes the terminal display

## Code Style and Conventions

The project follows these C conventions:
- **Standard**: GNU99 (allows strdup and other POSIX extensions)
- **Headers**: Clear separation between interface (.h) and implementation (.c)
- **Naming**: snake_case for functions and variables, UPPER_CASE for macros/constants
- **Error Handling**: Functions return -1 or NULL on error where applicable
- **Memory**: Explicit cleanup of dynamically allocated memory (strdup'd strings)
- **Comments**: Descriptive comments for complex logic, especially coordinate transformations
- **Modularity**: Each module handles a single concern (terminal, viewport, render, input)

**When modifying:**
- Keep header files minimal with clear function declarations
- Add bounds checking for screen coordinate operations
- Test with different terminal sizes (viewport resize handling)
- Consider zoom level impact on rendering performance
- Maintain the separation between world and screen coordinate systems
