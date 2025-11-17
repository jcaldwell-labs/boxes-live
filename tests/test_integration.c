#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/viewport.h"
#include "../include/persistence.h"

#define TEST_FILE "test_integration_temp.txt"

int main(void) {
    TEST_START();

    TEST("Complete workflow: Create, color, move, save, load") {
        Canvas canvas;
        canvas_init(&canvas, 2000.0, 1500.0);

        /* Step 1: Create multiple boxes */
        int box1_id = canvas_add_box(&canvas, 100.0, 100.0, 30, 8, "Task 1");
        int box2_id = canvas_add_box(&canvas, 200.0, 100.0, 30, 8, "Task 2");
        int box3_id = canvas_add_box(&canvas, 150.0, 200.0, 30, 8, "Note");

        ASSERT_EQ(canvas.box_count, 3, "Should have 3 boxes created");

        /* Step 2: Add content to boxes */
        const char *task1_content[] = {"High priority", "Due: Tomorrow"};
        const char *task2_content[] = {"Low priority", "Due: Next week"};
        const char *note_content[] = {"Important info", "Remember this"};

        canvas_add_box_content(&canvas, box1_id, task1_content, 2);
        canvas_add_box_content(&canvas, box2_id, task2_content, 2);
        canvas_add_box_content(&canvas, box3_id, note_content, 2);

        /* Step 3: Apply colors to boxes */
        Box *box1 = canvas_get_box(&canvas, box1_id);
        Box *box2 = canvas_get_box(&canvas, box2_id);
        Box *box3 = canvas_get_box(&canvas, box3_id);

        box1->color = BOX_COLOR_RED;    /* High priority = red */
        box2->color = BOX_COLOR_GREEN;  /* Low priority = green */
        box3->color = BOX_COLOR_YELLOW; /* Note = yellow */

        ASSERT_EQ(box1->color, BOX_COLOR_RED, "Task 1 colored red");
        ASSERT_EQ(box2->color, BOX_COLOR_GREEN, "Task 2 colored green");
        ASSERT_EQ(box3->color, BOX_COLOR_YELLOW, "Note colored yellow");

        /* Step 4: Select and move boxes (simulating drag) */
        canvas_select_box(&canvas, box1_id);
        ASSERT_EQ(canvas.selected_index, 0, "Box 1 selected");

        /* Move box 1 to new position */
        box1->x = 120.0;
        box1->y = 150.0;

        /* Select and move box 3 */
        canvas_select_box(&canvas, box3_id);
        box3->x = 180.0;
        box3->y = 250.0;

        /* Step 5: Save canvas */
        int save_result = canvas_save(&canvas, TEST_FILE);
        ASSERT_EQ(save_result, 0, "Canvas saved successfully");

        /* Step 6: Load canvas into new instance */
        Canvas loaded_canvas;
        canvas_init(&loaded_canvas, 0.0, 0.0);

        int load_result = canvas_load(&loaded_canvas, TEST_FILE);
        ASSERT_EQ(load_result, 0, "Canvas loaded successfully");

        /* Step 7: Verify all data preserved */
        ASSERT_EQ(loaded_canvas.box_count, 3, "All boxes loaded");
        ASSERT_NEAR(loaded_canvas.world_width, 2000.0, 0.01, "World width preserved");
        ASSERT_NEAR(loaded_canvas.world_height, 1500.0, 0.01, "World height preserved");

        /* Verify box 1 (moved and colored red) */
        Box *loaded_box1 = canvas_get_box(&loaded_canvas, box1_id);
        ASSERT_NOT_NULL(loaded_box1, "Box 1 loaded");
        if (loaded_box1) {
            ASSERT_NEAR(loaded_box1->x, 120.0, 0.01, "Box 1 moved position X");
            ASSERT_NEAR(loaded_box1->y, 150.0, 0.01, "Box 1 moved position Y");
            ASSERT_EQ(loaded_box1->color, BOX_COLOR_RED, "Box 1 color preserved");
            ASSERT_STR_EQ(loaded_box1->title, "Task 1", "Box 1 title preserved");
        }

        /* Verify box 2 (not moved, colored green) */
        Box *loaded_box2 = canvas_get_box(&loaded_canvas, box2_id);
        ASSERT_NOT_NULL(loaded_box2, "Box 2 loaded");
        if (loaded_box2) {
            ASSERT_NEAR(loaded_box2->x, 200.0, 0.01, "Box 2 original position X");
            ASSERT_NEAR(loaded_box2->y, 100.0, 0.01, "Box 2 original position Y");
            ASSERT_EQ(loaded_box2->color, BOX_COLOR_GREEN, "Box 2 color preserved");
        }

        /* Verify box 3 (moved and colored yellow, selected) */
        Box *loaded_box3 = canvas_get_box(&loaded_canvas, box3_id);
        ASSERT_NOT_NULL(loaded_box3, "Box 3 loaded");
        if (loaded_box3) {
            ASSERT_NEAR(loaded_box3->x, 180.0, 0.01, "Box 3 moved position X");
            ASSERT_NEAR(loaded_box3->y, 250.0, 0.01, "Box 3 moved position Y");
            ASSERT_EQ(loaded_box3->color, BOX_COLOR_YELLOW, "Box 3 color preserved");
            ASSERT(loaded_box3->selected, "Box 3 selection preserved");
        }

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded_canvas);
    }

    TEST("Workflow: Box selection cycling") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        /* Create 5 boxes */
        int ids[5];
        for (int i = 0; i < 5; i++) {
            ids[i] = canvas_add_box(&canvas, i * 100.0, i * 100.0, 25, 6, "Box");
        }

        /* Initially no selection */
        ASSERT_EQ(canvas.selected_index, -1, "No initial selection");

        /* Select first box */
        canvas_select_box(&canvas, ids[0]);
        ASSERT_EQ(canvas.selected_index, 0, "First box selected");

        /* Cycle through all boxes */
        for (int i = 1; i < 5; i++) {
            /* Simulate Tab key (cycle to next) */
            int next_index = (canvas.selected_index + 1) % canvas.box_count;
            Box *next_box = canvas_get_box_at(&canvas, next_index);
            canvas_select_box(&canvas, next_box->id);

            ASSERT_EQ(canvas.selected_index, i, "Cycled to next box");
        }

        /* One more cycle should wrap to first box */
        int next_index = (canvas.selected_index + 1) % canvas.box_count;
        Box *next_box = canvas_get_box_at(&canvas, next_index);
        canvas_select_box(&canvas, next_box->id);

        ASSERT_EQ(canvas.selected_index, 0, "Wrapped to first box");

        canvas_cleanup(&canvas);
    }

    TEST("Workflow: Box deletion and selection updates") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        /* Create 3 boxes */
        int id1 = canvas_add_box(&canvas, 100.0, 100.0, 20, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 200.0, 100.0, 20, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 300.0, 100.0, 20, 5, "Box 3");

        /* Select middle box */
        canvas_select_box(&canvas, id2);
        ASSERT_EQ(canvas.selected_index, 1, "Box 2 selected");

        /* Delete selected box */
        canvas_remove_box(&canvas, id2);

        ASSERT_EQ(canvas.box_count, 2, "Box count reduced to 2");
        ASSERT_EQ(canvas.selected_index, -1, "Selection cleared after deletion");

        /* Verify remaining boxes */
        Box *box1 = canvas_get_box(&canvas, id1);
        Box *box3 = canvas_get_box(&canvas, id3);

        ASSERT_NOT_NULL(box1, "Box 1 still exists");
        ASSERT_NOT_NULL(box3, "Box 3 still exists");

        /* Verify deleted box is gone */
        Box *deleted = canvas_get_box(&canvas, id2);
        ASSERT_NULL(deleted, "Box 2 was deleted");

        canvas_cleanup(&canvas);
    }

    TEST("Workflow: Viewport and box visibility") {
        Viewport vp;
        viewport_init(&vp);
        vp.term_width = 80;
        vp.term_height = 24;

        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        /* Create box at origin (should be visible) */
        int id1 = canvas_add_box(&canvas, 5.0, 5.0, 20, 5, "Visible");

        /* Create box far away (should not be visible) */
        int id2 = canvas_add_box(&canvas, 500.0, 500.0, 20, 5, "Hidden");

        Box *box1 = canvas_get_box(&canvas, id1);
        Box *box2 = canvas_get_box(&canvas, id2);

        /* Check visibility at origin */
        int visible1 = is_visible(&vp, box1->x, box1->y);
        int visible2 = is_visible(&vp, box2->x, box2->y);

        ASSERT(visible1, "Box at origin is visible");
        ASSERT(!visible2, "Box at (500,500) is not visible");

        /* Pan viewport to make box2 visible */
        vp.cam_x = 460.0;
        vp.cam_y = 490.0;

        visible1 = is_visible(&vp, box1->x, box1->y);
        visible2 = is_visible(&vp, box2->x, box2->y);

        ASSERT(!visible1, "Box 1 no longer visible after pan");
        ASSERT(visible2, "Box 2 now visible after pan");

        canvas_cleanup(&canvas);
    }

    TEST("Workflow: Finding boxes at screen coordinates") {
        Viewport vp;
        viewport_init(&vp);
        vp.term_width = 80;
        vp.term_height = 24;

        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        /* Create box at specific position */
        int id = canvas_add_box(&canvas, 10.0, 10.0, 20, 5, "Target");

        /* Click at box center in world coords */
        double click_x = 20.0;  /* Center of box (10 + 20/2) */
        double click_y = 12.5;  /* Center of box (10 + 5/2) */

        int found_id = canvas_find_box_at(&canvas, click_x, click_y);
        ASSERT_EQ(found_id, id, "Found box at click position");

        /* Click outside box */
        click_x = 100.0;
        click_y = 100.0;

        found_id = canvas_find_box_at(&canvas, click_x, click_y);
        ASSERT_EQ(found_id, -1, "No box found at empty position");

        canvas_cleanup(&canvas);
    }

    TEST("Workflow: Dynamic canvas growth during session") {
        Canvas canvas;
        canvas_init(&canvas, 5000.0, 5000.0);

        /* Initial capacity is 16 */
        ASSERT_EQ(canvas.box_capacity, 16, "Initial capacity is 16");

        /* Add boxes beyond initial capacity */
        for (int i = 0; i < 20; i++) {
            canvas_add_box(&canvas, i * 50.0, i * 50.0, 20, 5, "Box");
        }

        ASSERT_EQ(canvas.box_count, 20, "Added 20 boxes");
        ASSERT(canvas.box_capacity >= 20, "Capacity grew to accommodate boxes");

        /* Verify all boxes are accessible */
        for (int i = 1; i <= 20; i++) {
            Box *box = canvas_get_box(&canvas, i);
            ASSERT_NOT_NULL(box, "All boxes accessible after growth");
        }

        /* Save and load to verify persistence of large canvas */
        canvas_save(&canvas, TEST_FILE);

        Canvas loaded;
        canvas_init(&loaded, 0.0, 0.0);
        canvas_load(&loaded, TEST_FILE);

        ASSERT_EQ(loaded.box_count, 20, "All 20 boxes loaded");

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded);
    }

    /* Cleanup test file */
    unlink(TEST_FILE);

    TEST_END();
}
