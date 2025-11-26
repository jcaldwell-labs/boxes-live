#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/viewport.h"
#include "../include/persistence.h"

#define TEST_FILE "test_grid_temp.txt"

int main(void) {
    TEST_START();

    TEST("Grid persistence: Save and load grid configuration") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 800.0);

        /* Enable grid and snap with custom spacing */
        canvas.grid.visible = true;
        canvas.grid.snap_enabled = true;
        canvas.grid.spacing = 20;

        /* Add a box for completeness */
        canvas_add_box(&canvas, 100.0, 100.0, 30, 10, "Test Box");

        /* Save canvas */
        int save_result = canvas_save(&canvas, TEST_FILE);
        ASSERT_EQ(save_result, 0, "Canvas saved successfully");

        /* Load into new canvas */
        Canvas loaded_canvas;
        canvas_init(&loaded_canvas, 0.0, 0.0);

        int load_result = canvas_load(&loaded_canvas, TEST_FILE);
        ASSERT_EQ(load_result, 0, "Canvas loaded successfully");

        /* Verify grid configuration preserved */
        ASSERT_EQ(loaded_canvas.grid.visible, true, "Grid visibility preserved");
        ASSERT_EQ(loaded_canvas.grid.snap_enabled, true, "Grid snap preserved");
        ASSERT_EQ(loaded_canvas.grid.spacing, 20, "Grid spacing preserved");

        /* Cleanup */
        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded_canvas);
        unlink(TEST_FILE);
    }

    TEST("Grid persistence: Default grid values when not in file") {
        Canvas canvas;
        canvas_init(&canvas, 500.0, 500.0);
        canvas_add_box(&canvas, 50.0, 50.0, 20, 10, "Box");

        /* Save without modifying grid (should use defaults) */
        canvas_save(&canvas, TEST_FILE);

        /* Load canvas */
        Canvas loaded_canvas;
        canvas_init(&loaded_canvas, 0.0, 0.0);
        canvas_load(&loaded_canvas, TEST_FILE);

        /* Should have default grid values */
        ASSERT_EQ(loaded_canvas.grid.visible, false, "Default grid not visible");
        ASSERT_EQ(loaded_canvas.grid.snap_enabled, false, "Default snap disabled");
        ASSERT_EQ(loaded_canvas.grid.spacing, 10, "Default spacing is 10");

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded_canvas);
        unlink(TEST_FILE);
    }

    TEST("Grid persistence: Only grid visible") {
        Canvas canvas;
        canvas_init(&canvas, 500.0, 500.0);
        
        canvas.grid.visible = true;
        canvas.grid.snap_enabled = false;
        canvas.grid.spacing = 5;

        canvas_add_box(&canvas, 10.0, 10.0, 20, 10, "Box");
        canvas_save(&canvas, TEST_FILE);

        Canvas loaded_canvas;
        canvas_init(&loaded_canvas, 0.0, 0.0);
        canvas_load(&loaded_canvas, TEST_FILE);

        ASSERT_EQ(loaded_canvas.grid.visible, true, "Grid visible");
        ASSERT_EQ(loaded_canvas.grid.snap_enabled, false, "Snap disabled");
        ASSERT_EQ(loaded_canvas.grid.spacing, 5, "Spacing is 5");

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded_canvas);
        unlink(TEST_FILE);
    }

    TEST("Grid persistence: Only snap enabled") {
        Canvas canvas;
        canvas_init(&canvas, 500.0, 500.0);
        
        canvas.grid.visible = false;
        canvas.grid.snap_enabled = true;
        canvas.grid.spacing = 15;

        canvas_add_box(&canvas, 10.0, 10.0, 20, 10, "Box");
        canvas_save(&canvas, TEST_FILE);

        Canvas loaded_canvas;
        canvas_init(&loaded_canvas, 0.0, 0.0);
        canvas_load(&loaded_canvas, TEST_FILE);

        ASSERT_EQ(loaded_canvas.grid.visible, false, "Grid not visible");
        ASSERT_EQ(loaded_canvas.grid.snap_enabled, true, "Snap enabled");
        ASSERT_EQ(loaded_canvas.grid.spacing, 15, "Spacing is 15");

        canvas_cleanup(&canvas);
        canvas_cleanup(&loaded_canvas);
        unlink(TEST_FILE);
    }

    TEST_END();
}
