# Boxes-Live

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A terminal-based interactive canvas application written in C, similar to Miro boards but for the terminal.

## Overview

Boxes-live provides a pan-and-zoom interface for navigating a large 2D canvas containing boxes with content. It uses ncurses for terminal manipulation and box-drawing characters, creating smooth 3D-like effects through viewport transformations.

## Features

✅ **Phase 1 Complete** - Foundation features implemented!
✅ **Phase 2 (Partial)** - Smart Canvas features added!
✅ **NEW in v1.1** - Campaign Orchestration! 🎲

### Core Features
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

### Campaign Orchestration (v1.1+) 🎮
- **tmux Integration**: Multi-pane campaign sessions for adventure games
- **Signal-Based Sync**: SIGUSR1 for real-time canvas reload, SIGUSR2 for events
- **realm2canvas Connector**: Convert adventure-engine realms (JSON) to canvas
- **Auto-Sync Watcher**: Monitor realm files and auto-update visualization
- **Campaign Sessions**: Pre-configured tmux layouts for GM + players
- **Entity Management**: Heroes, NPCs, enemies with positions and stats
- **Quest Tracking**: Visual quest log and state management
- **See**: [CAMPAIGN_ORCHESTRATION.md](CAMPAIGN_ORCHESTRATION.md) for full guide

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

#### Keyboard

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

#### Joystick (NEW in v1.2!) 🎮

**Navigation Mode** (Default):
- **Left Stick**: Pan viewport (smooth analog control)
- **Button A**: Zoom in
- **Button B**: Zoom out
- **Button X**: Create new box at cursor
- **Button Y**: Delete selected box
- **Start**: Save canvas
- **Select**: Load canvas

**Edit Mode** (When box selected):
- **Left Stick**: Move selected box
- **Button A**: Enter parameter mode
- **Button B**: Return to navigation
- **Button X**: Cycle box color
- **Button Y**: Delete box

**Parameter Mode** (Adjust box properties):
- **Left Stick Y**: Select parameter (width/height/color)
- **Left Stick X**: Adjust value
- **Button A**: Confirm
- **Button B**: Cancel

See [JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md) for complete joystick documentation.

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

## Connector Ecosystem

boxes-live features a powerful connector ecosystem that transforms various data sources into interactive visual canvases.

### Quick Start with Connectors

```bash
# Visualize process tree
pstree -p | ./connectors/pstree2canvas > processes.txt

# Visualize Docker containers
docker ps | ./connectors/docker2canvas > containers.txt

# Visualize git history
./connectors/git2canvas --max 20 > git_history.txt

# Parse and visualize logs
./connectors/log2canvas /var/log/syslog > logs.txt

# Convert CSV data
./connectors/csv2canvas data.csv > data_canvas.txt

# View interactively
./boxes-live  # Press F3, load any .txt file
```

### Available Connectors

**System & DevOps:**
- `pstree2canvas` - Process tree visualization with hierarchical layout
- `docker2canvas` - Docker container status and metrics
- `log2canvas` - Log file parsing with severity color coding
- `git2canvas` - Git commit history visualization

**Data & Structure:**
- `csv2canvas` - CSV file to visual canvas conversion
- `json2canvas` - JSON data visualization
- `tree2canvas` - Directory structure visualization
- `text2canvas` - Text file to box conversion

**Generators & Templates:**
- `periodic2canvas` - Interactive periodic table of elements
- `jcaldwell-labs2canvas` - GitHub organization dashboard template

**CLI Tool:**
- `boxes-cli` - Comprehensive canvas manipulation tool

### boxes-cli: Canvas Manipulation Tool

```bash
# Create and edit canvases
./connectors/boxes-cli create my_canvas.txt
./connectors/boxes-cli add my_canvas.txt --x 100 --y 100 --title "Task" --color 3

# Search and filter
./connectors/boxes-cli search my_canvas.txt "keyword"
./connectors/boxes-cli list my_canvas.txt --color 2 --json

# Auto-arrange
./connectors/boxes-cli arrange my_canvas.txt --layout grid

# Export to multiple formats
./connectors/boxes-cli export my_canvas.txt --format markdown -o output.md
./connectors/boxes-cli export my_canvas.txt --format json -o data.json
./connectors/boxes-cli export my_canvas.txt --format csv -o data.csv

# Statistics
./connectors/boxes-cli stats my_canvas.txt
```

### Key Features

- **Context-efficient**: 13x smaller than Miro API responses
- **Unix composability**: Pipe-friendly text format
- **Agent integration**: JSON mode for AI workflows
- **Multiple formats**: Markdown, JSON, CSV export
- **Offline-first**: No internet required
- **Comprehensive testing**: 245+ unit tests, extensive integration tests

### Workflow Examples

**System Monitoring:**
```bash
# Monitor processes in real-time
while true; do
    pstree -p | ./connectors/pstree2canvas > monitor.txt
    sleep 10
done
```

**DevOps Dashboard:**
```bash
# Generate infrastructure overview
docker ps | ./connectors/docker2canvas > infrastructure.txt
./connectors/boxes-cli add infrastructure.txt \
    --title "System Status" \
    --content "Last updated: $(date)"
```

**Data Analysis:**
```bash
# Visualize and analyze CSV data
./connectors/csv2canvas sales.csv > sales.txt
./connectors/boxes-cli arrange sales.txt --layout grid
./connectors/boxes-cli export sales.txt --format markdown > report.md
```

### Documentation

- **[Connector Guide](docs/CONNECTOR-GUIDE.md)** - Complete guide for using and developing connectors
- **[Workflow Examples](docs/WORKFLOW-EXAMPLES.md)** - Real-world use cases and patterns
- **[CLI Reference](connectors/README.md)** - Complete boxes-cli documentation
- **[Testing Guide](TESTING.md)** - Connector testing best practices

See also:
- [CONNECTORS.md](CONNECTORS.md) - Connector design specifications
- [MCP_SERVER.md](MCP_SERVER.md) - AI agent integration via MCP
- [PLUGIN_ARCHITECTURE.md](PLUGIN_ARCHITECTURE.md) - Live-updating plugins

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) for terminal manipulation
- Inspired by visual collaboration tools like Miro
- Created with [Claude Code](https://claude.ai/code)
