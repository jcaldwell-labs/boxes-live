# Boxes-Live

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)

A terminal-based interactive canvas application written in C, similar to Miro boards but for the terminal.

## Why Boxes-Live?

**A visual canvas that lives in your terminal.** Stop context-switching to browser-based tools. Boxes-live brings collaborative canvas capabilities directly to your workflow.

- **Terminal-native**: Works where you already work - no browser required
- **Unix composable**: Pipe-friendly text format integrates with standard tools
- **Context-efficient**: 13x smaller than Miro API responses - perfect for AI agents
- **Offline-first**: Full functionality without internet connectivity
- **Lightweight**: ~1500 lines of C, minimal dependencies

**Perfect for:**
- Developers who live in the terminal
- AI/LLM agents needing visual workspace context
- System administrators creating dashboards
- Game masters running tabletop campaigns
- Anyone tired of switching to web-based canvas tools

## Quick Start

### 1. Install Dependencies

```bash
# Debian/Ubuntu
sudo apt-get install build-essential libncurses-dev

# Fedora/RHEL
sudo dnf install gcc ncurses-devel

# macOS
brew install ncurses
```

### 2. Build and Run

```bash
git clone https://github.com/jcaldwell-labs/boxes-live.git
cd boxes-live
make
./boxes-live
```

### 3. First Steps

```
[Arrow Keys]  Pan the viewport
[N]           Create a new box
[Click]       Select a box
[Drag]        Move selected box
[F2]          Save your canvas
[Q]           Quit
```

That's it! You're now navigating an infinite 2D canvas in your terminal.

## Demo

<!-- TODO: Add asciinema recording -->
```
Try it yourself:
$ ./boxes-live
$ # Press 'N' to create boxes, arrow keys to pan, +/- to zoom
```

## Features

### Core Canvas
- **Infinite Canvas**: Navigate a large 2D workspace with pan and zoom
- **Dynamic Memory**: Unlimited boxes (auto-grows as needed)
- **Mouse & Keyboard**: Full support for both input methods
- **Box Management**: Create, delete, move, resize, and color boxes
- **Display Modes**: Compact, Preview, and Full view modes
- **Box Types**: NOTE, TASK, CODE, STICKY with customizable icons
- **Save/Load**: Persist canvas to file and reload later

### Connector Ecosystem
Transform any data source into an interactive visual canvas:

| Connector | Purpose |
|-----------|---------|
| `pstree2canvas` | Process tree visualization |
| `docker2canvas` | Docker container status |
| `git2canvas` | Git commit history |
| `csv2canvas` | CSV data visualization |
| `log2canvas` | Log file parsing |
| `boxes-cli` | CLI canvas manipulation |

### Joystick Support (v1.2+)
Full gamepad integration with analog sticks, navigation/edit modes, and parameter adjustment.

### Campaign Orchestration (v1.1+)
Run tabletop RPG sessions with tmux integration, realm-to-canvas conversion, and real-time sync.

## Comparison

| Feature | Boxes-Live | Miro | Vim/Tmux |
|---------|------------|------|----------|
| Terminal-native | Yes | No | Yes |
| Visual canvas | Yes | Yes | No |
| Offline | Yes | Limited | Yes |
| AI-friendly format | Yes | No | Partial |
| Pipe composable | Yes | No | Partial |
| Gamepad support | Yes | No | No |

## Documentation

- **[USAGE.md](USAGE.md)** - Complete usage instructions
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System architecture
- **[JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md)** - Gamepad controls
- **[CAMPAIGN_ORCHESTRATION.md](CAMPAIGN_ORCHESTRATION.md)** - RPG integration
- **[Connector Guide](docs/CONNECTOR-GUIDE.md)** - Data connectors
- **[Workflow Examples](docs/WORKFLOW-EXAMPLES.md)** - Real-world patterns

See [docs/](docs/) for all documentation.

## Controls Reference

### Keyboard

| Key | Action |
|-----|--------|
| Arrow Keys / WASD | Pan viewport |
| + / - or Z / X | Zoom in/out |
| N | Create new box |
| D | Delete selected box |
| T | Cycle box type |
| Tab | Cycle display mode |
| 1-7 | Color selected box |
| F2 / F3 | Save / Load canvas |
| R / 0 | Reset view |
| Q / ESC | Quit |

### Mouse

| Action | Result |
|--------|--------|
| Click | Select box |
| Drag | Move selected box |
| Click empty | Pan viewport |

## Project Stats

- **Language**: C (GNU99)
- **Lines of Code**: ~1500+
- **Dependencies**: ncurses, POSIX
- **Test Coverage**: 245 assertions across 31 test cases
- **Version**: 1.2.0

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

```bash
make test        # Run test suite (245 assertions)
make valgrind    # Memory leak detection
```

## Roadmap

See [.github/planning/ROADMAP.md](.github/planning/ROADMAP.md) for planned features.

**Current priorities:**
- Text editing mode (in-place content editing)
- Connection lines between boxes
- Box resizing with mouse
- Undo/redo functionality

## Community

- **Issues**: [GitHub Issues](https://github.com/jcaldwell-labs/boxes-live/issues)
- **Discussions**: [GitHub Discussions](https://github.com/jcaldwell-labs/boxes-live/discussions)
- **Creator**: [@jcaldwell-labs](https://github.com/jcaldwell-labs)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/)
- Inspired by [Miro](https://miro.com) and [Excalidraw](https://excalidraw.com)
- Created with [Claude Code](https://claude.ai/code)
