# Feature Roadmap

## Vision

Transform boxes-live into a flexible, extensible terminal-based canvas framework that can serve as:
- **Smart Canvas**: Collaborative visual workspace (like Miro or Slack Canvas)
- **Game Engine**: Side-scrolling or top-down game renderer
- **Productivity Tool**: Visual task management and note-taking
- **Map Renderer**: Large-scale world visualization (realm-engine integration)

## Core Principles

1. **Modularity**: Keep components decoupled and reusable
2. **Performance**: Maintain 60 FPS even with hundreds of entities
3. **Extensibility**: Plugin architecture for custom behaviors
4. **Simplicity**: Terminal-first design, no bloat
5. **Portability**: Works on Linux, macOS, and Windows (via PDCurses)

## Phase 1: Foundation Enhancements (v0.2.0)

**Goal**: Make the system interactive and persistent

### 1.1 Dynamic Canvas
- [ ] Replace fixed-size box array with dynamic data structure
  - Use linked list or dynamic array (vector-like)
  - Support unlimited boxes (within memory limits)
  - Add efficient box insertion/removal
  - **Estimated effort**: 2-3 hours
  - **Files**: types.h, main.c

### 1.2 Box Management
- [ ] Box creation (press 'n' for new box)
  - Interactive box placement (click or specify coordinates)
  - Set initial size and title
  - **Estimated effort**: 3-4 hours
  - **Files**: input.c, new box_edit.c module

- [ ] Box deletion (press 'd' while box selected)
  - Confirm deletion prompt
  - Free associated memory
  - **Estimated effort**: 1-2 hours

- [ ] Box selection system
  - Click to select (with mouse support)
  - Keyboard navigation between boxes (Tab/Shift+Tab)
  - Visual indicator for selected box (highlighted border)
  - **Estimated effort**: 4-5 hours
  - **Files**: input.c, render.c, types.h

### 1.3 Mouse Support
- [ ] Enable ncurses mouse events
  - Click to select boxes
  - Click-and-drag to pan viewport
  - Scroll wheel to zoom
  - **Estimated effort**: 3-4 hours
  - **Files**: terminal.c, input.c

### 1.4 Persistence
- [ ] Save canvas to file
  - Simple text format (custom or JSON)
  - Include all boxes with positions and content
  - **Estimated effort**: 2-3 hours
  - **Files**: new persistence.c module

- [ ] Load canvas from file
  - Parse and reconstruct canvas
  - Validation and error handling
  - **Estimated effort**: 2-3 hours

- [ ] Auto-save functionality
  - Save on quit
  - Periodic auto-save option
  - **Estimated effort**: 1 hour

**Total Phase 1 Effort**: ~20-26 hours

## Phase 2: Smart Canvas Features (v0.3.0)

**Goal**: Support Miro-style visual collaboration

### 2.1 Box Editing
- [ ] Text editing mode
  - In-place content editing
  - Support for multi-line text
  - Basic text navigation (cursor movement)
  - **Estimated effort**: 6-8 hours
  - **Files**: new edit_mode.c module

- [ ] Box resizing
  - Drag corners/edges to resize
  - Minimum/maximum size constraints
  - **Estimated effort**: 3-4 hours

- [ ] Box moving
  - Drag to move (with mouse or keyboard)
  - Snap to grid option
  - **Estimated effort**: 2-3 hours

### 2.2 Connections
- [ ] Arrow/line system
  - Connect two boxes with lines
  - Line routing (straight, orthogonal, or bezier approximation)
  - Arrow direction indicators
  - **Estimated effort**: 8-10 hours
  - **Files**: types.h, new connections.c module, render.c

- [ ] Connection labels
  - Add text labels to connections
  - **Estimated effort**: 2-3 hours

### 2.3 Styling
- [ ] Color support
  - Initialize ncurses color pairs
  - Box border colors
  - Box background colors
  - Text colors
  - **Estimated effort**: 3-4 hours
  - **Files**: terminal.c, render.c, types.h

- [ ] Box styles
  - Border styles (single, double, rounded, dashed)
  - Fill patterns
  - **Estimated effort**: 2-3 hours

