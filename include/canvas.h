#ifndef CANVAS_H
#define CANVAS_H

#include "types.h"

/* Initialize canvas with dynamic memory allocation */
int canvas_init(Canvas *canvas, double world_width, double world_height);

/* Free canvas memory */
void canvas_cleanup(Canvas *canvas);

/* Add a box to the canvas (returns box ID, or -1 on error) */
int canvas_add_box(Canvas *canvas, double x, double y, int width, int height, const char *title);

/* Add content lines to a box by ID */
int canvas_add_box_content(Canvas *canvas, int box_id, const char **lines, int count);

/* Remove a box from canvas by ID */
int canvas_remove_box(Canvas *canvas, int box_id);

/* Get box by ID (returns NULL if not found) */
Box* canvas_get_box(Canvas *canvas, int box_id);

/* Get box by index (returns NULL if out of bounds) */
Box* canvas_get_box_at(Canvas *canvas, int index);

/* Find box at world coordinates (returns box ID, or -1 if none found) */
int canvas_find_box_at(Canvas *canvas, double x, double y);

/* Select a box by ID */
void canvas_select_box(Canvas *canvas, int box_id);

/* Deselect current box */
void canvas_deselect(Canvas *canvas);

/* Get currently selected box (returns NULL if none) */
Box* canvas_get_selected(Canvas *canvas);

/* Snap box position to grid (Phase 4) */
void canvas_snap_box_to_grid(Canvas *canvas, Box *box);

#endif /* CANVAS_H */
