#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/types.h"

int main(void) {
    TEST_START();

    TEST("Canvas initialization") {
        Canvas canvas;
        int result = canvas_init(&canvas, 200.0, 100.0);

        ASSERT_EQ(result, 0, "canvas_init returns 0");
        ASSERT_EQ(canvas.box_count, 0, "Initial box count is 0");
        ASSERT_EQ(canvas.box_capacity, INITIAL_BOX_CAPACITY, "Initial capacity is INITIAL_BOX_CAPACITY");
        ASSERT_NEAR(canvas.world_width, 200.0, 0.01, "World width is 200");
        ASSERT_NEAR(canvas.world_height, 100.0, 0.01, "World height is 100");
        ASSERT_EQ(canvas.next_id, 1, "Next ID starts at 1");
        ASSERT_EQ(canvas.selected_index, -1, "No box selected initially");
        ASSERT_NOT_NULL(canvas.boxes, "Boxes array is allocated");

        canvas_cleanup(&canvas);
    }

    TEST("Adding boxes") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Box 1");
        ASSERT_EQ(id1, 1, "First box ID is 1");
        ASSERT_EQ(canvas.box_count, 1, "Box count is 1");

        int id2 = canvas_add_box(&canvas, 50.0, 60.0, 25, 8, "Box 2");
        ASSERT_EQ(id2, 2, "Second box ID is 2");
        ASSERT_EQ(canvas.box_count, 2, "Box count is 2");

        Box *box1 = canvas_get_box(&canvas, id1);
        ASSERT_NOT_NULL(box1, "Can retrieve first box");
        ASSERT_NEAR(box1->x, 10.0, 0.01, "Box 1 x coordinate is correct");
        ASSERT_NEAR(box1->y, 20.0, 0.01, "Box 1 y coordinate is correct");
        ASSERT_EQ(box1->width, 30, "Box 1 width is correct");
        ASSERT_EQ(box1->height, 5, "Box 1 height is correct");
        ASSERT_STR_EQ(box1->title, "Box 1", "Box 1 title is correct");

        canvas_cleanup(&canvas);
    }

    TEST("Adding box content") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Test Box");
        const char *lines[] = {"Line 1", "Line 2", "Line 3"};
        int result = canvas_add_box_content(&canvas, id, lines, 3);

        ASSERT_EQ(result, 0, "Adding content succeeds");

        Box *box = canvas_get_box(&canvas, id);
        ASSERT_EQ(box->content_lines, 3, "Content has 3 lines");
        ASSERT_STR_EQ(box->content[0], "Line 1", "First line is correct");
        ASSERT_STR_EQ(box->content[1], "Line 2", "Second line is correct");
        ASSERT_STR_EQ(box->content[2], "Line 3", "Third line is correct");

        canvas_cleanup(&canvas);
    }

    TEST("Removing boxes") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 60.0, 25, 8, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 100.0, 20, 6, "Box 3");

        ASSERT_EQ(canvas.box_count, 3, "Have 3 boxes");

        int result = canvas_remove_box(&canvas, id2);
        ASSERT_EQ(result, 0, "Removing box succeeds");
        ASSERT_EQ(canvas.box_count, 2, "Now have 2 boxes");

        Box *box2 = canvas_get_box(&canvas, id2);
        ASSERT_NULL(box2, "Removed box cannot be retrieved");

        Box *box1 = canvas_get_box(&canvas, id1);
        Box *box3 = canvas_get_box(&canvas, id3);
        ASSERT_NOT_NULL(box1, "Box 1 still exists");
        ASSERT_NOT_NULL(box3, "Box 3 still exists");

        canvas_cleanup(&canvas);
    }

    TEST("Finding boxes at coordinates") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 20, 10, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 15, "Box 2");

        // Inside box 1
        int found = canvas_find_box_at(&canvas, 15.0, 15.0);
        ASSERT_EQ(found, id1, "Found box 1 at (15, 15)");

        // Inside box 2
        found = canvas_find_box_at(&canvas, 60.0, 60.0);
        ASSERT_EQ(found, id2, "Found box 2 at (60, 60)");

        // Outside all boxes
        found = canvas_find_box_at(&canvas, 100.0, 100.0);
        ASSERT_EQ(found, -1, "No box at (100, 100)");

        // At edge of box 1 (should be inside)
        found = canvas_find_box_at(&canvas, 10.0, 10.0);
        ASSERT_EQ(found, id1, "Found box 1 at top-left corner");

        canvas_cleanup(&canvas);
    }

    TEST("Box selection") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 20, 10, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 15, "Box 2");

        ASSERT_EQ(canvas.selected_index, -1, "No initial selection");

        canvas_select_box(&canvas, id1);
        ASSERT_EQ(canvas.selected_index, 0, "Box 1 is selected (index 0)");

        Box *selected = canvas_get_selected(&canvas);
        ASSERT_NOT_NULL(selected, "Can get selected box");
        ASSERT_EQ(selected->id, id1, "Selected box is box 1");
        ASSERT(selected->selected, "Box 1 has selected flag set");

        canvas_select_box(&canvas, id2);
        ASSERT_EQ(canvas.selected_index, 1, "Box 2 is selected (index 1)");
        ASSERT(!canvas.boxes[0].selected, "Box 1 no longer has selected flag");
        ASSERT(canvas.boxes[1].selected, "Box 2 has selected flag");

        canvas_deselect(&canvas);
        ASSERT_EQ(canvas.selected_index, -1, "No selection after deselect");
        ASSERT(!canvas.boxes[0].selected, "Box 1 not selected");
        ASSERT(!canvas.boxes[1].selected, "Box 2 not selected");

        canvas_cleanup(&canvas);
    }

    TEST("Dynamic array growth") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int initial_capacity = canvas.box_capacity;

        // Add more boxes than initial capacity
        for (int i = 0; i < initial_capacity + 5; i++) {
            char title[32];
            snprintf(title, sizeof(title), "Box %d", i);
            canvas_add_box(&canvas, i * 10.0, i * 10.0, 20, 5, title);
        }

        ASSERT_EQ(canvas.box_count, initial_capacity + 5, "All boxes added");
        ASSERT(canvas.box_capacity > initial_capacity, "Capacity increased");

        // Verify all boxes are accessible
        for (int i = 0; i < initial_capacity + 5; i++) {
            Box *box = canvas_get_box_at(&canvas, i);
            ASSERT_NOT_NULL(box, "Can access box after growth");
        }

        canvas_cleanup(&canvas);
    }

    TEST("Box color property") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id = canvas_add_box(&canvas, 10.0, 20.0, 30, 5, "Colored Box");
        Box *box = canvas_get_box(&canvas, id);

        ASSERT_EQ(box->color, BOX_COLOR_DEFAULT, "Default color is BOX_COLOR_DEFAULT");

        box->color = BOX_COLOR_RED;
        ASSERT_EQ(box->color, BOX_COLOR_RED, "Can set color to red");

        box->color = BOX_COLOR_BLUE;
        ASSERT_EQ(box->color, BOX_COLOR_BLUE, "Can change color to blue");

        canvas_cleanup(&canvas);
    }

    TEST_END();
}
