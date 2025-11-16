# Boxes-Live

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A terminal-based interactive canvas application written in C, similar to Miro boards but for the terminal.

## Overview

Boxes-live provides a pan-and-zoom interface for navigating a large 2D canvas containing boxes with content. It uses ncurses for terminal manipulation and box-drawing characters, creating smooth 3D-like effects through viewport transformations.

## Features

✅ **Phase 1 Complete** - Foundation features implemented!

- **Interactive Canvas**: Navigate a large 2D workspace in your terminal
- **Dynamic Memory**: Unlimited boxes (automatically grows as needed)
- **Pan & Zoom**: Smooth camera controls with arrow keys and zoom controls
- **Mouse Support**: Click to select boxes
- **Box Management**: Create new boxes, delete selected boxes
- **Box Selection**: Visual highlighting of selected box, Tab to cycle through boxes
- **Save/Load**: Persist canvas to file and reload later
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

- **Arrow Keys / W/S/A/H/L**: Pan the viewport
- **+ / - or Z / X**: Zoom in/out
- **Mouse Click**: Select box
- **N**: Create new box
- **D**: Delete selected box
- **Tab**: Cycle through boxes
- **F2 / F3**: Save / Load canvas
- **R or 0**: Reset view to origin
- **Q or ESC**: Quit

## Architecture

The project is organized into modular components:
- **Terminal**: ncurses initialization, terminal control, and mouse support
- **Viewport**: Camera position and coordinate transformations
- **Renderer**: Box drawing with ACS characters and selection highlighting
- **Input**: Keyboard and mouse event handling
- **Canvas**: Dynamic box storage and management
- **Persistence**: Save/load functionality for canvas state

See [CLAUDE.md](CLAUDE.md) for detailed architecture documentation.
See [USAGE.md](USAGE.md) for complete usage instructions.
See [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md) for planned enhancements.

## Project Stats

- **Language**: C (GNU99)
- **Lines of Code**: ~1400+
- **Dependencies**: ncurses, POSIX
- **Files**: 7 headers, 7 implementation files
- **Version**: 0.2.0 (Phase 1 Complete)

## Contributing

Contributions are welcome! This project serves as a foundation for a terminal-based canvas application.

### Completed (Phase 1)
- [x] Interactive box creation and deletion
- [x] Save/load canvas to/from files
- [x] Box selection and manipulation
- [x] Dynamic canvas (unlimited boxes)
- [x] Mouse support

### Next Up (Phase 2)
See [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md) for the complete roadmap. Phase 2 priorities:
- [ ] Text editing mode (edit box content)
- [ ] Box resizing and moving
- [ ] Connection lines between boxes
- [ ] Color support for boxes
- [ ] Box types (note, task, code)
- [ ] Copy/paste functionality
- [ ] Undo/redo
- [ ] Search and filter boxes
- [ ] Export canvas as ASCII art / Markdown

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) for terminal manipulation
- Inspired by visual collaboration tools like Miro
- Created with [Claude Code](https://claude.ai/code)
