#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "undo.h"
#include "canvas.h"

/* ============================================================
 * Helper Functions
 * ============================================================ */

/* Safe string duplication */
static char *safe_strdup(const char *s) {
    if (s == NULL) return NULL;
    return strdup(s);
}

/* Deep copy content array */
static char **copy_content(char **content, int count) {
    if (content == NULL || count <= 0) return NULL;

    char **copy = malloc(sizeof(char *) * count);
    if (copy == NULL) return NULL;

    for (int i = 0; i < count; i++) {
        copy[i] = safe_strdup(content[i]);
        if (content[i] != NULL && copy[i] == NULL) {
            /* Cleanup on failure */
            for (int j = 0; j < i; j++) {
                free(copy[j]);
            }
            free(copy);
            return NULL;
        }
    }
    return copy;
}

/* Free content array */
static void free_content(char **content, int count) {
    if (content == NULL) return;
    for (int i = 0; i < count; i++) {
        free(content[i]);
    }
    free(content);
}

/* Create a snapshot of a box */
static void snapshot_box(BoxSnapshot *snap, const Box *box) {
    snap->id = box->id;
    snap->x = box->x;
    snap->y = box->y;
    snap->width = box->width;
    snap->height = box->height;
    snap->title = safe_strdup(box->title);
    snap->content = copy_content(box->content, box->content_lines);
    snap->content_lines = box->content_lines;
    snap->color = box->color;
    snap->box_type = box->box_type;
    snap->content_type = box->content_type;
    snap->file_path = safe_strdup(box->file_path);
    snap->command = safe_strdup(box->command);
}

/* Free a box snapshot */
static void free_box_snapshot(BoxSnapshot *snap) {
    free(snap->title);
    free_content(snap->content, snap->content_lines);
    free(snap->file_path);
    free(snap->command);
    /* Zero out to prevent double-free */
    memset(snap, 0, sizeof(BoxSnapshot));
}

/* Create a snapshot of a connection */
static void snapshot_connection(ConnectionSnapshot *snap, const Connection *conn) {
    snap->id = conn->id;
    snap->source_id = conn->source_id;
    snap->dest_id = conn->dest_id;
    snap->color = conn->color;
}

/* Free a single operation */
static void free_operation(Operation *op) {
    if (op == NULL) return;

    /* Free box snapshots based on operation type */
    switch (op->type) {
        case OP_BOX_CREATE:
            free_box_snapshot(&op->after.box_after);
            break;
        case OP_BOX_DELETE:
            free_box_snapshot(&op->before.box_before);
            break;
        case OP_BOX_MOVE:
        case OP_BOX_RESIZE:
        case OP_BOX_COLOR:
            /* These only store position/size/color, no pointers */
            break;
        case OP_BOX_TITLE:
            free(op->before.box_before.title);
            free(op->after.box_after.title);
            break;
        case OP_BOX_CONTENT:
            free_content(op->before.box_before.content, op->before.box_before.content_lines);
            free_content(op->after.box_after.content, op->after.box_after.content_lines);
            break;
        case OP_CONNECTION_CREATE:
        case OP_CONNECTION_DELETE:
            /* ConnectionSnapshot has no pointers */
            break;
    }

    free(op);
}

/* Free the redo chain */
static void free_redo_chain(UndoStack *stack) {
    Operation *op = stack->redo_head;
    while (op != NULL) {
        Operation *next = op->next;
        free_operation(op);
        op = next;
    }
    stack->redo_head = NULL;
}

/* Trim undo stack to max size */
static void trim_undo_stack(UndoStack *stack) {
    while (stack->size > stack->max_size && stack->current != NULL) {
        /* Find the oldest operation */
        Operation *oldest = stack->current;
        while (oldest->prev != NULL) {
            oldest = oldest->prev;
        }

        /* Remove it */
        if (oldest->next != NULL) {
            oldest->next->prev = NULL;
        }
        free_operation(oldest);
        stack->size--;
    }
}

/* Push a new operation onto the undo stack */
static Operation* push_operation(Canvas *canvas, OpType type, int box_id, int conn_id) {
    UndoStack *stack = &canvas->undo_stack;

    /* When a new operation is recorded, discard the redo chain */
    free_redo_chain(stack);

    Operation *op = malloc(sizeof(Operation));
    if (op == NULL) return NULL;

    memset(op, 0, sizeof(Operation));
    op->type = type;
    op->box_id = box_id;
    op->conn_id = conn_id;

    /* Link to existing chain */
    op->prev = stack->current;
    op->next = NULL;

    if (stack->current != NULL) {
        stack->current->next = op;
    }

    stack->current = op;
    stack->size++;

    /* Trim if we exceed max size */
    trim_undo_stack(stack);

    return op;
}

/* ============================================================
 * Stack Management
 * ============================================================ */

void undo_stack_init(UndoStack *stack) {
    stack->current = NULL;
    stack->redo_head = NULL;
    stack->size = 0;
    stack->max_size = UNDO_STACK_MAX_SIZE;
}

