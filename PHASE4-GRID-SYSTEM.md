# Phase 4: Grid System Design

## Goals
1. **Visual alignment aid** - Dots or lines to help position boxes
2. **Snap-to-grid** - Automatic alignment when enabled
3. **Configurable spacing** - 5, 10, 20 world units
4. **Easy toggle** - Turn grid on/off quickly
5. **Non-intrusive** - Grid visible but doesn't obscure content

## Grid Display

### Visual Style Options

**Option 1: Dot Grid** (Recommended)
```
.  .  .  .  .  .  .  .

.  .  .  .  .  .  .  .

.  .  .  .  .  .  .  .
```
- Minimal visual noise
- Clear alignment points
- Doesn't obscure boxes

**Option 2: Light Lines**
```
|  |  |  |  |  |  |  |
---+--+--+--+--+--+--+--
|  |  |  |  |  |  |  |
---+--+--+--+--+--+--+--
```
- More traditional graph paper feel
- Can be harder to see boxes

**Decision**: Use dot grid (Option 1)

### Grid Spacing

**Default**: 10 world units (good balance)

**Configurable**:
- Small: 5 units (precise alignment)
- Medium: 10 units (default)
- Large: 20 units (coarse alignment)

### Grid Rendering
- Render BEFORE boxes (background layer)
- Use dim color (gray dots)
- Only render visible grid points (viewport culling)
- Respect zoom level (grid appears/disappears at extreme zoom)

---

## Grid State

### Global Grid Configuration
```c
typedef struct {
    bool visible;           // Is grid displayed?
    bool snap_enabled;      // Is snap-to-grid active?
    int spacing;            // Grid spacing in world units (5/10/20)
    int color;              // Grid color (dim gray)
} GridConfig;
```

Store in Canvas structure or as global state.

### Integration Points
- Canvas structure (include/types.h)
- Viewport for coordinate transformation
- Render module for drawing
- Input handlers for toggle

---

## Toggle Controls

### Grid Visibility Toggle

**Keyboard**:
```
G → Toggle grid on/off
```

**Joystick**:
```
Button LB (hold) + Button RB (press) → Toggle grid
```

**Why this mapping?**
- Button LB alone is "Show Grid" in VIEW mode
- Holding LB + RB is deliberate (prevents accidents)
- Easy to remember: Both bumpers = grid

### Snap-to-Grid Toggle

**Keyboard**:
```
Shift + G → Toggle snap on/off
```

**Joystick**:
```
Button START (hold 1 second) → Toggle snap
```

**Why this mapping?**
- Hold START prevents accidental toggle
- START = settings/configuration
- Separate from grid visibility toggle

### Visual Indicators

**Status Bar**:
```
Cam: (0.0, 0.0) | Zoom: 1.00x | Selected: Box #1 | [GRID] [SNAP]
```

**Visualizer Panel**:
```
Grid: ON (10 units)
Snap: ON
```

---

## Snap-to-Grid Behavior

### What Gets Snapped

**Box Position**:
- Top-left corner (x, y) snaps to grid
- Applies when creating, moving, or releasing

**Box Size** (optional):
- Width/height snap to grid multiples
- Keeps boxes aligned to grid cells

### When Snap Occurs

**Box Creation**:
- New box position snapped if snap enabled

**Box Movement**:
- Continuous snapping during drag (live)
- OR snap on release (less jarring)

**Parameter Editing**:
- Width/height snap to grid increments

### Snap Algorithm

```c
// Snap value to nearest grid point
int snap_to_grid(double value, int grid_spacing) {
    return (int)(round(value / grid_spacing) * grid_spacing);
}

// Apply to box
if (grid_config.snap_enabled) {
    box->x = snap_to_grid(box->x, grid_config.spacing);
    box->y = snap_to_grid(box->y, grid_config.spacing);
}
```

---

## Rendering Implementation

### Grid Drawing Function

