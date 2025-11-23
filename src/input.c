#include <ncurses.h>
#include <math.h>
#include "input.h"
#include "viewport.h"
#include "canvas.h"
#include "persistence.h"
#include "joystick.h"

/* Pan speed in world units */
#define PAN_SPEED 2.0

/* Zoom factor per key press */
#define ZOOM_FACTOR 1.2

/* Default save file */
#define DEFAULT_SAVE_FILE "canvas.txt"

/* Mouse drag state */
static bool dragging = false;
static int drag_box_id = -1;
static double drag_offset_x = 0.0;
static double drag_offset_y = 0.0;

/* Forward declarations for joystick mode handlers */
static int handle_joystick_navigation(Canvas *canvas, Viewport *vp, JoystickState *js,
                                       double axis_x, double axis_y);
static int handle_joystick_edit(Canvas *canvas, Viewport *vp, JoystickState *js,
                                 double axis_x, double axis_y);
static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y);

int handle_input(Canvas *canvas, Viewport *vp) {
    int ch = getch();

    if (ch == ERR) {
        /* No input available */
        return 0;
    }

    /* Handle mouse events */
    if (ch == KEY_MOUSE) {
        MEVENT event;
        if (getmouse(&event) == OK) {
            double wx = screen_to_world_x(vp, event.x);
            double wy = screen_to_world_y(vp, event.y);

            /* Mouse button pressed - start drag */
            if (event.bstate & BUTTON1_PRESSED) {
                int box_id = canvas_find_box_at(canvas, wx, wy);
                if (box_id >= 0) {
                    Box *box = canvas_get_box(canvas, box_id);
                    if (box) {
                        dragging = true;
                        drag_box_id = box_id;
                        drag_offset_x = wx - box->x;
                        drag_offset_y = wy - box->y;
                        canvas_select_box(canvas, box_id);
                    }
                } else {
                    canvas_deselect(canvas);
                }
            }
            /* Mouse drag - move box */
            else if (dragging && (event.bstate & REPORT_MOUSE_POSITION)) {
                Box *box = canvas_get_box(canvas, drag_box_id);
                if (box) {
                    box->x = wx - drag_offset_x;
                    box->y = wy - drag_offset_y;
                }
            }
            /* Mouse button released - end drag */
            else if (event.bstate & BUTTON1_RELEASED) {
                dragging = false;
                drag_box_id = -1;
            }
            /* Single click (no drag) */
            else if (event.bstate & BUTTON1_CLICKED) {
                int box_id = canvas_find_box_at(canvas, wx, wy);
                if (box_id >= 0) {
                    canvas_select_box(canvas, box_id);
                } else {
                    canvas_deselect(canvas);
                }
            }
        }
        return 0;
    }

    switch (ch) {
        /* Quit */
        case 'q':
        case 'Q':
        case 27: /* ESC */
            return 1;

        /* Create new box */
        case 'n':
        case 'N': {
            /* Create box at center of viewport */
            double center_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
            double center_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
            int box_id = canvas_add_box(canvas, center_x, center_y, 25, 6, "New Box");
            const char *content[] = {
                "Click to select",
                "D to delete"
            };
            canvas_add_box_content(canvas, box_id, content, 2);
            canvas_select_box(canvas, box_id);
            break;
        }

        /* Delete selected box */
        case 'd':
        case 'D': {
            Box *selected = canvas_get_selected(canvas);
            if (selected) {
                canvas_remove_box(canvas, selected->id);
            }
            break;
        }

        /* Tab: Cycle through boxes */
        case '\t': {
            if (canvas->box_count > 0) {
                int next_index = (canvas->selected_index + 1) % canvas->box_count;
                Box *next_box = canvas_get_box_at(canvas, next_index);
                if (next_box) {
                    canvas_select_box(canvas, next_box->id);
                }
            }
            break;
        }

        /* Color selection for selected box */
        case '1':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_RED;
            }
            break;

        case '2':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_GREEN;
            }
            break;

        case '3':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_BLUE;
            }
            break;

        case '4':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_YELLOW;
            }
            break;

        case '5':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_MAGENTA;
            }
            break;

        case '6':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_CYAN;
            }
            break;

        case '7':
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_WHITE;
            }
            break;

        case '0':
            /* Reset color to default (also resets view if not number row) */
            if (canvas->selected_index >= 0) {
                canvas->boxes[canvas->selected_index].color = BOX_COLOR_DEFAULT;
            } else {
                /* No box selected, reset view */
                vp->cam_x = 0.0;
                vp->cam_y = 0.0;
                vp->zoom = 1.0;
            }
            break;

        /* Pan with arrow keys */
        case KEY_UP:
            viewport_pan(vp, 0, -PAN_SPEED);
            break;

        case KEY_DOWN:
            viewport_pan(vp, 0, PAN_SPEED);
            break;

        case KEY_LEFT:
            viewport_pan(vp, -PAN_SPEED, 0);
            break;

        case KEY_RIGHT:
            viewport_pan(vp, PAN_SPEED, 0);
            break;

        /* Pan with WASD */
        case 'w':
        case 'W':
            viewport_pan(vp, 0, -PAN_SPEED);
            break;

        case 's':
        case 'S':
            viewport_pan(vp, 0, PAN_SPEED);
            break;

        case 'a':
        case 'A':
        case 'h':
        case 'H':
            viewport_pan(vp, -PAN_SPEED, 0);
            break;

        case 'l':
        case 'L':
            viewport_pan(vp, PAN_SPEED, 0);
            break;

        /* Zoom in */
        case '+':
        case '=':
        case 'z':
        case 'Z':
            viewport_zoom(vp, ZOOM_FACTOR);
            break;

        /* Zoom out */
        case '-':
        case '_':
        case 'x':
        case 'X':
            viewport_zoom(vp, 1.0 / ZOOM_FACTOR);
            break;

        /* Reset view */
        case 'r':
        case 'R':
            vp->cam_x = 0.0;
            vp->cam_y = 0.0;
            vp->zoom = 1.0;
            break;

        /* Save canvas */
        case KEY_F(2): {
            canvas_save(canvas, DEFAULT_SAVE_FILE);
            break;
        }

        /* Load/reload canvas */
        case KEY_F(3): {
            /* Determine which file to load */
            const char *file_to_load = persistence_get_current_file();
            if (file_to_load == NULL) {
                file_to_load = DEFAULT_SAVE_FILE;
            }

            /* Save current state */
            Canvas old_canvas = *canvas;

            /* Try to load */
            if (canvas_load(canvas, file_to_load) != 0) {
                /* Restore old state on failure */
                *canvas = old_canvas;
            } else {
                /* Clean up old state on success */
                canvas_cleanup(&old_canvas);
            }
            break;
        }
    }

    return 0;
}

