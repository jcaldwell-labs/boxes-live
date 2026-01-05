#ifndef EDITOR_H
#define EDITOR_H

#include "types.h"

/* ============================================================
 * Text Editing Functions (Issue #79)
 * ============================================================ */

/* Initialize editor state */
void editor_init(TextEditor *editor);

/* Cleanup editor state (frees original string) */
void editor_cleanup(TextEditor *editor);

/* Start editing a box's title */
int editor_start_title(Canvas *canvas, int box_id);

/* Cancel editing and restore original value */
void editor_cancel(Canvas *canvas);

/* Confirm editing and apply changes */
int editor_confirm(Canvas *canvas);

/* Insert a character at cursor position */
int editor_insert_char(TextEditor *editor, char c);

/* Delete character before cursor (backspace) */
int editor_backspace(TextEditor *editor);

/* Delete character at cursor (delete key) */
int editor_delete(TextEditor *editor);

/* Move cursor left */
void editor_cursor_left(TextEditor *editor);

/* Move cursor right */
void editor_cursor_right(TextEditor *editor);

/* Move cursor to start of line */
void editor_cursor_home(TextEditor *editor);

/* Move cursor to end of line */
void editor_cursor_end(TextEditor *editor);

/* Check if editor is active */
bool editor_is_active(const Canvas *canvas);

/* Get the current edit buffer for rendering */
const char* editor_get_buffer(const TextEditor *editor);

/* Get cursor position for rendering */
int editor_get_cursor_pos(const TextEditor *editor);

#endif /* EDITOR_H */
