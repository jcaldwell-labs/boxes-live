#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <string.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/undo.h"
#include "../include/types.h"

int main(void) {
    TEST_START();

    TEST("Undo stack initialization in canvas") {
        Canvas canvas;
        int result = canvas_init(&canvas, 200.0, 100.0);

        ASSERT_EQ(result, 0, "canvas_init returns 0");
        ASSERT(!canvas_can_undo(&canvas), "Cannot undo on fresh canvas");
        ASSERT(!canvas_can_redo(&canvas), "Cannot redo on fresh canvas");
        ASSERT_EQ(canvas.undo_stack.size, 0, "Undo stack is empty");
        ASSERT_EQ(canvas.undo_stack.max_size, UNDO_STACK_MAX_SIZE, "Max size is default");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo box creation") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test Box");
        ASSERT(id > 0, "Box created successfully");
        ASSERT_EQ(canvas.box_count, 1, "Box count is 1");

        /* Record the creation for undo */
        undo_record_box_create(&canvas, id);
        ASSERT(canvas_can_undo(&canvas), "Can undo after recording");

        /* Undo the creation */
        bool undone = canvas_undo(&canvas);
        ASSERT(undone, "Undo succeeded");
        ASSERT_EQ(canvas.box_count, 0, "Box removed after undo");
        ASSERT(!canvas_can_undo(&canvas), "Cannot undo again");
        ASSERT(canvas_can_redo(&canvas), "Can redo after undo");

        /* Redo the creation */
        bool redone = canvas_redo(&canvas);
        ASSERT(redone, "Redo succeeded");
        ASSERT_EQ(canvas.box_count, 1, "Box restored after redo");
        ASSERT(canvas_can_undo(&canvas), "Can undo after redo");
        ASSERT(!canvas_can_redo(&canvas), "Cannot redo again");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo box deletion") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test Box");
        ASSERT_EQ(canvas.box_count, 1, "Box created");

        /* Record the deletion before actually deleting */
        undo_record_box_delete(&canvas, id);

        /* Delete the box */
        canvas_remove_box(&canvas, id);
        ASSERT_EQ(canvas.box_count, 0, "Box deleted");

        /* Undo the deletion */
        bool undone = canvas_undo(&canvas);
        ASSERT(undone, "Undo succeeded");
        ASSERT_EQ(canvas.box_count, 1, "Box restored after undo");

        /* Verify box properties were restored */
        Box *box = canvas_get_box(&canvas, canvas.boxes[0].id);
        ASSERT_NOT_NULL(box, "Can get restored box");
        ASSERT(fabs(box->x - 10.0) < 0.001, "X position restored");
        ASSERT(fabs(box->y - 20.0) < 0.001, "Y position restored");
        ASSERT_EQ(box->width, 30, "Width restored");
        ASSERT_EQ(box->height, 5, "Height restored");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo box move") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test Box");
        Box *box = canvas_get_box(&canvas, id);

        /* Record and perform the move */
        undo_record_box_move(&canvas, id, 10.0, 20.0, 50.0, 60.0);
        box->x = 50.0;
        box->y = 60.0;

        ASSERT(fabs(box->x - 50.0) < 0.001, "Box moved to new X");
        ASSERT(fabs(box->y - 60.0) < 0.001, "Box moved to new Y");

        /* Undo the move */
        canvas_undo(&canvas);
        ASSERT(fabs(box->x - 10.0) < 0.001, "Box X restored");
        ASSERT(fabs(box->y - 20.0) < 0.001, "Box Y restored");

        /* Redo the move */
        canvas_redo(&canvas);
        ASSERT(fabs(box->x - 50.0) < 0.001, "Box X re-applied");
        ASSERT(fabs(box->y - 60.0) < 0.001, "Box Y re-applied");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo box color") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test Box");
        Box *box = canvas_get_box(&canvas, id);

        ASSERT_EQ(box->color, 0, "Initial color is 0");

        /* Record and perform the color change */
        undo_record_box_color(&canvas, id, 0, 3);
        box->color = 3;

        ASSERT_EQ(box->color, 3, "Color changed to 3");

        /* Undo the color change */
        canvas_undo(&canvas);
        ASSERT_EQ(box->color, 0, "Color restored to 0");

        /* Redo the color change */
        canvas_redo(&canvas);
        ASSERT_EQ(box->color, 3, "Color re-applied to 3");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo box title") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Original");
        Box *box = canvas_get_box(&canvas, id);

        ASSERT(strcmp(box->title, "Original") == 0, "Initial title correct");

        /* Record and perform the title change */
        undo_record_box_title(&canvas, id, "Original", "New Title");
        free(box->title);
        box->title = strdup("New Title");

        ASSERT(strcmp(box->title, "New Title") == 0, "Title changed");

        /* Undo the title change */
        canvas_undo(&canvas);
        ASSERT(strcmp(box->title, "Original") == 0, "Title restored");

        /* Redo the title change */
        canvas_redo(&canvas);
        ASSERT(strcmp(box->title, "New Title") == 0, "Title re-applied");

        canvas_cleanup(&canvas);
    }

    TEST("Undo stack limit") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Set a small max size for testing */
        canvas.undo_stack.max_size = 5;

        /* Create 10 boxes (10 operations) */
        for (int i = 0; i < 10; i++) {
            int id = canvas_add_box(&canvas, i * 10.0, 0.0, 20, 5, "Box");
            undo_record_box_create(&canvas, id);
        }

        ASSERT_EQ(canvas.box_count, 10, "Created 10 boxes");
        ASSERT_EQ(canvas.undo_stack.size, 5, "Stack limited to 5");

        /* Can only undo 5 times */
        int undo_count = 0;
        while (canvas_undo(&canvas)) {
            undo_count++;
        }
        ASSERT_EQ(undo_count, 5, "Could only undo 5 times");
        ASSERT_EQ(canvas.box_count, 5, "5 boxes remain after undo");

        canvas_cleanup(&canvas);
    }

    TEST("New operation clears redo stack") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Create and record a box */
        int id1 = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Box 1");
        undo_record_box_create(&canvas, id1);

        /* Undo it */
        canvas_undo(&canvas);
        ASSERT(canvas_can_redo(&canvas), "Can redo after undo");

        /* Create another box (new operation) */
        int id2 = canvas_add_box(&canvas, 50.0, 60.0, 25, 5, "Box 2");
        undo_record_box_create(&canvas, id2);

        /* Redo should now be unavailable */
        ASSERT(!canvas_can_redo(&canvas), "Redo cleared after new operation");

        canvas_cleanup(&canvas);
    }

    TEST("Multiple undos and redos") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Create 3 boxes */
        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 20, 5, "Box 1");
        undo_record_box_create(&canvas, id1);

        int id2 = canvas_add_box(&canvas, 30.0, 30.0, 20, 5, "Box 2");
        undo_record_box_create(&canvas, id2);

        int id3 = canvas_add_box(&canvas, 50.0, 50.0, 20, 5, "Box 3");
        undo_record_box_create(&canvas, id3);

        ASSERT_EQ(canvas.box_count, 3, "Created 3 boxes");

        /* Undo all 3 */
        canvas_undo(&canvas);
        ASSERT_EQ(canvas.box_count, 2, "2 boxes after first undo");

        canvas_undo(&canvas);
        ASSERT_EQ(canvas.box_count, 1, "1 box after second undo");

        canvas_undo(&canvas);
        ASSERT_EQ(canvas.box_count, 0, "0 boxes after third undo");

        /* Redo all 3 */
        canvas_redo(&canvas);
        ASSERT_EQ(canvas.box_count, 1, "1 box after first redo");

        canvas_redo(&canvas);
        ASSERT_EQ(canvas.box_count, 2, "2 boxes after second redo");

        canvas_redo(&canvas);
        ASSERT_EQ(canvas.box_count, 3, "3 boxes after third redo");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo connection creation") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 20, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 20, 5, "Box 2");

        int conn_id = canvas_add_connection(&canvas, id1, id2);
        ASSERT(conn_id > 0, "Connection created");
        ASSERT_EQ(canvas.conn_count, 1, "Connection count is 1");

        /* Record for undo */
        undo_record_connection_create(&canvas, conn_id);

        /* Undo the connection */
        canvas_undo(&canvas);
        ASSERT_EQ(canvas.conn_count, 0, "Connection removed after undo");

        /* Redo the connection */
        canvas_redo(&canvas);
        ASSERT_EQ(canvas.conn_count, 1, "Connection restored after redo");

        canvas_cleanup(&canvas);
    }

    TEST("Undo/Redo connection deletion") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 20, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 20, 5, "Box 2");

        int conn_id = canvas_add_connection(&canvas, id1, id2);
        ASSERT_EQ(canvas.conn_count, 1, "Connection created");

        /* Record before deletion */
        undo_record_connection_delete(&canvas, conn_id);

        /* Delete the connection */
        canvas_remove_connection(&canvas, conn_id);
        ASSERT_EQ(canvas.conn_count, 0, "Connection deleted");

        /* Undo the deletion */
        canvas_undo(&canvas);
        ASSERT_EQ(canvas.conn_count, 1, "Connection restored after undo");

        canvas_cleanup(&canvas);
    }

    TEST("NULL canvas safety") {
        /* These should not crash */
        bool result = canvas_undo(NULL);
        ASSERT(!result, "Undo on NULL returns false");

        result = canvas_redo(NULL);
        ASSERT(!result, "Redo on NULL returns false");

        ASSERT(!canvas_can_undo(NULL), "Can undo on NULL is false");
        ASSERT(!canvas_can_redo(NULL), "Can redo on NULL is false");

        const char *desc = canvas_get_undo_description(NULL);
        ASSERT_NULL(desc, "Undo description on NULL is NULL");

        desc = canvas_get_redo_description(NULL);
        ASSERT_NULL(desc, "Redo description on NULL is NULL");
    }

    TEST("Operation descriptions") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test");
        undo_record_box_create(&canvas, id);

        const char *desc = canvas_get_undo_description(&canvas);
        ASSERT_NOT_NULL(desc, "Has undo description");
        ASSERT(strcmp(desc, "create box") == 0, "Description is 'create box'");

        canvas_undo(&canvas);

        desc = canvas_get_redo_description(&canvas);
        ASSERT_NOT_NULL(desc, "Has redo description");
        ASSERT(strcmp(desc, "create box") == 0, "Redo description is 'create box'");

        canvas_cleanup(&canvas);
    }

    TEST("Undo stack cleanup frees memory") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Create several operations with string allocations */
        for (int i = 0; i < 10; i++) {
            int id = canvas_add_box(&canvas, i * 10.0, 0.0, 20, 5, "Box");
            undo_record_box_create(&canvas, id);
        }

        /* Undo half of them (creates redo chain) */
        for (int i = 0; i < 5; i++) {
            canvas_undo(&canvas);
        }

        /* Cleanup should free both chains without leaks */
        canvas_cleanup(&canvas);

        /* If we get here without valgrind errors, test passed */
        ASSERT(true, "Cleanup completed without crash");
    }

    TEST("Undo/Redo box resize") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, id);

        ASSERT_EQ(box->width, 30, "Initial width");
        ASSERT_EQ(box->height, 10, "Initial height");

        /* Record and perform the resize */
        undo_record_box_resize(&canvas, id, 30, 10, 50, 20);
        box->width = 50;
        box->height = 20;

        ASSERT_EQ(box->width, 50, "Width changed");
        ASSERT_EQ(box->height, 20, "Height changed");

        /* Undo the resize */
        canvas_undo(&canvas);
        ASSERT_EQ(box->width, 30, "Width restored");
        ASSERT_EQ(box->height, 10, "Height restored");

        /* Redo the resize */
        canvas_redo(&canvas);
        ASSERT_EQ(box->width, 50, "Width re-applied");
        ASSERT_EQ(box->height, 20, "Height re-applied");

        canvas_cleanup(&canvas);
    }

    TEST_END();
}
