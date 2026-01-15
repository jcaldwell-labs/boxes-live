#ifndef CANVAS_H
#define CANVAS_H

#include "types.h"

/* Initialize canvas with dynamic memory allocation */
int canvas_init(Canvas *canvas, double world_width, double world_height);

/* Free canvas memory */
void canvas_cleanup(Canvas *canvas);

/* Add a box to the canvas (returns box ID, or -1 on error) */
int canvas_add_box(Canvas *canvas, double x, double y, int width, int height, const char *title);

/* Restore a box with a specific ID (for undo/redo - skips grid snap, preserves ID) */
int canvas_restore_box_with_id(Canvas *canvas, int box_id, double x, double y,
                               int width, int height, const char *title);

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

/* Proportional sizing functions (Issue #18) */

/* Calculate proportional dimensions based on nearby boxes
 * Parameters:
 *   canvas - The canvas containing boxes
 *   x, y - World coordinates of the new box position
 *   proximity_radius - Search distance in world units
 *   use_nearest - true = use nearest box, false = average of all neighbors
 *   min_neighbors - Minimum neighbors required (falls back to template if not met)
 *   default_width, default_height - Template defaults to use if no neighbors found
 *   out_width, out_height - Output calculated dimensions
 * Returns: Number of neighbors found (0 means template defaults used)
 */
int canvas_calc_proportional_size(const Canvas *canvas, double x, double y,
                                  int proximity_radius, bool use_nearest,
                                  int min_neighbors,
                                  int default_width, int default_height,
                                  int *out_width, int *out_height);

/* Connection management functions (Issue #20) */

/* Add a connection between two boxes (returns connection ID, or -1 on error) */
int canvas_add_connection(Canvas *canvas, int source_id, int dest_id);

/* Restore a connection with specific ID and color (for undo/redo) */
int canvas_restore_connection_with_id(Canvas *canvas, int conn_id, int source_id,
                                      int dest_id, int color);

/* Remove a connection by ID (returns 0 on success, -1 if not found) */
int canvas_remove_connection(Canvas *canvas, int conn_id);

/* Get connection by ID (returns NULL if not found) */
Connection* canvas_get_connection(Canvas *canvas, int conn_id);

/* Find connection between two boxes (returns connection ID, or -1 if none) */
int canvas_find_connection(const Canvas *canvas, int source_id, int dest_id);

/* Get all connections involving a specific box (fills array, returns count) */
int canvas_get_box_connections(const Canvas *canvas, int box_id, int *conn_ids, int max_count);

/* Remove all connections involving a specific box */
void canvas_remove_box_connections(Canvas *canvas, int box_id);

/* Enter connection mode (sets source box) */
void canvas_start_connection(Canvas *canvas, int source_box_id);

/* Complete or cancel connection mode */
void canvas_finish_connection(Canvas *canvas, int dest_box_id);

/* Cancel connection mode without creating connection */
void canvas_cancel_connection(Canvas *canvas);

/* Check if in connection mode */
bool canvas_in_connection_mode(const Canvas *canvas);

#endif /* CANVAS_H */
