#include "input_unified.h"
#include "viewport.h"
#include "canvas.h"
#include <ncurses.h>
#include <string.h>
#include <math.h>

/* Pan speed in world units (for discrete keyboard input) */
#define PAN_SPEED 2.0

/* Internal state for mouse dragging */
static struct {
    bool dragging;
    int drag_box_id;
    double drag_offset_x;
    double drag_offset_y;
} mouse_state = {false, -1, 0.0, 0.0};

/* Initialize unified input system */
void input_unified_init(void) {
    mouse_state.dragging = false;
    mouse_state.drag_box_id = -1;
    mouse_state.drag_offset_x = 0.0;
    mouse_state.drag_offset_y = 0.0;
}

/* Get human-readable action name */
const char* input_unified_action_name(CanvasAction action) {
    switch (action) {
        case ACTION_NONE:            return "NONE";
        case ACTION_PAN_UP:          return "PAN_UP";
        case ACTION_PAN_DOWN:        return "PAN_DOWN";
        case ACTION_PAN_LEFT:        return "PAN_LEFT";
        case ACTION_PAN_RIGHT:       return "PAN_RIGHT";
        case ACTION_ZOOM_IN:         return "ZOOM_IN";
        case ACTION_ZOOM_OUT:        return "ZOOM_OUT";
        case ACTION_SELECT_BOX:      return "SELECT_BOX";
        case ACTION_DESELECT_BOX:    return "DESELECT_BOX";
        case ACTION_CYCLE_BOX:       return "CYCLE_BOX";
        case ACTION_CREATE_BOX:      return "CREATE_BOX";
        case ACTION_DELETE_BOX:      return "DELETE_BOX";
        case ACTION_MOVE_BOX:        return "MOVE_BOX";
        case ACTION_COLOR_BOX:       return "COLOR_BOX";
        case ACTION_RESET_VIEW:      return "RESET_VIEW";
        case ACTION_SAVE_CANVAS:     return "SAVE_CANVAS";
        case ACTION_LOAD_CANVAS:     return "LOAD_CANVAS";
        case ACTION_ENTER_EDIT_MODE: return "ENTER_EDIT_MODE";
        case ACTION_ENTER_PARAM_MODE: return "ENTER_PARAM_MODE";
        case ACTION_ENTER_NAV_MODE:  return "ENTER_NAV_MODE";
        case ACTION_QUIT:            return "QUIT";
        default:                     return "UNKNOWN";
    }
}

/* Process keyboard input */
int input_unified_process_keyboard(int ch, const Viewport *vp, InputEvent *event) {
    if (!event || !vp) return -1;
    
    /* Clear event */
    memset(event, 0, sizeof(InputEvent));
    event->action = ACTION_NONE;
    
    switch (ch) {
        /* Quit */
        case 'q':
        case 'Q':
        case 27: /* ESC */
            event->action = ACTION_QUIT;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Create new box */
        case 'n':
        case 'N': {
            event->action = ACTION_CREATE_BOX;
            /* Calculate center of viewport in world coordinates */
            event->data.box.world_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
            event->data.box.world_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
            event->data.box.box_id = -1;
            return INPUT_SOURCE_KEYBOARD;
        }
        
        /* Delete selected box */
        case 'd':
        case 'D':
            event->action = ACTION_DELETE_BOX;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Cycle through boxes */
        case '\t':
            event->action = ACTION_CYCLE_BOX;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Color selection (1-7) */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            event->action = ACTION_COLOR_BOX;
            event->data.color.color_index = ch - '0';
            return INPUT_SOURCE_KEYBOARD;
        
        /* Reset color or view */
        case '0':
            event->action = ACTION_COLOR_BOX;
            event->data.color.color_index = 0;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Pan up */
        case KEY_UP:
        case 'w':
        case 'W':
            event->action = ACTION_PAN_UP;
            event->data.pan.dx = 0.0;
            event->data.pan.dy = -PAN_SPEED;
            event->data.pan.continuous = false;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Pan down */
        case KEY_DOWN:
        case 's':
        case 'S':
            event->action = ACTION_PAN_DOWN;
            event->data.pan.dx = 0.0;
            event->data.pan.dy = PAN_SPEED;
            event->data.pan.continuous = false;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Pan left */
        case KEY_LEFT:
        case 'a':
        case 'A':
        case 'h':
        case 'H':
            event->action = ACTION_PAN_LEFT;
            event->data.pan.dx = -PAN_SPEED;
            event->data.pan.dy = 0.0;
            event->data.pan.continuous = false;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Pan right */
        case KEY_RIGHT:
        case 'l':
        case 'L':
            event->action = ACTION_PAN_RIGHT;
            event->data.pan.dx = PAN_SPEED;
            event->data.pan.dy = 0.0;
            event->data.pan.continuous = false;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Zoom in */
        case '+':
        case '=':
        case 'z':
        case 'Z':
            event->action = ACTION_ZOOM_IN;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Zoom out */
        case '-':
        case '_':
        case 'x':
        case 'X':
            event->action = ACTION_ZOOM_OUT;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Reset view */
        case 'r':
        case 'R':
            event->action = ACTION_RESET_VIEW;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Save canvas */
        case KEY_F(2):
            event->action = ACTION_SAVE_CANVAS;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Load canvas */
        case KEY_F(3):
            event->action = ACTION_LOAD_CANVAS;
            return INPUT_SOURCE_KEYBOARD;
        
        default:
            event->action = ACTION_NONE;
            return -1;
    }
}

