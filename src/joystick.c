#include "joystick.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <stdio.h>
#include <math.h>

// Initialize joystick subsystem
int joystick_init(JoystickState *state) {
    if (!state) return -1;

    // Initialize state
    memset(state, 0, sizeof(JoystickState));
    state->fd = -1;
    state->available = false;
    state->mode = MODE_NAVIGATION;
    state->selected_box_id = -1;
    state->selected_param = PARAM_WIDTH;
    state->cursor_x = 0.0;
    state->cursor_y = 0.0;

    // Try to open joystick device
    state->fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (state->fd < 0) {
        // No joystick available (not an error, just unavailable)
        return -1;
    }

    state->available = true;
    return 0;
}

// Close joystick device
void joystick_close(JoystickState *state) {
    if (!state) return;

    if (state->fd >= 0) {
        close(state->fd);
        state->fd = -1;
    }
    state->available = false;
}

// Poll joystick for events (non-blocking)
int joystick_poll(JoystickState *state) {
    if (!state || !state->available || state->fd < 0) {
        return 0;
    }

    // Copy current button state to previous state (for edge detection)
    memcpy(state->button_prev, state->button, sizeof(state->button));

    int events_processed = 0;
    JoystickEvent event;

    // Use select() for non-blocking read with timeout protection
    struct timeval timeout = {0, 0};  // Non-blocking (immediate return)
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(state->fd, &read_fds);

    // Check if data available
    int select_result = select(state->fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 0) {
        // Select error
        if (errno == EINTR) {
            return 0;  // Interrupted, try again next frame
        }
        // Other error - assume disconnection
        joystick_close(state);
        return 0;
    }

    if (select_result == 0) {
        // No data available
        return 0;
    }

    // Read all available events
    while (1) {
        ssize_t bytes = read(state->fd, &event, sizeof(event));

        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data available
                break;
            } else if (errno == ENODEV) {
                // Device disconnected
                joystick_close(state);
                break;
            } else {
                // Other error
                break;
            }
        }

        if (bytes != sizeof(event)) {
            // Incomplete read, stop processing
            break;
        }

        // Process event
        uint8_t event_type = event.type & ~JS_EVENT_INIT;

        if (event_type == JS_EVENT_BUTTON) {
            // Button event
            if (event.number < 16) {
                state->button[event.number] = (event.value != 0);
            }
        } else if (event_type == JS_EVENT_AXIS) {
            // Axis event
            if (event.number == AXIS_X) {
                state->axis_x = event.value;
            } else if (event.number == AXIS_Y) {
                state->axis_y = event.value;
            }
        }

        events_processed++;
    }

    return events_processed;
}

// Check if button was just pressed (rising edge)
bool joystick_button_pressed(const JoystickState *state, int button) {
    if (!state || button < 0 || button >= 16) {
        return false;
    }
    return state->button[button] && !state->button_prev[button];
}

// Check if button is currently held
bool joystick_button_held(const JoystickState *state, int button) {
    if (!state || button < 0 || button >= 16) {
        return false;
    }
    return state->button[button];
}

// Get normalized axis value (-1.0 to 1.0) with deadzone
double joystick_get_axis_normalized(const JoystickState *state, int axis_num) {
    if (!state) return 0.0;

    int16_t raw_value = 0;
    if (axis_num == AXIS_X) {
        raw_value = state->axis_x;
    } else if (axis_num == AXIS_Y) {
        raw_value = state->axis_y;
    } else {
        return 0.0;
    }

    // Normalize to -1.0 to 1.0
    double normalized = raw_value / 32768.0;

    // Apply deadzone
    if (fabs(normalized) < JOYSTICK_DEADZONE) {
        return 0.0;
    }

    return normalized;
}

// Mode transitions
void joystick_enter_navigation_mode(JoystickState *state) {
    if (!state) return;

    state->mode = MODE_NAVIGATION;
    state->selected_box_id = -1;
}

void joystick_enter_edit_mode(JoystickState *state, int box_id) {
    if (!state) return;

    state->mode = MODE_EDIT;
    state->selected_box_id = box_id;
}

void joystick_enter_parameter_mode(JoystickState *state) {
    if (!state) return;

    state->mode = MODE_PARAMETER;
    state->selected_param = PARAM_WIDTH;  // Start with width
    state->param_edit_value = 0;          // Will be set when entering mode
}

// Try to reconnect if disconnected
bool joystick_try_reconnect(JoystickState *state) {
    if (!state) return false;

    // Already connected?
    if (state->available && state->fd >= 0) {
        return true;
    }

    // Increment reconnection counter
    state->reconnect_counter++;

    // Only try reconnection every JOYSTICK_RECONNECT_DELAY frames (~1 second at 60 FPS)
    if (state->reconnect_counter < JOYSTICK_RECONNECT_DELAY) {
        return false;
    }

    // Reset counter
    state->reconnect_counter = 0;

    // Try to open joystick
    state->fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (state->fd < 0) {
        return false;
    }

    // Successfully reconnected
    state->available = true;

    // Reset state
    memset(state->button, 0, sizeof(state->button));
    memset(state->button_prev, 0, sizeof(state->button_prev));
    state->axis_x = 0;
    state->axis_y = 0;

    return true;
}
