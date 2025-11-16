# Boxes-Live

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A terminal-based interactive canvas application written in C, similar to Miro boards but for the terminal.

## Overview

Boxes-live provides a pan-and-zoom interface for navigating a large 2D canvas containing boxes with content. It uses ncurses for terminal manipulation and box-drawing characters, creating smooth 3D-like effects through viewport transformations.

## Features

✅ **Phase 1 Complete** - Foundation features implemented!
✅ **Phase 2 (Partial)** - Smart Canvas features added!

- **Interactive Canvas**: Navigate a large 2D workspace in your terminal
- **Dynamic Memory**: Unlimited boxes (automatically grows as needed)
- **Pan & Zoom**: Smooth camera controls with arrow keys and zoom controls
- **Mouse Support**: Click to select boxes, drag to move them
- **Box Management**: Create new boxes, delete selected boxes, drag to reposition
- **Box Selection**: Visual highlighting of selected box, Tab to cycle through boxes
- **Color Support**: 7 colors for boxes (red, green, blue, yellow, magenta, cyan, white)
- **Save/Load**: Persist canvas to file and reload later (includes colors and positions)
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
make test         # Run automated tests (245 assertions)
make clean        # Clean build artifacts
```

### Controls

- **Arrow Keys / W/S/A/H/L**: Pan the viewport
- **+ / - or Z / X**: Zoom in/out
- **Mouse Click**: Select box
- **Mouse Drag**: Move selected box
- **N**: Create new box
- **D**: Delete selected box
- **1-7**: Color selected box
- **0**: Reset color (or view if no selection)
- **Tab**: Cycle through boxes
- **F2 / F3**: Save / Load canvas
- **R**: Reset view to origin
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
See [TESTING.md](TESTING.md) for testing strategy and guide.

## Project Stats

- **Language**: C (GNU99)
- **Lines of Code**: ~1500+
- **Dependencies**: ncurses, POSIX
- **Files**: 7 headers, 7 implementation files
- **Version**: 0.3.0 (Phase 1 + Smart Canvas Features)

## Contributing

Contributions are welcome! This project serves as a foundation for a terminal-based canvas application.

### Completed
**Phase 1 (Foundation):**
- [x] Interactive box creation and deletion
- [x] Save/load canvas to/from files
- [x] Box selection and manipulation
- [x] Dynamic canvas (unlimited boxes)
- [x] Mouse support

**Phase 2 (Smart Canvas) - Partial:**
- [x] Box moving/dragging with mouse
- [x] Color support for boxes (7 colors)
- [x] Persistence of colors and positions

### Next Up (Phase 2 Continued)
See [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md) for the complete roadmap. Remaining Phase 2 priorities:
- [ ] Text editing mode (edit box content)
- [ ] Box resizing with mouse
- [ ] Connection lines between boxes
- [ ] Box types (note, task, code)
- [ ] Copy/paste functionality
- [ ] Undo/redo
- [ ] Search and filter boxes
- [ ] Export canvas as ASCII art / Markdown

## Testing

The project includes comprehensive automated testing:

- **245 test assertions** across 31 test cases
- **4 test suites**: Canvas, Viewport, Persistence, Integration
- **Headless execution**: No terminal interaction required
- **CI/CD ready**: Proper exit codes and fast execution

```bash
make test              # Run all tests
make test_canvas       # Run canvas unit tests
make test_viewport     # Run viewport unit tests
make test_persistence  # Run persistence snapshot tests
make test_integration  # Run integration workflow tests
```

See [TESTING.md](TESTING.md) for complete testing documentation.

## CLI and Connectors

boxes-live includes a powerful CLI tool for programmatic canvas manipulation:

```bash
# Create canvas
./connectors/boxes-cli create my_canvas.txt

# Add boxes
./connectors/boxes-cli add my_canvas.txt --x 100 --y 100 --title "Task" --color 3

# Search and filter
./connectors/boxes-cli search my_canvas.txt "keyword"
./connectors/boxes-cli list my_canvas.txt --color 2 --json

# Export to multiple formats
./connectors/boxes-cli export my_canvas.txt --format markdown -o output.md
./connectors/boxes-cli export my_canvas.txt --format json -o data.json
```

**Key Features:**
- **Context-efficient**: 13x smaller than Miro API responses
- **Unix composability**: Pipe-friendly text format
- **Agent integration**: JSON mode for AI workflows
- **Multiple exports**: Markdown, JSON, CSV, SVG
- **Offline-first**: No internet required

**Demonstration Projects:**
- Periodic table of elements (`./connectors/periodic2canvas`)
- Project planning canvas (`./examples/cli_demo.sh`)

See [connectors/README.md](connectors/README.md) for complete CLI documentation.
See [CONNECTORS.md](CONNECTORS.md) for connector designs and examples.
See [MCP_SERVER.md](MCP_SERVER.md) for AI agent integration via MCP.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) for terminal manipulation
- Inspired by visual collaboration tools like Miro
- Created with [Claude Code](https://claude.ai/code)
