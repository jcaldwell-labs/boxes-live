#include <math.h>
#include "viewport.h"

void viewport_init(Viewport *vp) {
    vp->cam_x = 0.0;
    vp->cam_y = 0.0;
    vp->zoom = 1.0;
    vp->term_width = 80;
    vp->term_height = 24;
}

void viewport_pan(Viewport *vp, double dx, double dy) {
    vp->cam_x += dx / vp->zoom;
    vp->cam_y += dy / vp->zoom;
}

void viewport_zoom(Viewport *vp, double factor) {
    double old_zoom = vp->zoom;
    vp->zoom *= factor;

    /* Clamp zoom level to reasonable bounds */
    if (vp->zoom < 0.1) {
        vp->zoom = 0.1;
    } else if (vp->zoom > 10.0) {
        vp->zoom = 10.0;
    }

    /* Adjust camera position to zoom towards center */
    double zoom_ratio = vp->zoom / old_zoom;
    if (zoom_ratio != 1.0) {
        double center_x = vp->cam_x + (vp->term_width / 2.0) / old_zoom;
        double center_y = vp->cam_y + (vp->term_height / 2.0) / old_zoom;

        vp->cam_x = center_x - (vp->term_width / 2.0) / vp->zoom;
        vp->cam_y = center_y - (vp->term_height / 2.0) / vp->zoom;
    }
}

int world_to_screen_x(const Viewport *vp, double world_x) {
    return (int)round((world_x - vp->cam_x) * vp->zoom);
}

int world_to_screen_y(const Viewport *vp, double world_y) {
    return (int)round((world_y - vp->cam_y) * vp->zoom);
}

double screen_to_world_x(const Viewport *vp, int screen_x) {
    return vp->cam_x + screen_x / vp->zoom;
}

double screen_to_world_y(const Viewport *vp, int screen_y) {
    return vp->cam_y + screen_y / vp->zoom;
}

int is_visible(const Viewport *vp, double x, double y) {
    int sx = world_to_screen_x(vp, x);
    int sy = world_to_screen_y(vp, y);

    return (sx >= 0 && sx < vp->term_width &&
            sy >= 0 && sy < vp->term_height);
}
