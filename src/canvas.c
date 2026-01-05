#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "canvas.h"
#include "undo.h"
#include "editor.h"

/* Initialize canvas with dynamic memory allocation */
int canvas_init(Canvas *canvas, double world_width, double world_height) {
    canvas->boxes = malloc(sizeof(Box) * INITIAL_BOX_CAPACITY);
    if (canvas->boxes == NULL) {
        return -1;
    }

    canvas->box_count = 0;
    canvas->box_capacity = INITIAL_BOX_CAPACITY;
    canvas->world_width = world_width;
    canvas->world_height = world_height;
    canvas->next_id = 1;
    canvas->selected_index = -1;

    /* Initialize grid configuration (Phase 4) */
    canvas->grid.visible = false;
    canvas->grid.snap_enabled = false;
    canvas->grid.spacing = 10;        /* Default minor: 10 world units */
    canvas->grid.major_spacing = 50;  /* Default major: 50 world units (Issue #49) */

    /* Initialize focus mode state (Phase 5b) */
    canvas->focus.active = false;
    canvas->focus.focused_box_id = -1;
    canvas->focus.scroll_offset = 0;
    canvas->focus.scroll_max = 0;

    /* Initialize connections (Issue #20) */
    canvas->connections = malloc(sizeof(Connection) * INITIAL_CONNECTION_CAPACITY);
    if (canvas->connections == NULL) {
        free(canvas->boxes);
        canvas->boxes = NULL;
        return -1;
    }
    canvas->conn_count = 0;
    canvas->conn_capacity = INITIAL_CONNECTION_CAPACITY;
    canvas->next_conn_id = 1;

    /* Initialize connection mode state */
    canvas->conn_mode.active = false;
    canvas->conn_mode.source_box_id = -1;
    canvas->conn_mode.pending_delete = false;
    canvas->conn_mode.delete_conn_id = -1;

    /* Initialize sidebar (Issue #35) */
    canvas->document = NULL;
    canvas->sidebar_state = SIDEBAR_HIDDEN;
    canvas->sidebar_width = 30;  /* Default width */

    /* Initialize display mode (Issue #33) */
    canvas->display_mode = DISPLAY_MODE_FULL;  /* Default to full display */

    /* Initialize help overlay state (Issue #34) */
    canvas->help.visible = false;

    /* Initialize command line state (Issue #55) */
    canvas->command_line.active = false;
    canvas->command_line.buffer[0] = '\0';
    canvas->command_line.cursor_pos = 0;
    canvas->command_line.length = 0;
    canvas->command_line.error_msg[0] = '\0';
    canvas->command_line.has_error = false;

    /* Initialize canvas metadata */
    canvas->filename = NULL;

    /* Initialize undo/redo stack (Issue #81) */
    undo_stack_init(&canvas->undo_stack);

    /* Initialize text editor (Issue #79) */
    editor_init(&canvas->editor);

    return 0;
}

/* Free canvas memory */
void canvas_cleanup(Canvas *canvas) {
    for (int i = 0; i < canvas->box_count; i++) {
        Box *box = &canvas->boxes[i];
        if (box->title) {
            free(box->title);
        }
        if (box->content) {
            for (int j = 0; j < box->content_lines; j++) {
                free(box->content[j]);
            }
            free(box->content);
        }
        /* Free content source fields (Issue #54) */
        if (box->file_path) {
            free(box->file_path);
        }
        if (box->command) {
            free(box->command);
        }
    }

    if (canvas->boxes) {
        free(canvas->boxes);
        canvas->boxes = NULL;
    }
    canvas->box_count = 0;
    canvas->box_capacity = 0;

    /* Free connections (Issue #20) */
    if (canvas->connections) {
        free(canvas->connections);
        canvas->connections = NULL;
    }
    canvas->conn_count = 0;
    canvas->conn_capacity = 0;

    /* Free sidebar document (Issue #35) */
    if (canvas->document) {
        free(canvas->document);
        canvas->document = NULL;
    }

    /* Free canvas metadata */
    if (canvas->filename) {
        free(canvas->filename);
        canvas->filename = NULL;
    }

    /* Free undo/redo stack (Issue #81) */
    undo_stack_cleanup(&canvas->undo_stack);

    /* Free text editor (Issue #79) */
    editor_cleanup(&canvas->editor);
}

/* Grow the box array if needed */
static int canvas_ensure_capacity(Canvas *canvas) {
    if (canvas->box_count >= canvas->box_capacity) {
        int new_capacity = canvas->box_capacity * 2;
        Box *new_boxes = realloc(canvas->boxes, sizeof(Box) * new_capacity);
        if (new_boxes == NULL) {
            return -1;
        }
        canvas->boxes = new_boxes;
        canvas->box_capacity = new_capacity;
    }
    return 0;
}

