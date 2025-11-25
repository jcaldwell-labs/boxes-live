#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "canvas.h"

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
    canvas->grid.spacing = 10;  /* Default: 10 world units */

    /* Initialize focus mode state (Phase 5b) */
    canvas->focus.active = false;
    canvas->focus.focused_box_id = -1;
    canvas->focus.scroll_offset = 0;
    canvas->focus.scroll_max = 0;

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
    }

    if (canvas->boxes) {
        free(canvas->boxes);
        canvas->boxes = NULL;
    }
    canvas->box_count = 0;
    canvas->box_capacity = 0;
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
