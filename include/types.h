#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

/* Initial capacity for dynamic box array */
#define INITIAL_BOX_CAPACITY 16

/* Color pair indices for boxes (avoid conflict with ncurses COLOR_* macros) */
#define BOX_COLOR_DEFAULT 0
#define BOX_COLOR_RED 1
#define BOX_COLOR_GREEN 2
#define BOX_COLOR_BLUE 3
#define BOX_COLOR_YELLOW 4
#define BOX_COLOR_MAGENTA 5
#define BOX_COLOR_CYAN 6
#define BOX_COLOR_WHITE 7

/* Grid color pair index */
#define GRID_COLOR_PAIR 8

/* Initial capacity for dynamic connection array */
#define INITIAL_CONNECTION_CAPACITY 8

/* Default connection color */
#define CONNECTION_COLOR_DEFAULT BOX_COLOR_CYAN

/* Box type enumeration (Issue #33) */
typedef enum {
    BOX_TYPE_NOTE = 0,      /* Default note box */
    BOX_TYPE_TASK,          /* Task/todo box */
    BOX_TYPE_CODE,          /* Code snippet box */
    BOX_TYPE_STICKY         /* Sticky note box */
} BoxType;

/* Display mode for boxes (Issue #33) */
typedef enum {
    DISPLAY_MODE_COMPACT = 0,   /* Icon + Title only */
    DISPLAY_MODE_PREVIEW,       /* Icon + Title + 1-2 lines of content */
    DISPLAY_MODE_FULL           /* Icon + Title + All content (current behavior) */
} DisplayMode;

/* Connection structure representing a visual link between two boxes */
typedef struct {
    int id;             /* Unique connection identifier */
    int source_id;      /* ID of source box */
    int dest_id;        /* ID of destination box */
    int color;          /* Color pair index (default: cyan) */
} Connection;

/* Box structure representing a rectangular region with content */
typedef struct {
    double x;           /* World X coordinate */
    double y;           /* World Y coordinate */
    int width;          /* Width in characters */
    int height;         /* Height in characters */
    char *title;        /* Box title */
    char **content;     /* Array of content lines */
    int content_lines;  /* Number of content lines */
    bool selected;      /* Is this box currently selected? */
    int id;             /* Unique box identifier */
    int color;          /* Color pair index (0 = default) */
    BoxType box_type;   /* Box type (note, task, code, sticky) - Issue #33 */
} Box;

/* Viewport structure for camera/view control */
typedef struct {
    double cam_x;       /* Camera X position in world space */
    double cam_y;       /* Camera Y position in world space */
    double zoom;        /* Zoom level (1.0 = normal, >1.0 = zoomed in) */
    int term_width;     /* Terminal width in characters */
    int term_height;    /* Terminal height in characters */
} Viewport;

/* Grid configuration (Phase 4) */
typedef struct {
    bool visible;           /* Is grid displayed? */
    bool snap_enabled;      /* Is snap-to-grid active? */
    int spacing;            /* Grid spacing in world units (5/10/20) */
} GridConfig;

/* Focus mode state (Phase 5b) */
typedef struct {
    bool active;            /* Is a box in focus mode? */
    int focused_box_id;     /* Which box is focused (-1 if none) */
    int scroll_offset;      /* Scroll position in focused box */
    int scroll_max;         /* Maximum scroll value */
} FocusState;

/* Connection mode state (Issue #20) */
typedef struct {
    bool active;            /* Is connection mode active? */
    int source_box_id;      /* Source box ID for pending connection (-1 if none) */
    bool pending_delete;    /* Is a delete confirmation pending? */
    int delete_conn_id;     /* Connection ID pending deletion (-1 if none) */
} ConnectionMode;

/* Canvas structure containing all boxes (dynamic array) */
typedef struct {
    Box *boxes;         /* Dynamic array of boxes */
    int box_count;      /* Number of boxes currently in use */
    int box_capacity;   /* Allocated capacity */
    double world_width;
    double world_height;
    int next_id;        /* Next unique ID to assign */
    int selected_index; /* Index of selected box, -1 if none */
    GridConfig grid;    /* Grid configuration (Phase 4) */
    FocusState focus;   /* Focus mode state (Phase 5b) */

    /* Connections (Issue #20) */
    Connection *connections;    /* Dynamic array of connections */
    int conn_count;             /* Number of connections currently in use */
    int conn_capacity;          /* Allocated capacity for connections */
    int next_conn_id;           /* Next unique connection ID to assign */
    ConnectionMode conn_mode;   /* Connection mode state */

    /* Display mode (Issue #33) */
    DisplayMode display_mode;   /* Global display mode for all boxes */
} Canvas;

#endif /* TYPES_H */
