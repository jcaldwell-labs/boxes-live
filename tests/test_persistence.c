#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/persistence.h"

#define TEST_FILE "test_canvas_temp.txt"
#define SNAPSHOT_FILE "tests/snapshots/expected_canvas.txt"

/* Helper function to compare two files */
static int files_equal(const char *file1, const char *file2) __attribute__((unused));
static int files_equal(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");

    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }

    int equal = 1;
    int ch1, ch2;

    while ((ch1 = fgetc(f1)) != EOF && (ch2 = fgetc(f2)) != EOF) {
        if (ch1 != ch2) {
            equal = 0;
            break;
        }
    }

    /* Check if both files ended */
    if (fgetc(f1) != EOF || fgetc(f2) != EOF) {
        equal = 0;
    }

    fclose(f1);
    fclose(f2);
    return equal;
}

int main(void) {
    TEST_START();

    TEST("Save canvas to file") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int id1 = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 60.0, 25, 8, "Test Box 2");

        const char *content1[] = {"Line 1", "Line 2"};
        canvas_add_box_content(&canvas, id1, content1, 2);

        /* Apply colors */
        Box *box1 = canvas_get_box(&canvas, id1);
        Box *box2 = canvas_get_box(&canvas, id2);
        box1->color = BOX_COLOR_RED;
        box2->color = BOX_COLOR_BLUE;

        canvas_select_box(&canvas, id1);

        int result = canvas_save(&canvas, TEST_FILE);
        ASSERT_EQ(result, 0, "Save should succeed");

        /* Verify file exists */
        FILE *f = fopen(TEST_FILE, "r");
        ASSERT_NOT_NULL(f, "Saved file should exist");
        if (f) fclose(f);

        canvas_cleanup(&canvas);
    }

    TEST("Load canvas from file") {
        Canvas canvas;
        canvas_init(&canvas, 100.0, 100.0);

        int result = canvas_load(&canvas, TEST_FILE);
        ASSERT_EQ(result, 0, "Load should succeed");

        ASSERT_EQ(canvas.box_count, 2, "Should load 2 boxes");
        ASSERT_NEAR(canvas.world_width, 1000.0, 0.01, "World width should match");
        ASSERT_NEAR(canvas.world_height, 1000.0, 0.01, "World height should match");

        Box *box1 = canvas_get_box(&canvas, 1);
        ASSERT_NOT_NULL(box1, "Box 1 should exist");
        if (box1) {
            ASSERT_NEAR(box1->x, 10.0, 0.01, "Box 1 X position");
            ASSERT_NEAR(box1->y, 20.0, 0.01, "Box 1 Y position");
            ASSERT_EQ(box1->width, 30, "Box 1 width");
            ASSERT_EQ(box1->height, 10, "Box 1 height");
            ASSERT_STR_EQ(box1->title, "Test Box 1", "Box 1 title");
            ASSERT_EQ(box1->color, BOX_COLOR_RED, "Box 1 color");
            ASSERT(box1->selected, "Box 1 should be selected");
        }

        Box *box2 = canvas_get_box(&canvas, 2);
        ASSERT_NOT_NULL(box2, "Box 2 should exist");
        if (box2) {
            ASSERT_NEAR(box2->x, 50.0, 0.01, "Box 2 X position");
            ASSERT_NEAR(box2->y, 60.0, 0.01, "Box 2 Y position");
            ASSERT_EQ(box2->color, BOX_COLOR_BLUE, "Box 2 color");
            ASSERT(!box2->selected, "Box 2 should not be selected");
        }

        canvas_cleanup(&canvas);
    }

    TEST("Roundtrip save and load preserves data") {
        Canvas original;
        canvas_init(&original, 2000.0, 1500.0);

        /* Create diverse canvas */
        int id1 = canvas_add_box(&original, 100.0, 200.0, 40, 12, "Box Alpha");
        int id2 = canvas_add_box(&original, 300.0, 150.0, 30, 8, "Box Beta");
        int id3 = canvas_add_box(&original, 500.0, 400.0, 35, 15, "Box Gamma");

        const char *content[] = {"Content line 1", "Content line 2", "Content line 3"};
        canvas_add_box_content(&original, id1, content, 3);

        /* Apply different colors */
        Box *box1 = canvas_get_box(&original, id1);
        Box *box2 = canvas_get_box(&original, id2);
        Box *box3 = canvas_get_box(&original, id3);
        box1->color = BOX_COLOR_GREEN;
        box2->color = BOX_COLOR_YELLOW;
        box3->color = BOX_COLOR_MAGENTA;

        canvas_select_box(&original, id2);

        /* Save */
        canvas_save(&original, TEST_FILE);

        /* Load into new canvas */
        Canvas loaded;
        canvas_init(&loaded, 0.0, 0.0);
        canvas_load(&loaded, TEST_FILE);

        /* Verify all properties match */
        ASSERT_EQ(loaded.box_count, original.box_count, "Box count matches");
        ASSERT_NEAR(loaded.world_width, original.world_width, 0.01, "World width matches");
        ASSERT_NEAR(loaded.world_height, original.world_height, 0.01, "World height matches");

        for (int i = 0; i < original.box_count; i++) {
            Box *orig_box = &original.boxes[i];
            Box *load_box = canvas_get_box(&loaded, orig_box->id);

            ASSERT_NOT_NULL(load_box, "Loaded box exists");
            if (load_box) {
                ASSERT_NEAR(load_box->x, orig_box->x, 0.01, "Position X matches");
                ASSERT_NEAR(load_box->y, orig_box->y, 0.01, "Position Y matches");
                ASSERT_EQ(load_box->width, orig_box->width, "Width matches");
                ASSERT_EQ(load_box->height, orig_box->height, "Height matches");
                ASSERT_STR_EQ(load_box->title, orig_box->title, "Title matches");
                ASSERT_EQ(load_box->color, orig_box->color, "Color matches");
                ASSERT_EQ(load_box->selected, orig_box->selected, "Selection state matches");
            }
        }

        canvas_cleanup(&original);
        canvas_cleanup(&loaded);
    }

    TEST("Load non-existent file fails gracefully") {
        Canvas canvas;
        canvas_init(&canvas, 100.0, 100.0);

        int result = canvas_load(&canvas, "nonexistent_file_12345.txt");
        ASSERT_EQ(result, -1, "Load should fail for non-existent file");

        /* Canvas should remain initialized */
        ASSERT_EQ(canvas.box_count, 0, "Canvas should be empty after failed load");

        canvas_cleanup(&canvas);
    }

    TEST("Save with empty canvas") {
        Canvas canvas;
        canvas_init(&canvas, 500.0, 500.0);

        int result = canvas_save(&canvas, TEST_FILE);
        ASSERT_EQ(result, 0, "Should save empty canvas");

        Canvas loaded;
        canvas_init(&loaded, 0.0, 0.0);
        result = canvas_load(&loaded, TEST_FILE);

        ASSERT_EQ(result, 0, "Should load empty canvas");
        ASSERT_EQ(loaded.box_count, 0, "Loaded canvas should be empty");
        ASSERT_NEAR(loaded.world_width, 500.0, 0.01, "World dimensions preserved");

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded);
    }

    TEST("Save preserves box colors correctly") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        /* Create boxes with all available colors */
        int colors[] = {
            BOX_COLOR_DEFAULT,
            BOX_COLOR_RED,
            BOX_COLOR_GREEN,
            BOX_COLOR_BLUE,
            BOX_COLOR_YELLOW,
            BOX_COLOR_MAGENTA,
            BOX_COLOR_CYAN,
            BOX_COLOR_WHITE
        };

        for (int i = 0; i < 8; i++) {
            int id = canvas_add_box(&canvas, i * 50.0, i * 50.0, 20, 5, "Color Box");
            Box *box = canvas_get_box(&canvas, id);
            box->color = colors[i];
        }

        canvas_save(&canvas, TEST_FILE);

        Canvas loaded;
        canvas_init(&loaded, 0.0, 0.0);
        canvas_load(&loaded, TEST_FILE);

        ASSERT_EQ(loaded.box_count, 8, "All color boxes loaded");

        for (int i = 0; i < 8; i++) {
            Box *box = canvas_get_box(&loaded, i + 1);
            ASSERT_NOT_NULL(box, "Color box exists");
            if (box) {
                ASSERT_EQ(box->color, colors[i], "Color preserved correctly");
            }
        }

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded);
    }

    /* Cleanup test file */
    unlink(TEST_FILE);

    TEST_END();
}
