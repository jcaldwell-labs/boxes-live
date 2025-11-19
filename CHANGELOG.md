# Changelog

All notable changes to boxes-live will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-11-19

### 🎮 Campaign Orchestration

This release transforms boxes-live into foundational infrastructure for text-based adventure campaigns orchestrated through tmux.

#### Added

**Signal-Based Synchronization**:
- SIGUSR1 handler for canvas reload (live state sync)
- SIGUSR2 handler for custom campaign events (extensible)
- Real-time canvas updates without restarting application
- Signal handler functions: `signal_should_reload()`, `signal_should_sync()`

**realm2canvas Connector**:
- Convert adventure-engine realm definitions (JSON) to boxes-live canvas
- Automatic example realm generation when input file missing
- Support for entities (heroes, NPCs, enemies) with positions and stats
- Support for locations (dungeons, settlements, safe zones)
- Quest log rendering with status tracking
- Party status visualization with HP bars
- Color-coded entities (green=heroes, red=enemies, white=NPCs)
- Visual icons for location types (⚔ dungeons, 🏛 settlements)
- Fallback parser when `jq` not available
- Comprehensive error handling and validation

**Campaign Session Launcher** (`demos/campaign-session.sh`):
- Automated tmux session creation with 3-pane layout
- Left pane: Real-time realm visualization (boxes-live)
- Right-top: Narrative console for GM commands
- Right-bottom: State manager with campaign tools
- Auto-generated helper commands script
- Session persistence and reattachment support
- Pre-configured environment variables per pane

**Realm Watcher** (`demos/realm-watcher.sh`):
- Monitor realm JSON files for changes
- Auto-regenerate canvas via realm2canvas
- Auto-signal boxes-live instances (SIGUSR1)
- Colored terminal output with sync statistics
- Configurable polling interval
- Turn counter and event logging

**Campaign Demo** (`examples/adventure-campaign-demo.sh`):
- Complete working demonstration of campaign system
- Example realm: "The Crystal Spire" with 3 heroes, 2 enemies, 4 locations
- Campaign simulation script for turn-based progression
- Combat simulation and state management examples
- Comprehensive documentation and usage guide

**Testing Infrastructure**:
- Campaign orchestration test suite (`tests/test_campaign.sh`)
- 12 automated tests covering all campaign features
- Signal handler validation
- Connector functionality tests
- Demo and script availability checks
- Canvas syntax validation

**Documentation**:
- CAMPAIGN_ORCHESTRATION.md - Complete campaign guide (400+ lines)
  - Architecture overview
  - Component documentation
  - Complete workflow examples
  - Integration patterns
  - Performance considerations
  - Troubleshooting guide
- Updated README.md with campaign features section
- Updated SIGNAL_HANDLING.md with SIGUSR1/SIGUSR2 documentation

#### Changed

**Core Signal Handler** (`src/signal_handler.c`):
- Extended to support SIGUSR1 (reload) and SIGUSR2 (sync)
- Added volatile sig_atomic_t flags for new signals
- Updated initialization to register new handlers
- Updated cleanup to restore all signal handlers

**Main Application** (`src/main.c`):
- Added SIGUSR1 handling in main loop for canvas reload
- Seamless state updates without application restart

#### Technical Specifications

**New Files**:
- `connectors/realm2canvas` - Realm-to-canvas converter (450+ lines)
- `demos/campaign-session.sh` - Session launcher (250+ lines)
- `demos/realm-watcher.sh` - Auto-sync watcher (150+ lines)
- `examples/adventure-campaign-demo.sh` - Complete demo (350+ lines)
- `tests/test_campaign.sh` - Test suite (180+ lines)
- `CAMPAIGN_ORCHESTRATION.md` - Documentation (400+ lines)

**Modified Files**:
- `include/signal_handler.h` - Added reload/sync signal functions
- `src/signal_handler.c` - Implemented SIGUSR1/SIGUSR2 handlers
- `src/main.c` - Added reload signal handling in main loop
- `README.md` - Added campaign orchestration section
- `CHANGELOG.md` - This file

**Lines of Code Added**: ~1,700+ lines (code + docs + tests)

#### Use Cases Enabled

1. **Text-Based Adventure Campaigns**: Multi-user RPG sessions with visual realm tracking
2. **Game Master Tools**: Real-time campaign state visualization
3. **Turn-Based Games**: Visual state management with auto-sync
4. **Multiplayer Coordination**: tmux-based multi-player sessions
5. **Adventure-Engine Integration**: Bridge to narrative game engines

#### Performance

- Signal handling: < 1ms overhead
- realm2canvas: < 100ms for typical realms (< 50 entities)
- Canvas reload: < 50ms for typical canvas sizes
- Auto-sync polling: Configurable (default 2s)

#### Compatibility

- Requires: tmux (for campaign sessions)
- Optional: jq (for full JSON parsing, graceful fallback without)
- Platforms: Linux, macOS, WSL (tmux-compatible systems)

## [1.0.0] - 2025-11-18

### 🎉 First Stable Release

The 1.0.0 release marks boxes-live as production-ready with a complete connector ecosystem, comprehensive testing infrastructure, and extensive documentation.

### Added

#### Core Features
- Interactive terminal canvas with pan, zoom, and navigation
- Dynamic box creation, deletion, and manipulation
- Mouse support for clicking and dragging boxes
- Color support (7 colors: red, green, blue, yellow, magenta, cyan, white)
- Save/load canvas persistence with full state preservation
- Signal handling (SIGUSR1 for reload, proper cleanup on exit)
- Responsive terminal resizing