### 2.4 Box Types
- [ ] Define box types
  - Note (default)
  - Task (with checkbox/status)
  - Sticky note (smaller, colored)
  - Code block (monospace content)
  - **Estimated effort**: 4-5 hours
  - **Files**: types.h, render.c

- [ ] Type-specific rendering
  - Custom icons or indicators
  - Type-specific behavior
  - **Estimated effort**: 3-4 hours

### 2.5 Collaboration Preparation
- [ ] Export to portable formats
  - Export to SVG (approximation)
  - Export to ASCII art
  - Export to Markdown
  - **Estimated effort**: 4-6 hours
  - **Files**: new export.c module

**Total Phase 2 Effort**: ~37-50 hours

## Phase 3: Game Engine Features (v0.4.0)

**Goal**: Support side-scrolling and top-down games

### 3.1 Entity System
- [ ] Replace Box with Entity
  - Generic entity structure
  - Entity types (sprite, platform, player, enemy, etc.)
  - Entity components (position, velocity, sprite, collider)
  - **Estimated effort**: 6-8 hours
  - **Files**: types.h, new entity.c module

- [ ] Entity update loop
  - Separate update from render
  - Update all entities each frame
  - **Estimated effort**: 2-3 hours
  - **Files**: main.c

### 3.2 Physics
- [ ] Basic physics system
  - Position, velocity, acceleration
  - Gravity
  - Friction/damping
  - **Estimated effort**: 4-5 hours
  - **Files**: new physics.c module

- [ ] Collision detection
  - AABB collision
  - Collision response (bounce, stop, slide)
  - Collision callbacks
  - **Estimated effort**: 6-8 hours
  - **Files**: new collision.c module

### 3.3 Input for Games
- [ ] Continuous input
  - Key pressed/released state tracking
  - Support for simultaneous keys
  - **Estimated effort**: 2-3 hours
  - **Files**: input.c

- [ ] Input mapping
  - Configurable key bindings
  - Action-based input (jump, shoot, move)
  - **Estimated effort**: 3-4 hours

### 3.4 Animation
- [ ] Sprite animation
  - Frame-based animation
  - Animation state machine
  - **Estimated effort**: 5-6 hours
  - **Files**: types.h, new animation.c, render.c

### 3.5 Game Modes
- [ ] Side-scrolling mode
  - Lock camera to player (horizontal or vertical)
  - Parallax background layers
  - **Estimated effort**: 4-5 hours
  - **Files**: viewport.c

- [ ] Top-down mode
  - Free camera or player-centered
  - **Estimated effort**: 2-3 hours

### 3.6 Level System
- [ ] Level loading
  - Load entities from file
  - Support for tile-based levels
  - **Estimated effort**: 5-6 hours
  - **Files**: new level.c module

**Total Phase 3 Effort**: ~39-51 hours

## Phase 4: Productivity Features (v0.5.0)

**Goal**: Task management and note-taking

### 4.1 Task Management
- [ ] Task boxes
  - Checkbox rendering
  - Status (todo, in-progress, done)
  - Priority levels
  - **Estimated effort**: 4-5 hours

- [ ] Due dates
  - Date display
  - Overdue highlighting
  - **Estimated effort**: 2-3 hours

- [ ] Tags and categories
  - Tagging system
  - Tag-based filtering
  - **Estimated effort**: 3-4 hours
  - **Files**: types.h, new tags.c module

### 4.2 Search and Filter
- [ ] Search functionality
  - Search box titles and content
  - Highlight matching boxes
  - **Estimated effort**: 3-4 hours
  - **Files**: new search.c module

- [ ] Filter by criteria
  - Filter by type, tag, status
  - Show/hide filtered boxes
  - **Estimated effort**: 2-3 hours

### 4.3 Rich Content
- [ ] Markdown rendering
  - Parse markdown in box content
  - Render formatting (bold, italic, lists)
  - **Estimated effort**: 6-8 hours
  - **Files**: render.c, maybe use external markdown parser

- [ ] Links
  - Clickable links between boxes
  - External links (open in browser)
  - **Estimated effort**: 3-4 hours

### 4.4 Import/Export
- [ ] Import from markdown
  - Parse markdown files into boxes
  - **Estimated effort**: 4-5 hours

- [ ] Export to org-mode
  - For Emacs users
  - **Estimated effort**: 2-3 hours

**Total Phase 4 Effort**: ~29-39 hours

