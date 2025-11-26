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

    /* Proportional sizing tests (Issue #18) */

    TEST("Proportional sizing - No boxes returns defaults") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int out_width = 0, out_height = 0;
        int neighbors = canvas_calc_proportional_size(
            &canvas, 50.0, 50.0,  /* position */
            30,                    /* proximity_radius */
            false,                 /* use_nearest */
            1,                     /* min_neighbors */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 0, "No neighbors found when canvas is empty");
        ASSERT_EQ(out_width, 25, "Returns default width when no neighbors");
        ASSERT_EQ(out_height, 10, "Returns default height when no neighbors");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Single neighbor within radius") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add a 30x15 box at position (20, 20) */
        canvas_add_box(&canvas, 20.0, 20.0, 30, 15, "Neighbor");

        int out_width = 0, out_height = 0;
        int neighbors = canvas_calc_proportional_size(
            &canvas, 40.0, 35.0,  /* position near the box center (35, 27.5) */
            30,                    /* proximity_radius */
            false,                 /* use_nearest */
            1,                     /* min_neighbors */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 1, "Found 1 neighbor");
        ASSERT_EQ(out_width, 30, "Inherits neighbor width");
        ASSERT_EQ(out_height, 15, "Inherits neighbor height");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Box outside radius uses defaults") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add a box far away */
        canvas_add_box(&canvas, 100.0, 100.0, 30, 15, "Far Box");

        int out_width = 0, out_height = 0;
        int neighbors = canvas_calc_proportional_size(
            &canvas, 10.0, 10.0,  /* position far from the box */
            30,                    /* proximity_radius */
            false,                 /* use_nearest */
            1,                     /* min_neighbors */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 0, "No neighbors within radius");
        ASSERT_EQ(out_width, 25, "Uses default width");
        ASSERT_EQ(out_height, 10, "Uses default height");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Averages multiple neighbors") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add three boxes with different dimensions near center */
        canvas_add_box(&canvas, 40.0, 40.0, 20, 10, "Box A");  /* center: (50, 45) */
        canvas_add_box(&canvas, 45.0, 55.0, 30, 12, "Box B");  /* center: (60, 61) */
        canvas_add_box(&canvas, 55.0, 45.0, 40, 8, "Box C");   /* center: (75, 49) */

        int out_width = 0, out_height = 0;
        int neighbors = canvas_calc_proportional_size(
            &canvas, 60.0, 50.0,  /* position central to all boxes */
            50,                    /* proximity_radius (large enough for all) */
            false,                 /* use_nearest (average mode) */
            1,                     /* min_neighbors */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 3, "Found all 3 neighbors");
        /* Average: (20+30+40)/3 = 30, (10+12+8)/3 = 10 */
        ASSERT_EQ(out_width, 30, "Width is average of neighbors");
        ASSERT_EQ(out_height, 10, "Height is average of neighbors");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Nearest neighbor mode") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add two boxes at different distances */
        canvas_add_box(&canvas, 40.0, 40.0, 20, 8, "Near Box");  /* center: (50, 44) */
        canvas_add_box(&canvas, 80.0, 80.0, 40, 16, "Far Box");  /* center: (100, 88) */

        int out_width = 0, out_height = 0;
        int neighbors = canvas_calc_proportional_size(
            &canvas, 55.0, 50.0,  /* position closer to Near Box */
            100,                   /* proximity_radius (covers both) */
            true,                  /* use_nearest */
            1,                     /* min_neighbors */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 2, "Found both neighbors");
        ASSERT_EQ(out_width, 20, "Uses nearest neighbor width");
        ASSERT_EQ(out_height, 8, "Uses nearest neighbor height");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - min_neighbors threshold") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add only one box */
        canvas_add_box(&canvas, 40.0, 40.0, 30, 15, "Single Box");

        int out_width = 0, out_height = 0;

        /* Require 2 neighbors, but only 1 exists */
        int neighbors = canvas_calc_proportional_size(
            &canvas, 50.0, 50.0,
            50,                    /* proximity_radius */
            false,                 /* use_nearest */
            2,                     /* min_neighbors = 2 */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 0, "Returns 0 when below min_neighbors threshold");
        ASSERT_EQ(out_width, 25, "Uses default width when threshold not met");
        ASSERT_EQ(out_height, 10, "Uses default height when threshold not met");

        /* Now add another box and try again */
        canvas_add_box(&canvas, 45.0, 45.0, 40, 20, "Second Box");

        neighbors = canvas_calc_proportional_size(
            &canvas, 50.0, 50.0,
            50,                    /* proximity_radius */
            false,                 /* use_nearest */
            2,                     /* min_neighbors = 2 */
            25, 10,                /* defaults */
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 2, "Found 2 neighbors, meets threshold");
        /* Average: (30+40)/2 = 35, (15+20)/2 = 17 (rounded) */
        ASSERT_EQ(out_width, 35, "Uses proportional width when threshold met");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Bounds clamping (width)") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add a very small box (below min 10) */
        canvas_add_box(&canvas, 40.0, 40.0, 5, 10, "Tiny Box");

        int out_width = 0, out_height = 0;
        canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 50, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(out_width, 10, "Width clamped to minimum 10");

        canvas_cleanup(&canvas);

        /* Test max clamping */
        canvas_init(&canvas, 200.0, 100.0);
        canvas_add_box(&canvas, 40.0, 40.0, 100, 10, "Huge Box");

        canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 50, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(out_width, 80, "Width clamped to maximum 80");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Bounds clamping (height)") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add a very short box (below min 3) */
        canvas_add_box(&canvas, 40.0, 40.0, 20, 1, "Short Box");

        int out_width = 0, out_height = 0;
        canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 50, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(out_height, 3, "Height clamped to minimum 3");

        canvas_cleanup(&canvas);

        /* Test max clamping */
        canvas_init(&canvas, 200.0, 100.0);
        canvas_add_box(&canvas, 40.0, 40.0, 20, 50, "Tall Box");

        canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 50, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(out_height, 30, "Height clamped to maximum 30");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - NULL safety") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int out_width = 999, out_height = 999;

        /* NULL canvas */
        int neighbors = canvas_calc_proportional_size(
            NULL, 50.0, 50.0, 30, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 0, "Returns 0 for NULL canvas");
        ASSERT_EQ(out_width, 25, "Sets default width for NULL canvas");
        ASSERT_EQ(out_height, 10, "Sets default height for NULL canvas");

        /* NULL out_width */
        out_height = 999;
        neighbors = canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 30, false, 1, 25, 10,
            NULL, &out_height);

        ASSERT_EQ(neighbors, 0, "Returns 0 for NULL out_width");
        ASSERT_EQ(out_height, 10, "Sets default height when out_width is NULL");

        /* NULL out_height */
        out_width = 999;
        neighbors = canvas_calc_proportional_size(
            &canvas, 50.0, 50.0, 30, false, 1, 25, 10,
            &out_width, NULL);

        ASSERT_EQ(neighbors, 0, "Returns 0 for NULL out_height");
        ASSERT_EQ(out_width, 25, "Sets default width when out_height is NULL");

        canvas_cleanup(&canvas);
    }

    TEST("Proportional sizing - Boundary condition at exact radius") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        /* Add box at (0,0) with center at (15, 7.5) */
        canvas_add_box(&canvas, 0.0, 0.0, 30, 15, "Box");

        int out_width = 0, out_height = 0;

        /* Position exactly 30 units from box center */
        /* Box center is (15, 7.5), position (45, 7.5) is exactly 30 units away */
        int neighbors = canvas_calc_proportional_size(
            &canvas, 45.0, 7.5, 30, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 1, "Box at exact radius boundary is included");
        ASSERT_EQ(out_width, 30, "Uses neighbor dimensions at boundary");

        /* Position just outside radius */
        neighbors = canvas_calc_proportional_size(
            &canvas, 46.0, 7.5, 30, false, 1, 25, 10,
            &out_width, &out_height);

        ASSERT_EQ(neighbors, 0, "Box just outside radius is excluded");

        canvas_cleanup(&canvas);
    }

    TEST_END();
}
