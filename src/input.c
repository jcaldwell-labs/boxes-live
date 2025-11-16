#include <ncurses.h>
#include "input.h"
#include "viewport.h"

/* Pan speed in world units */
#define PAN_SPEED 2.0

/* Zoom factor per key press */
#define ZOOM_FACTOR 1.2

int handle_input(Viewport *vp) {
    int ch = getch();

    if (ch == ERR) {
        /* No input available */
        return 0;
    }

    switch (ch) {
        /* Quit */
        case 'q':
        case 'Q':
        case 27: /* ESC */
            return 1;

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
            viewport_pan(vp, -PAN_SPEED, 0);
            break;

        case 'd':
        case 'D':
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
    }

    return 0;
}