/* Add a box to the canvas (returns box ID, or -1 on error) */
int canvas_add_box(Canvas *canvas, double x, double y, int width, int height, const char *title) {
    if (canvas_ensure_capacity(canvas) != 0) {
        return -1;
    }

    /* Apply snap-to-grid if enabled (Phase 4) */
    if (canvas->grid.snap_enabled && canvas->grid.spacing > 0) {
        x = round(x / canvas->grid.spacing) * canvas->grid.spacing;
        y = round(y / canvas->grid.spacing) * canvas->grid.spacing;
    }

    Box *box = &canvas->boxes[canvas->box_count];
    box->x = x;
    box->y = y;
    box->width = width;
    box->height = height;
    box->title = title ? strdup(title) : NULL;
    box->content = NULL;
    box->content_lines = 0;
    box->selected = false;
    box->id = canvas->next_id++;
    box->color = BOX_COLOR_DEFAULT;
    box->box_type = BOX_TYPE_NOTE;  /* Default to NOTE type (Issue #33) */

    /* Initialize content source fields (Issue #54) */
    box->content_type = BOX_CONTENT_TEXT;  /* Default to static text */
    box->file_path = NULL;
    box->command = NULL;

    canvas->box_count++;

    return box->id;
}

/* Add content lines to a box by ID */
int canvas_add_box_content(Canvas *canvas, int box_id, const char **lines, int count) {
    for (int i = 0; i < canvas->box_count; i++) {
        if (canvas->boxes[i].id == box_id) {
            Box *box = &canvas->boxes[i];

            box->content = malloc(sizeof(char *) * count);
            if (box->content == NULL) {
                return -1;
            }

            for (int j = 0; j < count; j++) {
                box->content[j] = strdup(lines[j]);
                if (box->content[j] == NULL) {
                    /* Cleanup on failure */
                    for (int k = 0; k < j; k++) {
                        free(box->content[k]);
                    }
                    free(box->content);
                    box->content = NULL;
                    return -1;
                }
            }
            box->content_lines = count;
            return 0;
        }
    }
    return -1;  /* Box not found */
}

/* Remove a box from canvas by ID */
int canvas_remove_box(Canvas *canvas, int box_id) {
    for (int i = 0; i < canvas->box_count; i++) {
        if (canvas->boxes[i].id == box_id) {
            Box *box = &canvas->boxes[i];

            /* Remove any connections involving this box (Issue #20) */
            canvas_remove_box_connections(canvas, box_id);

            /* Free box memory */
            if (box->title) {
                free(box->title);
            }
            if (box->content) {
                for (int j = 0; j < box->content_lines; j++) {
                    free(box->content[j]);
                }
                free(box->content);
            }
            /* Free content source fields (Issue #54) */
            if (box->file_path) {
                free(box->file_path);
            }
            if (box->command) {
                free(box->command);
            }

            /* Shift remaining boxes down */
            for (int j = i; j < canvas->box_count - 1; j++) {
                canvas->boxes[j] = canvas->boxes[j + 1];
            }

            canvas->box_count--;

            /* Update selected index if needed */
            if (canvas->selected_index == i) {
                canvas->selected_index = -1;
            } else if (canvas->selected_index > i) {
                canvas->selected_index--;
            }

            return 0;
        }
    }
    return -1;  /* Box not found */
}

/* Get box by ID (returns NULL if not found) */
Box* canvas_get_box(Canvas *canvas, int box_id) {
    for (int i = 0; i < canvas->box_count; i++) {
        if (canvas->boxes[i].id == box_id) {
            return &canvas->boxes[i];
        }
    }
    return NULL;
}

/* Get box by index (returns NULL if out of bounds) */
Box* canvas_get_box_at(Canvas *canvas, int index) {
    if (index < 0 || index >= canvas->box_count) {
        return NULL;
    }
    return &canvas->boxes[index];
}

/* Find box at world coordinates (returns box ID, or -1 if none found) */
int canvas_find_box_at(Canvas *canvas, double x, double y) {
    /* Search in reverse order to find topmost box */
    for (int i = canvas->box_count - 1; i >= 0; i--) {
        Box *box = &canvas->boxes[i];
        if (x >= box->x && x <= box->x + box->width &&
            y >= box->y && y <= box->y + box->height) {
            return box->id;
        }
    }
    return -1;
}

/* Select a box by ID */
void canvas_select_box(Canvas *canvas, int box_id) {
    /* Deselect current box */
    if (canvas->selected_index >= 0 && canvas->selected_index < canvas->box_count) {
        canvas->boxes[canvas->selected_index].selected = false;
    }

    /* Find and select new box */
    for (int i = 0; i < canvas->box_count; i++) {
        if (canvas->boxes[i].id == box_id) {
            canvas->boxes[i].selected = true;
            canvas->selected_index = i;
            return;
        }
    }

    /* Box not found, deselect */
    canvas->selected_index = -1;
}

