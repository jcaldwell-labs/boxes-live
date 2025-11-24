#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>
#include "types.h"  // For Box typedef

// Using evdev interface instead of joydev (WSL compatible)
// Note: linux/input.h included in joystick.c only to avoid conflicts with ncurses

// Input modes (4-mode system for clear workflow)
typedef enum {
    MODE_VIEW,      // Default: navigate and observe (pan/zoom only)
    MODE_SELECT,    // Choose and move boxes
    MODE_EDIT,      // Modify selected box properties
    MODE_CONNECT    // Link boxes together
} InputMode;

// Parameter types for parameter mode
typedef enum {
    PARAM_WIDTH,
    PARAM_HEIGHT,
    PARAM_COLOR
} ParamType;

// Note: Using struct input_event from linux/input.h (evdev interface)

// Joystick state
typedef struct {
    int fd;                     // File descriptor for /dev/input/js*
    bool available;             // Is joystick connected?
    InputMode mode;             // Current input mode

    // Cursor position (navigation mode)
    double cursor_x;
    double cursor_y;

    // Selected box (edit/parameter modes)
    int selected_box_id;

    // Parameter editor state (Phase 2)
    bool param_editor_active;   // Is parameter panel open?
    int param_selected_field;   // Which field has cursor (0-3)

    // Backup of original values (for cancel)
    int param_original_width;
    int param_original_height;
    int param_original_color;

    // Live editing values (applied to box in real-time)
    int param_edit_width;
    int param_edit_height;
    int param_edit_color;

    // Text editor state (Phase 3)
    bool text_editor_active;    // Is text editor open?
    char *text_edit_buffer;     // Current text being edited
    int text_cursor_pos;        // Cursor position in buffer
    int text_buffer_size;       // Allocated size of buffer

    // Axis state (for analog input)
    int16_t axis_x;             // Left/right (raw value)
    int16_t axis_y;             // Up/down (raw value)

    // Axis calibration (queried from device)
    int32_t axis_x_min;         // Minimum X value
    int32_t axis_x_max;         // Maximum X value
    int32_t axis_x_center;      // Center/neutral X value
    int32_t axis_y_min;         // Minimum Y value
    int32_t axis_y_max;         // Maximum Y value
    int32_t axis_y_center;      // Center/neutral Y value

    // Button state (for digital input)
    bool button[16];            // Up to 16 buttons
    bool button_prev[16];       // Previous frame state (for edge detection)

    // Reconnection
    int reconnect_counter;      // Frames since last reconnect attempt

    // Settling period (ignore axis input on startup)
    int settling_frames;        // Frames remaining in settling period

    // Visualizer toggle
    bool show_visualizer;       // Show joystick visualizer panel
} JoystickState;

// Initialize joystick subsystem
// Returns 0 on success, -1 if no joystick found
int joystick_init(JoystickState *state);

// Close joystick device
void joystick_close(JoystickState *state);

// Poll joystick for events (non-blocking)
// Updates state with current axis/button values
// Returns number of events processed
int joystick_poll(JoystickState *state);

// Check if button was just pressed (rising edge)
bool joystick_button_pressed(const JoystickState *state, int button);

// Check if button is currently held
bool joystick_button_held(const JoystickState *state, int button);

// Get normalized axis value (-1.0 to 1.0) with deadzone
double joystick_get_axis_normalized(const JoystickState *state, int axis_num);

// Mode transitions
void joystick_enter_view_mode(JoystickState *state);
void joystick_enter_select_mode(JoystickState *state);
void joystick_enter_edit_mode(JoystickState *state, int box_id);
void joystick_enter_connect_mode(JoystickState *state);

// Mode cycling (MENU button)
void joystick_cycle_mode(JoystickState *state);

// Parameter editor control (Phase 2)
void joystick_open_param_editor(JoystickState *state, const Box *box);
void joystick_close_param_editor(JoystickState *state, bool apply_changes, Box *box);

// Text editor control (Phase 3)
void joystick_open_text_editor(JoystickState *state, const Box *box);
void joystick_close_text_editor(JoystickState *state, bool save_changes, Box *box);
void joystick_text_editor_insert_char(JoystickState *state, char ch);
void joystick_text_editor_backspace(JoystickState *state);
void joystick_text_editor_move_cursor(JoystickState *state, int delta);

// Try to reconnect if disconnected
// Returns true if reconnection successful
bool joystick_try_reconnect(JoystickState *state);

// Joystick button mappings
#define BUTTON_A      0  // Button 0 (South/A)
#define BUTTON_B      1  // Button 1 (East/B)
#define BUTTON_X      2  // Button 2 (West/X)
#define BUTTON_Y      3  // Button 3 (North/Y)
#define BUTTON_LB     4  // Button 4 (Left Bumper/LB)
#define BUTTON_RB     5  // Button 5 (Right Bumper/RB)
#define BUTTON_BACK   6  // Button 6 (Back button)
#define BUTTON_MENU   7  // Button 7 (Menu button)
#define BUTTON_SELECT 8  // Button 8 (Select button)
#define BUTTON_START  9  // Button 9 (Start button)

// Joystick axes
#define AXIS_X  0  // Left/right
#define AXIS_Y  1  // Up/down

// Constants
#define JOYSTICK_DEADZONE 0.15       // Ignore small movements
#define JOYSTICK_RECONNECT_DELAY 60  // Frames between reconnect attempts
#define JOYSTICK_SETTLING_FRAMES 30  // Frames to ignore axis input on startup

#endif // JOYSTICK_H
