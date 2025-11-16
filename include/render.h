#ifndef RENDER_H
#define RENDER_H

#include "types.h"

/* Render all boxes in the canvas through the viewport */
void render_canvas(const Canvas *canvas, const Viewport *vp);

/* Render a single box */
void render_box(const Box *box, const Viewport *vp);

/* Render status bar with viewport and canvas info */
void render_status(const Canvas *canvas, const Viewport *vp);

#endif /* RENDER_H */