```c
void render_grid(const Canvas *canvas, const Viewport *vp, const GridConfig *grid) {
    if (!grid || !grid->visible) return;

    // Calculate visible world bounds
    double world_left = vp->cam_x;
    double world_top = vp->cam_y;
    double world_right = vp->cam_x + vp->term_width / vp->zoom;
    double world_bottom = vp->cam_y + vp->term_height / vp->zoom;

    // Find grid lines within viewport
    int grid_start_x = (int)(world_left / grid->spacing) * grid->spacing;
    int grid_start_y = (int)(world_top / grid->spacing) * grid->spacing;

    // Draw vertical lines (dots)
    for (int x = grid_start_x; x <= world_right; x += grid->spacing) {
        for (int y = grid_start_y; y <= world_bottom; y += grid->spacing) {
            int sx = world_to_screen_x(vp, x);
            int sy = world_to_screen_y(vp, y);

            if (sx >= 0 && sx < vp->term_width && sy >= 0 && sy < vp->term_height) {
                attron(COLOR_PAIR(8));  // Dim gray
                mvaddch(sy, sx, '.');
                attroff(COLOR_PAIR(8));
            }
        }
    }
}
```

### Render Order
1. Clear screen
2. **Render grid** (background)
3. Render boxes
4. Render cursor
5. Render status bar
6. Render panels (parameter, text, visualizer)

---

## Configuration System

### Grid Settings (Future)

```c
typedef struct {
    int spacing_options[3];  // {5, 10, 20}
    int current_spacing_idx; // 0, 1, or 2
    char dot_char;           // '.' or '·' or '+'
    int color_pair;          // ncurses color pair
    bool show_origin;        // Highlight (0,0) point
} GridSettings;
```

### User Preferences (Future Config File)

```ini
[grid]
visible = true
snap_enabled = false
spacing = 10
dot_char = .
color = gray

[controls]
grid_toggle_key = g
snap_toggle_button = start
mode_cycle_button = menu
```

---

## Implementation Steps

### Step 1: Grid State
- Add GridConfig to Canvas structure
- Initialize with defaults (visible=false, snap=false, spacing=10)
- Add grid color to terminal initialization (COLOR_PAIR 8)

### Step 2: Grid Rendering
- Implement render_grid() function
- Call before render_canvas() in main loop
- Optimize with viewport culling

### Step 3: Grid Toggle
- Add keyboard 'G' handler
- Add joystick LB+RB handler (both buttons simultaneously)
- Update status bar to show [GRID] indicator

### Step 4: Snap-to-Grid
- Add snap_to_grid() helper function
- Apply on box creation
- Apply on box movement (on release)
- Apply in parameter editor (optional)
- Add [SNAP] indicator to status bar

### Step 5: Grid Configuration
- Cycle spacing with Shift+G (5 → 10 → 20 → 5)
- Show current spacing in status bar "[GRID:10]"
- (Future) Add to settings panel

---

## Testing Scenarios

### Grid Visibility
1. Start application (grid off)
2. Press G → Grid appears
3. Press G again → Grid disappears
4. Zoom in → Grid spacing appears larger
5. Zoom out → Grid dots closer together
6. Pan around → Grid scrolls with canvas

### Snap-to-Grid
1. Enable grid (G)
2. Enable snap (Shift+G or hold START)
3. Create box with N
4. Notice box position snapped to grid
5. Drag box with mouse
6. Release → Box snaps to nearest grid point
7. Move box with joystick
8. Watch it snap during movement

### Alignment Workflow
1. Enable grid and snap
2. Create multiple boxes
3. Notice they all align perfectly
4. Create diagram/flowchart
5. Boxes line up naturally
6. Disable snap for fine-tuning
7. Re-enable for bulk work

---

## Configuration Points (For Later)

These will be easy to adjust:
- Grid toggle button (LB+RB, or Button BACK, or G key)
- Snap toggle method (hold vs press, which button)
- Default spacing (5, 10, or 20)
- Dot character (., ·, +, ×)
- Grid color (gray, cyan, dim white)
- Snap strength (snap on move vs snap on release)
- Snap tolerance (exact vs fuzzy snapping)
- Show origin marker at (0,0)

---

## Next: Implementation

Start implementing:
1. Add GridConfig to Canvas structure
2. Initialize grid color in terminal
3. Implement render_grid() function
4. Add grid toggle handlers
5. Add snap-to-grid logic
6. Update status bar with indicators

Then commit as Phase 4.