void undo_stack_cleanup(UndoStack *stack) {
    /* Free redo chain */
    free_redo_chain(stack);

    /* Free undo chain */
    Operation *op = stack->current;
    while (op != NULL) {
        Operation *prev = op->prev;
        free_operation(op);
        op = prev;
    }

    stack->current = NULL;
    stack->size = 0;
}

/* ============================================================
 * Recording Operations
 * ============================================================ */

void undo_record_box_create(Canvas *canvas, int box_id) {
    Box *box = canvas_get_box(canvas, box_id);
    if (box == NULL) return;

    Operation *op = push_operation(canvas, OP_BOX_CREATE, box_id, -1);
    if (op == NULL) return;

    /* Store the created box state for redo */
    snapshot_box(&op->after.box_after, box);
}

void undo_record_box_delete(Canvas *canvas, int box_id) {
    Box *box = canvas_get_box(canvas, box_id);
    if (box == NULL) return;

    Operation *op = push_operation(canvas, OP_BOX_DELETE, box_id, -1);
    if (op == NULL) return;

    /* Store the box state before deletion for undo */
    snapshot_box(&op->before.box_before, box);
}

void undo_record_box_move(Canvas *canvas, int box_id,
                          double old_x, double old_y,
                          double new_x, double new_y) {
    Operation *op = push_operation(canvas, OP_BOX_MOVE, box_id, -1);
    if (op == NULL) return;

    op->before.box_before.id = box_id;
    op->before.box_before.x = old_x;
    op->before.box_before.y = old_y;

    op->after.box_after.id = box_id;
    op->after.box_after.x = new_x;
    op->after.box_after.y = new_y;
}

void undo_record_box_resize(Canvas *canvas, int box_id,
                            int old_width, int old_height,
                            int new_width, int new_height) {
    Operation *op = push_operation(canvas, OP_BOX_RESIZE, box_id, -1);
    if (op == NULL) return;

    op->before.box_before.id = box_id;
    op->before.box_before.width = old_width;
    op->before.box_before.height = old_height;

    op->after.box_after.id = box_id;
    op->after.box_after.width = new_width;
    op->after.box_after.height = new_height;
}

void undo_record_box_title(Canvas *canvas, int box_id,
                           const char *old_title, const char *new_title) {
    Operation *op = push_operation(canvas, OP_BOX_TITLE, box_id, -1);
    if (op == NULL) return;

    op->before.box_before.id = box_id;
    op->before.box_before.title = safe_strdup(old_title);

    op->after.box_after.id = box_id;
    op->after.box_after.title = safe_strdup(new_title);
}

void undo_record_box_color(Canvas *canvas, int box_id,
                           int old_color, int new_color) {
    Operation *op = push_operation(canvas, OP_BOX_COLOR, box_id, -1);
    if (op == NULL) return;

    op->before.box_before.id = box_id;
    op->before.box_before.color = old_color;

    op->after.box_after.id = box_id;
    op->after.box_after.color = new_color;
}

void undo_record_connection_create(Canvas *canvas, int conn_id) {
    Connection *conn = canvas_get_connection(canvas, conn_id);
    if (conn == NULL) return;

    Operation *op = push_operation(canvas, OP_CONNECTION_CREATE, -1, conn_id);
    if (op == NULL) return;

    snapshot_connection(&op->after.conn_after, conn);
}

void undo_record_connection_delete(Canvas *canvas, int conn_id) {
    Connection *conn = canvas_get_connection(canvas, conn_id);
    if (conn == NULL) return;

    Operation *op = push_operation(canvas, OP_CONNECTION_DELETE, -1, conn_id);
    if (op == NULL) return;

    snapshot_connection(&op->before.conn_before, conn);
}

/* ============================================================
 * Undo/Redo Execution
 * ============================================================ */

/* Restore a box from snapshot (used by both undo delete and redo create) */
static int restore_box_from_snapshot(Canvas *canvas, const BoxSnapshot *snap) {
    /* Add the box back */
    int new_id = canvas_add_box(canvas, snap->x, snap->y,
                                snap->width, snap->height, snap->title);
    if (new_id < 0) return -1;

    Box *box = canvas_get_box(canvas, new_id);
    if (box == NULL) return -1;

    /* Restore additional properties */
    box->color = snap->color;
    box->box_type = snap->box_type;
    box->content_type = snap->content_type;

    /* Restore content */
    if (snap->content != NULL && snap->content_lines > 0) {
        canvas_add_box_content(canvas, new_id,
                               (const char **)snap->content, snap->content_lines);
    }

    /* Restore file_path and command */
    if (snap->file_path) {
        box->file_path = safe_strdup(snap->file_path);
    }
    if (snap->command) {
        box->command = safe_strdup(snap->command);
    }

    return new_id;
}

