#include <ncurses.h>
#include "input.h"
#include "viewport.h"
#include "canvas.h"
#include "persistence.h"

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
