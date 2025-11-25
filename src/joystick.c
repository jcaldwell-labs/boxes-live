#include "joystick.h"
#include "types.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <linux/input.h>

// Max buttons for evdev API checks
#define MAX_BUTTONS 16

// Debug logging
#ifdef DEBUG
#include <stdarg.h>
static FILE *debug_log = NULL;

static void debug_init_log(void) {
    if (!debug_log) {
        debug_log = fopen("/tmp/joystick_debug.log", "w");
        if (debug_log) {
            fprintf(debug_log, "=== Joystick Debug Log ===\n");
            fflush(debug_log);
        }
    }
}

static void debug_log_event(const char *fmt, ...) {
    if (!debug_log) debug_init_log();
    if (debug_log) {
        va_list args;
        va_start(args, fmt);
        vfprintf(debug_log, fmt, args);
        va_end(args);
        fflush(debug_log);
    }
}

static void debug_close_log(void) {
    if (debug_log) {
        fclose(debug_log);
        debug_log = NULL;
    }
}
#else
#define debug_init_log()
#define debug_log_event(...)
#define debug_close_log()
#endif

// Initialize joystick subsystem
int joystick_init(JoystickState *state) {
    if (!state) return -1;

    debug_init_log();

    // Initialize state
    memset(state, 0, sizeof(JoystickState));
    state->fd = -1;
    state->available = false;
    state->mode = MODE_NAV;  // Start in NAV mode (safe exploration)
    state->selected_box_id = -1;
    state->cursor_x = 0.0;
    state->cursor_y = 0.0;
    state->settling_frames = JOYSTICK_SETTLING_FRAMES;
    state->show_visualizer = true;
    state->param_editor_active = false;
    state->text_editor_active = false;
    state->text_edit_buffer = NULL;

    // Try to open joystick device (evdev interface for WSL compatibility)
    state->fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (state->fd < 0) {
        // No joystick available (not an error, just unavailable)
        debug_log_event("Failed to open /dev/input/event0: %s\n", strerror(errno));
        return -1;
    }

    debug_log_event("Joystick opened successfully on /dev/input/event0 (fd=%d)\n", state->fd);

    // Query axis information from device
    struct input_absinfo abs_info;

    // Get X axis info
    if (ioctl(state->fd, EVIOCGABS(ABS_X), &abs_info) == 0) {
        state->axis_x_min = abs_info.minimum;
        state->axis_x_max = abs_info.maximum;
        state->axis_x_center = (abs_info.minimum + abs_info.maximum) / 2;
        debug_log_event("X-axis: min=%d, max=%d, center=%d (from device)\n",
                       state->axis_x_min, state->axis_x_max, state->axis_x_center);
    } else {
        // Fallback: assume standard 16-bit range
        state->axis_x_min = -32768;
        state->axis_x_max = 32767;
        state->axis_x_center = 0;
        debug_log_event("X-axis: using default 16-bit range (ioctl failed)\n");
    }

    // Get Y axis info
    if (ioctl(state->fd, EVIOCGABS(ABS_Y), &abs_info) == 0) {
        state->axis_y_min = abs_info.minimum;
        state->axis_y_max = abs_info.maximum;
        state->axis_y_center = (abs_info.minimum + abs_info.maximum) / 2;
        debug_log_event("Y-axis: min=%d, max=%d, center=%d (from device)\n",
                       state->axis_y_min, state->axis_y_max, state->axis_y_center);
    } else {
        // Fallback: assume standard 16-bit range
        state->axis_y_min = -32768;
        state->axis_y_max = 32767;
        state->axis_y_center = 0;
        debug_log_event("Y-axis: using default 16-bit range (ioctl failed)\n");
    }

    state->available = true;
    return 0;
}

// Close joystick device
void joystick_close(JoystickState *state) {
    if (!state) return;

    // Clean up text editor if active
    if (state->text_edit_buffer) {
        free(state->text_edit_buffer);
        state->text_edit_buffer = NULL;
    }

    if (state->fd >= 0) {
        debug_log_event("Closing joystick (fd=%d)\n", state->fd);
        close(state->fd);
        state->fd = -1;
    }
    state->available = false;
    debug_close_log();
}

