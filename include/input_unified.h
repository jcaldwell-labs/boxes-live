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
    
    /* View actions */
    ACTION_RESET_VIEW,      /* Reset viewport to origin */

    /* Grid actions (Phase 4) */
    ACTION_TOGGLE_GRID,     /* Toggle grid visibility */
    ACTION_TOGGLE_SNAP,     /* Toggle snap-to-grid */

    /* Focus actions (Phase 5b) */
    ACTION_FOCUS_BOX,       /* Enter focus mode on selected box */
    ACTION_EXIT_FOCUS,      /* Exit focus mode */

    /* File operations */
    ACTION_SAVE_CANVAS,     /* Save canvas to file */
    ACTION_LOAD_CANVAS,     /* Load canvas from file */
    
    /* Mode transitions (joystick) */
    ACTION_ENTER_EDIT_MODE,      /* Enter box edit mode */
    ACTION_ENTER_PARAM_MODE,     /* Enter parameter adjustment mode */
    ACTION_ENTER_NAV_MODE,       /* Enter navigation mode */
    
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
