# boxes-live Architecture

This document provides a comprehensive overview of the boxes-live architecture, including module organization, data flow, and coordinate transformation systems.

## Table of Contents

- [System Overview](#system-overview)
- [Module Architecture](#module-architecture)
- [Data Flow](#data-flow)
- [Coordinate Systems](#coordinate-systems)
- [Rendering Pipeline](#rendering-pipeline)
- [Memory Management](#memory-management)
- [Persistence Format](#persistence-format)

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        boxes-live                               │
│                                                                 │
│  Terminal-based Interactive Canvas with Pan/Zoom Interface      │
└─────────────────────────────────────────────────────────────────┘
         │                    │                    │
         ▼                    ▼                    ▼
   ┌─────────┐          ┌──────────┐        ┌──────────┐
   │ Terminal│          │ Viewport │        │  Canvas  │
   │ (ncurses│  <──>    │ (Camera) │  <──>  │  (Boxes) │
   │  I/O)   │          │          │        │          │
   └─────────┘          └──────────┘        └──────────┘
         │                                        │
         ▼                                        ▼
   ┌─────────┐                            ┌──────────┐
   │  Input  │                            │Persistence│
   │ Handler │                            │(Save/Load)│
   └─────────┘                            └──────────┘
```

### Key Concepts

- **Canvas**: Infinite 2D world space containing boxes
- **Viewport**: Camera view into the canvas with pan/zoom
- **Rendering**: Transforms world coordinates to screen coordinates
- **Persistence**: Text-based save/load format
- **Input**: Keyboard and mouse event handling

## Module Architecture

### Directory Structure

```
boxes-live/
├── include/                 # Public interfaces
│   ├── types.h             # Core data structures
│   ├── canvas.h            # Canvas management API
│   ├── viewport.h          # Viewport transformation API
│   ├── render.h            # Rendering API
│   ├── input.h             # Input handling API
│   ├── terminal.h          # Terminal control API
│   ├── persistence.h       # Save/load API
│   └── signal_handler.h    # Signal handling API
│
├── src/                    # Implementation
│   ├── canvas.c            # Dynamic box array
│   ├── viewport.c          # Coordinate transformations
│   ├── render.c            # ncurses rendering
│   ├── input.c             # Event handling
│   ├── terminal.c          # ncurses initialization
│   ├── persistence.c       # File I/O
│   ├── signal_handler.c    # POSIX signals
│   └── main.c              # Main loop
│
└── tests/                  # Test suites
    ├── test_canvas.c       # Canvas unit tests
    ├── test_viewport.c     # Viewport unit tests
    ├── test_persistence.c  # Save/load tests
    └── test_integration.c  # Integration tests
```

### Module Dependencies

```
                    ┌──────────┐
                    │  main.c  │
                    └────┬─────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
    ┌─────────┐    ┌──────────┐    ┌─────────┐
    │ terminal│    │ viewport │    │ canvas  │
    └─────────┘    └──────────┘    └─────────┘
         │               │               │
         ▼               ▼               ▼
    ┌─────────┐    ┌──────────┐    ┌─────────┐
    │  input  │◄───┤  render  │◄───┤persistence│
    └─────────┘    └──────────┘    └─────────┘
         │               │
         └───────┬───────┘
                 │
                 ▼
            ┌─────────┐
            │  types  │ (shared data structures)
            └─────────┘
```

### Module Responsibilities

#### types.h - Core Data Structures

Defines three fundamental types:

```c
typedef struct Box {
    double x, y;              // World coordinates
    int width, height;        // Dimensions in characters
    char *title;              // Box title
    char **content;           // Array of content lines
    int content_lines;        // Number of lines
    bool selected;            // Selection state
    int id;                   // Unique identifier
    int color;                // Color pair index
} Box;

typedef struct Viewport {
    double cam_x, cam_y;      // Camera position (world coordinates)
    double zoom;              // Zoom level (1.0 = normal)
    int term_width, term_height; // Terminal size
} Viewport;

typedef struct Canvas {
    Box *boxes;               // Dynamic array of boxes
    int box_count;            // Current number of boxes
    int box_capacity;         // Allocated capacity
    double world_width;       // World dimensions
    double world_height;
    int next_id;              // Next box ID to assign
    int selected_index;       // Selected box index (-1 = none)
} Canvas;
```

#### canvas.c - Dynamic Box Management

Responsibilities:
- Dynamic array allocation and growth
- Box addition/removal
- Box lookup by ID or coordinates
- Selection management

Key functions:
```c
int canvas_init(Canvas *canvas, double world_width, double world_height);
void canvas_cleanup(Canvas *canvas);
int canvas_add_box(Canvas *canvas, double x, double y, int w, int h, const char *title);
int canvas_remove_box(Canvas *canvas, int box_id);
Box* canvas_get_box(Canvas *canvas, int box_id);
int canvas_find_box_at(Canvas *canvas, double x, double y);
void canvas_select_box(Canvas *canvas, int box_id);
```

#### viewport.c - Coordinate Transformations

Responsibilities:
- Camera position and zoom management
- World ↔ Screen coordinate transformations
- Visibility testing

Key transformations:
```c
// World to Screen
int screen_x = (int)((world_x - cam_x) * zoom);
int screen_y = (int)((world_y - cam_y) * zoom);

// Screen to World
double world_x = (screen_x / zoom) + cam_x;
double world_y = (screen_y / zoom) + cam_y;
```

#### render.c - ncurses Rendering

Responsibilities:
- Box drawing with ACS box-drawing characters
- Title and content rendering
- Status bar display
- Selection highlighting

Rendering features:
- Viewport culling (only visible boxes)
- Zoom scaling
- Color support
- Safe text rendering (bounds checking)

#### input.c - Event Handling

Responsibilities:
- Keyboard input processing
- Mouse event handling
- Drag-and-drop support
- Command execution

Input modes:
- Pan (arrow keys, WASD)
- Zoom (+/-, Z/X)
- Box selection (mouse click, Tab)
- Box manipulation (drag, delete, color)
- Save/load (F2/F3)

#### persistence.c - File I/O

Responsibilities:
- Canvas serialization to text format
- Canvas deserialization from files
- File format validation
- Reload functionality

File format:
```
BOXES_CANVAS_V1
<world_width> <world_height>
<box_count>
<box_id> <x> <y> <width> <height> <selected> <color>
<title>
<content_line_count>
<content_line_1>
<content_line_2>
...
```

## Data Flow

### Main Loop Flow

```
┌───────────────────────────────────────────────────────┐
│                    Main Loop                          │
│                   (60 FPS target)                     │
└───────┬───────────────────────────────────────────────┘
        │
        ▼
    ┌───────────────┐
    │ Check Signals │
    │ - Quit (^C)   │
    │ - Resize      │
    │ - Reload      │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │ Update Size   │
    │ (if resized)  │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │ Clear Screen  │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │ Render Canvas │
    │   For each    │
    │   visible box │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │ Render Status │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │    Refresh    │
    │    Display    │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │ Handle Input  │
    │  (non-block)  │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐
    │     Sleep     │
    │   (~16.7ms)   │
    └───────┬───────┘
            │
            └──────► Loop or Exit
```

### Input Processing Flow

```
    User Input
        │
        ▼
┌───────────────┐
│   getch()     │
│ (non-blocking)│
└───────┬───────┘
        │
        ├─── KEY_MOUSE ────────────────────────────┐
        │                                           ▼
        │                                   ┌────────────────┐
        │                                   │ Mouse Handler  │
        │                                   │ - Click: Select│
        │                                   │ - Drag: Move   │
        │                                   └────────────────┘
        │
        ├─── Arrow Keys / WASD ─────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │ Pan Viewport │
        │                            └──────────────┘
        │
        ├─── +/- or Z/X ────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │ Zoom Viewport│
        │                            └──────────────┘
        │
        ├─── N ─────────────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │  Create Box  │
        │                            └──────────────┘
        │
        ├─── D ─────────────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │  Delete Box  │
        │                            └──────────────┘
        │
        ├─── 1-7 ───────────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │  Color Box   │
        │                            └──────────────┘
        │
        ├─── F2 ────────────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │ Save Canvas  │
        │                            └──────────────┘
        │
        ├─── F3 ────────────────────────────┐
        │                                    ▼
        │                            ┌──────────────┐
        │                            │ Load Canvas  │
        │                            └──────────────┘
        │
        └─── Q / ESC ───────────────────────┐
                                             ▼
                                     ┌──────────────┐
                                     │  Quit App    │
                                     └──────────────┘
```

## Coordinate Systems

### World vs Screen Coordinates

boxes-live uses two coordinate systems:

1. **World Coordinates** (double precision)
   - Infinite 2D space
   - Box positions stored in world coordinates
   - Independent of terminal size

2. **Screen Coordinates** (integer)
   - Terminal display space
   - Limited by terminal width/height
   - Derived from world coordinates via viewport transformation

### Coordinate Transformation

```
World Space (Infinite)              Screen Space (Terminal)

        Box at (100, 50)
            │                                Terminal (80x24)
            │                                ┌──────────┐
            ▼                                │          │
    ┌─────────────┐                         │  Visible │
    │   Box 1     │                         │   Area   │
    │  (100, 50)  │  ──Transform──>         │          │
    └─────────────┘                         └──────────┘

    Camera at (95, 48)
    Zoom = 1.0

    Transformation:
    screen_x = (box.x - camera.x) * zoom
    screen_y = (box.y - camera.y) * zoom

    Result:
    screen_x = (100 - 95) * 1.0 = 5
    screen_y = (50 - 48) * 1.0 = 2

    Box appears at (5, 2) on terminal
```

### Zoom Behavior

```
Zoom = 1.0 (Normal)          Zoom = 2.0 (Zoomed In)       Zoom = 0.5 (Zoomed Out)

┌────────────────┐          ┌────────────────┐           ┌────────────────┐
│  ┌──┐  ┌──┐   │          │                │           │┌┐┌┐┌┐┌┐┌┐┌┐┌┐│
│  │  │  │  │   │          │   ┌─────┐      │           │└┘└┘└┘└┘└┘└┘└┘│
│  └──┘  └──┘   │          │   │     │      │           │┌┐┌┐┌┐┌┐┌┐┌┐┌┐│
│                │          │   │  B  │      │           │└┘└┘└┘└┘└┘└┘└┘│
│  ┌──┐         │          │   │  O  │      │           └────────────────┘
│  │  │         │          │   │  X  │      │           Many small boxes
│  └──┘         │          │   │     │      │           visible
└────────────────┘          │   └─────┘      │
Normal view                 │                │
                           └────────────────┘
                           Fewer, larger boxes
```

## Rendering Pipeline

### Box Rendering Process

```
For each box in canvas:
    │
    ├─► Convert world → screen coordinates
    │
    ├─► Viewport culling
    │   (Skip if box completely off-screen)
    │
    ├─► Enable color/selection attributes
    │
    ├─► Draw border (ACS box-drawing characters)
    │   ┌─────────┐  ← ACS_ULCORNER, ACS_HLINE, ACS_URCORNER
    │   │         │  ← ACS_VLINE
    │   └─────────┘  ← ACS_LLCORNER, ACS_HLINE, ACS_LRCORNER
    │
    ├─► Draw title (if exists, with bounds checking)
    │
    ├─► Draw content lines (if exist, with bounds checking)
    │
    └─► Disable attributes
```

### Viewport Culling

```
Terminal Screen (80x24)
┌────────────────────────────────────────┐
│                                        │
│   ┌──────┐ Visible                    │
│   │ Box1 │                             │
│   └──────┘                             │
│                                        │
│                                        │
└────────────────────────────────────────┘

    Box2 ──────┐
    (off-screen)│
               ▼
        ┌──────────┐
        │   Box2   │  (not rendered - culled)
        └──────────┘

Culling logic:
if (box_screen_x + box_width < 0 ||
    box_screen_x >= terminal_width ||
    box_screen_y + box_height < 0 ||
    box_screen_y >= terminal_height) {
    return; // Skip rendering
}
```

## Memory Management

### Dynamic Canvas Growth

```
Initial State:
┌─────────────────────────────────────────┐
│ Capacity: 16 boxes                      │
│ Count: 0                                │
│ boxes: [_, _, _, _, _, _, _, _, _, ...] │
└─────────────────────────────────────────┘

After adding 16 boxes:
┌─────────────────────────────────────────┐
│ Capacity: 16 (FULL)                     │
│ Count: 16                               │
│ boxes: [B1,B2,B3,...,B16]               │
└─────────────────────────────────────────┘

Adding 17th box triggers realloc:
┌─────────────────────────────────────────┐
│ Capacity: 32 (doubled)                  │
│ Count: 17                               │
│ boxes: [B1,B2,...,B17,_,_,_,_,_,_,...]  │
└─────────────────────────────────────────┘

Growth strategy: capacity *= 2
```

### Memory Allocation Points

```c
// Canvas initialization
boxes = malloc(sizeof(Box) * INITIAL_BOX_CAPACITY);  // 16 boxes

// Canvas growth
new_boxes = realloc(boxes, sizeof(Box) * new_capacity);  // 2x

// Box title
box->title = strdup(title);  // Dynamic string

// Box content
box->content = malloc(sizeof(char*) * content_lines);
for each line:
    box->content[i] = strdup(line);  // Dynamic strings
```

### Cleanup Process

```c
// Canvas cleanup (reverse order of allocation)
for each box:
    free(box->title);
    for each content line:
        free(box->content[i]);
    free(box->content);
free(canvas->boxes);
```

## Persistence Format

### File Structure

```
Line 1:  BOXES_CANVAS_V1          # Magic number (format version)
Line 2:  <world_width> <world_height>
Line 3:  <box_count>

For each box:
    Line:    <id> <x> <y> <width> <height> <selected> <color>
    Line:    <title_or_NULL>
    Line:    <content_line_count>
    Lines:   <content_line_1>
             <content_line_2>
             ...

Final Line: <next_id> <selected_index>
```

### Example Canvas File

```
BOXES_CANVAS_V1
200.00 100.00
2
1 10.00 20.00 30 10 1 1
Welcome Box
2
Line 1 content
Line 2 content
2 50.00 60.00 25 8 0 3
Test Box
NULL
0
3 -1
```

### Load Process Flow

```
Open file ───► Read magic ───► Validate ───► Read world size
                                  │                │
                                  │                ▼
                                  │          Initialize canvas
                                  │                │
                                  │                ▼
                                  │          Read box count
                                  │                │
                                  │                ▼
                                  │          For each box:
                                  │            - Read properties
                                  │            - Read title
                                  │            - Read content
                                  │            - Add to canvas
                                  │                │
                                  │                ▼
                                  │          Read metadata
                                  │                │
                                  ▼                ▼
                              Cleanup          Success
                              Return -1        Return 0
```

## Performance Characteristics

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Add box | O(1) amortized | Realloc when capacity reached |
| Remove box | O(n) | Shifts remaining boxes |
| Find box by ID | O(n) | Linear search |
| Find box at coords | O(n) | Linear search (reverse) |
| Render frame | O(b) | b = visible boxes (culled) |
| Pan/Zoom | O(1) | Just updates camera |

### Space Complexity

- Canvas: O(n) where n = number of boxes
- Each box: O(m) where m = content lines
- Total: O(n × m) worst case

### Rendering Optimization

- **Viewport culling**: Only visible boxes rendered
- **Lazy rendering**: Render on-demand, not continuous
- **Efficient coordinates**: Integer screen coords for fast drawing
- **Target**: 60 FPS (~16.7ms per frame)

---

**Last Updated:** 2025-11-18
**Version:** 1.0
**Maintainer:** boxes-live project
