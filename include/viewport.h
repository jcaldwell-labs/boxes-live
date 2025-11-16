#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "types.h"

/* Initialize viewport with default values */
void viewport_init(Viewport *vp);

/* Pan the viewport by dx, dy in world space */
void viewport_pan(Viewport *vp, double dx, double dy);

/* Zoom in or out (factor > 1.0 zooms in, < 1.0 zooms out) */
void viewport_zoom(Viewport *vp, double factor);

/* Convert world coordinates to screen coordinates */
int world_to_screen_x(const Viewport *vp, double world_x);
int world_to_screen_y(const Viewport *vp, double world_y);

/* Convert screen coordinates to world coordinates */
double screen_to_world_x(const Viewport *vp, int screen_x);
double screen_to_world_y(const Viewport *vp, int screen_y);

/* Check if a point in world space is visible in the viewport */
int is_visible(const Viewport *vp, double x, double y);

#endif /* VIEWPORT_H */