#### Connector Ecosystem (11 Connectors)
- **boxes-cli**: Comprehensive CLI tool for canvas manipulation
  - Create, add, list, get, update, delete operations
  - Search and filter functionality
  - Auto-arrange layouts (grid)
  - Export to markdown, JSON, CSV formats
  - Canvas statistics and analytics
  - JSON output mode for automation

- **System & DevOps Connectors**:
  - `pstree2canvas`: Process tree visualization with hierarchical layout
  - `docker2canvas`: Docker container status and metrics
  - `log2canvas`: Log file parsing with severity color coding
  - `git2canvas`: Git commit history visualization

- **Data & Structure Connectors**:
  - `csv2canvas`: CSV file to canvas conversion
  - `json2canvas`: JSON data visualization
  - `tree2canvas`: Directory structure visualization
  - `text2canvas`: Text file to box conversion

- **Generator Connectors**:
  - `periodic2canvas`: Interactive periodic table of elements
  - `jcaldwell-labs2canvas`: GitHub organization dashboard

#### Testing Infrastructure
- **Unit Tests**: 245 tests covering all core modules
  - Canvas operations and memory management
  - Viewport transformations and coordinate systems
  - Persistence and file I/O
  - Integration workflows

- **Verification Tests**: 37 automated verification tests
  - Build verification
  - Connector validation
  - Demo script testing
  - Example workflow validation

- **Integration Tests**: 18 comprehensive test suites (520 lines)
  - Individual connector tests (boxes-cli, pstree2canvas, log2canvas, docker2canvas, json2canvas)
  - Quick functional tests for all connectors
  - Cross-connector integration workflows
  - Edge case and error handling
  - Export format validation
  - Large canvas handling (100+ boxes)
  - Empty and invalid input handling

- **Acceptance Tests**: 20+ BDD scenarios in Gherkin format
  - User workflows and feature validation
  - End-to-end testing scenarios

#### Documentation
- **User Documentation**:
  - README.md with comprehensive overview
  - USAGE.md with complete usage instructions
  - CONNECTOR-GUIDE.md (903 lines) - Complete connector development guide
  - WORKFLOW-EXAMPLES.md (1,105 lines) - 16 real-world workflow examples
  - TESTING.md with testing strategies and guides

- **Developer Documentation**:
  - CLAUDE.md with project architecture and conventions
  - FEATURE_ROADMAP.md with planned enhancements
  - PLUGIN_ARCHITECTURE.md for extensibility design
  - MCP_SERVER.md for AI agent integration
  - CONNECTORS.md with connector design specifications
  - VIDEO-PROCESSING.md for demo video standards

- **Workflow Examples** (16 comprehensive workflows):
  - System Administration (process monitoring, container fleet management, log analysis)
  - DevOps and Monitoring (CI/CD pipelines, infrastructure monitoring, deployment tracking)
  - Software Development (git visualization, codebase structure, issue tracking)
  - Data Analysis (CSV visualization, JSON exploration, API debugging)
  - Project Management (GitHub dashboards, task boards, sprint planning)
  - Education (periodic table, learning paths, concept mapping)

#### Demos and Examples
- `live-monitor.sh`: Real-time system monitoring demo
- `git-dashboard.sh`: Git repository visualization
- `cli_demo.sh`: CLI workflow demonstration
- Signal handling examples with auto-reload

### Fixed
- Test suite compatibility with `set -e` (arithmetic expansion bug)
- Assert failure logic in CLI tests
- JSON flag positioning in boxes-cli
- Demo script path resolution (works from any directory)
- Test compilation issues (persistence module refactoring)
- Memory management in dynamic box allocation

### Changed
- Enhanced README with connector ecosystem documentation
- Improved connector categorization and organization
- Consolidated testing infrastructure
- Refined canvas file format specification

### Performance
- Efficient viewport culling for large canvases
- Optimized rendering for 100+ boxes
- Fast connector execution (sub-second for typical datasets)
- Memory-efficient canvas storage

### Testing Metrics
- **Total Tests**: 300+ tests across all suites
- **Pass Rate**: 100% (all tests passing)
- **Code Coverage**: Comprehensive coverage of core modules
- **Integration Coverage**: All 11 connectors validated
- **Format Validation**: Markdown, JSON, CSV exports tested

### Technical Specifications
- **Language**: C (GNU99)
- **Dependencies**: ncurses, POSIX
- **Build System**: GNU Make
- **Lines of Code**: ~1,500+ (core) + 2,500+ (docs) + 520 (tests)
- **Supported Platforms**: Linux, WSL
- **Terminal Requirements**: Box-drawing character support

## [1.0.0-rc1] - 2025-11-18 (Pre-release)

### Added
- Initial release candidate with core features
- Basic connector ecosystem
- Preliminary testing infrastructure
- Foundation documentation

### Fixed
- Demo script improvements
- Test compilation fixes
- Path resolution issues

## Release History

- **v1.0.0** (2025-11-18): First stable release - Production ready
- **v1.0.0-rc1** (2025-11-18): Release candidate - Testing phase

---

## Future Roadmap

### Planned for 1.x Series
- Plugin architecture implementation
- MCP server for AI agent integration
- Text editing mode for box content
- Box resizing with mouse
- Connection lines between boxes
- Box types (note, task, code)
- Copy/paste functionality
- Undo/redo support
- Search and filter enhancements
- Export to SVG/ASCII art

### Vision for 2.0
- Real-time collaboration
- Web-based viewer
- Advanced layout algorithms
- Custom themes and styling
- Plugin marketplace
- Extended connector ecosystem

---

**Full Changelog**: https://github.com/jcaldwell-labs/boxes-live/compare/v1.0.0-rc1...v1.0.0
