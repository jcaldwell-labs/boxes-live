# Phase 5a: Configuration System

## Goals
1. **Make joystick configurable** - User can remap buttons easily
2. **Simple format** - Easy to read and edit by hand
3. **Sensible defaults** - Works without config file
4. **Extensible** - Easy to add new settings later

## Config File Format

Using simple INI-style format (easy to parse in C, no dependencies needed).

**Location**: `~/.config/boxes-live/config.ini`

**Example**:
```ini
[general]
default_spacing = 10
auto_save = false
show_visualizer = true

[grid]
visible = false
snap_enabled = false
spacing = 10

[joystick]
deadzone = 0.15
settling_frames = 30

# Button mappings for VIEW mode
[joystick.view]
button_a = zoom_in
button_b = zoom_out
button_x = create_box
button_y = enter_select
button_lb = toggle_grid
button_rb = toggle_snap
button_menu = cycle_mode
button_start = save_canvas
button_select = quit
button_back = toggle_visualizer

# Button mappings for SELECT mode
[joystick.select]
button_a = cycle_box
button_b = back_to_view
button_x = enter_edit
button_y = delete_box

# Button mappings for EDIT mode
[joystick.edit]
button_a = edit_text
button_b = back_to_select
button_x = cycle_color
button_y = parameters
button_lb = decrease_value
button_rb = increase_value

# Button mappings for CONNECT mode
[joystick.connect]
button_a = connect_boxes
button_b = cancel
button_x = cycle_style
button_y = delete_connection
```

## Implementation

### Config Structure
```c
typedef struct {
    // General settings
    int default_spacing;
    bool auto_save;
    bool show_visualizer;

    // Grid settings
    bool grid_visible;
    bool grid_snap_enabled;
    int grid_spacing;

    // Joystick settings
    double joystick_deadzone;
    int joystick_settling_frames;

    // Button mappings per mode (action names as strings)
    char *view_button_a;
    char *view_button_b;
    // ... etc for all buttons and modes
} AppConfig;
```

### Config Parser
Simple line-by-line parser:
1. Read file line by line
2. Track current section [section]
3. Parse key=value pairs
4. Store in AppConfig structure
5. Fall back to defaults for missing values

### Action Name → Function Mapping
```c
typedef void (*ActionFunction)(Canvas*, Viewport*, JoystickState*, InputEvent*);

typedef struct {
    const char *name;       // "zoom_in"
    CanvasAction action;    // ACTION_ZOOM_IN
} ActionMapping;

ActionMapping action_map[] = {
    {"zoom_in", ACTION_ZOOM_IN},
    {"zoom_out", ACTION_ZOOM_OUT},
    {"create_box", ACTION_CREATE_BOX},
    {"toggle_grid", ACTION_TOGGLE_GRID},
    // ... etc
};
```

## Default Config Generation

On first run:
1. Check if config file exists
2. If not, create with defaults
3. User can then edit as needed

```c
void create_default_config(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "# Boxes-Live Configuration\n\n");
    fprintf(f, "[general]\n");
    fprintf(f, "show_visualizer = true\n\n");
    fprintf(f, "[joystick.view]\n");
    fprintf(f, "button_a = zoom_in\n");
    // ... write all defaults
    fclose(f);
}
```

## Integration with Existing Code

### Minimal Changes
- Load config in main() before creating canvas
- Pass config to input handlers
- Look up button action from config instead of hardcoded switch/case
- Update visualizer to show configured action names

### Backwards Compatibility
- All current behavior is default
- Config file is optional
- Missing settings use hardcoded defaults

## File Structure

```
include/config.h        # Config structure and function declarations
src/config.c            # Config parser implementation
config.ini.example      # Example config file for users
```

## Implementation Steps

1. Create config.h with AppConfig structure
2. Create config.c with simple INI parser
3. Add config loading to main()
4. Update input_unified.c to use config
5. Create default config file
6. Test with custom button mappings

Let's start implementing!
