#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "joystick.h"
#include "config.h"

/* Render all boxes in the canvas through the viewport */
void render_canvas(const Canvas *canvas, const Viewport *vp, const AppConfig *config);

/* Render a single box with specified display mode (Issue #33) */
void render_box(const Box *box, const Viewport *vp, DisplayMode mode, const char *icon);

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

/* Render text editor panel for editing box title (Phase 3) */
void render_text_editor(const JoystickState *js, const Box *box);

/* Render grid (Phase 4) */
void render_grid(const Canvas *canvas, const Viewport *vp);

/* Render focused box (Phase 5b) */
void render_focused_box(const Canvas *canvas);

/* Render all connections between boxes (Issue #20) */
void render_connections(const Canvas *canvas, const Viewport *vp);

/* Render connection mode indicator (Issue #20) */
void render_connection_mode(const Canvas *canvas, const Viewport *vp);

#endif /* RENDER_H */
