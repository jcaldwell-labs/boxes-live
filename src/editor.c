#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "canvas.h"
#include "undo.h"

/* ============================================================
 * Editor Initialization and Cleanup
 * ============================================================ */

void editor_init(TextEditor *editor) {
    if (!editor) return;

    editor->active = false;
    editor->target = EDIT_NONE;
    editor->box_id = -1;
    editor->buffer[0] = '\0';
    editor->cursor_pos = 0;
    editor->length = 0;
    editor->original = NULL;
}

void editor_cleanup(TextEditor *editor) {
    if (!editor) return;

    if (editor->original) {
        free(editor->original);
        editor->original = NULL;
    }

    editor->active = false;
    editor->target = EDIT_NONE;
    editor->box_id = -1;
    editor->buffer[0] = '\0';
    editor->cursor_pos = 0;
    editor->length = 0;
}

/* ============================================================
 * Edit Mode Management
 * ============================================================ */

int editor_start_title(Canvas *canvas, int box_id) {
    if (!canvas) return -1;

    Box *box = canvas_get_box(canvas, box_id);
    if (!box) return -1;

    TextEditor *editor = &canvas->editor;

    /* Clean up any previous edit state */
    editor_cleanup(editor);

    /* Save original value for cancel */
    if (box->title) {
        editor->original = strdup(box->title);
    } else {
        editor->original = strdup("");
    }

    if (!editor->original) return -1;

    /* Initialize edit buffer with current title */
    size_t title_len = box->title ? strlen(box->title) : 0;
    if (title_len >= MAX_TITLE_LENGTH) {
        title_len = MAX_TITLE_LENGTH - 1;
    }

    if (box->title) {
        strncpy(editor->buffer, box->title, MAX_TITLE_LENGTH - 1);
        editor->buffer[MAX_TITLE_LENGTH - 1] = '\0';
    } else {
        editor->buffer[0] = '\0';
    }

    /* Cast is safe: buffer is capped at MAX_TITLE_LENGTH (256), well within int range */
    editor->length = (int)strlen(editor->buffer);
    editor->cursor_pos = editor->length;  /* Start at end */
    editor->box_id = box_id;
    editor->target = EDIT_TITLE;
    editor->active = true;

    return 0;
}

void editor_cancel(Canvas *canvas) {
    if (!canvas) return;

    TextEditor *editor = &canvas->editor;
    if (!editor->active) return;

    /* Restore original value if editing title */
    if (editor->target == EDIT_TITLE && editor->original) {
        Box *box = canvas_get_box(canvas, editor->box_id);
        if (box) {
            char *restored = strdup(editor->original);
            if (restored) {
                free(box->title);
                box->title = restored;
            }
            /* On strdup failure, keep current title rather than setting NULL */
        }
    }

    editor_cleanup(editor);
}

int editor_confirm(Canvas *canvas) {
    if (!canvas) return -1;

    TextEditor *editor = &canvas->editor;
    if (!editor->active) return -1;

    if (editor->target == EDIT_TITLE) {
        Box *box = canvas_get_box(canvas, editor->box_id);
        if (box) {
            /* Allocate new title first to ensure we don't lose the old one on failure */
            char *new_title = strdup(editor->buffer);
            if (!new_title) {
                /* Memory allocation failed - keep old title, return error */
                editor_cleanup(editor);
                return -1;
            }

            /* Record for undo before changing */
            undo_record_box_title(canvas, editor->box_id,
                                  editor->original, editor->buffer);

            /* Apply the new title */
            free(box->title);
            box->title = new_title;
        }
    }

    editor_cleanup(editor);
    return 0;
}

/* ============================================================
 * Text Manipulation
 * ============================================================ */

int editor_insert_char(TextEditor *editor, char c) {
    if (!editor || !editor->active) return -1;
    if (editor->length >= MAX_TITLE_LENGTH - 1) return -1;

    /* Make room for new character */
    memmove(&editor->buffer[editor->cursor_pos + 1],
            &editor->buffer[editor->cursor_pos],
            editor->length - editor->cursor_pos + 1);  /* +1 for null terminator */

    editor->buffer[editor->cursor_pos] = c;
    editor->cursor_pos++;
    editor->length++;

    return 0;
}

int editor_backspace(TextEditor *editor) {
    if (!editor || !editor->active) return -1;
    if (editor->cursor_pos <= 0) return -1;

    /* Remove character before cursor */
    memmove(&editor->buffer[editor->cursor_pos - 1],
            &editor->buffer[editor->cursor_pos],
            editor->length - editor->cursor_pos + 1);

    editor->cursor_pos--;
    editor->length--;

    return 0;
}

int editor_delete(TextEditor *editor) {
    if (!editor || !editor->active) return -1;
    if (editor->cursor_pos >= editor->length) return -1;

    /* Remove character at cursor */
    memmove(&editor->buffer[editor->cursor_pos],
            &editor->buffer[editor->cursor_pos + 1],
            editor->length - editor->cursor_pos);

    editor->length--;

    return 0;
}

/* ============================================================
 * Cursor Movement
 * ============================================================ */

void editor_cursor_left(TextEditor *editor) {
    if (!editor || !editor->active) return;
    if (editor->cursor_pos > 0) {
        editor->cursor_pos--;
    }
}

void editor_cursor_right(TextEditor *editor) {
    if (!editor || !editor->active) return;
    if (editor->cursor_pos < editor->length) {
        editor->cursor_pos++;
    }
}

void editor_cursor_home(TextEditor *editor) {
    if (!editor || !editor->active) return;
    editor->cursor_pos = 0;
}

void editor_cursor_end(TextEditor *editor) {
    if (!editor || !editor->active) return;
    editor->cursor_pos = editor->length;
}

/* ============================================================
 * Utility Functions
 * ============================================================ */

bool editor_is_active(const Canvas *canvas) {
    return canvas && canvas->editor.active;
}

const char* editor_get_buffer(const TextEditor *editor) {
    if (!editor) return "";
    return editor->buffer;
}

int editor_get_cursor_pos(const TextEditor *editor) {
    if (!editor) return 0;
    return editor->cursor_pos;
}