/* Deselect current box */
void canvas_deselect(Canvas *canvas) {
    if (canvas->selected_index >= 0 && canvas->selected_index < canvas->box_count) {
        canvas->boxes[canvas->selected_index].selected = false;
    }
    canvas->selected_index = -1;
}

/* Get currently selected box (returns NULL if none) */
Box* canvas_get_selected(Canvas *canvas) {
    if (canvas->selected_index >= 0 && canvas->selected_index < canvas->box_count) {
        return &canvas->boxes[canvas->selected_index];
    }
    return NULL;
}

/* Snap box position to grid (Phase 4) */
void canvas_snap_box_to_grid(Canvas *canvas, Box *box) {
    if (!canvas || !box || !canvas->grid.snap_enabled || canvas->grid.spacing <= 0) {
        return;
    }

    /* Snap position to nearest grid point */
    box->x = round(box->x / canvas->grid.spacing) * canvas->grid.spacing;
    box->y = round(box->y / canvas->grid.spacing) * canvas->grid.spacing;
}

/* Calculate proportional dimensions based on nearby boxes (Issue #18) */
int canvas_calc_proportional_size(const Canvas *canvas, double x, double y,
                                  int proximity_radius, bool use_nearest,
                                  int min_neighbors,
                                  int default_width, int default_height,
                                  int *out_width, int *out_height) {
    if (!canvas || !out_width || !out_height) {
        if (out_width) *out_width = default_width;
        if (out_height) *out_height = default_height;
        return 0;
    }

    /* Initialize with defaults */
    *out_width = default_width;
    *out_height = default_height;

    /* If no boxes exist, use defaults */
    if (canvas->box_count == 0) {
        return 0;
    }

    /* Find neighbors within proximity radius */
    double radius_sq = (double)proximity_radius * proximity_radius;
    int neighbor_count = 0;
    int total_width = 0;
    int total_height = 0;
    double nearest_dist_sq = -1.0;
    int nearest_width = default_width;
    int nearest_height = default_height;

    for (int i = 0; i < canvas->box_count; i++) {
        const Box *box = &canvas->boxes[i];

        /* Calculate distance from new box position to box center */
        double box_center_x = box->x + box->width / 2.0;
        double box_center_y = box->y + box->height / 2.0;
        double dx = x - box_center_x;
        double dy = y - box_center_y;
        double dist_sq = dx * dx + dy * dy;

        if (dist_sq <= radius_sq) {
            neighbor_count++;
            total_width += box->width;
            total_height += box->height;

            /* Track nearest neighbor */
            if (nearest_dist_sq < 0.0 || dist_sq < nearest_dist_sq) {
                nearest_dist_sq = dist_sq;
                nearest_width = box->width;
                nearest_height = box->height;
            }
        }
    }

    /* Check if we have enough neighbors */
    if (neighbor_count < min_neighbors) {
        return 0;
    }

    /* Calculate dimensions based on mode */
    if (use_nearest) {
        /* Use nearest neighbor's dimensions */
        *out_width = nearest_width;
        *out_height = nearest_height;
    } else {
        /* Use average of all neighbors */
        *out_width = (total_width + neighbor_count / 2) / neighbor_count;  /* Rounded division */
        *out_height = (total_height + neighbor_count / 2) / neighbor_count;
    }

    /* Apply bounds (matching template limits from config.c) */
    if (*out_width < 10) *out_width = 10;
    if (*out_width > 80) *out_width = 80;
    if (*out_height < 3) *out_height = 3;
    if (*out_height > 30) *out_height = 30;

    return neighbor_count;
}

/* ============================================================
 * Connection Management Functions (Issue #20)
 * ============================================================ */

/* Grow the connection array if needed */
static int canvas_ensure_conn_capacity(Canvas *canvas) {
    if (canvas->conn_count >= canvas->conn_capacity) {
        int new_capacity = canvas->conn_capacity * 2;
        Connection *new_conns = realloc(canvas->connections, sizeof(Connection) * new_capacity);
        if (new_conns == NULL) {
            return -1;
        }
        canvas->connections = new_conns;
        canvas->conn_capacity = new_capacity;
    }
    return 0;
}

