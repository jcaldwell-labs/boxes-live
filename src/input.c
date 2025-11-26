#include <ncurses.h>
#include <math.h>
#include <string.h>
#include "input.h"
#include "input_unified.h"
#include "viewport.h"
#include "canvas.h"
#include "persistence.h"
#include "joystick.h"
#include "config.h"

/* Zoom factor per key press */
#define ZOOM_FACTOR 1.2

/* Default save file */
#define DEFAULT_SAVE_FILE "canvas.txt"

/* Pan speed for joystick input (scaled by zoom) */
#define PAN_SPEED 2.0

/* Forward declarations for joystick mode handlers (legacy, kept for parameter mode) */
/* Removed in Phase 1 - will be reimplemented in Phase 2 */
/* static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y); */

/* Helper function to execute canvas actions */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event, const AppConfig *config);

int handle_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config) {
    int ch = getch();

    if (ch == ERR) {
        /* No input available */
        return 0;
    }

    /* Focus mode active - handle focus mode input (Phase 5b) */
    if (canvas->focus.active) {
        Box *box = canvas_get_box(canvas, canvas->focus.focused_box_id);

        if (ch == 27 || ch == 'q') {  /* ESC or q */
            /* Exit focus mode */
            canvas->focus.active = false;
            canvas->focus.focused_box_id = -1;
            return 0;
        } else if (ch == 'j' || ch == KEY_DOWN) {
            /* Scroll down */
            if (canvas->focus.scroll_offset < canvas->focus.scroll_max) {
                canvas->focus.scroll_offset++;
            }
            return 0;
        } else if (ch == 'k' || ch == KEY_UP) {
            /* Scroll up */
            if (canvas->focus.scroll_offset > 0) {
                canvas->focus.scroll_offset--;
            }
            return 0;
        } else if (ch == 'g') {
            /* Jump to top */
            canvas->focus.scroll_offset = 0;
            return 0;
        } else if (ch == 'G') {
            /* Jump to bottom */
            canvas->focus.scroll_offset = canvas->focus.scroll_max;
            return 0;
        } else if (ch == KEY_NPAGE) {  /* Page Down */
            canvas->focus.scroll_offset += (LINES - 4) / 2;
            if (canvas->focus.scroll_offset > canvas->focus.scroll_max) {
                canvas->focus.scroll_offset = canvas->focus.scroll_max;
            }
            return 0;
        } else if (ch == KEY_PPAGE) {  /* Page Up */
            canvas->focus.scroll_offset -= (LINES - 4) / 2;
            if (canvas->focus.scroll_offset < 0) {
                canvas->focus.scroll_offset = 0;
            }
            return 0;
        }

        /* Ignore other keys in focus mode */
        (void)box;  /* Suppress unused warning */
        return 0;
    }

    /* Text editor active - handle keyboard input for text editing */
    if (js && js->text_editor_active) {
        Box *box = canvas_get_box(canvas, js->selected_box_id);

        if (ch == 27 || ch == KEY_F(10)) {  /* ESC or F10 */
            /* Save and close editor */
            joystick_close_text_editor(js, true, box);
            return 0;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            /* Backspace */
            joystick_text_editor_backspace(js);
            return 0;
        } else if (ch == KEY_LEFT) {
            /* Move cursor left */
            joystick_text_editor_move_cursor(js, -1);
            return 0;
        } else if (ch == KEY_RIGHT) {
            /* Move cursor right */
            joystick_text_editor_move_cursor(js, 1);
            return 0;
        } else if (ch == KEY_HOME) {
            /* Move to start */
            if (js->text_edit_buffer) {
                js->text_cursor_pos = 0;
            }
            return 0;
        } else if (ch == KEY_END) {
            /* Move to end */
            if (js->text_edit_buffer) {
                js->text_cursor_pos = strlen(js->text_edit_buffer);
            }
            return 0;
        } else if (ch >= 32 && ch < 127) {
            /* Printable character */
            joystick_text_editor_insert_char(js, (char)ch);
            return 0;
        }

        /* Ignore other keys while in text editor */
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
        return execute_canvas_action(canvas, vp, js, &event, config);
    }

    return 0;
}

/* Handle joystick input based on current mode */
int handle_joystick_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config) {
    if (!canvas || !vp || !js || !js->available) {
        return 0;
    }

    /* Parameter mode handling removed - now part of unified input layer */

    /* Use unified input layer for navigation and edit modes */
    InputEvent event;
    int source = input_unified_process_joystick(js, canvas, vp, &event);

    if (source >= 0 && event.action != ACTION_NONE) {
        return execute_canvas_action(canvas, vp, js, &event, config);
    }

    return 0;
}

/* Execute canvas action from unified input event */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event, const AppConfig *config) {
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
                
                /* Update joystick cursor if in NAV mode */
                if (js && js->mode == MODE_NAV) {
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
            /* Get template dimensions (Issue #17) */
            int box_width = 25;
            int box_height = 6;
            const char *template_name = "New Box";

            if (config) {
                config_get_template_dimensions(config, event->data.box.template,
                                               &box_width, &box_height);
                template_name = config_get_template_name(event->data.box.template);

                /* Apply proportional sizing if enabled (Issue #18) */
                if (config->proportional_sizing) {
                    int prop_width, prop_height;
                    int neighbors = canvas_calc_proportional_size(
                        canvas,
                        event->data.box.world_x,
                        event->data.box.world_y,
                        config->proximity_radius,
                        config->use_nearest_neighbor,
                        config->min_neighbors_required,
                        box_width, box_height,
                        &prop_width, &prop_height);

                    if (neighbors > 0) {
                        box_width = prop_width;
                        box_height = prop_height;
                    }
                }
            }

            /* Create box at specified position with calculated dimensions */
            int box_id = canvas_add_box(canvas, event->data.box.world_x,
                                       event->data.box.world_y, box_width, box_height,
                                       template_name);
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
                
                /* If joystick, return to NAV mode */
                if (js) {
                    joystick_enter_nav_mode(js);
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

        case ACTION_TOGGLE_GRID:
            canvas->grid.visible = !canvas->grid.visible;
            break;

        case ACTION_TOGGLE_SNAP:
            canvas->grid.snap_enabled = !canvas->grid.snap_enabled;
            break;

        case ACTION_FOCUS_BOX:
            /* Enter focus mode on selected box (Phase 5b) */
            if (canvas->selected_index >= 0) {
                Box *box = canvas_get_box_at(canvas, canvas->selected_index);
                if (box) {
                    canvas->focus.active = true;
                    canvas->focus.focused_box_id = box->id;
                    canvas->focus.scroll_offset = 0;
                    canvas->focus.scroll_max = 0;  /* Will be calculated in render */
                }
            }
            break;

        case ACTION_EXIT_FOCUS:
            /* Exit focus mode (Phase 5b) */
            canvas->focus.active = false;
            canvas->focus.focused_box_id = -1;
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
            /* TODO: Implement parameter panel in Phase 2 */
            /* For now, parameter editing is done directly in EDIT mode */
            break;

        case ACTION_ENTER_NAV_MODE:
            if (js) {
                joystick_enter_nav_mode(js);
                canvas_deselect(canvas);
            }
            break;

        default:
            break;
    }

    return 0;
}

/* Parameter mode: Removed in Phase 1 - will be reimplemented in Phase 2 */
/*
static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y) {
    (void)canvas; (void)vp; (void)js; (void)axis_x; (void)axis_y;
    return 0;
}
*/