## Phase 5: Large-Scale Maps (v0.6.0)

**Goal**: Realm-engine integration and massive worlds

### 5.1 Chunking System
- [ ] Chunk-based world
  - Divide world into chunks (e.g., 64x64)
  - Load/unload chunks based on viewport
  - **Estimated effort**: 8-10 hours
  - **Files**: new chunk.c module, types.h

- [ ] Chunk streaming
  - Background loading of nearby chunks
  - Async I/O for large maps
  - **Estimated effort**: 6-8 hours

### 5.2 Tile System
- [ ] Tile-based rendering
  - More efficient than boxes for maps
  - Tile atlas/tileset support
  - **Estimated effort**: 6-8 hours
  - **Files**: new tile.c module, render.c

### 5.3 Layers
- [ ] Layer system
  - Ground, objects, overlay layers
  - Layer visibility toggling
  - Z-ordering
  - **Estimated effort**: 5-6 hours
  - **Files**: types.h, render.c

### 5.4 Spatial Optimization
- [ ] Spatial partitioning
  - Quadtree or spatial hash
  - Fast visibility queries
  - **Estimated effort**: 8-10 hours
  - **Files**: new spatial.c module

### 5.5 Minimap
- [ ] Minimap view
  - Small overview window
  - Show player position
  - Click minimap to jump
  - **Estimated effort**: 4-5 hours
  - **Files**: render.c

### 5.6 Realm-Engine Integration
- [ ] Entity-component system
  - Compatible with realm-engine entities
  - Component-based architecture
  - **Estimated effort**: 10-12 hours
  - **Files**: major refactor to entity system

- [ ] Network sync (if multiplayer)
  - Sync entities over network
  - Client-server architecture
  - **Estimated effort**: 15-20 hours
  - **Files**: new network.c module

**Total Phase 5 Effort**: ~62-79 hours

## Phase 6: Polish and Optimization (v1.0.0)

**Goal**: Production-ready release

### 6.1 Performance
- [ ] Profiling and optimization
  - Identify bottlenecks
  - Optimize hot paths
  - **Estimated effort**: 8-10 hours

- [ ] Dirty rectangle rendering
  - Only redraw changed regions
  - **Estimated effort**: 6-8 hours

- [ ] Multi-threading
  - Async chunk loading
  - Background saving
  - **Estimated effort**: 8-10 hours

### 6.2 User Experience
- [ ] Tutorial/help system
  - Interactive tutorial
  - Built-in help (press 'F1')
  - **Estimated effort**: 4-5 hours

- [ ] Undo/redo
  - Command pattern
  - Undo stack
  - **Estimated effort**: 6-8 hours

- [ ] Copy/paste
  - Clipboard for boxes
  - Duplicate boxes
  - **Estimated effort**: 3-4 hours

### 6.3 Configuration
- [ ] Config file support
  - Settings file (~/.boxes-live/config)
  - Customizable keybindings
  - Theme configuration
  - **Estimated effort**: 4-5 hours

### 6.4 Plugin System
- [ ] Plugin architecture
  - Loadable modules (.so files)
  - Plugin API
  - Event hooks
  - **Estimated effort**: 10-12 hours

### 6.5 Testing
- [ ] Unit tests
  - Test core functions
  - Automated testing
  - **Estimated effort**: 8-10 hours

- [ ] Integration tests
  - Test full workflows
  - **Estimated effort**: 4-5 hours

### 6.6 Documentation
- [ ] API documentation
  - Doxygen comments
  - Generated docs
  - **Estimated effort**: 6-8 hours

- [ ] Video tutorials
  - Screen recordings
  - YouTube demos
  - **Estimated effort**: 4-6 hours

**Total Phase 6 Effort**: ~71-91 hours

## Total Estimated Effort

| Phase | Effort Range | Focus |
|-------|--------------|-------|
| Phase 1 | 20-26 hours | Foundation |
| Phase 2 | 37-50 hours | Smart Canvas |
| Phase 3 | 39-51 hours | Game Engine |
| Phase 4 | 29-39 hours | Productivity |
| Phase 5 | 62-79 hours | Large Maps |
| Phase 6 | 71-91 hours | Polish |
| **Total** | **258-336 hours** | Full roadmap |

