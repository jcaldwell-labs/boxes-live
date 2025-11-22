#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>

// Using evdev interface instead of joydev (WSL compatible)
// Note: linux/input.h included in joystick.c only to avoid conflicts with ncurses

// Input modes
typedef enum {
    MODE_NAVIGATION,    // Default: pan/zoom viewport
    MODE_EDIT,          // Move/manipulate selected box
    MODE_PARAMETER      // Adjust box properties
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

    // Parameter editing
    ParamType selected_param;
    int param_edit_value;       // Temporary value during editing

    // Axis state (for analog input)
    int16_t axis_x;             // Left/right (-32768 to 32767)
    int16_t axis_y;             // Up/down

    // Button state (for digital input)
    bool button[16];            // Up to 16 buttons
    bool button_prev[16];       // Previous frame state (for edge detection)

    // Reconnection
    int reconnect_counter;      // Frames since last reconnect attempt
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
void joystick_enter_navigation_mode(JoystickState *state);
void joystick_enter_edit_mode(JoystickState *state, int box_id);
void joystick_enter_parameter_mode(JoystickState *state);

// Try to reconnect if disconnected
// Returns true if reconnection successful
bool joystick_try_reconnect(JoystickState *state);

// Joystick button mappings
#define BUTTON_A      0  // Button 0
#define BUTTON_B      1  // Button 1
#define BUTTON_X      2  // Button 2
#define BUTTON_Y      3  // Button 3
#define BUTTON_START  9  // Start button
#define BUTTON_SELECT 8  // Select button

// Joystick axes
#define AXIS_X  0  // Left/right
#define AXIS_Y  1  // Up/down

// Constants
#define JOYSTICK_DEADZONE 0.15       // Ignore small movements
#define JOYSTICK_RECONNECT_DELAY 60  // Frames between reconnect attempts

#endif // JOYSTICK_H