/* Handle joystick input based on current mode */
int handle_joystick_input(Canvas *canvas, Viewport *vp, JoystickState *js) {
    if (!canvas || !vp || !js || !js->available) {
        return 0;
    }

    /* Get axis values (normalized -1.0 to 1.0 with deadzone) */
    double axis_x = joystick_get_axis_normalized(js, AXIS_X);
    double axis_y = joystick_get_axis_normalized(js, AXIS_Y);

    /* Mode-specific input handling */
    switch (js->mode) {
        case MODE_NAVIGATION:
            return handle_joystick_navigation(canvas, vp, js, axis_x, axis_y);

        case MODE_EDIT:
            return handle_joystick_edit(canvas, vp, js, axis_x, axis_y);

        case MODE_PARAMETER:
            return handle_joystick_parameter(canvas, vp, js, axis_x, axis_y);

        default:
            return 0;
    }
}

/* Navigation mode: Pan viewport, zoom, create/select boxes */
static int handle_joystick_navigation(Canvas *canvas, Viewport *vp, JoystickState *js,
                                       double axis_x, double axis_y) {
    /* Pan viewport with joystick axes */
    if (fabs(axis_x) > 0.0 || fabs(axis_y) > 0.0) {
        /* Scale pan speed by zoom (faster when zoomed out, slower when zoomed in) */
        double scaled_speed = PAN_SPEED / vp->zoom;

        /* Update viewport */
        viewport_pan(vp, axis_x * scaled_speed, axis_y * scaled_speed);

        /* Update cursor to follow viewport center */
        js->cursor_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
        js->cursor_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
    }

    /* Button 0 (A): Zoom in */
    if (joystick_button_pressed(js, BUTTON_A)) {
        viewport_zoom(vp, ZOOM_FACTOR);
    }

    /* Button 1 (B): Zoom out */
    if (joystick_button_pressed(js, BUTTON_B)) {
        viewport_zoom(vp, 1.0 / ZOOM_FACTOR);
    }

    /* Button 2 (X): Create new box at cursor position */
    if (joystick_button_pressed(js, BUTTON_X)) {
        /* Find nearby boxes for smart sizing */
        int last_box_id = canvas->box_count > 0 ? canvas->box_count - 1 : -1;
        int width = 25;   /* Default */
        int height = 6;   /* Default */

        /* If there are existing boxes, use proportional sizing */
        if (last_box_id >= 0) {
            Box *last_box = canvas_get_box(canvas, last_box_id);
            if (last_box) {
                /* Vary size slightly (90% to 110% of last box) */
                width = last_box->width + ((canvas->box_count % 5) - 2) * 2;
                height = last_box->height + ((canvas->box_count % 3) - 1);

                /* Clamp to reasonable sizes */
                if (width < 15) width = 15;
                if (width > 50) width = 50;
                if (height < 4) height = 4;
                if (height > 20) height = 20;
            }
        }

        /* Create box at cursor position */
        int box_id = canvas_add_box(canvas, js->cursor_x, js->cursor_y,
                                     width, height, "New Box");
        if (box_id >= 0) {
            /* Select the newly created box and enter edit mode */
            canvas_select_box(canvas, box_id);
            joystick_enter_edit_mode(js, box_id);
        }
    }

    /* Button 3 (Y): Delete selected box */
    if (joystick_button_pressed(js, BUTTON_Y)) {
        Box *selected = canvas_get_selected(canvas);
        if (selected) {
            int selected_id = selected->id;
            canvas_remove_box(canvas, selected_id);
            canvas_deselect(canvas);
        }
    }

    /* Start button: Save canvas */
    if (joystick_button_pressed(js, BUTTON_START)) {
        canvas_save(canvas, DEFAULT_SAVE_FILE);
    }

    /* Select button: Load canvas */
    if (joystick_button_pressed(js, BUTTON_SELECT)) {
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
    }

    return 0;
}

