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
        case ACTION_TOGGLE_GRID:     return "TOGGLE_GRID";
        case ACTION_TOGGLE_SNAP:     return "TOGGLE_SNAP";
        case ACTION_FOCUS_BOX:       return "FOCUS_BOX";
        case ACTION_EXIT_FOCUS:      return "EXIT_FOCUS";
        case ACTION_START_CONNECTION:    return "START_CONNECTION";
        case ACTION_FINISH_CONNECTION:   return "FINISH_CONNECTION";
        case ACTION_CANCEL_CONNECTION:   return "CANCEL_CONNECTION";
        case ACTION_DELETE_CONNECTION:   return "DELETE_CONNECTION";
        case ACTION_SAVE_CANVAS:     return "SAVE_CANVAS";
        case ACTION_LOAD_CANVAS:     return "LOAD_CANVAS";
        case ACTION_EXPORT_CANVAS:   return "EXPORT_CANVAS";
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
        
        /* Create new box with templates (Issue #17) */
        /* n = Square, N (Shift+N) = Horizontal, Ctrl+N = Vertical */
        case 'n': {
            event->action = ACTION_CREATE_BOX;
            event->data.box.world_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
            event->data.box.world_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
            event->data.box.box_id = -1;
            event->data.box.template = BOX_TEMPLATE_SQUARE;
            return INPUT_SOURCE_KEYBOARD;
        }
        case 'N': {  /* Shift+N = Horizontal */
            event->action = ACTION_CREATE_BOX;
            event->data.box.world_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
            event->data.box.world_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
            event->data.box.box_id = -1;
            event->data.box.template = BOX_TEMPLATE_HORIZONTAL;
            return INPUT_SOURCE_KEYBOARD;
        }
        case 14: {  /* Ctrl+N = Vertical */
            event->action = ACTION_CREATE_BOX;
            event->data.box.world_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
            event->data.box.world_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
            event->data.box.box_id = -1;
            event->data.box.template = BOX_TEMPLATE_VERTICAL;
            return INPUT_SOURCE_KEYBOARD;
        }
        
        /* Delete selected box */
        case 'd':
        case 'D':
            event->action = ACTION_DELETE_BOX;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Cycle through display modes (Issue #33) */
        case '\t':
            event->action = ACTION_CYCLE_DISPLAY_MODE;
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
        
        /* Cycle box type (Issue #33) */
        case 't':
        case 'T':
            event->action = ACTION_CYCLE_BOX_TYPE;
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

        /* Toggle grid */
        case 'g':
        case 'G':
            event->action = ACTION_TOGGLE_GRID;
            return INPUT_SOURCE_KEYBOARD;

        /* Connection mode (Issue #20) */
        /* c = Start/finish connection from/to selected box */
        case 'c':
        case 'C':
            event->action = ACTION_START_CONNECTION;
            return INPUT_SOURCE_KEYBOARD;

        /* Focus box (Phase 5b) */
        case '\n':  /* Enter */
        case '\r':
        case ' ':   /* Space */
            event->action = ACTION_FOCUS_BOX;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Toggle help overlay (Issue #34) */
        case KEY_F(1):
            event->action = ACTION_TOGGLE_HELP;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Save canvas */
        case KEY_F(2):
            event->action = ACTION_SAVE_CANVAS;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Load canvas */
        case KEY_F(3):
            event->action = ACTION_LOAD_CANVAS;
            return INPUT_SOURCE_KEYBOARD;
        
        /* Export canvas (Ctrl+E) */
        case 5:  /* Ctrl+E */
            event->action = ACTION_EXPORT_CANVAS;
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

/* Process parameter editor input (Phase 2) */
static int input_unified_process_param_editor(JoystickState *js, Canvas *canvas, InputEvent *event) {
    if (!js || !canvas || !event) return -1;

    /* Clear event */
    memset(event, 0, sizeof(InputEvent));
    event->action = ACTION_NONE;

    /* Get selected box for live updates */
    Box *box = canvas_get_box(canvas, js->selected_box_id);
    if (!box) {
        /* Box was deleted, close editor */
        joystick_close_param_editor(js, false, NULL);
        return -1;
    }

    /* Get axis values for navigation and adjustment */
    double axis_y = joystick_get_axis_normalized(js, AXIS_Y);
    double axis_x = joystick_get_axis_normalized(js, AXIS_X);

    /* Vertical navigation (with deadzone to prevent accidental changes) */
    static int nav_cooldown = 0;
    if (nav_cooldown > 0) {
        nav_cooldown--;
    } else {
        if (axis_y < -0.7) {  /* Up */
            if (js->param_selected_field > 0) {
                js->param_selected_field--;
                nav_cooldown = 15;  /* Cooldown frames */
            }
        } else if (axis_y > 0.7) {  /* Down */
            if (js->param_selected_field < 2) {  /* 0=width, 1=height, 2=color */
                js->param_selected_field++;
                nav_cooldown = 15;
            }
        }
    }

    /* Horizontal adjustment (live update) */
    if (fabs(axis_x) > 0.0) {
        int delta = (axis_x > 0.5) ? 1 : (axis_x < -0.5) ? -1 : 0;

        switch (js->param_selected_field) {
            case 0:  /* Width */
                js->param_edit_width += delta;
                if (js->param_edit_width < 10) js->param_edit_width = 10;
                if (js->param_edit_width > 80) js->param_edit_width = 80;
                box->width = js->param_edit_width;  /* Live update */
                break;

            case 1:  /* Height */
                js->param_edit_height += delta;
                if (js->param_edit_height < 3) js->param_edit_height = 3;
                if (js->param_edit_height > 30) js->param_edit_height = 30;
                box->height = js->param_edit_height;  /* Live update */
                break;

            case 2:  /* Color */
                if (delta != 0) {
                    js->param_edit_color = (js->param_edit_color + delta + 8) % 8;
                    box->color = js->param_edit_color;  /* Live update */
                }
                break;
        }
    }

    /* Button LB - Decrease value (large step) */
    if (joystick_button_pressed(js, BUTTON_LB)) {
        switch (js->param_selected_field) {
            case 0:  /* Width */
                js->param_edit_width -= 5;
                if (js->param_edit_width < 10) js->param_edit_width = 10;
                box->width = js->param_edit_width;
                break;
            case 1:  /* Height */
                js->param_edit_height -= 3;
                if (js->param_edit_height < 3) js->param_edit_height = 3;
                box->height = js->param_edit_height;
                break;
            case 2:  /* Color */
                js->param_edit_color = (js->param_edit_color + 7) % 8;
                box->color = js->param_edit_color;
                break;
        }
    }

    /* Button RB - Increase value (large step) */
    if (joystick_button_pressed(js, BUTTON_RB)) {
        switch (js->param_selected_field) {
            case 0:  /* Width */
                js->param_edit_width += 5;
                if (js->param_edit_width > 80) js->param_edit_width = 80;
                box->width = js->param_edit_width;
                break;
            case 1:  /* Height */
                js->param_edit_height += 3;
                if (js->param_edit_height > 30) js->param_edit_height = 30;
                box->height = js->param_edit_height;
                break;
            case 2:  /* Color */
                js->param_edit_color = (js->param_edit_color + 1) % 8;
                box->color = js->param_edit_color;
                break;
        }
    }

    /* Button A - Apply and close */
    if (joystick_button_pressed(js, BUTTON_A)) {
        joystick_close_param_editor(js, true, box);
        return -1;  /* No canvas action */
    }

    /* Button B - Cancel and close */
    if (joystick_button_pressed(js, BUTTON_B)) {
        joystick_close_param_editor(js, false, box);
        return -1;  /* No canvas action */
    }

    return -1;  /* No canvas action while in parameter editor */
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
    
    /* Text editor active - handle Button B to save and close */
    if (js->text_editor_active) {
        if (joystick_button_pressed(js, BUTTON_B)) {
            Box *box = canvas_get_box(canvas, js->selected_box_id);
            joystick_close_text_editor(js, true, box);
        }
        return -1;  /* No canvas action while text editor active */
    }

    /* Parameter editor active - handle separately */
    if (js->param_editor_active) {
        return input_unified_process_param_editor(js, canvas, event);
    }

    /* Button LB (4) - Cycle through modes (global toggle per Issue #15) */
    /* Note: LB is also used as a modifier for templates (LB+X = Horizontal, Issue #17) */
    /* To avoid timing issues, we cycle mode on LB RELEASE, not press */
    if (joystick_button_pressed(js, BUTTON_LB)) {
        /* Reset modifier flag when LB is first pressed */
        js->lb_used_as_modifier = false;
    }
    if (joystick_button_released(js, BUTTON_LB)) {
        /* Only cycle mode if LB wasn't used as a modifier for templates */
        if (!js->lb_used_as_modifier) {
            joystick_cycle_mode(js);
        }
        js->lb_used_as_modifier = false;  /* Reset for next time */
        return -1;  /* No canvas action */
    }

    /* Button BACK (6) - Toggle visualizer (global) */
    if (joystick_button_pressed(js, BUTTON_BACK)) {
        js->show_visualizer = !js->show_visualizer;
        return -1;  /* No canvas action */
    }

    /* Button SELECT (8) - Quit (global) */
    if (joystick_button_pressed(js, BUTTON_SELECT)) {
        event->action = ACTION_QUIT;
        return INPUT_SOURCE_JOYSTICK;
    }

    /* Process based on current mode */
    switch (js->mode) {
        case MODE_NAV:
            /* NAV MODE: Navigate and observe only (pan/zoom) */

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

            /* Button X - Quick-create box at cursor (Issue #17: templates) */
            /* X = Square, LB+X = Horizontal, RB+X = Vertical */
            if (joystick_button_pressed(js, BUTTON_X)) {
                event->action = ACTION_CREATE_BOX;
                event->data.box.world_x = js->cursor_x;
                event->data.box.world_y = js->cursor_y;
                event->data.box.box_id = -1;

                /* Check for modifier buttons and mark them as used */
                if (joystick_button_held(js, BUTTON_LB)) {
                    event->data.box.template = BOX_TEMPLATE_HORIZONTAL;
                    js->lb_used_as_modifier = true;  /* Prevent mode cycle on LB release */
                } else if (joystick_button_held(js, BUTTON_RB)) {
                    event->data.box.template = BOX_TEMPLATE_VERTICAL;
                    js->rb_used_as_modifier = true;  /* Prevent snap toggle on RB release */
                } else {
                    event->data.box.template = BOX_TEMPLATE_SQUARE;
                }
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button Y - Toggle grid */
            if (joystick_button_pressed(js, BUTTON_Y)) {
                event->action = ACTION_TOGGLE_GRID;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button RB - Toggle snap (Phase 4) */
            /* Note: RB is also used as modifier for vertical template (RB+X, Issue #17) */
            /* To avoid timing issues, we toggle snap on RB RELEASE, not press */
            if (joystick_button_pressed(js, BUTTON_RB)) {
                /* Reset modifier flag when RB is first pressed */
                js->rb_used_as_modifier = false;
            }
            if (joystick_button_released(js, BUTTON_RB)) {
                /* Only toggle snap if RB wasn't used as a modifier for templates */
                if (!js->rb_used_as_modifier) {
                    event->action = ACTION_TOGGLE_SNAP;
                    js->rb_used_as_modifier = false;  /* Reset for next time */
                    return INPUT_SOURCE_JOYSTICK;
                }
                js->rb_used_as_modifier = false;  /* Reset for next time */
            }

            /* Button START - Save canvas */
            if (joystick_button_pressed(js, BUTTON_START)) {
                event->action = ACTION_SAVE_CANVAS;
                return INPUT_SOURCE_JOYSTICK;
            }
            break;

        case MODE_SELECTION:
            /* SELECTION MODE: Choose boxes (viewport locked - no panning) */
            /* Per Issue #16: Viewport is locked, only cycle through visible boxes */

            /* Note: Left stick does NOT pan in SELECTION mode (viewport lock) */
            /* Axis input is intentionally ignored for navigation */

            /* Button A - Cycle through visible boxes */
            if (joystick_button_pressed(js, BUTTON_A)) {
                event->action = ACTION_CYCLE_BOX;
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button B - Deselect and return to NAV */
            if (joystick_button_pressed(js, BUTTON_B)) {
                event->action = ACTION_DESELECT_BOX;
                joystick_enter_nav_mode(js);
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button X - Enter EDIT mode for selected box */
            if (joystick_button_pressed(js, BUTTON_X)) {
                if (js->selected_box_id >= 0) {
                    joystick_enter_edit_mode(js, js->selected_box_id);
                }
                return -1;  /* Mode change only */
            }

            /* Button Y - Delete selected box */
            if (joystick_button_pressed(js, BUTTON_Y)) {
                if (js->selected_box_id >= 0) {
                    event->action = ACTION_DELETE_BOX;
                    event->data.box.box_id = js->selected_box_id;
                    return INPUT_SOURCE_JOYSTICK;
                }
                return -1;
            }

            /* Button RB - Duplicate selected box (future) */
            if (joystick_button_pressed(js, BUTTON_RB)) {
                /* TODO: Duplicate box */
                return -1;
            }
            break;

        case MODE_EDIT:
            /* EDIT MODE: Modify selected box properties */

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

            /* Button A - Edit text (Phase 3) */
            if (joystick_button_pressed(js, BUTTON_A)) {
                Box *box = canvas_get_box(canvas, js->selected_box_id);
                if (box) {
                    joystick_open_text_editor(js, box);
                }
                return -1;  /* No canvas action */
            }

            /* Button B - Apply changes and return to SELECTION */
            if (joystick_button_pressed(js, BUTTON_B)) {
                joystick_enter_selection_mode(js);
                return -1;  /* Mode change only */
            }

            /* Button X - Cycle box color */
            if (joystick_button_pressed(js, BUTTON_X)) {
                event->action = ACTION_COLOR_BOX;
                event->data.color.color_index = -1; /* Cycle */
                return INPUT_SOURCE_JOYSTICK;
            }

            /* Button Y - Open parameter panel */
            if (joystick_button_pressed(js, BUTTON_Y)) {
                Box *box = canvas_get_box(canvas, js->selected_box_id);
                if (box) {
                    joystick_open_param_editor(js, box);
                }
                return -1;  /* No canvas action */
            }

            /* Button RB - Increase value (context-dependent) */
            if (joystick_button_pressed(js, BUTTON_RB)) {
                /* TODO: Increase current property */
                return -1;
            }
            break;
    }
    
    return -1;
}