/* Add a connection between two boxes (returns connection ID, or -1 on error) */
int canvas_add_connection(Canvas *canvas, int source_id, int dest_id) {
    if (!canvas) return -1;

    /* Validate source and destination boxes exist */
    Box *source = canvas_get_box(canvas, source_id);
    Box *dest = canvas_get_box(canvas, dest_id);
    if (!source || !dest) return -1;

    /* Don't allow self-connections */
    if (source_id == dest_id) return -1;

    /* Check if connection already exists (in either direction) */
    if (canvas_find_connection(canvas, source_id, dest_id) >= 0) return -1;
    if (canvas_find_connection(canvas, dest_id, source_id) >= 0) return -1;

    /* Ensure capacity */
    if (canvas_ensure_conn_capacity(canvas) != 0) {
        return -1;
    }

    /* Add the connection */
    Connection *conn = &canvas->connections[canvas->conn_count];
    conn->id = canvas->next_conn_id++;
    conn->source_id = source_id;
    conn->dest_id = dest_id;
    conn->color = CONNECTION_COLOR_DEFAULT;

    canvas->conn_count++;

    return conn->id;
}

/* Remove a connection by ID (returns 0 on success, -1 if not found) */
int canvas_remove_connection(Canvas *canvas, int conn_id) {
    if (!canvas) return -1;

    for (int i = 0; i < canvas->conn_count; i++) {
        if (canvas->connections[i].id == conn_id) {
            /* Shift remaining connections down */
            for (int j = i; j < canvas->conn_count - 1; j++) {
                canvas->connections[j] = canvas->connections[j + 1];
            }
            canvas->conn_count--;
            return 0;
        }
    }
    return -1;  /* Connection not found */
}

/* Get connection by ID (returns NULL if not found) */
Connection* canvas_get_connection(Canvas *canvas, int conn_id) {
    if (!canvas) return NULL;

    for (int i = 0; i < canvas->conn_count; i++) {
        if (canvas->connections[i].id == conn_id) {
            return &canvas->connections[i];
        }
    }
    return NULL;
}

/* Find connection between two boxes (returns connection ID, or -1 if none) */
int canvas_find_connection(const Canvas *canvas, int source_id, int dest_id) {
    if (!canvas) return -1;

    for (int i = 0; i < canvas->conn_count; i++) {
        const Connection *conn = &canvas->connections[i];
        if (conn->source_id == source_id && conn->dest_id == dest_id) {
            return conn->id;
        }
    }
    return -1;
}

/* Get all connections involving a specific box (fills array, returns count) */
int canvas_get_box_connections(const Canvas *canvas, int box_id, int *conn_ids, int max_count) {
    if (!canvas || !conn_ids || max_count <= 0) return 0;

    int count = 0;
    for (int i = 0; i < canvas->conn_count && count < max_count; i++) {
        const Connection *conn = &canvas->connections[i];
        if (conn->source_id == box_id || conn->dest_id == box_id) {
            conn_ids[count++] = conn->id;
        }
    }
    return count;
}

/* Remove all connections involving a specific box */
void canvas_remove_box_connections(Canvas *canvas, int box_id) {
    if (!canvas) return;

    /* Remove connections in reverse order to avoid index issues */
    for (int i = canvas->conn_count - 1; i >= 0; i--) {
        const Connection *conn = &canvas->connections[i];
        if (conn->source_id == box_id || conn->dest_id == box_id) {
            canvas_remove_connection(canvas, conn->id);
        }
    }
}

/* Enter connection mode (sets source box) */
void canvas_start_connection(Canvas *canvas, int source_box_id) {
    if (!canvas) return;

    /* Verify the source box exists */
    Box *box = canvas_get_box(canvas, source_box_id);
    if (!box) return;

    canvas->conn_mode.active = true;
    canvas->conn_mode.source_box_id = source_box_id;
    canvas->conn_mode.pending_delete = false;
    canvas->conn_mode.delete_conn_id = -1;
}

/* Complete or cancel connection mode */
void canvas_finish_connection(Canvas *canvas, int dest_box_id) {
    if (!canvas || !canvas->conn_mode.active) return;

    /* If destination is -1 or same as source, cancel */
    if (dest_box_id < 0 || dest_box_id == canvas->conn_mode.source_box_id) {
        canvas_cancel_connection(canvas);
        return;
    }

    /* Try to create the connection */
    canvas_add_connection(canvas, canvas->conn_mode.source_box_id, dest_box_id);

    /* Reset connection mode regardless of success */
    canvas->conn_mode.active = false;
    canvas->conn_mode.source_box_id = -1;
}

/* Cancel connection mode without creating connection */
void canvas_cancel_connection(Canvas *canvas) {
    if (!canvas) return;

    canvas->conn_mode.active = false;
    canvas->conn_mode.source_box_id = -1;
    canvas->conn_mode.pending_delete = false;
    canvas->conn_mode.delete_conn_id = -1;
}

/* Check if in connection mode */
bool canvas_in_connection_mode(const Canvas *canvas) {
    return canvas && canvas->conn_mode.active;
}
