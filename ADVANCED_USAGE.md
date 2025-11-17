# Advanced Usage Guide

## Overview

This guide covers advanced usage patterns, architectural insights, and how to extend boxes-live for different use cases.

## Understanding the Architecture

### Coordinate System

Boxes-live uses a dual coordinate system:

**World Coordinates** (double precision):
- Infinite 2D plane where boxes exist
- Independent of terminal size
- Current canvas is 200x100 units
- Can be extended to any size

**Screen Coordinates** (integer):
- Terminal character positions
- Viewport-dependent
- Transforms via camera position and zoom

**Transformation Functions** (viewport.c):
```c
int world_to_screen_x(const Viewport *vp, double world_x);
int world_to_screen_y(const Viewport *vp, double world_y);
double screen_to_world_x(const Viewport *vp, int screen_x);
double screen_to_world_y(const Viewport *vp, int screen_y);
```

### Viewport System

The viewport acts as a camera into the world:
- **cam_x, cam_y**: Camera position in world space
- **zoom**: Scale factor (0.1x to 10.0x)
- **term_width, term_height**: Current terminal size

**Zoom Behavior**:
- Zooms toward center of screen
- Adjusts camera position to maintain center point
- See viewport.c:17-37 for implementation

### Rendering Pipeline

1. **Update Loop** (main.c:167-192):
   - Update terminal size (handles resize)
   - Clear screen
   - Render all boxes
   - Render status bar
   - Refresh display
   - Handle input
   - Sleep (~60 FPS)

2. **Culling** (render.c:49-53):
   - Only renders boxes visible in viewport
   - Checks if box bounds intersect screen
   - Performance optimization for large canvases

3. **Box Drawing** (render.c:42-104):
   - Uses ncurses ACS characters for borders
   - Scales box size by zoom factor
   - Clips content to visible area
   - Handles partial visibility

## Performance Characteristics

### Current Limits
- **Max Boxes**: 100 (static array, can be increased)
- **Zoom Range**: 0.1x to 10.0x
- **Frame Rate**: ~60 FPS (16.67ms sleep)
- **Memory**: ~50KB for 100 boxes with content

### Optimization Opportunities

1. **Spatial Partitioning**:
   - Current: O(n) rendering (checks all boxes)
   - Could use: Quadtree for O(log n) visibility queries

2. **Dirty Rectangles**:
   - Current: Full screen clear/redraw every frame
   - Could use: Only redraw changed regions

3. **Dynamic Allocations**:
   - Current: Static array of 100 boxes
   - Could use: Dynamic array or linked list

## Extending for Different Use Cases

### Smart Canvas (Miro/Slack Canvas Style)

**Required Features**:
- ✅ Pan and zoom (already implemented)
- ❌ Interactive box creation
- ❌ Box editing (text content)
- ❌ Box connections/arrows
- ❌ Box styling (colors, borders)
- ❌ Box selection/dragging
- ❌ Save/load functionality
- ❌ Collaboration (multiplayer)
- ❌ Rich content (images, links)

**Architecture Recommendations**:
1. Add box creation mode (press 'n' for new box)
2. Add text editing mode (press 'e' to edit selected box)
3. Add connection system (arrow drawing between boxes)
4. Add serialization (save/load to JSON or custom format)
5. Consider adding ncurses panels for layering

### Side-Scrolling Game

**Required Features**:
- ✅ Side-scrolling (pan left/right)
- ✅ Viewport system (already implemented)
- ❌ Collision detection
- ❌ Entity/sprite system
- ❌ Animation frames
- ❌ Physics (gravity, velocity)
- ❌ Player input (jump, move, shoot)
- ❌ Level loading
- ❌ Game state management

**Architecture Recommendations**:
1. Replace Box with Entity/Sprite structure
2. Add entity types (player, enemy, platform, item)
3. Add physics system (position, velocity, acceleration)
4. Add collision detection (AABB or precise)
5. Add entity update loop (before rendering)
6. Add level format (tile-based or entity-based)
7. Lock vertical panning, constrain to horizontal scrolling

### Productivity Application