## Prioritized Feature List

### High Priority (Must Have)
1. Dynamic canvas (unlimited boxes)
2. Box creation/deletion
3. Box selection
4. Mouse support
5. Save/load functionality
6. Text editing mode
7. Basic colors

### Medium Priority (Should Have)
8. Box connections
9. Box types (note, task, code)
10. Export formats (markdown, ASCII)
11. Search functionality
12. Tags and filtering
13. Undo/redo

### Low Priority (Nice to Have)
14. Minimap
15. Plugin system
16. Animation system
17. Network/multiplayer
18. Advanced markdown rendering

## Implementation Strategy

### Recommended Order

**For Smart Canvas Use Case**:
Phase 1 → Phase 2 → Phase 4 (partial) → Phase 6 (partial)

**For Game Engine Use Case**:
Phase 1 → Phase 3 → Phase 6 (partial)

**For Realm-Engine Integration**:
Phase 1 → Phase 5 → Phase 3 (entity system) → Phase 6

**For General Purpose**:
Phase 1 → Phase 2 → Phase 3 → Phase 4 → Phase 5 → Phase 6

### Quick Wins (1-2 hour tasks)
- [ ] Add auto-save on quit
- [ ] Add grid display option
- [ ] Add box count to status bar
- [ ] Add FPS counter
- [ ] Add different border styles
- [ ] Add keyboard shortcuts help screen
- [ ] Add box duplication (copy)
- [ ] Add center viewport on box (focus)

## Architecture Patterns to Adopt

### Entity-Component System (Phase 3+)
Instead of monolithic Box structure, use components:
```c
typedef struct {
    int entity_id;
    Component *components;  // Linked list of components
} Entity;

// Components
typedef struct {
    ComponentType type;
    void *data;
} Component;

// Example components:
// - PositionComponent (x, y)
// - SpriteComponent (char sprite, color)
// - VelocityComponent (dx, dy)
// - ColliderComponent (width, height)
// - RenderComponent (render function)
```

### Event System
Decouple modules with events:
```c
typedef void (*EventHandler)(Event *e, void *userdata);

void event_subscribe(EventType type, EventHandler handler);
void event_publish(Event *event);

// Examples:
// - BOX_CREATED
// - BOX_SELECTED
// - VIEWPORT_MOVED
// - SAVE_REQUESTED
```

### Command Pattern (for undo/redo)
```c
typedef struct {
    void (*execute)(Command *cmd);
    void (*undo)(Command *cmd);
    void *data;
} Command;

void command_execute(Command *cmd);
void command_undo(Command *cmd);
```

## Technology Considerations

### Alternative Rendering Backends
- ncurses (current)
- notcurses (more features, modern)
- termbox (simpler, portable)
- direct ANSI escape codes (maximum control)

### Serialization Libraries
- cJSON (JSON parsing)
- TOML (config files)
- Custom binary format (performance)

### Scripting Support
- Lua (embedded scripting for plugins)
- Python bindings (via ctypes or Cython)

## Community & Ecosystem

### Contribution Guidelines
- [ ] CONTRIBUTING.md
- [ ] Code of conduct
- [ ] Issue templates
- [ ] PR templates

### Examples & Templates
- [ ] Example canvas files
- [ ] Template projects (game, task board, mind map)
- [ ] Plugin examples

### Integration Examples
- [ ] Vim plugin (edit boxes in Vim)
- [ ] Tmux integration
- [ ] Git hook integrations
- [ ] Export to other tools (Trello, Notion, etc.)

## Success Metrics

- **Performance**: Maintain 60 FPS with 1000+ entities
- **Memory**: Stay under 100MB for typical use cases
- **Startup**: Launch in under 100ms
- **Save/Load**: Handle 10MB+ canvas files in under 1 second
- **Compatibility**: Work on Linux, macOS, Windows (WSL/PDCurses)

## Conclusion

This roadmap provides a clear path from the current read-only canvas to a full-featured terminal application framework. The modular design allows picking and choosing features based on use case, whether that's a smart canvas, game engine, productivity tool, or large-scale map renderer.

See [REALM_ENGINE_INTEGRATION.md](REALM_ENGINE_INTEGRATION.md) for specific guidance on integrating with realm-engine for massive multiplayer worlds.
