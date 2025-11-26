#ifndef INPUT_UNIFIED_H
#define INPUT_UNIFIED_H

#include "types.h"
#include "joystick.h"
#include "config.h"

/*
 * Unified Input Layer for Boxes-Live
 * 
 * This module abstracts all user input (keyboard, mouse, joystick) into
 * standardized canvas actions, ensuring 1:1 intent mapping across all
 * input methods for a polished, fast user experience.
 */

/* Canvas action types - standardized user intents */
typedef enum {
    ACTION_NONE = 0,
    
    /* Navigation actions */
    ACTION_PAN_UP,
    ACTION_PAN_DOWN,
    ACTION_PAN_LEFT,
    ACTION_PAN_RIGHT,
    
    /* Zoom actions */
    ACTION_ZOOM_IN,
    ACTION_ZOOM_OUT,
    
    /* Box management actions */
    ACTION_SELECT_BOX,      /* Select/focus a box */
    ACTION_DESELECT_BOX,    /* Deselect current box */
    ACTION_CYCLE_BOX,       /* Cycle to next box */
    ACTION_CREATE_BOX,      /* Create new box */
    ACTION_DELETE_BOX,      /* Delete selected box */
    ACTION_MOVE_BOX,        /* Move/drag box */
    
    /* Box property actions */
    ACTION_COLOR_BOX,       /* Change box color */
    ACTION_CYCLE_BOX_TYPE,  /* Cycle box type (NOTE/TASK/CODE/STICKY) - Issue #33 */
    
    /* View actions */
    ACTION_RESET_VIEW,      /* Reset viewport to origin */

    /* Grid actions (Phase 4) */
    ACTION_TOGGLE_GRID,     /* Toggle grid visibility */
    ACTION_TOGGLE_SNAP,     /* Toggle snap-to-grid */

    /* Display mode actions (Issue #33) */
    ACTION_CYCLE_DISPLAY_MODE,  /* Cycle through display modes (Compact/Preview/Full) */

    /* Focus actions (Phase 5b) */
    ACTION_FOCUS_BOX,       /* Enter focus mode on selected box */
    ACTION_EXIT_FOCUS,      /* Exit focus mode */

    /* Connection actions (Issue #20) */
    ACTION_START_CONNECTION,    /* Start connection mode from selected box */
    ACTION_FINISH_CONNECTION,   /* Complete connection to destination box */
    ACTION_CANCEL_CONNECTION,   /* Cancel connection mode */
    ACTION_DELETE_CONNECTION,   /* Delete connection (D key while selected) */

    /* Sidebar actions (Issue #35) */
    ACTION_TOGGLE_SIDEBAR,      /* Toggle sidebar state (D key) */
    ACTION_WIDEN_SIDEBAR,       /* Increase sidebar width (] key) */
    ACTION_NARROW_SIDEBAR,      /* Decrease sidebar width ([ key) */
    ACTION_EDIT_SIDEBAR,        /* Edit sidebar document (E key) */

    /* File operations */
    ACTION_SAVE_CANVAS,     /* Save canvas to file */
    ACTION_LOAD_CANVAS,     /* Load canvas from file */
    ACTION_EXPORT_CANVAS,   /* Export viewport to ASCII art file */
    
    /* Mode transitions (joystick) */
    ACTION_ENTER_EDIT_MODE,      /* Enter box edit mode */
    ACTION_ENTER_PARAM_MODE,     /* Enter parameter adjustment mode */
    ACTION_ENTER_NAV_MODE,       /* Enter navigation mode */
    
    /* Help overlay (Issue #34) */
    ACTION_TOGGLE_HELP,     /* Toggle help overlay visibility */
    
    /* Application control */
    ACTION_QUIT,            /* Quit application */
    
    ACTION_MAX
} CanvasAction;

/* Input event data structure */
typedef struct {
    CanvasAction action;
    
    /* Action-specific data */
    union {
        /* For panning (continuous analog or discrete) */
        struct {
            double dx;      /* Delta X (normalized) */
            double dy;      /* Delta Y (normalized) */
            bool continuous; /* True for analog input */
        } pan;
        
        /* For box selection/creation */
        struct {
            double world_x;     /* World coordinates */
            double world_y;
            int box_id;         /* Target box ID (if known) */
            BoxTemplate template; /* Template type for creation (Issue #17) */
        } box;
        
        /* For box movement */
        struct {
            double world_x;
            double world_y;
            double offset_x; /* Drag offset */
            double offset_y;
            int box_id;
        } move;
        
        /* For color changes */
        struct {
            int color_index; /* 0-7 color index */
        } color;
    } data;
} InputEvent;

/* Input source type for debugging/logging */
typedef enum {
    INPUT_SOURCE_KEYBOARD,
    INPUT_SOURCE_MOUSE,
    INPUT_SOURCE_JOYSTICK
} InputSource;

/* Keyboard control codes */
#define CTRL_D 4

/*
 * Process keyboard input and translate to canvas actions
 * 
 * @param ch - ncurses character code
 * @param vp - viewport for coordinate translation
 * @param event - output event structure
 * @return InputSource or -1 if no action generated
 */
int input_unified_process_keyboard(int ch, const Viewport *vp, InputEvent *event);

/*
 * Process mouse input and translate to canvas actions
 * 
 * @param mouse_event - ncurses mouse event
 * @param canvas - canvas for box lookup
 * @param vp - viewport for coordinate translation
 * @param event - output event structure
 * @return InputSource or -1 if no action generated
 */
int input_unified_process_mouse(void *mouse_event, Canvas *canvas, const Viewport *vp, InputEvent *event);

/*
 * Process joystick input and translate to canvas actions
 * 
 * @param js - joystick state
 * @param canvas - canvas for box lookup
 * @param vp - viewport for coordinate translation
 * @param event - output event structure
 * @return InputSource or -1 if no action generated
 */
int input_unified_process_joystick(JoystickState *js, Canvas *canvas, const Viewport *vp, InputEvent *event);

/*
 * Initialize the unified input system
 * Must be called before processing input
 */
void input_unified_init(void);

/*
 * Get a human-readable name for a canvas action (for debugging)
 */
const char* input_unified_action_name(CanvasAction action);

#endif /* INPUT_UNIFIED_H */
