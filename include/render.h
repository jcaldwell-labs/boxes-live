#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "joystick.h"

/* Render all boxes in the canvas through the viewport */
void render_canvas(const Canvas *canvas, const Viewport *vp);

/* Render a single box */
void render_box(const Box *box, const Viewport *vp);

/* Render status bar with viewport and canvas info */
void render_status(const Canvas *canvas, const Viewport *vp);

/* Render joystick cursor indicator */
void render_joystick_cursor(const JoystickState *js, const Viewport *vp);

/* Render joystick mode indicator (enhanced status bar) */
void render_joystick_mode(const JoystickState *js, const Canvas *canvas);

/* Render parameter edit panel (when in parameter mode) */
void render_parameter_panel(const JoystickState *js, const Box *box);

/* Render joystick visualizer panel showing button states and stick position */
void render_joystick_visualizer(const JoystickState *js, const Viewport *vp);

#endif /* RENDER_H */
