# boxes-live Feature Planning & Architecture Recommendations

**Document Version**: 1.0
**Date**: 2025-11-18
**Based On**: PR #3 & #4 Review
**Timeframe**: 1.0.0 Final through 2.0.0

---

## Executive Summary

Based on comprehensive review of PRs #3 and #4, this document provides:
1. Short-term improvements for 1.0.0 final release
2. Medium-term features for 1.x series
3. Long-term vision for 2.0.0+
4. Architecture recommendations
5. Technical debt assessment
6. Prioritized roadmap with effort estimates

**Current State**: boxes-live is ready for 1.0.0-rc1 with 245 passing tests and a solid foundation.

**Strategic Direction**: Maintain Unix philosophy while expanding ecosystem integration and plugin extensibility.

---

## Table of Contents

1. [Short-Term (1.0.0 Final)](#short-term-100-final)
2. [Medium-Term (1.x Series)](#medium-term-1x-series)
3. [Long-Term (2.0.0+)](#long-term-200)
4. [Architecture Recommendations](#architecture-recommendations)
5. [Technical Debt](#technical-debt)
6. [Prioritized Roadmap](#prioritized-roadmap)
7. [Integration Opportunities](#integration-opportunities)

---

## Short-Term (1.0.0 Final)

**Timeline**: Next 2-4 weeks
**Goal**: Polish 1.0.0-rc1 into production-ready 1.0.0

### 1.1 Critical Fixes from Review

#### 1.1.1 Memory Leaks in Test Suite
**Priority**: HIGH
**Effort**: 2 hours
**Status**: Identified in PR #3 review

**Issue**:
```
LEAK SUMMARY (valgrind):
   definitely lost: 3,584 bytes in 4 blocks
```

**Root Cause**: `canvas_cleanup()` not called in some test functions

**Files Affected**:
- tests/test_persistence.c
- tests/test_integration.c (potentially)

**Solution**:
```c
// Add cleanup to all test functions
void test_something(void) {
    Canvas canvas;
    canvas_init(&canvas, 200, 150);

    // ... test code ...

    canvas_cleanup(&canvas);  // <-- Add this
}
```

**Acceptance Criteria**:
- valgrind shows 0 bytes lost in all test binaries
- All 245 tests still pass

#### 1.1.2 PR #4 Canvas Loading Bug
**Priority**: CRITICAL
**Effort**: 4 hours
**Status**: Blocking PR #4 merge

**Issue**: boxes-cli generates canvases that boxes-live cannot load

**Root Cause**: boxes-cli doesn't handle multiple `--content` arguments with empty strings properly

**Solution Options**:
1. **Fix boxes-cli** (RECOMMENDED): Modify Python script to handle empty content
2. **Modify connectors**: Avoid using empty strings in --content
3. **Document limitation**: Defer to future release

**Files to Fix**:
- connectors/boxes-cli (Python, ~line 100-150)

**Acceptance Criteria**:
- `./connectors/jcaldwell-labs2canvas` generates loadable canvas
- `./boxes-live <generated-canvas>` loads successfully
- boxes-cli list/stats work on generated files

### 1.2 Polish & Documentation

#### 1.2.1 Help Text Improvements
**Priority**: MEDIUM
**Effort**: 2 hours

**Current State**: Most connectors have basic --help

**Enhancements**:
```bash
# Add examples to each connector help
./connectors/git2canvas --help

# Should show:
EXAMPLES:
  # Last 20 commits on current branch
  git2canvas --max 20 | boxes-live

  # Feature branch commits
  git2canvas --branch feature/foo | boxes-live

  # Save to file
  git2canvas --max 10 > git-history.txt
```

**Files**:
- connectors/git2canvas
- connectors/csv2canvas
- connectors/tree2canvas
- connectors/text2canvas
- connectors/docker2canvas
- connectors/json2canvas
- connectors/log2canvas
- connectors/pstree2canvas
- connectors/periodic2canvas

#### 1.2.2 README Enhancements
**Priority**: MEDIUM
**Effort**: 1 hour

**Add**:
- Screenshots from demos
- Quick start guide (30-second version)
- Link to demo videos
- Badge for test status

#### 1.2.3 CHANGELOG for 1.0.0
**Priority**: HIGH
**Effort**: 1 hour

Create comprehensive changelog documenting:
- All features in 1.0.0
- Breaking changes (if any)
- Migration guide from dev versions
- Known limitations
- Credits and acknowledgments

### 1.3 Release Engineering

#### 1.3.1 Release Automation
**Priority**: MEDIUM
**Effort**: 3 hours

**Create**:
- `scripts/release.sh` - Automates release process
- GitHub Actions workflow for releases
- Automated artifact building

**Process**:
1. Version bump in Makefile/headers
2. Update CHANGELOG.md
3. Run full test suite
4. Create git tag
5. Build release artifacts
6. Generate release notes
7. Publish to GitHub releases

#### 1.3.2 Installation Script
**Priority**: LOW
**Effort**: 2 hours

Create `install.sh` for easy system-wide installation:
```bash
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/boxes-live/main/install.sh | bash
```

**Installs to**:
- `/usr/local/bin/boxes-live`
- `/usr/local/bin/boxes-cli`
- `/usr/local/share/boxes-live/connectors/*`

---

## Medium-Term (1.x Series)

**Timeline**: 1-3 months after 1.0.0
**Goal**: Expand capabilities while maintaining stability

### 2.1 Core Features

#### 2.1.1 Enhanced Persistence (v1.1.0)
**Priority**: HIGH
**Effort**: 8 hours

**Current State**: Basic save/load working

**Enhancements**:
1. **Multiple Save Slots**
   - F2: Quick save to last file
   - Shift+F2: Save as... (prompt for filename)
   - Recent files menu (F4)

2. **Auto-Save**
   - Configurable interval (default: 5 minutes)
   - Save to `.boxes-live-autosave.txt`
   - Recovery on crash

3. **Export Formats**
   - SVG export for documentation
   - HTML export with interactive viewer
   - PNG export (requires imagemagick)

**Files**:
- `src/persistence.c` - Enhance save/load
- `src/input.c` - Add new keybindings
- `src/export.c` - New file for export functions

#### 2.1.2 Box Grouping and Layers (v1.2.0)
**Priority**: MEDIUM
**Effort**: 16 hours

**New Capabilities**:
1. **Groups**
   - Select multiple boxes (Ctrl+Click)
   - Group selected boxes (Ctrl+G)
   - Move groups together
   - Collapse/expand groups

2. **Layers**
   - Background, main, foreground layers
   - Layer visibility toggle
   - Layer opacity
   - Layer z-order management

**New Data Structures**:
```c
typedef struct Group {
    int id;
    char *name;
    int *box_ids;
    int box_count;
    bool collapsed;
} Group;

typedef enum Layer {
    LAYER_BACKGROUND = 0,
    LAYER_MAIN = 1,
    LAYER_FOREGROUND = 2
} Layer;

// Add to Box struct:
int group_id;    // -1 if not grouped
Layer layer;
```

**UI**:
- Ctrl+1, Ctrl+2, Ctrl+3: Switch layers
- Ctrl+G: Group selected boxes
- Ctrl+Shift+G: Ungroup

#### 2.1.3 Box Connections and Arrows (v1.3.0)
**Priority**: HIGH
**Effort**: 20 hours

**Feature**: Visual connections between boxes

**Capabilities**:
- Draw arrows from box to box
- Arrow types: straight, curved, stepped
- Arrow labels
- Connection metadata (relationship type)

**New Data Structures**:
```c
typedef enum ArrowType {
    ARROW_STRAIGHT,
    ARROW_CURVED,
    ARROW_STEPPED
} ArrowType;

typedef struct Connection {
    int id;
    int from_box_id;
    int to_box_id;
    ArrowType type;
    char *label;
    int color;
} Connection;

typedef struct Canvas {
    // ... existing fields ...
    Connection *connections;
    int connection_count;
    int connection_capacity;
} Canvas;
```

**UI**:
- 'C': Create connection mode
- Click box 1, click box 2: Create arrow
- Hover connection: Show label
- Delete connection: Select + 'D'

**Use Cases**:
- Dependency graphs
- Workflow diagrams
- Mind maps
- System architecture

#### 2.1.4 Rich Text in Boxes (v1.4.0)
**Priority**: MEDIUM
**Effort**: 12 hours

**Current**: Plain text only

**Enhancements**:
- **Markdown Support**: Render `**bold**`, `*italic*`, `- lists`
- **Color Coding**: `{red}text{/red}` for inline colors
- **Icons**: ASCII art mini-icons
- **Links**: `[url]http://example.com[/url]` (stored, not clickable)

**Example**:
```
┌─────────────────────────┐
│ **Task**: Implement     │
│                         │
│ {green}✓{/green} Design │
│ {yellow}○{/yellow} Code │
│ {red}✗{/red} Test       │
│                         │
│ [url]github.com/…[/url] │
└─────────────────────────┘
```

**Implementation**:
- Markdown parser (use existing library or minimal impl)
- Rendering updates in render.c
- Format preservation in persistence

### 2.2 Connector Ecosystem

#### 2.2.1 Connector Framework Library (v1.5.0)
**Priority**: MEDIUM
**Effort**: 16 hours

**Problem**: Connectors duplicate common patterns

**Solution**: Extract common functionality

**New Files**:
- `connectors/lib/connector-framework.sh`
- `connectors/lib/layout-algorithms.sh`
- `connectors/lib/color-schemes.sh`

**connector-framework.sh**:
```bash
#!/bin/bash

# Source this in connectors
source "$(dirname "$0")/lib/connector-framework.sh"

# Provides:
connector_init()        # Setup canvas
connector_add_box()     # Simplified box adding
connector_layout_grid() # Auto grid layout
connector_layout_tree() # Tree layout
connector_layout_timeline() # Timeline layout
connector_output()      # Finalize and output

# Example usage:
connector_init "My Canvas" 2000 1500

for item in $data; do
    connector_add_box "$title" "$content" --color auto
done

connector_layout_grid 3 columns
connector_output
```

**Benefits**:
- Faster connector development
- Consistent patterns
- Easier maintenance
- Better error handling

#### 2.2.2 Dynamic Data Connectors (v1.5.0)
**Priority**: HIGH
**Effort**: 24 hours

**Current**: Most connectors read static data or current state

**New Connectors**:

1. **github2canvas** - Real-time GitHub data
   ```bash
   github2canvas --org jcaldwell-labs \
                 --include-prs \
                 --include-issues \
                 --api-token $GITHUB_TOKEN
   ```

2. **jira2canvas** - Jira project visualization
   ```bash
   jira2canvas --project MYPROJ \
               --jql "sprint = 'Sprint 10'" \
               --group-by status
   ```

3. **database2canvas** - SQL query visualization
   ```bash
   database2canvas --db postgres://... \
                   --query "SELECT * FROM tasks" \
                   --title-col name \
                   --group-by status
   ```

4. **logs2canvas** - Real-time log aggregation
   ```bash
   tail -f /var/log/app.log | \
       logs2canvas --pattern "ERROR|WARN" \
                   --window 100
   ```

**Architecture**:
- API client libraries
- Authentication handling
- Rate limiting
- Caching
- Incremental updates

#### 2.2.3 Bidirectional Connectors (v1.6.0)
**Priority**: LOW
**Effort**: 32 hours

**Current**: Connectors are read-only (data → canvas)

**New**: Write-back capability (canvas → data)

**Example**:
```bash
# Import Jira tickets
jira2canvas --project MYPROJ > tickets.txt

# Edit in boxes-live (move boxes, update status)
boxes-live tickets.txt

# Write changes back to Jira
canvas2jira tickets.txt --update --api-token $JIRA_TOKEN
```

**Use Cases**:
- Task management
- Project planning
- Issue triaging
- Sprint planning

**Challenges**:
- Conflict resolution
- Partial updates
- Permissions
- Validation

### 2.3 Plugin System (v1.7.0)

**Priority**: HIGH
**Effort**: 40 hours

**Reference**: PLUGIN_ARCHITECTURE.md (already exists in repo)

**Goal**: Allow third-party extensions without forking

**Capabilities**:
1. **Custom Renderers**: Alternative box styles
2. **Input Handlers**: Custom key bindings
3. **Export Formats**: New export types
4. **Data Sources**: Custom connectors
5. **Themes**: Color schemes and UI customization

**Architecture**:
```
boxes-live
├── plugins/
│   ├── README.md
│   ├── example-plugin.c
│   └── enabled/
│       ├── plugin-name.so
│       └── plugin-config.json
└── src/plugin.c (plugin manager)
```

**Plugin API**:
```c
// Plugin interface
typedef struct Plugin {
    const char *name;
    const char *version;
    int (*init)(Canvas *canvas);
    int (*render)(Canvas *canvas, Viewport *vp);
    int (*handle_input)(int key);
    void (*cleanup)(void);
} Plugin;

// Register plugin
PLUGIN_EXPORT Plugin *plugin_register(void) {
    static Plugin my_plugin = {
        .name = "My Plugin",
        .version = "1.0.0",
        .init = my_init,
        .render = my_render,
        .handle_input = my_handle_input,
        .cleanup = my_cleanup
    };
    return &my_plugin;
}
```

**Plugin Examples**:
1. **vim-bindings**: Vim-style navigation
2. **emacs-bindings**: Emacs-style key bindings
3. **themes**: Solarized, Dracula, etc.
4. **box-shapes**: Rounded corners, shadows
5. **minimap**: Show canvas overview

### 2.4 MCP Server Integration (v1.8.0)

**Priority**: HIGH
**Effort**: 24 hours

**Reference**: MCP_SERVER.md (already exists in repo)

**Goal**: Expose boxes-live as MCP server for AI agents

**Capabilities**:
1. **Canvas Management**: Create, list, load canvases
2. **Box Operations**: Add, update, delete, search boxes
3. **Layout**: Auto-layout algorithms
4. **Export**: Generate canvas in multiple formats
5. **Query**: Search and filter boxes

**MCP Tools Exposed**:
```json
{
  "tools": [
    {
      "name": "create_canvas",
      "description": "Create a new canvas",
      "inputSchema": {
        "type": "object",
        "properties": {
          "width": {"type": "number"},
          "height": {"type": "number"},
          "title": {"type": "string"}
        }
      }
    },
    {
      "name": "add_box",
      "description": "Add a box to canvas",
      "inputSchema": {
        "type": "object",
        "properties": {
          "canvas_id": {"type": "string"},
          "x": {"type": "number"},
          "y": {"type": "number"},
          "title": {"type": "string"},
          "content": {"type": "array"}
        }
      }
    },
    {
      "name": "layout_auto",
      "description": "Auto-layout boxes",
      "inputSchema": {
        "type": "object",
        "properties": {
          "canvas_id": {"type": "string"},
          "algorithm": {"enum": ["grid", "tree", "force"]}
        }
      }
    }
  ]
}
```

**Use Cases**:
- AI agents create project visualizations
- Claude generates mind maps from conversations
- Automated documentation generation
- Visual code analysis

**Integration with jcaldwell-labs**:
- my-context queries → canvas visualizations
- Codebase analysis → architecture diagrams
- Issue tracking → progress boards

---

## Long-Term (2.0.0+)

**Timeline**: 6-12 months
**Goal**: Major architectural evolution

### 3.1 Client-Server Architecture

**Problem**: boxes-live is single-user, local only

**Vision**: Collaborative canvas editing

**Architecture**:
```
┌─────────────────┐
│  boxes-live-cli │ ← Terminal UI (existing + enhancements)
└────────┬────────┘
         │ REST/WebSocket
┌────────▼────────┐
│boxes-live-server│ ← New: Server component
│  - Multi-user   │
│  - Persistence  │
│  - Real-time    │
└────────┬────────┘
         │
┌────────▼────────┐
│   PostgreSQL    │ ← Canvas storage
│   or SQLite     │
└─────────────────┘
```

**Capabilities**:
- Multiple users editing same canvas
- Real-time updates
- Access control
- Version history
- Cloud storage

**Protocol**:
```
# REST API
GET    /canvases
POST   /canvases
GET    /canvases/:id
PUT    /canvases/:id
DELETE /canvases/:id

# WebSocket
/ws/canvas/:id
  → {type: "box_moved", box_id: 1, x: 100, y: 200}
  ← {type: "box_updated", box: {...}}
```

### 3.2 Web Frontend

**Complement to Terminal UI**

**Technology**: Lightweight SPA
- Pure JS (no framework bloat)
- Canvas API for rendering
- WebSocket for real-time
- Responsive design

**Features**:
- View canvases in browser
- Edit boxes (WYSIWYG)
- Export and share
- Embed canvases in docs

**Architecture**:
```
boxes-live-web/
├── index.html
├── css/
│   └── boxes-live.css
├── js/
│   ├── canvas-renderer.js
│   ├── box-editor.js
│   ├── websocket-client.js
│   └── main.js
└── server.js (optional static server)
```

### 3.3 Mobile Viewers

**iOS/Android apps for viewing canvases**

**Scope**:
- Read-only initially
- Sync with server
- Offline support
- Share to canvas from phone

**Technology**:
- React Native or Flutter
- Shared server backend

### 3.4 Advanced Rendering

**Current**: ASCII box-drawing characters

**Future**:
- **Sixel graphics**: Raster images in terminal
- **Kitty graphics protocol**: Better image support
- **True color**: 24-bit color
- **Unicode symbols**: Rich icons
- **SVG export**: Vector graphics

---

## Architecture Recommendations

### 4.1 Current Architecture Assessment

**Strengths**:
- ✅ Clean module separation
- ✅ Clear responsibility boundaries
- ✅ Efficient rendering with viewport culling
- ✅ Good coordinate transformation abstraction
- ✅ Extensible canvas format

**Weaknesses**:
- ⚠️ Global state coupling (get_current_file issue - now fixed)
- ⚠️ Limited error handling in some paths
- ⚠️ No plugin system yet
- ⚠️ Tight coupling between UI and logic

### 4.2 Recommended Refactorings

#### 4.2.1 Separate Core from UI (v1.6.0)

**Current**:
```
src/
├── canvas.c      (mixed: data + some UI logic)
├── input.c       (UI)
├── render.c      (UI)
├── viewport.c    (mixed)
└── persistence.c (core)
```

**Proposed**:
```
src/
├── core/
│   ├── canvas.c       (pure data structures)
│   ├── persistence.c  (I/O)
│   ├── validation.c   (canvas validation)
│   └── transform.c    (coordinate math)
├── ui/
│   ├── terminal.c     (ncurses)
│   ├── input.c        (key handling)
│   ├── render.c       (drawing)
│   └── viewport.c     (camera)
└── plugins/
    └── plugin.c       (plugin manager)
```

**Benefits**:
- Core can be used by MCP server
- Core can be tested without UI
- Easier to add web frontend
- Clear API boundaries

#### 4.2.2 Introduce Event System

**Current**: Direct function calls

**Proposed**: Event-driven architecture

```c
// Event types
typedef enum EventType {
    EVENT_BOX_ADDED,
    EVENT_BOX_REMOVED,
    EVENT_BOX_MOVED,
    EVENT_BOX_SELECTED,
    EVENT_CANVAS_LOADED,
    EVENT_CANVAS_SAVED
} EventType;

// Event structure
typedef struct Event {
    EventType type;
    void *data;
    time_t timestamp;
} Event;

// Event handler
typedef void (*EventHandler)(Event *event);

// Register handler
void canvas_on(EventType type, EventHandler handler);

// Emit event
void canvas_emit(EventType type, void *data);
```

**Benefits**:
- Plugins can listen to events
- Undo/redo becomes easier
- Collaboration support
- Audit logging

#### 4.2.3 Configuration System

**Current**: Hardcoded values

**Proposed**: Config file + CLI overrides

**Config File** (`~/.config/boxes-live/config.toml`):
```toml
[general]
autosave_interval = 300  # seconds
max_recent_files = 10

[ui]
theme = "default"
show_grid = false
snap_to_grid = false
grid_size = 10

[keybindings]
pan_up = "w,Up"
pan_down = "s,Down"
pan_left = "a,Left"
pan_right = "d,Right"
zoom_in = "+,z"
zoom_out = "-,x"

[colors]
background = "#1e1e1e"
box_default = "#3c3c3c"
box_selected = "#ffcc00"
```

**Implementation**:
```c
// config.h
typedef struct Config {
    int autosave_interval;
    int max_recent_files;
    char *theme;
    bool show_grid;
    // ... more fields
} Config;

Config *config_load(const char *path);
void config_save(Config *config, const char *path);
void config_free(Config *config);
```

---

## Technical Debt

### 5.1 Identified Debt

#### Priority 1: High Impact, Low Effort

1. **Test Memory Leaks** (2 hours)
   - Fix: Add canvas_cleanup() calls
   - Impact: Clean valgrind reports

2. **PR #4 Canvas Loading** (4 hours)
   - Fix: boxes-cli --content handling
   - Impact: Unblocks PR merge

3. **Error Handling in Scripts** (2 hours)
   - Fix: Add error checks in connectors
   - Impact: Better failure messages

#### Priority 2: High Impact, Medium Effort

4. **Persistent File Tracking** (4 hours)
   - Current: persistence_set_current_file stores pointer
   - Risk: Dangling pointer if caller frees
   - Fix: strdup() the filename
   - Files: src/persistence.c

5. **Input Validation** (8 hours)
   - Current: Basic validation in persistence.c
   - Needed: Comprehensive validation
   - Add: bounds checks, format validation, security

6. **Error Messages** (4 hours)
   - Current: Generic "Failed to load"
   - Needed: Specific error codes and messages
   - Example: "Canvas format error: expected number on line 3"

#### Priority 3: Medium Impact, Medium Effort

7. **Code Duplication in Connectors** (16 hours)
   - Current: Each connector duplicates setup/teardown
   - Fix: Connector framework library
   - See: Section 2.2.1

8. **boxes-cli Python Quality** (8 hours)
   - Current: Basic script
   - Needed: Proper error handling, typing, tests
   - Add: Type hints, docstrings, unit tests

### 5.2 Debt Payoff Plan

**Sprint 1 (Post-1.0.0)**:
- Item 1: Test memory leaks
- Item 2: PR #4 canvas loading
- Item 3: Error handling in scripts

**Sprint 2**:
- Item 4: Persistent file tracking
- Item 6: Error messages

**Sprint 3**:
- Item 5: Input validation
- Item 8: boxes-cli quality

**Sprint 4**:
- Item 7: Connector framework

---

## Prioritized Roadmap

### Phase 1: 1.0.0 Final (2-4 weeks)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Fix test memory leaks | HIGH | 2h | Clean tests |
| Fix PR #4 canvas bug | CRITICAL | 4h | Merge PR #4 |
| Add help examples | MEDIUM | 2h | Better UX |
| Create CHANGELOG | HIGH | 1h | Release docs |
| Polish README | MEDIUM | 1h | First impressions |

**Total**: ~10 hours

### Phase 2: 1.1.0 - Enhanced Persistence (1 month)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Multiple save slots | HIGH | 8h | Better UX |
| Error handling | HIGH | 4h | Stability |
| boxes-cli quality | MEDIUM | 8h | Robustness |

**Total**: ~20 hours

### Phase 3: 1.2.0-1.4.0 - Core Features (2 months)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Box grouping/layers | MEDIUM | 16h | Organization |
| Box connections | HIGH | 20h | Relationships |
| Rich text | MEDIUM | 12h | Better content |
| Input validation | HIGH | 8h | Security |

**Total**: ~56 hours

### Phase 4: 1.5.0-1.6.0 - Ecosystem (2 months)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Connector framework | MEDIUM | 16h | Developer UX |
| Dynamic connectors | HIGH | 24h | Real-time data |
| Architecture refactor | HIGH | 16h | Maintainability |
| Config system | MEDIUM | 8h | Customization |

**Total**: ~64 hours

### Phase 5: 1.7.0-1.8.0 - Extensibility (3 months)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Plugin system | HIGH | 40h | Extensibility |
| MCP server | HIGH | 24h | AI integration |
| Bidirectional connectors | LOW | 32h | Workflow tool |

**Total**: ~96 hours

### Phase 6: 2.0.0 - Major Evolution (6 months)

| Item | Priority | Effort | Impact |
|------|----------|--------|--------|
| Client-server arch | HIGH | 80h | Collaboration |
| Web frontend | MEDIUM | 60h | Accessibility |
| Mobile viewers | LOW | 40h | Convenience |
| Advanced rendering | MEDIUM | 32h | Visual quality |

**Total**: ~212 hours

**Grand Total**: ~458 hours (~3 person-months)

---

## Integration Opportunities

### 7.1 jcaldwell-labs Ecosystem

**Current Projects**:
1. my-context (production)
2. boxes-live (MVP)
3. fintrack (phase 1)
4. terminal-stars
5. tario
6. adventure-engine-v2
7. smartterm-prototype

**Integration Scenarios**:

#### 7.1.1 my-context + boxes-live

**Vision**: Visualize my-context data as canvases

**Connector**: `context2canvas`
```bash
# Visualize recent work across contexts
my-context export --format json | \
    context2canvas --layout timeline | \
    boxes-live

# Show context relationships
context2canvas --show-links \
               --from "2025-11-01" \
               --to "2025-11-18" > work-map.txt
```

**Use Cases**:
- Weekly review visualization
- Project interconnections
- Work pattern analysis
- Context switching visualization

#### 7.1.2 fintrack + boxes-live

**Vision**: Financial data visualization

**Connector**: `fintrack2canvas`
```bash
# Account overview
fintrack accounts list --format json | \
    fintrack2canvas --layout hierarchy > accounts.txt

# Transaction flow
fintrack transactions --month 2025-11 | \
    fintrack2canvas --layout sankey > cashflow.txt
```

**Use Cases**:
- Account hierarchy
- Cash flow diagrams
- Budget tracking
- Investment portfolio view

#### 7.1.3 Code Analysis Connectors

**New Connectors**:
1. `codebase2canvas` - Project structure
2. `dependencies2canvas` - Dependency graph
3. `git-history2canvas` - Commit relationships
4. `todos2canvas` - TODO/FIXME visualization

**Example**:
```bash
# Visualize codebase structure
codebase2canvas --language c \
                --show-dependencies \
                --depth 3 > architecture.txt

# Show TODOs across projects
grep -r "TODO" ~/projects/**/src | \
    todos2canvas --group-by project > todos.txt
```

### 7.2 External Tool Integration

#### 7.2.1 tmux Integration

**Vision**: boxes-live as tmux dashboard

**Setup**:
```bash
# .tmux.conf
bind-key M-b split-window -h "boxes-live ~/dashboards/default.txt"
bind-key M-m split-window -h "boxes-live ~/dashboards/monitor.txt"
```

**Use Cases**:
- System monitoring panel
- Project dashboard
- Task board
- Reference canvas

#### 7.2.2 Vim/Neovim Integration

**Plugin**: `vim-boxes-live`

**Features**:
```vim
" Open canvas for current project
:BoxesOpen project.txt

" Add current file as box
:BoxesAdd

" Add TODO items as boxes
:BoxesTodo

" Visualize buffer structure
:BoxesOutline
```

#### 7.2.3 Shell Integration

**Functions**:
```bash
# ~/.bashrc or ~/.zshrc

# Quick note to canvas
note() {
    echo "$*" | boxes-cli add ~/notes.txt --title "$(date +%Y-%m-%d)"
}

# Open quick canvas
canvas() {
    boxes-live "${1:-~/quick.txt}"
}

# Visualize directory
vis() {
    tree2canvas "${1:-.}" | boxes-live
}
```

---

## Conclusion

### Summary of Recommendations

**Immediate (1.0.0)**:
- Fix identified bugs
- Polish documentation
- Establish release process

**Near-Term (1.x)**:
- Enhance core features (connections, grouping, rich text)
- Build connector ecosystem
- Implement plugin system
- Add MCP server support

**Long-Term (2.0.0+)**:
- Client-server architecture
- Web frontend
- Mobile support
- Advanced rendering

### Success Metrics

**1.0.0 Success**:
- ✅ All tests passing
- ✅ Zero critical bugs
- ✅ Comprehensive documentation
- ✅ 10+ connectors
- ✅ Positive community feedback

**1.x Success**:
- 50+ GitHub stars
- 5+ external contributors
- 10+ community plugins
- 3+ blog posts/tutorials
- Integration with 3+ tools

**2.0.0 Success**:
- 500+ active users
- Collaborative editing working
- Web frontend launched
- Enterprise deployments
- Sustainable development model

### Risk Assessment

**Technical Risks**:
- Plugin API stability - Mitigation: Careful design, versioning
- Performance with large canvases - Mitigation: Profiling, optimization
- Cross-platform compatibility - Mitigation: CI testing

**Project Risks**:
- Scope creep - Mitigation: Strict prioritization
- Maintenance burden - Mitigation: Community involvement
- Competition - Mitigation: Unique value proposition (Unix philosophy)

### Final Thoughts

boxes-live has a strong foundation and clear vision. The roadmap balances:
- **Immediate needs** (bug fixes, 1.0.0)
- **User value** (features users want)
- **Technical excellence** (architecture, maintainability)
- **Ecosystem integration** (jcaldwell-labs, external tools)

The project is well-positioned for success. With consistent execution of this roadmap, boxes-live can become the standard terminal-based visual workspace tool.

---

**Document Prepared By**: Claude (Sonnet 4.5)
**Date**: 2025-11-18
**Next Review**: After 1.0.0 release

**Status**: ✅ Ready for implementation