/* Edit mode: Move selected box, change color, enter parameter mode */
static int handle_joystick_edit(Canvas *canvas, Viewport *vp, JoystickState *js,
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

    /* Move box with joystick axes */
    if (fabs(axis_x) > 0.0 || fabs(axis_y) > 0.0) {
        /* Scale movement by zoom (slower when zoomed in) */
        double scaled_speed = PAN_SPEED / vp->zoom;

        /* Update box position */
        box->x += axis_x * scaled_speed;
        box->y += axis_y * scaled_speed;

        /* Update cursor to box position */
        js->cursor_x = box->x;
        js->cursor_y = box->y;
    }

    /* Button 0 (A): Enter parameter edit mode */
    if (joystick_button_pressed(js, BUTTON_A)) {
        joystick_enter_parameter_mode(js);
        /* Store current values for editing */
        js->param_edit_value = box->width;  /* Start with width */
    }

    /* Button 1 (B): Return to navigation mode */
    if (joystick_button_pressed(js, BUTTON_B)) {
        joystick_enter_navigation_mode(js);
        canvas_deselect(canvas);
    }

    /* Button 2 (X): Cycle box color */
    if (joystick_button_pressed(js, BUTTON_X)) {
        box->color = (box->color + 1) % 8;  /* Cycle through 0-7 */
    }

    /* Button 3 (Y): Delete this box and return to navigation */
    if (joystick_button_pressed(js, BUTTON_Y)) {
        canvas_remove_box(canvas, js->selected_box_id);
        joystick_enter_navigation_mode(js);
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
