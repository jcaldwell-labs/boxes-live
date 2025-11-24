#include <ncurses.h>
#include <math.h>
#include "input.h"
#include "input_unified.h"
#include "viewport.h"
#include "canvas.h"
#include "persistence.h"
#include "joystick.h"

/* Zoom factor per key press */
#define ZOOM_FACTOR 1.2

/* Default save file */
#define DEFAULT_SAVE_FILE "canvas.txt"

/* Pan speed for joystick input (scaled by zoom) */
#define PAN_SPEED 2.0

/* Forward declarations for joystick mode handlers (legacy, kept for parameter mode) */
static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y);

/* Helper function to execute canvas actions */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event);

int handle_input(Canvas *canvas, Viewport *vp) {
    int ch = getch();

    if (ch == ERR) {
        /* No input available */
        return 0;
    }

    InputEvent event;
    int source = -1;

    /* Handle mouse events */
    if (ch == KEY_MOUSE) {
        MEVENT mouse_event;
        if (getmouse(&mouse_event) == OK) {
            source = input_unified_process_mouse(&mouse_event, canvas, vp, &event);
        }
    } else {
        /* Handle keyboard input */
        source = input_unified_process_keyboard(ch, vp, &event);
    }

    /* Execute the action if one was generated */
    if (source >= 0 && event.action != ACTION_NONE) {
        return execute_canvas_action(canvas, vp, NULL, &event);
    }

    return 0;
}

/* Handle joystick input based on current mode */
int handle_joystick_input(Canvas *canvas, Viewport *vp, JoystickState *js) {
    if (!canvas || !vp || !js || !js->available) {
        return 0;
    }

    /* Special handling for parameter mode (not yet in unified layer) */
    if (js->mode == MODE_PARAMETER) {
        double axis_x = joystick_get_axis_normalized(js, AXIS_X);
        double axis_y = joystick_get_axis_normalized(js, AXIS_Y);
        return handle_joystick_parameter(canvas, vp, js, axis_x, axis_y);
    }

    /* Use unified input layer for navigation and edit modes */
    InputEvent event;
    int source = input_unified_process_joystick(js, canvas, vp, &event);

    if (source >= 0 && event.action != ACTION_NONE) {
        return execute_canvas_action(canvas, vp, js, &event);
    }

    return 0;
}

/* Execute canvas action from unified input event */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event) {
    if (!canvas || !vp || !event) return 0;

    switch (event->action) {
        case ACTION_QUIT:
            return 1;

        case ACTION_PAN_UP:
        case ACTION_PAN_DOWN:
        case ACTION_PAN_LEFT:
        case ACTION_PAN_RIGHT:
            /* Handle panning */
            if (event->data.pan.continuous) {
                /* Continuous analog input (joystick) - scale by zoom */
                double scaled_speed = PAN_SPEED / vp->zoom;
                viewport_pan(vp, event->data.pan.dx * scaled_speed,
                            event->data.pan.dy * scaled_speed);
                
                /* Update joystick cursor if in navigation mode */
                if (js && js->mode == MODE_NAVIGATION) {
                    js->cursor_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
                    js->cursor_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
                }
            } else {
                /* Discrete input (keyboard) */
                viewport_pan(vp, event->data.pan.dx, event->data.pan.dy);
            }
            break;

        case ACTION_ZOOM_IN:
            viewport_zoom(vp, ZOOM_FACTOR);
            break;

        case ACTION_ZOOM_OUT:
            viewport_zoom(vp, 1.0 / ZOOM_FACTOR);
            break;

        case ACTION_SELECT_BOX:
            canvas_select_box(canvas, event->data.box.box_id);
            break;

        case ACTION_DESELECT_BOX:
            canvas_deselect(canvas);
            break;

        case ACTION_CYCLE_BOX:
            if (canvas->box_count > 0) {
                int next_index = (canvas->selected_index + 1) % canvas->box_count;
                Box *next_box = canvas_get_box_at(canvas, next_index);
                if (next_box) {
                    canvas_select_box(canvas, next_box->id);
                    
                    /* If joystick, enter edit mode */
                    if (js) {
                        joystick_enter_edit_mode(js, next_box->id);
                    }
                }
            }
            break;

        case ACTION_CREATE_BOX: {
            /* Create box at specified position */
            int box_id = canvas_add_box(canvas, event->data.box.world_x,
                                       event->data.box.world_y, 25, 6, "New Box");
            if (box_id >= 0) {
                const char *content[] = {
                    "Click to select",
                    "D to delete"
                };
                canvas_add_box_content(canvas, box_id, content, 2);
                canvas_select_box(canvas, box_id);
                
                /* If joystick, enter edit mode */
                if (js) {
                    joystick_enter_edit_mode(js, box_id);
                }
            }
            break;
        }

        case ACTION_DELETE_BOX: {
            Box *selected = canvas_get_selected(canvas);
            if (selected) {
                int selected_id = selected->id;
                canvas_remove_box(canvas, selected_id);
                canvas_deselect(canvas);
                
                /* If joystick, return to navigation mode */
                if (js) {
                    joystick_enter_navigation_mode(js);
                }
            }
            break;
        }

        case ACTION_MOVE_BOX: {
            /* Mouse drag or joystick movement */
            if (event->data.move.box_id >= 0) {
                Box *box = canvas_get_box(canvas, event->data.move.box_id);
                if (box) {
                    if (js && js->mode == MODE_EDIT) {
                        /* Joystick - relative movement (analog) */
                        double scaled_speed = PAN_SPEED / vp->zoom;
                        box->x += event->data.move.world_x * scaled_speed;
                        box->y += event->data.move.world_y * scaled_speed;
                        
                        /* Update cursor to box position */
                        js->cursor_x = box->x;
                        js->cursor_y = box->y;
                    } else {
                        /* Mouse - absolute position with offset */
                        box->x = event->data.move.world_x - event->data.move.offset_x;
                        box->y = event->data.move.world_y - event->data.move.offset_y;
                    }
                }
            }
            break;
        }

        case ACTION_COLOR_BOX:
            if (canvas->selected_index >= 0) {
                if (event->data.color.color_index == -1) {
                    /* Cycle color (joystick) */
                    canvas->boxes[canvas->selected_index].color =
                        (canvas->boxes[canvas->selected_index].color + 1) % 8;
                } else if (event->data.color.color_index == 0 && canvas->selected_index < 0) {
                    /* Reset view (keyboard '0' with no selection) */
                    vp->cam_x = 0.0;
                    vp->cam_y = 0.0;
                    vp->zoom = 1.0;
                } else {
                    /* Set specific color */
                    canvas->boxes[canvas->selected_index].color = event->data.color.color_index;
                }
            } else if (event->data.color.color_index == 0) {
                /* No box selected, reset view */
                vp->cam_x = 0.0;
                vp->cam_y = 0.0;
                vp->zoom = 1.0;
            }
            break;

        case ACTION_RESET_VIEW:
            vp->cam_x = 0.0;
            vp->cam_y = 0.0;
            vp->zoom = 1.0;
            break;

        case ACTION_SAVE_CANVAS:
            canvas_save(canvas, DEFAULT_SAVE_FILE);
            break;

        case ACTION_LOAD_CANVAS: {
            const char *file_to_load = persistence_get_current_file();
            if (file_to_load == NULL) {
                file_to_load = DEFAULT_SAVE_FILE;
            }

            Canvas old_canvas = *canvas;
            if (canvas_load(canvas, file_to_load) != 0) {
                *canvas = old_canvas;
            } else {
                canvas_cleanup(&old_canvas);
            }
            break;
        }

        case ACTION_ENTER_EDIT_MODE:
            if (js && js->selected_box_id >= 0) {
                joystick_enter_edit_mode(js, js->selected_box_id);
            }
            break;

        case ACTION_ENTER_PARAM_MODE:
            if (js) {
                joystick_enter_parameter_mode(js);
                /* Store current width for editing */
                Box *box = canvas_get_box(canvas, js->selected_box_id);
                if (box) {
                    js->param_edit_value = box->width;
                }
            }
            break;

        case ACTION_ENTER_NAV_MODE:
            if (js) {
                joystick_enter_navigation_mode(js);
                canvas_deselect(canvas);
            }
            break;

        default:
            break;
    }

    return 0;
}

