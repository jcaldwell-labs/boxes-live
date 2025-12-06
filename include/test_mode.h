#ifndef TEST_MODE_H
#define TEST_MODE_H

#include <stdbool.h>
#include <time.h>

/* Maximum event log entries */
#define TEST_MODE_MAX_EVENTS 50
#define TEST_MODE_EVENT_LEN 128

/* Maximum markers */
#define TEST_MODE_MAX_MARKERS 50

/* Grid style options */
typedef enum {
    GRID_STYLE_NONE = 0,      /* No grid */
    GRID_STYLE_AXES,          /* X/Y axes with subtle dots (recommended) */
    GRID_STYLE_DOTS,          /* Dots at intersections */
    GRID_STYLE_LINES,         /* Full lines */
    GRID_STYLE_DASHED,        /* Dashed lines */
    GRID_STYLE_CROSSHAIRS,    /* Light crosshairs */
    GRID_STYLE_COUNT          /* Number of styles */
} GridStyle;

/* Marker for visual debugging */
typedef struct {
    float x;
    float y;
    int number;
    time_t timestamp;
} TestMarker;

/* Event log entry */
typedef struct {
    char message[TEST_MODE_EVENT_LEN];
    time_t timestamp;
    int ms;  /* Milliseconds */
} TestEvent;

/* Test mode state */
typedef struct {
    bool enabled;              /* Test mode active */
    bool debug_overlay;        /* Show debug overlay */
    bool event_logging;        /* Log events to file */
    bool event_overlay;        /* Show events in overlay */
    GridStyle grid_style;      /* Current grid style */
    char mode_variant;         /* 'A', 'B', 'C' for experiments */

    /* Event log (circular buffer) */
    TestEvent events[TEST_MODE_MAX_EVENTS];
    int event_head;            /* Next write position */
    int event_count;           /* Total events (capped at max) */

    /* Markers */
    TestMarker markers[TEST_MODE_MAX_MARKERS];
    int marker_count;
    int next_marker_number;

    /* Stats */
    int frame_count;
    double fps;
    double fps_update_time;
    int fps_frame_count;
    time_t start_time;

    /* Event log file */
    FILE *log_file;
} TestMode;

/**
 * Initialize test mode state.
 * Call once at startup.
 *
 * @param tm Test mode state to initialize
 */
void test_mode_init(TestMode *tm);

/**
 * Cleanup test mode resources.
 * Closes log file if open.
 *
 * @param tm Test mode state
 */
void test_mode_cleanup(TestMode *tm);

/**
 * Enable test mode with optional variant.
 *
 * @param tm Test mode state
 * @param variant Mode variant ('A', 'B', 'C') or 0 for default
 */
void test_mode_enable(TestMode *tm, char variant);

/**
 * Toggle debug overlay visibility.
 *
 * @param tm Test mode state
 */
void test_mode_toggle_overlay(TestMode *tm);

/**
 * Toggle event logging to file.
 *
 * @param tm Test mode state
 * @param filename Log file path, or NULL for default "events.log"
 * @return 0 on success, -1 on error
 */
int test_mode_toggle_event_logging(TestMode *tm, const char *filename);

/**
 * Cycle to next grid style.
 *
 * @param tm Test mode state
 */
void test_mode_cycle_grid_style(TestMode *tm);

/**
 * Log an event with coordinates.
 *
 * @param tm Test mode state
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void test_mode_log_event(TestMode *tm, const char *format, ...);

/**
 * Log a key event.
 *
 * @param tm Test mode state
 * @param key_code Key code from ncurses
 * @param key_name Human-readable key name
 * @param world_x World X coordinate (or -1 if N/A)
 * @param world_y World Y coordinate (or -1 if N/A)
 * @param mode Current input mode name
 */
void test_mode_log_key(TestMode *tm, int key_code, const char *key_name,
                       float world_x, float world_y, const char *mode);

/**
 * Log a joystick event.
 *
 * @param tm Test mode state
 * @param event_type Event type (button, axis, etc.)
 * @param value Event value
 * @param context Additional context string
 */
void test_mode_log_joystick(TestMode *tm, const char *event_type,
                            int value, const char *context);

/**
 * Add a marker at the specified position.
 *
 * @param tm Test mode state
 * @param x World X coordinate
 * @param y World Y coordinate
 * @return Marker number, or -1 if markers full
 */
int test_mode_add_marker(TestMode *tm, float x, float y);

/**
 * Clear all markers.
 *
 * @param tm Test mode state
 */
void test_mode_clear_markers(TestMode *tm);

/**
 * Update FPS calculation.
 * Call once per frame.
 *
 * @param tm Test mode state
 */
void test_mode_update_fps(TestMode *tm);

/**
 * Get grid style name for display.
 *
 * @param style Grid style enum
 * @return Human-readable name
 */
const char *test_mode_grid_style_name(GridStyle style);

/**
 * Render debug overlay.
 * Should be called after main render, before refresh.
 *
 * @param tm Test mode state
 * @param cam_x Camera X position
 * @param cam_y Camera Y position
 * @param zoom Current zoom level
 * @param cursor_x Cursor world X
 * @param cursor_y Cursor world Y
 * @param mode_name Current mode name
 * @param box_count Number of boxes
 * @param conn_count Number of connections
 */
void test_mode_render_overlay(TestMode *tm, float cam_x, float cam_y,
                              float zoom, float cursor_x, float cursor_y,
                              const char *mode_name, int box_count,
                              int conn_count);

/**
 * Render markers on canvas.
 * Call during main render pass.
 *
 * @param tm Test mode state
 * @param cam_x Camera X position
 * @param cam_y Camera Y position
 * @param zoom Current zoom level
 */
void test_mode_render_markers(TestMode *tm, float cam_x, float cam_y,
                              float zoom);

/**
 * Render grid with current style.
 * Replaces default grid rendering when test mode enabled.
 *
 * @param tm Test mode state
 * @param cam_x Camera X position
 * @param cam_y Camera Y position
 * @param zoom Current zoom level
 * @param spacing Grid spacing
 * @param screen_width Terminal width
 * @param screen_height Terminal height
 */
void test_mode_render_grid(TestMode *tm, float cam_x, float cam_y,
                           float zoom, int spacing, int screen_width,
                           int screen_height);

/**
 * Get global test mode pointer (set during main initialization).
 * Returns NULL if test mode not initialized.
 *
 * @return Pointer to global TestMode state, or NULL
 */
TestMode *test_mode_get_global(void);

/**
 * Set global test mode pointer (call from main).
 *
 * @param tm Test mode state to set as global
 */
void test_mode_set_global(TestMode *tm);

/**
 * Handle test mode specific key input.
 * Should be called before normal input handling.
 *
 * @param ch Input character
 * @param cursor_x Cursor world X
 * @param cursor_y Cursor world Y
 * @return 1 if key was handled (consume), 0 if key should be passed through
 */
int test_mode_handle_key(int ch, float cursor_x, float cursor_y);

#endif /* TEST_MODE_H */
