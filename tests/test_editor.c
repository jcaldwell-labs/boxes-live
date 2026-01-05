#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/editor.h"
#include "../include/undo.h"
#include "../include/types.h"

int main(void) {
    TEST_START();

    TEST("Editor initialization in canvas") {
        Canvas canvas;
        int result = canvas_init(&canvas, 200.0, 100.0);

        ASSERT_EQ(result, 0, "canvas_init returns 0");
        ASSERT(!editor_is_active(&canvas), "Editor not active on fresh canvas");
        ASSERT_EQ(canvas.editor.target, EDIT_NONE, "No edit target");
        ASSERT_EQ(canvas.editor.box_id, -1, "No box being edited");

        canvas_cleanup(&canvas);
    }

    TEST("Start title editing") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Original Title");
        ASSERT(id > 0, "Box created");

        int result = editor_start_title(&canvas, id);
        ASSERT_EQ(result, 0, "Start edit succeeded");
        ASSERT(editor_is_active(&canvas), "Editor is active");
        ASSERT_EQ(canvas.editor.target, EDIT_TITLE, "Editing title");
        ASSERT_EQ(canvas.editor.box_id, id, "Editing correct box");
        ASSERT(strcmp(canvas.editor.buffer, "Original Title") == 0, "Buffer has title");
        ASSERT_EQ(canvas.editor.cursor_pos, 14, "Cursor at end");

        canvas_cleanup(&canvas);
    }

    TEST("Insert character") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test");
        editor_start_title(&canvas, id);

        /* Cursor is at end */
        editor_insert_char(&canvas.editor, '!');
        ASSERT(strcmp(canvas.editor.buffer, "Test!") == 0, "Appended character");
        ASSERT_EQ(canvas.editor.cursor_pos, 5, "Cursor moved forward");

        canvas_cleanup(&canvas);
    }

    TEST("Insert character in middle") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "AC");
        editor_start_title(&canvas, id);

        /* Move cursor to position 1 */
        canvas.editor.cursor_pos = 1;
        editor_insert_char(&canvas.editor, 'B');

        ASSERT(strcmp(canvas.editor.buffer, "ABC") == 0, "Inserted in middle");
        ASSERT_EQ(canvas.editor.cursor_pos, 2, "Cursor after inserted char");

        canvas_cleanup(&canvas);
    }

    TEST("Backspace") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test!");
        editor_start_title(&canvas, id);

        editor_backspace(&canvas.editor);
        ASSERT(strcmp(canvas.editor.buffer, "Test") == 0, "Removed last char");
        ASSERT_EQ(canvas.editor.cursor_pos, 4, "Cursor moved back");

        canvas_cleanup(&canvas);
    }

    TEST("Delete at cursor") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "ABCD");
        editor_start_title(&canvas, id);

        canvas.editor.cursor_pos = 1;  /* Cursor at 'B' */
        editor_delete(&canvas.editor);

        ASSERT(strcmp(canvas.editor.buffer, "ACD") == 0, "Deleted at cursor");
        ASSERT_EQ(canvas.editor.cursor_pos, 1, "Cursor stayed in place");

        canvas_cleanup(&canvas);
    }

    TEST("Cursor movement") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Hello");
        editor_start_title(&canvas, id);

        ASSERT_EQ(canvas.editor.cursor_pos, 5, "Cursor at end");

        editor_cursor_left(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 4, "Moved left");

        editor_cursor_home(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 0, "At home");

        editor_cursor_left(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 0, "Stayed at 0");

        editor_cursor_right(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 1, "Moved right");

        editor_cursor_end(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 5, "At end");

        editor_cursor_right(&canvas.editor);
        ASSERT_EQ(canvas.editor.cursor_pos, 5, "Stayed at end");

        canvas_cleanup(&canvas);
    }

    TEST("Cancel editing restores original") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Original");
        editor_start_title(&canvas, id);

        /* Modify the buffer */
        editor_insert_char(&canvas.editor, '!');
        ASSERT(strcmp(canvas.editor.buffer, "Original!") == 0, "Buffer modified");

        /* Cancel */
        editor_cancel(&canvas);

        /* Check box title is restored */
        Box *box = canvas_get_box(&canvas, id);
        ASSERT(strcmp(box->title, "Original") == 0, "Title restored");
        ASSERT(!editor_is_active(&canvas), "Editor deactivated");

        canvas_cleanup(&canvas);
    }

    TEST("Confirm editing applies changes") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Original");
        editor_start_title(&canvas, id);

        /* Modify the buffer */
        editor_cursor_home(&canvas.editor);
        editor_insert_char(&canvas.editor, 'N');
        editor_insert_char(&canvas.editor, 'e');
        editor_insert_char(&canvas.editor, 'w');
        editor_insert_char(&canvas.editor, ' ');

        /* Confirm */
        editor_confirm(&canvas);

        /* Check box title is changed */
        Box *box = canvas_get_box(&canvas, id);
        ASSERT(strcmp(box->title, "New Original") == 0, "Title changed");
        ASSERT(!editor_is_active(&canvas), "Editor deactivated");

        canvas_cleanup(&canvas);
    }

    TEST("Start editing invalid box fails") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int result = editor_start_title(&canvas, 999);
        ASSERT_EQ(result, -1, "Cannot edit non-existent box");
        ASSERT(!editor_is_active(&canvas), "Editor not active");

        canvas_cleanup(&canvas);
    }

    TEST("Confirm with undo integration") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Original");

        /* Start editing */
        editor_start_title(&canvas, id);

        /* Make changes */
        canvas.editor.cursor_pos = 0;
        canvas.editor.buffer[0] = '\0';
        canvas.editor.length = 0;
        editor_insert_char(&canvas.editor, 'N');
        editor_insert_char(&canvas.editor, 'e');
        editor_insert_char(&canvas.editor, 'w');

        /* Confirm */
        editor_confirm(&canvas);

        /* Title should be changed */
        Box *box = canvas_get_box(&canvas, id);
        ASSERT(strcmp(box->title, "New") == 0, "Title changed to 'New'");

        /* Undo should restore original */
        ASSERT(canvas_can_undo(&canvas), "Can undo");
        canvas_undo(&canvas);
        ASSERT(strcmp(box->title, "Original") == 0, "Title restored by undo");

        /* Redo should re-apply */
        canvas_redo(&canvas);
        ASSERT(strcmp(box->title, "New") == 0, "Title changed again by redo");

        canvas_cleanup(&canvas);
    }

    TEST("NULL safety") {
        /* These should not crash */
        editor_init(NULL);
        editor_cleanup(NULL);
        editor_cancel(NULL);

        ASSERT(!editor_is_active(NULL), "NULL canvas is not active");
        ASSERT(editor_get_buffer(NULL) != NULL, "NULL editor returns empty string");
        ASSERT(strlen(editor_get_buffer(NULL)) == 0, "Empty string from NULL");
        ASSERT_EQ(editor_get_cursor_pos(NULL), 0, "Zero cursor from NULL");
    }

    TEST_END();
}
