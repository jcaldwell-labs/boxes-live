/**
 * Joystick unit tests
 *
 * Tests for joystick logic that doesn't require hardware:
 * - Axis normalization calculations
 * - Button edge detection
 * - Mode state machine transitions
 * - Text editor string operations
 *
 * NOTE: On Windows, the joystick functions are stubs that return default values.
 * The full tests only run on Linux where the real implementation exists.
 */

#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <string.h>
#include "test.h"
#include "../include/joystick.h"
#include "../include/types.h"

#ifdef _WIN32
#define JOYSTICK_AVAILABLE 0
#else
#define JOYSTICK_AVAILABLE 1
#endif

/* Helper to create a mock joystick state for testing */
static void init_mock_state(JoystickState *state) {
    memset(state, 0, sizeof(JoystickState));
    state->fd = -1;
    state->available = false;
    state->mode = MODE_NAV;
    state->selected_box_id = -1;

    /* Standard 16-bit axis range */
    state->axis_x_min = -32768;
    state->axis_x_max = 32767;
    state->axis_x_center = 0;
    state->axis_y_min = -32768;
    state->axis_y_max = 32767;
    state->axis_y_center = 0;

    /* Start at center */
    state->axis_x = 0;
    state->axis_y = 0;
}

#if JOYSTICK_AVAILABLE
/* Helper to create a mock joystick state with 0-255 range (like some controllers) */
static void init_mock_state_0_255(JoystickState *state) {
    memset(state, 0, sizeof(JoystickState));
    state->fd = -1;
    state->available = false;
    state->mode = MODE_NAV;
    state->selected_box_id = -1;

    /* 0-255 axis range (center at 128) */
    state->axis_x_min = 0;
    state->axis_x_max = 255;
    state->axis_x_center = 128;
    state->axis_y_min = 0;
    state->axis_y_max = 255;
    state->axis_y_center = 128;

    /* Start at center */
    state->axis_x = 128;
    state->axis_y = 128;
}
#endif