/* Process mouse input */
int input_unified_process_mouse(void *mouse_event_ptr, Canvas *canvas, const Viewport *vp, InputEvent *event) {
    if (!mouse_event_ptr || !canvas || !vp || !event) return -1;
    
    MEVENT *mevent = (MEVENT *)mouse_event_ptr;
    
    /* Convert screen to world coordinates */
    double wx = screen_to_world_x(vp, mevent->x);
    double wy = screen_to_world_y(vp, mevent->y);
    
    /* Clear event */
    memset(event, 0, sizeof(InputEvent));
    event->action = ACTION_NONE;
    
    /* Mouse button pressed - start drag */
    if (mevent->bstate & BUTTON1_PRESSED) {
        int box_id = canvas_find_box_at(canvas, wx, wy);
        if (box_id >= 0) {
            Box *box = canvas_get_box(canvas, box_id);
            if (box) {
                mouse_state.dragging = true;
                mouse_state.drag_box_id = box_id;
                mouse_state.drag_offset_x = wx - box->x;
                mouse_state.drag_offset_y = wy - box->y;
                
                event->action = ACTION_SELECT_BOX;
                event->data.box.box_id = box_id;
                event->data.box.world_x = wx;
                event->data.box.world_y = wy;
                return INPUT_SOURCE_MOUSE;
            }
        } else {
            /* Clicked on empty space - deselect */
            event->action = ACTION_DESELECT_BOX;
            return INPUT_SOURCE_MOUSE;
        }
    }
    /* Mouse drag - move box */
    else if (mouse_state.dragging && (mevent->bstate & REPORT_MOUSE_POSITION)) {
        event->action = ACTION_MOVE_BOX;
        event->data.move.box_id = mouse_state.drag_box_id;
        event->data.move.world_x = wx;
        event->data.move.world_y = wy;
        event->data.move.offset_x = mouse_state.drag_offset_x;
        event->data.move.offset_y = mouse_state.drag_offset_y;
        return INPUT_SOURCE_MOUSE;
    }
    /* Mouse button released - end drag */
    else if (mevent->bstate & BUTTON1_RELEASED) {
        mouse_state.dragging = false;
        mouse_state.drag_box_id = -1;
        /* No action needed - just state cleanup */
        return -1;
    }
    /* Single click (no drag) */
    else if (mevent->bstate & BUTTON1_CLICKED) {
        int box_id = canvas_find_box_at(canvas, wx, wy);
        if (box_id >= 0) {
            event->action = ACTION_SELECT_BOX;
            event->data.box.box_id = box_id;
            event->data.box.world_x = wx;
            event->data.box.world_y = wy;
            return INPUT_SOURCE_MOUSE;
        } else {
            event->action = ACTION_DESELECT_BOX;
            return INPUT_SOURCE_MOUSE;
        }
    }
    /* Mouse wheel scroll up - zoom in */
    else if (mevent->bstate & BUTTON4_PRESSED) {
        event->action = ACTION_ZOOM_IN;
        return INPUT_SOURCE_MOUSE;
    }
    /* Mouse wheel scroll down - zoom out */
    else if (mevent->bstate & BUTTON5_PRESSED) {
        event->action = ACTION_ZOOM_OUT;
        return INPUT_SOURCE_MOUSE;
    }
    
    return -1;
}