**Required Features**:
- ✅ Visual organization (boxes)
- ✅ Pan and zoom
- ❌ Text editing in boxes
- ❌ Task management (checkboxes, status)
- ❌ Calendar integration
- ❌ Search functionality
- ❌ Tags/categories
- ❌ Links between items
- ❌ Export to markdown/text

**Architecture Recommendations**:
1. Add box types (task, note, calendar, link)
2. Add metadata to boxes (tags, dates, priority)
3. Add search/filter system
4. Add text editor mode (using ncurses forms or custom)
5. Add export functions (to markdown, org-mode, etc.)

### Large Map System (Realm-Engine Integration)

**Required Features**:
- ✅ Large world support
- ✅ Viewport system
- ❌ Chunking/streaming
- ❌ Procedural generation
- ❌ Entity management
- ❌ Map layers (terrain, objects, fog of war)
- ❌ Minimap view
- ❌ Distance-based detail (LOD)

**Architecture Recommendations**:
1. Implement chunk system (load/unload map regions)
2. Add layer system (ground, objects, overlay)
3. Add tile-based rendering (more efficient than boxes)
4. Add entity-component system for game objects
5. Add spatial hash or grid for fast queries
6. Consider multiple viewports (main + minimap)

## Modifying the Code

### Adding a New Box Type

Current box structure (types.h:10-18):
```c
typedef struct {
    double x, y;
    int width, height;
    char *title;
    char **content;
    int content_lines;
} Box;
```

To add types, extend the structure:
```c
typedef struct {
    double x, y;
    int width, height;
    char *title;
    char **content;
    int content_lines;
    int box_type;        // NEW: 0=note, 1=task, 2=link, etc.
    int color_pair;      // NEW: ncurses color pair
    void *metadata;      // NEW: type-specific data
} Box;
```

### Adding Mouse Support

ncurses supports mouse input:
```c
// In terminal_init():
mousemask(ALL_MOUSE_EVENTS, NULL);

// In input handler:
MEVENT event;
if (ch == KEY_MOUSE) {
    if (getmouse(&event) == OK) {
        // Convert screen coords to world coords
        double wx = screen_to_world_x(vp, event.x);
        double wy = screen_to_world_y(vp, event.y);

        // Handle click, drag, etc.
        if (event.bstate & BUTTON1_CLICKED) {
            // Find box at world coordinates
            // Select or interact with box
        }
    }
}
```

### Adding Colors

ncurses supports colors:
```c
// In terminal_init():
start_color();
init_pair(1, COLOR_RED, COLOR_BLACK);
init_pair(2, COLOR_GREEN, COLOR_BLACK);
init_pair(3, COLOR_BLUE, COLOR_BLACK);

// In render_box():
if (box->color_pair > 0) {
    attron(COLOR_PAIR(box->color_pair));
}
// ... draw box ...
if (box->color_pair > 0) {
    attroff(COLOR_PAIR(box->color_pair));
}
```

### Adding Save/Load

Simple serialization approach:
```c
// Save canvas to file
void save_canvas(const Canvas *canvas, const char *filename) {
    FILE *f = fopen(filename, "w");
    fprintf(f, "%d\n", canvas->box_count);

    for (int i = 0; i < canvas->box_count; i++) {
        Box *box = &canvas->boxes[i];
        fprintf(f, "%f %f %d %d\n", box->x, box->y,
                box->width, box->height);
        fprintf(f, "%s\n", box->title ? box->title : "");
        fprintf(f, "%d\n", box->content_lines);
        for (int j = 0; j < box->content_lines; j++) {
            fprintf(f, "%s\n", box->content[j]);
        }
    }
    fclose(f);
}

// Load canvas from file (reverse process)
```

Or use JSON for better interoperability (requires JSON library like cJSON).

## Integration Patterns

### As a Rendering Backend

The viewport and rendering system can be extracted as a reusable module:

```c
// Your application
typedef struct {
    Viewport viewport;
    YourDataStructure data;
} App;

// Render callback
void render_callback(const Viewport *vp, void *userdata) {
    App *app = (App *)userdata;
    // Render your data using viewport transforms
    // world_to_screen_x/y for positioning
}

// Main loop
while (running) {
    terminal_update_size(&app.viewport);
    terminal_clear();
    render_callback(&app.viewport, &app);
    terminal_refresh();
    handle_input(&app.viewport, &app);
    nanosleep(&ts, NULL);
}
```

### As a Library

To use boxes-live as a library:
1. Extract core modules (viewport, terminal, render)
2. Create header-only or static library
3. Provide initialization/cleanup functions
4. Provide render callbacks for custom content
5. Expose coordinate transformation functions

### With External Data Sources

Current canvas is hardcoded. To load from external sources:

```c
// Load from file
Canvas canvas;
load_canvas_from_file(&canvas, "world.dat");

// Load from database
load_canvas_from_db(&canvas, db_connection);

// Generate procedurally
generate_canvas(&canvas, seed);

// Stream from network
stream_canvas_from_server(&canvas, server_url);
```

## Advanced Input Handling

### Vim-Style Modal Input

Add modes for different contexts:
```c
typedef enum {
    MODE_NORMAL,     // Navigation
    MODE_INSERT,     // Text editing
    MODE_VISUAL,     // Selection
    MODE_COMMAND     // Command entry
} InputMode;

// In input handler:
switch (mode) {
    case MODE_NORMAL:
        // hjkl navigation, 'i' for insert, etc.
        break;
    case MODE_INSERT:
        // Text entry, ESC to normal
        break;
    // etc.
}
```

### Keyboard Shortcuts

Current shortcuts are hardcoded. To make configurable:
```c
typedef struct {
    int key;
    void (*handler)(Viewport *vp, void *data);
} KeyBinding;

KeyBinding bindings[] = {
    {'w', pan_up},
    {'s', pan_down},
    // etc.
};
```

## Debugging Tips

### Visual Debugging

Add debug overlay:
```c
void render_debug(const Viewport *vp) {
    // Show grid
    for (int x = 0; x < vp->term_width; x += 10) {
        draw_vline(x, 0, vp->term_height, '|');
    }

    // Show world coordinates at mouse
    // Show viewport bounds
    // Show performance stats
}
```

### Logging

Since ncurses takes over the terminal, log to file:
```c
FILE *logfile = fopen("debug.log", "w");
fprintf(logfile, "Camera: (%.2f, %.2f) Zoom: %.2f\n",
        vp->cam_x, vp->cam_y, vp->zoom);
fflush(logfile);
```

### Memory Debugging

Use valgrind:
```bash
valgrind --leak-check=full ./boxes-live
```

Use address sanitizer:
```bash
# In Makefile, add to CFLAGS:
-fsanitize=address -g

# Run normally:
./boxes-live
```

## Performance Profiling

### CPU Profiling

Use gprof:
```bash
# Add to CFLAGS in Makefile:
-pg

# Run program:
./boxes-live

# Generate profile:
gprof boxes-live gmon.out > analysis.txt
```

### Frame Time Measurement

Add timing code:
```c
#include <sys/time.h>

struct timeval start, end;
gettimeofday(&start, NULL);

// Render frame
render_canvas(&canvas, &viewport);

gettimeofday(&end, NULL);
long ms = (end.tv_sec - start.tv_sec) * 1000 +
          (end.tv_usec - start.tv_usec) / 1000;

// Log if frame took too long
if (ms > 16) {
    fprintf(logfile, "Slow frame: %ld ms\n", ms);
}
```

## Multi-Platform Considerations

### Windows Support

ncurses alternatives on Windows:
- PDCurses
- ncurses via WSL
- Windows Console API (rewrite terminal module)

### macOS Support

Should work with standard ncurses:
```bash
brew install ncurses
```

May need to link explicitly:
```makefile
LDFLAGS += -L/usr/local/opt/ncurses/lib
CFLAGS += -I/usr/local/opt/ncurses/include
```

## Next Steps

See [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md) for planned enhancements and implementation priorities.

See [REALM_ENGINE_INTEGRATION.md](REALM_ENGINE_INTEGRATION.md) for specific guidance on integrating with large-scale map systems.