bool canvas_undo(Canvas *canvas) {
    if (canvas == NULL) return false;

    UndoStack *stack = &canvas->undo_stack;
    if (stack->current == NULL) return false;

    Operation *op = stack->current;

    switch (op->type) {
        case OP_BOX_CREATE: {
            /* Undo create = delete the box */
            canvas_remove_box(canvas, op->box_id);
            break;
        }

        case OP_BOX_DELETE: {
            /* Undo delete = recreate the box */
            restore_box_from_snapshot(canvas, &op->before.box_before);
            break;
        }

        case OP_BOX_MOVE: {
            /* Undo move = restore old position */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->x = op->before.box_before.x;
                box->y = op->before.box_before.y;
            }
            break;
        }

        case OP_BOX_RESIZE: {
            /* Undo resize = restore old dimensions */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->width = op->before.box_before.width;
                box->height = op->before.box_before.height;
            }
            break;
        }

        case OP_BOX_TITLE: {
            /* Undo title change = restore old title */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                free(box->title);
                box->title = safe_strdup(op->before.box_before.title);
            }
            break;
        }

        case OP_BOX_CONTENT: {
            /* Undo content change = restore old content */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                free_content(box->content, box->content_lines);
                box->content = copy_content(op->before.box_before.content,
                                            op->before.box_before.content_lines);
                box->content_lines = op->before.box_before.content_lines;
            }
            break;
        }

        case OP_BOX_COLOR: {
            /* Undo color change = restore old color */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->color = op->before.box_before.color;
            }
            break;
        }

        case OP_CONNECTION_CREATE: {
            /* Undo create connection = remove connection */
            canvas_remove_connection(canvas, op->conn_id);
            break;
        }

        case OP_CONNECTION_DELETE: {
            /* Undo delete connection = recreate it */
            ConnectionSnapshot *snap = &op->before.conn_before;
            canvas_add_connection(canvas, snap->source_id, snap->dest_id);
            break;
        }
    }

    /* Move current operation to redo chain */
    stack->current = op->prev;
    if (stack->current) {
        stack->current->next = NULL;
    }
    stack->size--;

    /* Add to redo chain */
    op->prev = NULL;
    op->next = stack->redo_head;
    if (stack->redo_head) {
        stack->redo_head->prev = op;
    }
    stack->redo_head = op;

    return true;
}

bool canvas_redo(Canvas *canvas) {
    if (canvas == NULL) return false;

    UndoStack *stack = &canvas->undo_stack;
    if (stack->redo_head == NULL) return false;

    Operation *op = stack->redo_head;

    switch (op->type) {
        case OP_BOX_CREATE: {
            /* Redo create = recreate the box */
            restore_box_from_snapshot(canvas, &op->after.box_after);
            break;
        }

        case OP_BOX_DELETE: {
            /* Redo delete = delete the box again */
            canvas_remove_box(canvas, op->box_id);
            break;
        }

        case OP_BOX_MOVE: {
            /* Redo move = apply new position */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->x = op->after.box_after.x;
                box->y = op->after.box_after.y;
            }
            break;
        }

        case OP_BOX_RESIZE: {
            /* Redo resize = apply new dimensions */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->width = op->after.box_after.width;
                box->height = op->after.box_after.height;
            }
            break;
        }

        case OP_BOX_TITLE: {
            /* Redo title change = apply new title */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                free(box->title);
                box->title = safe_strdup(op->after.box_after.title);
            }
            break;
        }

        case OP_BOX_CONTENT: {
            /* Redo content change = apply new content */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                free_content(box->content, box->content_lines);
                box->content = copy_content(op->after.box_after.content,
                                            op->after.box_after.content_lines);
                box->content_lines = op->after.box_after.content_lines;
            }
            break;
        }

        case OP_BOX_COLOR: {
            /* Redo color change = apply new color */
            Box *box = canvas_get_box(canvas, op->box_id);
            if (box) {
                box->color = op->after.box_after.color;
            }
            break;
        }

        case OP_CONNECTION_CREATE: {
            /* Redo create connection = create it again */
            ConnectionSnapshot *snap = &op->after.conn_after;
            canvas_add_connection(canvas, snap->source_id, snap->dest_id);
            break;
        }

        case OP_CONNECTION_DELETE: {
            /* Redo delete connection = remove it again */
            canvas_remove_connection(canvas, op->conn_id);
            break;
        }
    }

    /* Remove from redo chain */
    stack->redo_head = op->next;
    if (stack->redo_head) {
        stack->redo_head->prev = NULL;
    }

    /* Add back to undo chain */
    op->next = NULL;
    op->prev = stack->current;
    if (stack->current) {
        stack->current->next = op;
    }
    stack->current = op;
    stack->size++;

    return true;
}

bool canvas_can_undo(const Canvas *canvas) {
    return canvas != NULL && canvas->undo_stack.current != NULL;
}

bool canvas_can_redo(const Canvas *canvas) {
    return canvas != NULL && canvas->undo_stack.redo_head != NULL;
}

/* Operation type descriptions */
static const char* op_type_descriptions[] = {
    "create box",
    "delete box",
    "move box",
    "resize box",
    "change content",
    "change title",
    "change color",
    "create connection",
    "delete connection"
};

const char* canvas_get_undo_description(const Canvas *canvas) {
    if (!canvas_can_undo(canvas)) return NULL;
    return op_type_descriptions[canvas->undo_stack.current->type];
}

const char* canvas_get_redo_description(const Canvas *canvas) {
    if (!canvas_can_redo(canvas)) return NULL;
    return op_type_descriptions[canvas->undo_stack.redo_head->type];
}
