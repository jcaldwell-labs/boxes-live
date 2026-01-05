#ifndef UNDO_H
#define UNDO_H

#include "types.h"

/* Canvas is already defined in types.h, no forward declaration needed */

/* ============================================================
 * Undo Stack Management (Issue #81)
 * ============================================================ */

/* Initialize undo stack with default settings */
void undo_stack_init(UndoStack *stack);

/* Free all memory in the undo stack */
void undo_stack_cleanup(UndoStack *stack);

/* ============================================================
 * Recording Operations
 * ============================================================ */

/* Record a box creation operation */
void undo_record_box_create(Canvas *canvas, int box_id);

/* Record a box deletion operation (captures full box state before delete) */
void undo_record_box_delete(Canvas *canvas, int box_id);

/* Record a box move operation */
void undo_record_box_move(Canvas *canvas, int box_id,
                          double old_x, double old_y,
                          double new_x, double new_y);

/* Record a box resize operation */
void undo_record_box_resize(Canvas *canvas, int box_id,
                            int old_width, int old_height,
                            int new_width, int new_height);

/* Record a box title change */
void undo_record_box_title(Canvas *canvas, int box_id,
                           const char *old_title, const char *new_title);

/* Record a box color change */
void undo_record_box_color(Canvas *canvas, int box_id,
                           int old_color, int new_color);

/* Record a connection creation */
void undo_record_connection_create(Canvas *canvas, int conn_id);

/* Record a connection deletion */
void undo_record_connection_delete(Canvas *canvas, int conn_id);

/* ============================================================
 * Undo/Redo Operations
 * ============================================================ */

/* Perform undo operation. Returns true if undo was performed. */
bool canvas_undo(Canvas *canvas);

/* Perform redo operation. Returns true if redo was performed. */
bool canvas_redo(Canvas *canvas);

/* Check if undo is available */
bool canvas_can_undo(const Canvas *canvas);

/* Check if redo is available */
bool canvas_can_redo(const Canvas *canvas);

/* Get description of next undo operation (for status display) */
const char* canvas_get_undo_description(const Canvas *canvas);

/* Get description of next redo operation (for status display) */
const char* canvas_get_redo_description(const Canvas *canvas);

#endif /* UNDO_H */
