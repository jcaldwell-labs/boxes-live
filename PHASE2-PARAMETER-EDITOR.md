# Phase 2: Parameter Editor Design

## Goals
1. **Obvious workflow** - Clear what you're editing, how to change it, how to apply
2. **Live preview** - See changes immediately
3. **Easy to cancel** - Undo changes if you don't like them
4. **Configurable** - Easy to add new parameters or change controls later

## Activation
From EDIT mode, press **Button Y** → Opens parameter panel (modal overlay)

## Parameter Panel Layout

```
┌─ Edit Box Properties ────────────────────────────────┐
│                                                       │
│  ┌─ Preview ─────────────┐                           │
│  │                        │  Current Box:             │
│  │    Example Box         │  Name: "Example Box"     │
│  │                        │  Position: (25, 10)      │
│  └────────────────────────┘                           │
│                                                       │
│  ┌─ Properties ──────────────────────────────────┐   │
│  │                                                │   │
│  │  [>] Width:   25   ◄ ───────── ►              │   │
│  │  [ ] Height:  10   ◄ ───────── ►              │   │
│  │  [ ] Color:   Blue ◄ ───────── ►              │   │
│  │  [ ] Border:  Solid◄ ───────── ►              │   │
│  │                                                │   │
│  └────────────────────────────────────────────────┘   │
│                                                       │
│  Controls:                                            │
│    Up/Down:    Select field                           │
│    Left/Right: Adjust value (or LB/RB)                │
│    A: Apply & Close      B: Cancel & Close            │
│                                                       │
└───────────────────────────────────────────────────────┘
```

## Controls

### Field Navigation
- **Stick Up/Down**: Move cursor between fields
- **D-pad Up/Down** (future): Alternative navigation

### Value Adjustment
- **Stick Left/Right**: Adjust value (slow, precise)
- **Button LB**: Decrease value (large step)
- **Button RB**: Increase value (large step)

### Actions
- **Button A**: Apply changes and close panel
- **Button B**: Cancel changes and close panel (restore original values)
- **Button X**: Reset field to default
- **Button Y**: Toggle between presets (future)

## Parameter Types

### Numeric (Width, Height)
- Range: 10-80 (width), 3-30 (height)
- Increment: 1 (stick), 5 (LB/RB)
- Visual: `Width: 25  ◄ ────●──── ►` (slider representation)

### Enumerated (Color, Border Style)
- Options: List of choices
- Navigation: Left/Right cycles through options
- Visual: `Color: Blue  ◄ [Red|Green|Blue|Yellow] ►`

### Boolean (future: Shadow, Grid Snap)
- Values: On/Off
- Toggle with A or Left/Right
- Visual: `Shadow: [✓] On` or `Shadow: [ ] Off`

## State Management

```c
typedef struct {
    bool active;              // Is panel open?
    int selected_field;       // Which field has cursor

    // Backup of original values (for cancel)
    int original_width;
    int original_height;
    int original_color;
    int original_border_style;

    // Live editing values
    int edit_width;
    int edit_height;
    int edit_color;
    int edit_border_style;
} ParameterEditorState;
```

Stored in `JoystickState` or separate global state.

## Configuration System (Future)

Make everything configurable via struct:

```c
typedef struct {
    const char *name;         // "Width"
    ParamType type;           // NUMERIC, ENUM, BOOLEAN
    int min_value;            // For NUMERIC
    int max_value;
    int step_small;           // Stick adjustment
    int step_large;           // Button adjustment
    const char **options;     // For ENUM
    int num_options;
} ParameterConfig;

// Easy to add new parameters:
ParameterConfig box_params[] = {
    {"Width",  PARAM_NUMERIC, 10, 80, 1, 5, NULL, 0},
    {"Height", PARAM_NUMERIC, 3, 30, 1, 3, NULL, 0},
    {"Color",  PARAM_ENUM, 0, 0, 0, 0, color_names, 8},
    // Add new parameters here easily
};
```

## Implementation Steps

1. **Add ParameterEditorState to JoystickState**
2. **Create parameter panel rendering function**
   - Draw modal overlay (darkened background)
   - Draw box outline and preview
   - Draw field list with cursor indicator
   - Draw control hints at bottom
3. **Handle parameter mode input**
   - Field navigation (up/down)
   - Value adjustment (left/right, LB/RB)
   - Apply/Cancel (A/B)
4. **Backup/restore mechanism**
   - Store original values on open
   - Restore on cancel
   - Apply on confirm
5. **Live preview**
   - Update box in real-time as values change
   - Visual preview box in panel shows changes

## Open Design Questions

1. **Panel positioning**: Center of screen (current) or side panel?
2. **Live preview**: Update actual box, or show separate preview?
3. **Field focus**: Highlight whole row, or just field name?
4. **Value sliders**: Show visual slider, or just text?
5. **Undo**: Single-level (cancel), or multi-level undo history?

## Configuration Points for Later

These can be easily adjusted:
- Button assignments (A=apply, B=cancel can be swapped)
- Navigation (stick vs d-pad preference)
- Adjustment speed (stick sensitivity, button step size)
- Panel appearance (size, position, colors)
- Available parameters (add/remove fields)
- Parameter constraints (min/max values)

## Next: Implementation

Start with basic version:
- Modal panel with 4 fields (width, height, color, border)
- Simple navigation with stick
- Clear Apply/Cancel buttons
- Backup/restore on cancel

Then iterate to add:
- Live preview
- Better visual indicators
- Configuration system
