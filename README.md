# Boxes-Live

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A terminal-based interactive canvas application written in C, similar to Miro boards but for the terminal.

## Overview

Boxes-live provides a pan-and-zoom interface for navigating a large 2D canvas containing boxes with content. It uses ncurses for terminal manipulation and box-drawing characters, creating smooth 3D-like effects through viewport transformations.

## Features

- **Interactive Canvas**: Navigate a large 2D workspace in your terminal
- **Pan & Zoom**: Smooth camera controls with arrow keys and zoom controls
- **Box Rendering**: Display boxes with titles and content using box-drawing characters
- **Viewport System**: Efficient world-to-screen coordinate transformation
- **Responsive**: Handles terminal resizing dynamically

## Quick Start

### Prerequisites

- GCC compiler
- ncurses library (`sudo apt-get install libncurses-dev` on Debian/Ubuntu)
- Terminal with box-drawing character support

### Building

```bash
make              # Build the application
make run          # Build and run
make clean        # Clean build artifacts
```

### Controls

- **Arrow Keys / WASD**: Pan the viewport
- **+ / - or Z / X**: Zoom in/out
- **R or 0**: Reset view to origin
- **Q or ESC**: Quit

## Architecture

The project is organized into modular components:
- **Terminal**: ncurses initialization and terminal control
- **Viewport**: Camera position and coordinate transformations
- **Renderer**: Box drawing with ACS characters
- **Input**: Keyboard event handling
- **Canvas**: Box storage and management

See [CLAUDE.md](CLAUDE.md) for detailed architecture documentation.
See [USAGE.md](USAGE.md) for complete usage instructions.

## Project Stats

- **Language**: C (GNU99)
- **Lines of Code**: ~636
- **Dependencies**: ncurses, POSIX
- **Files**: 5 headers, 5 implementation files

## Contributing

Contributions are welcome! This project serves as a foundation for a terminal-based canvas application. Some ideas for enhancements:

- [ ] Interactive box creation and editing
- [ ] Save/load canvas to/from files
- [ ] Connection lines between boxes
- [ ] Color support for boxes
- [ ] Box selection and manipulation
- [ ] Copy/paste functionality
- [ ] Search and filter boxes
- [ ] Export canvas as ASCII art

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) for terminal manipulation
- Inspired by visual collaboration tools like Miro
- Created with [Claude Code](https://claude.ai/code)