// Poll joystick for events (non-blocking)
int joystick_poll(JoystickState *state) {
    if (!state || !state->available || state->fd < 0) {
        return 0;
    }

    // Decrement settling period counter
    if (state->settling_frames > 0) {
        state->settling_frames--;
    }

    // Copy current button state to previous state (for edge detection)
    memcpy(state->button_prev, state->button, sizeof(state->button));

    int events_processed = 0;
    struct input_event event;

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

        // Process evdev event (different from joydev)
        if (event.type == EV_ABS) {
            // Absolute axis event (analog sticks)
            debug_log_event("EV_ABS: code=%d value=%d\n", event.code, event.value);
            if (event.code == ABS_X) {
                state->axis_x = event.value;
                debug_log_event("  -> AXIS_X = %d (normalized: %.3f)\n", event.value,
                               joystick_get_axis_normalized(state, AXIS_X));
            } else if (event.code == ABS_Y) {
                state->axis_y = event.value;
                debug_log_event("  -> AXIS_Y = %d (normalized: %.3f)\n", event.value,
                               joystick_get_axis_normalized(state, AXIS_Y));
            } else {
                debug_log_event("  -> Unknown ABS axis (code=%d)\n", event.code);
            }
        } else if (event.type == EV_KEY) {
            // Button event
            int button = -1;

            // Map evdev button codes to 0-15
            if (event.code >= BTN_JOYSTICK && event.code < BTN_JOYSTICK + MAX_BUTTONS) {
                button = event.code - BTN_JOYSTICK;
                debug_log_event("EV_KEY (BTN_JOYSTICK): code=%d (0x%x) value=%d -> button=%d\n",
                               event.code, event.code, event.value, button);
            } else if (event.code >= BTN_GAMEPAD && event.code < BTN_GAMEPAD + MAX_BUTTONS) {
                button = event.code - BTN_GAMEPAD;
                debug_log_event("EV_KEY (BTN_GAMEPAD): code=%d (0x%x) value=%d -> button=%d\n",
                               event.code, event.code, event.value, button);
            } else {
                debug_log_event("EV_KEY (UNKNOWN): code=%d (0x%x) value=%d\n",
                               event.code, event.code, event.value);
            }

            if (button >= 0 && button < 16) {
                state->button[button] = (event.value != 0);
            }
        } else if (event.type == EV_SYN) {
            // Sync event (marks end of event frame)
            debug_log_event("EV_SYN\n");
        } else {
            debug_log_event("Event: type=%d code=%d value=%d\n", event.type, event.code, event.value);
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
    int32_t min_val = 0, max_val = 0, center_val = 0;

    if (axis_num == AXIS_X) {
        raw_value = state->axis_x;
        min_val = state->axis_x_min;
        max_val = state->axis_x_max;
        center_val = state->axis_x_center;
    } else if (axis_num == AXIS_Y) {
        raw_value = state->axis_y;
        min_val = state->axis_y_min;
        max_val = state->axis_y_max;
        center_val = state->axis_y_center;
    } else {
        return 0.0;
    }

    // Normalize based on actual device range
    // Center the value around 0
    double centered = raw_value - center_val;

    // Normalize to -1.0 to 1.0 based on the range
    double normalized;
    if (centered > 0) {
        // Positive direction: map center to max -> 0.0 to 1.0
        double range = max_val - center_val;
        normalized = (range > 0) ? (centered / range) : 0.0;
    } else {
        // Negative direction: map min to center -> -1.0 to 0.0
        double range = center_val - min_val;
        normalized = (range > 0) ? (centered / range) : 0.0;
    }

    // Apply deadzone
    if (fabs(normalized) < JOYSTICK_DEADZONE) {
        return 0.0;
    }

    return normalized;
}

// Mode transitions
void joystick_enter_nav_mode(JoystickState *state) {
    if (!state) return;

    state->mode = MODE_NAV;
    state->selected_box_id = -1;
}

void joystick_enter_selection_mode(JoystickState *state) {
    if (!state) return;

    state->mode = MODE_SELECTION;
    // Keep selected_box_id if already set
}

void joystick_enter_edit_mode(JoystickState *state, int box_id) {
    if (!state) return;

    state->mode = MODE_EDIT;
    state->selected_box_id = box_id;
}

// Mode cycling (LB button - global toggle)
// Cycles: NAV → SELECTION → EDIT → NAV
void joystick_cycle_mode(JoystickState *state) {
    if (!state) return;

    switch (state->mode) {
        case MODE_NAV:
            joystick_enter_selection_mode(state);
            break;
        case MODE_SELECTION:
            // Only allow EDIT if a box is selected
            if (state->selected_box_id >= 0) {
                joystick_enter_edit_mode(state, state->selected_box_id);
            } else {
                // Skip EDIT and go back to NAV if no box selected
                joystick_enter_nav_mode(state);
            }
            break;
        case MODE_EDIT:
            joystick_enter_nav_mode(state);
            break;
    }
}

// Open parameter editor (Phase 2)
void joystick_open_param_editor(JoystickState *state, const Box *box) {
    if (!state || !box) return;

    // Backup original values
    state->param_original_width = box->width;
    state->param_original_height = box->height;
    state->param_original_color = box->color;

    // Initialize edit values
    state->param_edit_width = box->width;
    state->param_edit_height = box->height;
    state->param_edit_color = box->color;

    // Reset field selection
    state->param_selected_field = 0;

    // Activate panel
    state->param_editor_active = true;
}

// Close parameter editor (Phase 2)
void joystick_close_param_editor(JoystickState *state, bool apply_changes, Box *box) {
    if (!state) return;

    if (apply_changes && box) {
        // Apply changes to box
        box->width = state->param_edit_width;
        box->height = state->param_edit_height;
        box->color = state->param_edit_color;
    } else if (box) {
        // Cancel: restore original values
        box->width = state->param_original_width;
        box->height = state->param_original_height;
        box->color = state->param_original_color;
    }

    // Deactivate panel
    state->param_editor_active = false;
}

// Open text editor (Phase 3 - basic title editing)
void joystick_open_text_editor(JoystickState *state, const Box *box) {
    if (!state || !box) return;

    // Allocate buffer for text editing (start with reasonable size)
    state->text_buffer_size = 256;
    state->text_edit_buffer = malloc(state->text_buffer_size);
    if (!state->text_edit_buffer) {
        return;  // Allocation failed
    }

    // Copy current title into buffer
    if (box->title) {
        strncpy(state->text_edit_buffer, box->title, state->text_buffer_size - 1);
        state->text_edit_buffer[state->text_buffer_size - 1] = '\0';
    } else {
        state->text_edit_buffer[0] = '\0';
    }

    // Set cursor at end of text
    state->text_cursor_pos = strlen(state->text_edit_buffer);

    // Activate editor
    state->text_editor_active = true;
}

// Close text editor (Phase 3)
void joystick_close_text_editor(JoystickState *state, bool save_changes, Box *box) {
    if (!state) return;

    if (save_changes && box && state->text_edit_buffer) {
        // Free old title
        free(box->title);

        // Allocate and copy new title
        box->title = strdup(state->text_edit_buffer);
    }

    // Free edit buffer
    free(state->text_edit_buffer);
    state->text_edit_buffer = NULL;
    state->text_buffer_size = 0;
    state->text_cursor_pos = 0;

    // Deactivate editor
    state->text_editor_active = false;
}

// Insert character at cursor position (Phase 3)
void joystick_text_editor_insert_char(JoystickState *state, char ch) {
    if (!state || !state->text_edit_buffer || !state->text_editor_active) return;

    int len = strlen(state->text_edit_buffer);

    // Check if we need to resize buffer
    if (len + 2 >= state->text_buffer_size) {
        int new_size = state->text_buffer_size * 2;
        char *new_buffer = realloc(state->text_edit_buffer, new_size);
        if (!new_buffer) {
            return;  // Allocation failed, don't insert
        }
        state->text_edit_buffer = new_buffer;
        state->text_buffer_size = new_size;
    }

    // Clamp cursor to valid range
    if (state->text_cursor_pos < 0) state->text_cursor_pos = 0;
    if (state->text_cursor_pos > len) state->text_cursor_pos = len;

    // Insert character by shifting text right
    memmove(&state->text_edit_buffer[state->text_cursor_pos + 1],
            &state->text_edit_buffer[state->text_cursor_pos],
            len - state->text_cursor_pos + 1);  // +1 for null terminator

    // Insert character
    state->text_edit_buffer[state->text_cursor_pos] = ch;
    state->text_cursor_pos++;
}

// Delete character before cursor (backspace) (Phase 3)
void joystick_text_editor_backspace(JoystickState *state) {
    if (!state || !state->text_edit_buffer || !state->text_editor_active) return;

    int len = strlen(state->text_edit_buffer);

    // Can't backspace if cursor at start
    if (state->text_cursor_pos <= 0) return;

    // Clamp cursor to valid range
    if (state->text_cursor_pos > len) state->text_cursor_pos = len;

    // Delete character by shifting text left
    memmove(&state->text_edit_buffer[state->text_cursor_pos - 1],
            &state->text_edit_buffer[state->text_cursor_pos],
            len - state->text_cursor_pos + 1);  // +1 for null terminator

    // Move cursor back
    state->text_cursor_pos--;
}

// Move cursor left or right (Phase 3)
void joystick_text_editor_move_cursor(JoystickState *state, int delta) {
    if (!state || !state->text_edit_buffer || !state->text_editor_active) return;

    int len = strlen(state->text_edit_buffer);

    state->text_cursor_pos += delta;

    // Clamp to valid range
    if (state->text_cursor_pos < 0) state->text_cursor_pos = 0;
    if (state->text_cursor_pos > len) state->text_cursor_pos = len;
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

    // Try to open joystick (evdev interface)
    state->fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (state->fd < 0) {
        return false;
    }

    // Query axis information from device
    struct input_absinfo abs_info;

    // Get X axis info
    if (ioctl(state->fd, EVIOCGABS(ABS_X), &abs_info) == 0) {
        state->axis_x_min = abs_info.minimum;
        state->axis_x_max = abs_info.maximum;
        state->axis_x_center = (abs_info.minimum + abs_info.maximum) / 2;
    } else {
        // Fallback: assume standard 16-bit range
        state->axis_x_min = -32768;
        state->axis_x_max = 32767;
        state->axis_x_center = 0;
    }

    // Get Y axis info
    if (ioctl(state->fd, EVIOCGABS(ABS_Y), &abs_info) == 0) {
        state->axis_y_min = abs_info.minimum;
        state->axis_y_max = abs_info.maximum;
        state->axis_y_center = (abs_info.minimum + abs_info.maximum) / 2;
    } else {
        // Fallback: assume standard 16-bit range
        state->axis_y_min = -32768;
        state->axis_y_max = 32767;
        state->axis_y_center = 0;
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
