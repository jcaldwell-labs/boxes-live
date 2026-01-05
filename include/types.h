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

/* Box type enumeration for visual styling (Issue #33) */
typedef enum {
    BOX_TYPE_NOTE = 0,      /* Default note box */
    BOX_TYPE_TASK,          /* Task/todo box */
    BOX_TYPE_CODE,          /* Code snippet box */
    BOX_TYPE_STICKY,        /* Sticky note box */
    BOX_TYPE_COUNT          /* Total number of box types */
} BoxType;

/* Box content type enumeration for content source (Issue #54) */
typedef enum {
    BOX_CONTENT_TEXT = 0,   /* Static text content (default, current behavior) */
    BOX_CONTENT_FILE,       /* Content loaded from file */
    BOX_CONTENT_COMMAND     /* Content from command output */
} BoxContentType;

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
    BoxType box_type;   /* Box visual type (note, task, code, sticky) - Issue #33 */

    /* Content source fields (Issue #54) */
    BoxContentType content_type;  /* Content source type (text, file, command) */
    char *file_path;              /* File path for BOX_CONTENT_FILE (NULL otherwise) */
    char *command;                /* Command string for BOX_CONTENT_COMMAND (NULL otherwise) */
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
    int spacing;            /* Minor grid spacing in world units (default: 10) */
    int major_spacing;      /* Major grid spacing in world units (default: 50) (Issue #49) */
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

/* Sidebar state (Issue #35) */
typedef enum {
    SIDEBAR_HIDDEN = 0,     /* Sidebar completely hidden */
    SIDEBAR_COLLAPSED = 1,  /* Thin strip with toggle hint */
    SIDEBAR_EXPANDED = 2    /* Full panel with document */
} SidebarState;

/* Help overlay state (Issue #34) */
typedef struct {
    bool visible;           /* Is help overlay currently displayed? */
} HelpOverlay;

/* Command line state (Issue #55) */
#define COMMAND_BUFFER_SIZE 256
typedef struct {
    bool active;                            /* Is command line active? */
    char buffer[COMMAND_BUFFER_SIZE];       /* Command input buffer */
    int cursor_pos;                         /* Cursor position in buffer */
    int length;                             /* Current length of input */
    char error_msg[COMMAND_BUFFER_SIZE];    /* Last error message (if any) */
    bool has_error;                         /* Is there an error to display? */
} CommandLine;

/* ============================================================
 * Text Editing Mode (Issue #79)
 * ============================================================ */

/* Maximum title length for editing */
#define MAX_TITLE_LENGTH 256

/* Edit mode targets */
typedef enum {
    EDIT_NONE = 0,      /* Not in edit mode */
    EDIT_TITLE,         /* Editing box title */
    EDIT_BODY           /* Editing box body (Phase 2) */
} EditTarget;

/* Text editor state for in-place editing */
typedef struct {
    bool active;                        /* Is edit mode active? */
    EditTarget target;                  /* What are we editing? */
    int box_id;                         /* Which box is being edited */
    char buffer[MAX_TITLE_LENGTH];      /* Edit buffer */
    int cursor_pos;                     /* Cursor position in buffer */
    int length;                         /* Current length of buffer */
    char *original;                     /* Original value (for cancel) */
} TextEditor;

/* ============================================================
 * Undo/Redo System (Issue #81)
 * ============================================================ */

/* Default maximum undo stack size */
#define UNDO_STACK_MAX_SIZE 50

/* Operation types that can be undone */
typedef enum {
    OP_BOX_CREATE,          /* Box was created */
    OP_BOX_DELETE,          /* Box was deleted */
    OP_BOX_MOVE,            /* Box was moved */
    OP_BOX_RESIZE,          /* Box was resized */
    OP_BOX_CONTENT,         /* Box content was changed */
    OP_BOX_TITLE,           /* Box title was changed */
    OP_BOX_COLOR,           /* Box color was changed */
    OP_CONNECTION_CREATE,   /* Connection was created */
    OP_CONNECTION_DELETE    /* Connection was deleted */
} OpType;

/* Stored box state for undo/redo */
typedef struct {
    int id;
    double x, y;
    int width, height;
    char *title;
    char **content;
    int content_lines;
    int color;
    BoxType box_type;
    BoxContentType content_type;
    char *file_path;
    char *command;
} BoxSnapshot;

/* Stored connection state for undo/redo */
typedef struct {
    int id;
    int source_id;
    int dest_id;
    int color;
} ConnectionSnapshot;

/* A single undoable operation */
typedef struct Operation {
    OpType type;
    int box_id;                     /* Box affected (for box operations) */
    int conn_id;                    /* Connection affected (for connection operations) */

    /* State before the operation (for undo) */
    union {
        BoxSnapshot box_before;
        ConnectionSnapshot conn_before;
    } before;

    /* State after the operation (for redo) */
    union {
        BoxSnapshot box_after;
        ConnectionSnapshot conn_after;
    } after;

    struct Operation *prev;         /* Previous operation (for undo) */
    struct Operation *next;         /* Next operation (for redo) */
} Operation;

/* Undo stack managing all operations */
typedef struct {
    Operation *current;             /* Most recent operation (head of undo chain) */
    Operation *redo_head;           /* Head of redo chain (after undo) */
    int size;                       /* Current number of operations in undo chain */
    int max_size;                   /* Maximum operations to keep (default: 50) */
} UndoStack;

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

    /* Sidebar document (Issue #35) */
    char *document;             /* Free-form document text (can contain newlines) */
    SidebarState sidebar_state; /* Current sidebar state */
    int sidebar_width;          /* Sidebar width in columns (20-40) */

    /* Display mode (Issue #33) */
    DisplayMode display_mode;   /* Global display mode for all boxes */

    /* Help overlay (Issue #34) */
    HelpOverlay help;           /* Help overlay state */

    /* Command line (Issue #55) */
    CommandLine command_line;   /* Command line input state */

    /* Undo/Redo (Issue #81) */
    UndoStack undo_stack;       /* Undo/redo operation stack */

    /* Text Editing (Issue #79) */
    TextEditor editor;          /* In-place text editor state */

    /* Canvas metadata */
    char *filename;             /* Loaded filename (NULL if new/unsaved) */
} Canvas;

#endif /* TYPES_H */