int main(void) {
    TEST_START();

#if JOYSTICK_AVAILABLE
    /* ========================================================================
     * Axis normalization tests (Linux only - Windows stub returns 0.0)
     * ======================================================================== */

    TEST("Axis normalization: center returns 0") {
        JoystickState state;
        init_mock_state(&state);

        state.axis_x = 0;  /* At center */
        state.axis_y = 0;

        double norm_x = joystick_get_axis_normalized(&state, AXIS_X);
        double norm_y = joystick_get_axis_normalized(&state, AXIS_Y);

        ASSERT_NEAR(norm_x, 0.0, 0.01, "X axis at center is 0.0");
        ASSERT_NEAR(norm_y, 0.0, 0.01, "Y axis at center is 0.0");
    }

    TEST("Axis normalization: full positive returns ~1.0") {
        JoystickState state;
        init_mock_state(&state);

        state.axis_x = 32767;  /* Full right */
        state.axis_y = 32767;  /* Full down */

        double norm_x = joystick_get_axis_normalized(&state, AXIS_X);
        double norm_y = joystick_get_axis_normalized(&state, AXIS_Y);

        ASSERT_NEAR(norm_x, 1.0, 0.01, "X axis full right is ~1.0");
        ASSERT_NEAR(norm_y, 1.0, 0.01, "Y axis full down is ~1.0");
    }

    TEST("Axis normalization: full negative returns ~-1.0") {
        JoystickState state;
        init_mock_state(&state);

        state.axis_x = -32768;  /* Full left */
        state.axis_y = -32768;  /* Full up */

        double norm_x = joystick_get_axis_normalized(&state, AXIS_X);
        double norm_y = joystick_get_axis_normalized(&state, AXIS_Y);

        ASSERT_NEAR(norm_x, -1.0, 0.01, "X axis full left is ~-1.0");
        ASSERT_NEAR(norm_y, -1.0, 0.01, "Y axis full up is ~-1.0");
    }

    TEST("Axis normalization: deadzone filters small values") {
        JoystickState state;
        init_mock_state(&state);

        /* Small movement within deadzone (15% = ~4915 on 32767 scale) */
        state.axis_x = 3000;  /* About 9%, should be in deadzone */
        state.axis_y = -3000;

        double norm_x = joystick_get_axis_normalized(&state, AXIS_X);
        double norm_y = joystick_get_axis_normalized(&state, AXIS_Y);

        ASSERT_NEAR(norm_x, 0.0, 0.01, "Small X movement is filtered to 0");
        ASSERT_NEAR(norm_y, 0.0, 0.01, "Small Y movement is filtered to 0");
    }

    TEST("Axis normalization: outside deadzone passes through") {
        JoystickState state;
        init_mock_state(&state);

        /* Movement outside deadzone (~25% = ~8192) */
        state.axis_x = 10000;

        double norm_x = joystick_get_axis_normalized(&state, AXIS_X);

        ASSERT(norm_x > 0.1, "Movement outside deadzone is not filtered");
        ASSERT(norm_x < 1.0, "Movement is normalized to less than 1.0");
    }

    TEST("Axis normalization: 0-255 range (center at 128)") {
        JoystickState state;
        init_mock_state_0_255(&state);

        /* At center (128) */
        state.axis_x = 128;
        double norm_center = joystick_get_axis_normalized(&state, AXIS_X);
        ASSERT_NEAR(norm_center, 0.0, 0.01, "Center (128) normalizes to 0.0");

        /* Full right (255) */
        state.axis_x = 255;
        double norm_max = joystick_get_axis_normalized(&state, AXIS_X);
        ASSERT_NEAR(norm_max, 1.0, 0.01, "Max (255) normalizes to ~1.0");

        /* Full left (0) */
        state.axis_x = 0;
        double norm_min = joystick_get_axis_normalized(&state, AXIS_X);
        ASSERT_NEAR(norm_min, -1.0, 0.01, "Min (0) normalizes to ~-1.0");
    }

    /* ========================================================================
     * Button edge detection tests (Linux only - Windows stubs return false)
     * ======================================================================== */

    TEST("Button pressed: detects rising edge") {
        JoystickState state;
        init_mock_state(&state);

        /* Button was not pressed, now is pressed */
        state.button_prev[BUTTON_A] = false;
        state.button[BUTTON_A] = true;

        ASSERT(joystick_button_pressed(&state, BUTTON_A), "Button press detected");
        ASSERT(!joystick_button_released(&state, BUTTON_A), "Not a release");
        ASSERT(joystick_button_held(&state, BUTTON_A), "Button is held");
    }

    TEST("Button released: detects falling edge") {
        JoystickState state;
        init_mock_state(&state);

        /* Button was pressed, now is released */
        state.button_prev[BUTTON_A] = true;
        state.button[BUTTON_A] = false;

        ASSERT(!joystick_button_pressed(&state, BUTTON_A), "Not a press");
        ASSERT(joystick_button_released(&state, BUTTON_A), "Button release detected");
        ASSERT(!joystick_button_held(&state, BUTTON_A), "Button not held");
    }

    TEST("Button held: no edge when continuously pressed") {
        JoystickState state;
        init_mock_state(&state);

        /* Button was pressed and still is */
        state.button_prev[BUTTON_A] = true;
        state.button[BUTTON_A] = true;

        ASSERT(!joystick_button_pressed(&state, BUTTON_A), "No rising edge when held");
        ASSERT(!joystick_button_released(&state, BUTTON_A), "No falling edge when held");
        ASSERT(joystick_button_held(&state, BUTTON_A), "Button is held");
    }

    TEST("Button idle: no edge when continuously released") {
        JoystickState state;
        init_mock_state(&state);

        /* Button was not pressed and still isn't */
        state.button_prev[BUTTON_A] = false;
        state.button[BUTTON_A] = false;

        ASSERT(!joystick_button_pressed(&state, BUTTON_A), "No rising edge when idle");
        ASSERT(!joystick_button_released(&state, BUTTON_A), "No falling edge when idle");
        ASSERT(!joystick_button_held(&state, BUTTON_A), "Button not held");
    }

    /* ========================================================================
     * Mode state machine tests (Linux only - Windows stubs are no-ops)
     * ======================================================================== */

    TEST("Mode transitions: NAV -> SELECTION -> EDIT -> NAV") {
        JoystickState state;
        init_mock_state(&state);

        ASSERT_EQ(state.mode, MODE_NAV, "Starts in NAV mode");

        /* Select a box first (needed for EDIT mode) */
        state.selected_box_id = 1;

        joystick_cycle_mode(&state);
        ASSERT_EQ(state.mode, MODE_SELECTION, "Cycles to SELECTION");

        joystick_cycle_mode(&state);
        ASSERT_EQ(state.mode, MODE_EDIT, "Cycles to EDIT (box selected)");

        joystick_cycle_mode(&state);
        ASSERT_EQ(state.mode, MODE_NAV, "Cycles back to NAV");
    }

    TEST("Mode transition: SELECTION skips EDIT if no box selected") {
        JoystickState state;
        init_mock_state(&state);

        state.selected_box_id = -1;  /* No box selected */

        joystick_cycle_mode(&state);
        ASSERT_EQ(state.mode, MODE_SELECTION, "Cycles to SELECTION");

        joystick_cycle_mode(&state);
        ASSERT_EQ(state.mode, MODE_NAV, "Skips EDIT, goes to NAV (no box)");
    }

    TEST("Mode enter functions: direct mode setting") {
        JoystickState state;
        init_mock_state(&state);

        joystick_enter_selection_mode(&state);
        ASSERT_EQ(state.mode, MODE_SELECTION, "Enter selection mode works");

        joystick_enter_edit_mode(&state, 5);
        ASSERT_EQ(state.mode, MODE_EDIT, "Enter edit mode works");
        ASSERT_EQ(state.selected_box_id, 5, "Edit mode sets selected box ID");

        joystick_enter_nav_mode(&state);
        ASSERT_EQ(state.mode, MODE_NAV, "Enter nav mode works");
        ASSERT_EQ(state.selected_box_id, -1, "Nav mode clears selection");
    }

    /* ========================================================================
     * Text editor tests (Linux only - Windows stubs are no-ops)
     * ======================================================================== */

    TEST("Text editor: insert character") {
        JoystickState state;
        init_mock_state(&state);

        /* Manually set up text editor state */
        state.text_editor_active = true;
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Hello");
        state.text_cursor_pos = 5;  /* At end */

        joystick_text_editor_insert_char(&state, '!');

        ASSERT_STR_EQ(state.text_edit_buffer, "Hello!", "Character inserted at end");
        ASSERT_EQ(state.text_cursor_pos, 6, "Cursor advanced");

        free(state.text_edit_buffer);
    }

    TEST("Text editor: insert in middle") {
        JoystickState state;
        init_mock_state(&state);

        state.text_editor_active = true;
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Helo");
        state.text_cursor_pos = 3;  /* After "Hel" */

        joystick_text_editor_insert_char(&state, 'l');

        ASSERT_STR_EQ(state.text_edit_buffer, "Hello", "Character inserted in middle");
        ASSERT_EQ(state.text_cursor_pos, 4, "Cursor advanced");

        free(state.text_edit_buffer);
    }

    TEST("Text editor: backspace") {
        JoystickState state;
        init_mock_state(&state);

        state.text_editor_active = true;
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Hello");
        state.text_cursor_pos = 5;  /* At end */

        joystick_text_editor_backspace(&state);

        ASSERT_STR_EQ(state.text_edit_buffer, "Hell", "Character deleted");
        ASSERT_EQ(state.text_cursor_pos, 4, "Cursor moved back");

        free(state.text_edit_buffer);
    }

    TEST("Text editor: backspace at start does nothing") {
        JoystickState state;
        init_mock_state(&state);

        state.text_editor_active = true;
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Hello");
        state.text_cursor_pos = 0;  /* At start */

        joystick_text_editor_backspace(&state);

        ASSERT_STR_EQ(state.text_edit_buffer, "Hello", "String unchanged");
        ASSERT_EQ(state.text_cursor_pos, 0, "Cursor stays at 0");

        free(state.text_edit_buffer);
    }

    TEST("Text editor: cursor movement") {
        JoystickState state;
        init_mock_state(&state);

        state.text_editor_active = true;
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Hello");
        state.text_cursor_pos = 3;

        joystick_text_editor_move_cursor(&state, 1);
        ASSERT_EQ(state.text_cursor_pos, 4, "Cursor moved right");

        joystick_text_editor_move_cursor(&state, -2);
        ASSERT_EQ(state.text_cursor_pos, 2, "Cursor moved left");

        /* Test clamping */
        joystick_text_editor_move_cursor(&state, -100);
        ASSERT_EQ(state.text_cursor_pos, 0, "Cursor clamped to 0");

        joystick_text_editor_move_cursor(&state, 100);
        ASSERT_EQ(state.text_cursor_pos, 5, "Cursor clamped to string length");

        free(state.text_edit_buffer);
    }

    TEST("Text editor: inactive editor ignores input") {
        JoystickState state;
        init_mock_state(&state);

        state.text_editor_active = false;  /* Not active */
        state.text_buffer_size = 256;
        state.text_edit_buffer = malloc(256);
        strcpy(state.text_edit_buffer, "Hello");
        state.text_cursor_pos = 5;

        joystick_text_editor_insert_char(&state, '!');

        ASSERT_STR_EQ(state.text_edit_buffer, "Hello", "Inactive editor ignores insert");

        free(state.text_edit_buffer);
    }

#else
    /* Windows: Joystick is not available, run minimal tests */
    TEST("Windows: Joystick stub returns expected defaults") {
        JoystickState state;
        init_mock_state(&state);

        /* Axis normalization returns 0 on Windows */
        double norm = joystick_get_axis_normalized(&state, AXIS_X);
        ASSERT_NEAR(norm, 0.0, 0.01, "Windows stub returns 0.0 for axis");

        /* Button functions return false on Windows */
        ASSERT(!joystick_button_pressed(&state, BUTTON_A), "Windows stub returns false for button_pressed");
        ASSERT(!joystick_button_released(&state, BUTTON_A), "Windows stub returns false for button_released");
        ASSERT(!joystick_button_held(&state, BUTTON_A), "Windows stub returns false for button_held");

        /* Init returns -1 (not available) */
        int result = joystick_init(&state);
        ASSERT_EQ(result, -1, "Windows joystick_init returns -1 (unavailable)");
        ASSERT(!state.available, "Windows joystick not available");
    }
#endif

    /* ========================================================================
     * Cross-platform tests (should work on both Linux and Windows)
     * ======================================================================== */

    TEST("Axis normalization: NULL state returns 0") {
        double result = joystick_get_axis_normalized(NULL, AXIS_X);
        ASSERT_NEAR(result, 0.0, 0.01, "NULL state returns 0.0");
    }

    TEST("Button bounds checking: invalid button returns false") {
        JoystickState state;
        init_mock_state(&state);

        ASSERT(!joystick_button_pressed(&state, -1), "Negative button returns false");
        ASSERT(!joystick_button_pressed(&state, 16), "Button >= 16 returns false");
        ASSERT(!joystick_button_held(&state, 100), "Way out of range returns false");
    }

    TEST("Button NULL state: returns false") {
        ASSERT(!joystick_button_pressed(NULL, BUTTON_A), "NULL state returns false for pressed");
        ASSERT(!joystick_button_released(NULL, BUTTON_A), "NULL state returns false for released");
        ASSERT(!joystick_button_held(NULL, BUTTON_A), "NULL state returns false for held");
    }

    TEST("Mode transitions: NULL state is safe") {
        /* These should not crash */
        joystick_enter_nav_mode(NULL);
        joystick_enter_selection_mode(NULL);
        joystick_enter_edit_mode(NULL, 1);
        joystick_cycle_mode(NULL);

        ASSERT(1, "NULL state mode functions don't crash");
    }

    TEST_END();
}
