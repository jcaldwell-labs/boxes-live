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
            if (event.bstate & BUTTON1_CLICKED) {
                /* Convert screen coordinates to world coordinates */
                double wx = screen_to_world_x(vp, event.x);
                double wy = screen_to_world_y(vp, event.y);

                /* Find and select box at clicked position */
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
        case '0':
            vp->cam_x = 0.0;
            vp->cam_y = 0.0;
            vp->zoom = 1.0;
            break;

        /* Save canvas */
        case KEY_F(2): {
            canvas_save(canvas, DEFAULT_SAVE_FILE);
            break;
        }

        /* Load canvas */
        case KEY_F(3): {
            /* Save current state */
            Canvas old_canvas = *canvas;

            /* Try to load */
            if (canvas_load(canvas, DEFAULT_SAVE_FILE) != 0) {
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