/* Parameter mode: Adjust box properties (width/height/color) */
static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y) {
    /* Get selected box */
    if (js->selected_box_id < 0) {
        /* No box selected, return to navigation */
        joystick_enter_navigation_mode(js);
        return 0;
    }

    Box *box = canvas_get_box(canvas, js->selected_box_id);
    if (!box) {
        /* Box no longer exists, return to navigation */
        joystick_enter_navigation_mode(js);
        return 0;
    }

    /* Y-axis: Select parameter (with deadzone to prevent accidental switching) */
    if (axis_y > 0.5) {
        /* Down: Next parameter */
        if (js->selected_param < PARAM_COLOR) {
            js->selected_param++;
            /* Update edit value to new parameter's current value */
            switch (js->selected_param) {
                case PARAM_WIDTH:  js->param_edit_value = box->width; break;
                case PARAM_HEIGHT: js->param_edit_value = box->height; break;
                case PARAM_COLOR:  js->param_edit_value = box->color; break;
            }
        }
    } else if (axis_y < -0.5) {
        /* Up: Previous parameter */
        if (js->selected_param > PARAM_WIDTH) {
            js->selected_param--;
            /* Update edit value to new parameter's current value */
            switch (js->selected_param) {
                case PARAM_WIDTH:  js->param_edit_value = box->width; break;
                case PARAM_HEIGHT: js->param_edit_value = box->height; break;
                case PARAM_COLOR:  js->param_edit_value = box->color; break;
            }
        }
    }

    /* X-axis: Adjust selected parameter value */
    if (fabs(axis_x) > 0.0) {
        int delta = (int)(axis_x * 2.0);  /* Scale adjustment speed */

        switch (js->selected_param) {
            case PARAM_WIDTH:
                box->width += delta;
                if (box->width < 10) box->width = 10;
                if (box->width > 80) box->width = 80;
                js->param_edit_value = box->width;
                break;

            case PARAM_HEIGHT:
                box->height += delta;
                if (box->height < 3) box->height = 3;
                if (box->height > 30) box->height = 30;
                js->param_edit_value = box->height;
                break;

            case PARAM_COLOR:
                if (delta > 0) {
                    box->color = (box->color + 1) % 8;
                } else if (delta < 0) {
                    box->color = (box->color + 7) % 8;  /* +7 same as -1 mod 8 */
                }
                js->param_edit_value = box->color;
                break;
        }
    }

    /* Button 0 (A): Confirm and return to edit mode */
    if (joystick_button_pressed(js, BUTTON_A)) {
        joystick_enter_edit_mode(js, js->selected_box_id);
    }

    /* Button 1 (B): Cancel and return to edit mode (values already applied live) */
    if (joystick_button_pressed(js, BUTTON_B)) {
        joystick_enter_edit_mode(js, js->selected_box_id);
    }

    (void)vp;  /* Suppress unused parameter warning */
    return 0;
}