/* Process joystick input */
int input_unified_process_joystick(JoystickState *js, Canvas *canvas, const Viewport *vp, InputEvent *event) {
    if (!js || !canvas || !vp || !event || !js->available) return -1;

    /* Clear event */
    memset(event, 0, sizeof(InputEvent));
    event->action = ACTION_NONE;

    /* Get normalized axis values (but ignore during settling period) */
    double axis_x = 0.0;
    double axis_y = 0.0;

    if (js->settling_frames == 0) {
        axis_x = joystick_get_axis_normalized(js, AXIS_X);
        axis_y = joystick_get_axis_normalized(js, AXIS_Y);
    }
    
    /* Process based on current mode */
    switch (js->mode) {
        case MODE_NAVIGATION:
            /* Analog panning with left stick */
            if (fabs(axis_x) > 0.0 || fabs(axis_y) > 0.0) {
                event->action = ACTION_PAN_UP; /* Generic pan action */
                event->data.pan.dx = axis_x;
                event->data.pan.dy = axis_y;
                event->data.pan.continuous = true;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button A - Zoom in */
            if (joystick_button_pressed(js, BUTTON_A)) {
                event->action = ACTION_ZOOM_IN;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button B - Zoom out */
            if (joystick_button_pressed(js, BUTTON_B)) {
                event->action = ACTION_ZOOM_OUT;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button X - Cycle to next box */
            if (joystick_button_pressed(js, BUTTON_X)) {
                event->action = ACTION_CYCLE_BOX;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button Y - Create new box */
            if (joystick_button_pressed(js, BUTTON_Y)) {
                event->action = ACTION_CREATE_BOX;
                /* Use cursor position or viewport center */
                event->data.box.world_x = js->cursor_x;
                event->data.box.world_y = js->cursor_y;
                event->data.box.box_id = -1;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button LB (4) - Reset View */
            if (joystick_button_pressed(js, BUTTON_LB)) {
                event->action = ACTION_RESET_VIEW;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button RB (5) - Deselect Box */
            if (joystick_button_pressed(js, BUTTON_RB)) {
                event->action = ACTION_DESELECT_BOX;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Start button - Save */
            if (joystick_button_pressed(js, BUTTON_START)) {
                event->action = ACTION_SAVE_CANVAS;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Select button - Quit */
            if (joystick_button_pressed(js, BUTTON_SELECT)) {
                event->action = ACTION_QUIT;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button BACK (6) - Toggle visualizer */
            if (joystick_button_pressed(js, BUTTON_BACK)) {
                js->show_visualizer = !js->show_visualizer;
                return -1;  /* No action needed */
            }
            break;
        
        case MODE_EDIT:
            /* Analog box movement with left stick */
            if (fabs(axis_x) > 0.0 || fabs(axis_y) > 0.0) {
                event->action = ACTION_MOVE_BOX;
                event->data.move.box_id = js->selected_box_id;
                event->data.move.world_x = axis_x;
                event->data.move.world_y = axis_y;
                event->data.move.offset_x = 0.0;
                event->data.move.offset_y = 0.0;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button A - Enter parameter mode */
            if (joystick_button_pressed(js, BUTTON_A)) {
                event->action = ACTION_ENTER_PARAM_MODE;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button B - Return to navigation */
            if (joystick_button_pressed(js, BUTTON_B)) {
                event->action = ACTION_ENTER_NAV_MODE;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button X - Cycle color */
            if (joystick_button_pressed(js, BUTTON_X)) {
                /* Determine next color (handled in input handler) */
                event->action = ACTION_COLOR_BOX;
                event->data.color.color_index = -1; /* Cycle */
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button LB+Y - Delete box (requires holding LB to prevent accidents) */
            if (joystick_button_pressed(js, BUTTON_Y) && joystick_button_held(js, BUTTON_LB)) {
                event->action = ACTION_DELETE_BOX;
                event->data.box.box_id = js->selected_box_id;
                return INPUT_SOURCE_JOYSTICK;
            }
            break;
        
        case MODE_PARAMETER:
            /* Parameter mode - not fully implemented in unified layer yet */
            /* This would require more complex state management */
            /* For now, just handle basic mode transitions */
            
            /* Button A - Confirm (return to edit) */
            if (joystick_button_pressed(js, BUTTON_A)) {
                event->action = ACTION_ENTER_EDIT_MODE;
                return INPUT_SOURCE_JOYSTICK;
            }
            
            /* Button B - Cancel (return to navigation) */
            if (joystick_button_pressed(js, BUTTON_B)) {
                event->action = ACTION_ENTER_NAV_MODE;
                return INPUT_SOURCE_JOYSTICK;
            }
            break;
    }
    
    return -1;
}
