#include <math.h>
#include "test.h"
#include "../include/viewport.h"
#include "../include/types.h"

int main(void) {
    TEST_START();

    TEST("Viewport initialization") {
        Viewport vp;
        viewport_init(&vp);

        ASSERT_NEAR(vp.cam_x, 0.0, 0.01, "Camera X starts at 0");
        ASSERT_NEAR(vp.cam_y, 0.0, 0.01, "Camera Y starts at 0");
        ASSERT_NEAR(vp.zoom, 1.0, 0.01, "Zoom starts at 1.0");
        ASSERT_EQ(vp.term_width, 80, "Default terminal width is 80");
        ASSERT_EQ(vp.term_height, 24, "Default terminal height is 24");
    }

    TEST("World to screen transformation (no zoom, no pan)") {
        Viewport vp;
        viewport_init(&vp);

        int sx = world_to_screen_x(&vp, 0.0);
        int sy = world_to_screen_y(&vp, 0.0);

        ASSERT_EQ(sx, 0, "World (0,0) maps to screen (0,0)");
        ASSERT_EQ(sy, 0, "World (0,0) maps to screen (0,0)");

        sx = world_to_screen_x(&vp, 10.0);
        sy = world_to_screen_y(&vp, 20.0);

        ASSERT_EQ(sx, 10, "World (10,20) maps to screen (10,20)");
        ASSERT_EQ(sy, 20, "World (10,20) maps to screen (10,20)");
    }

    TEST("World to screen transformation (with pan)") {
        Viewport vp;
        viewport_init(&vp);

        vp.cam_x = 10.0;
        vp.cam_y = 5.0;

        int sx = world_to_screen_x(&vp, 10.0);
        int sy = world_to_screen_y(&vp, 5.0);

        ASSERT_EQ(sx, 0, "Camera position maps to screen (0,0)");
        ASSERT_EQ(sy, 0, "Camera position maps to screen (0,0)");

        sx = world_to_screen_x(&vp, 20.0);
        sy = world_to_screen_y(&vp, 15.0);

        ASSERT_EQ(sx, 10, "World (20,15) with cam (10,5) maps to screen (10,10)");
        ASSERT_EQ(sy, 10, "World (20,15) with cam (10,5) maps to screen (10,10)");
    }

    TEST("World to screen transformation (with zoom)") {
        Viewport vp;
        viewport_init(&vp);

        vp.zoom = 2.0;  // 2x zoom

        int sx = world_to_screen_x(&vp, 10.0);
        int sy = world_to_screen_y(&vp, 20.0);

        ASSERT_EQ(sx, 20, "2x zoom: world (10,20) maps to screen (20,40)");
        ASSERT_EQ(sy, 40, "2x zoom: world (10,20) maps to screen (20,40)");
    }

    TEST("Screen to world transformation") {
        Viewport vp;
        viewport_init(&vp);

        double wx = screen_to_world_x(&vp, 0);
        double wy = screen_to_world_y(&vp, 0);

        ASSERT_NEAR(wx, 0.0, 0.01, "Screen (0,0) maps to world (0,0)");
        ASSERT_NEAR(wy, 0.0, 0.01, "Screen (0,0) maps to world (0,0)");

        wx = screen_to_world_x(&vp, 10);
        wy = screen_to_world_y(&vp, 20);

        ASSERT_NEAR(wx, 10.0, 0.01, "Screen (10,20) maps to world (10,20)");
        ASSERT_NEAR(wy, 20.0, 0.01, "Screen (10,20) maps to world (10,20)");
    }

    TEST("Screen to world transformation (with pan and zoom)") {
        Viewport vp;
        viewport_init(&vp);

        vp.cam_x = 10.0;
        vp.cam_y = 5.0;
        vp.zoom = 2.0;

        double wx = screen_to_world_x(&vp, 0);
        double wy = screen_to_world_y(&vp, 0);

        ASSERT_NEAR(wx, 10.0, 0.01, "Screen (0,0) with cam (10,5) zoom 2x maps to world (10,5)");
        ASSERT_NEAR(wy, 5.0, 0.01, "Screen (0,0) with cam (10,5) zoom 2x maps to world (10,5)");

        wx = screen_to_world_x(&vp, 20);
        wy = screen_to_world_y(&vp, 40);

        ASSERT_NEAR(wx, 20.0, 0.01, "Screen (20,40) with cam (10,5) zoom 2x maps to world (20,25)");
        ASSERT_NEAR(wy, 25.0, 0.01, "Screen (20,40) with cam (10,5) zoom 2x maps to world (20,25)");
    }

    TEST("Roundtrip coordinate transformation") {
        Viewport vp;
        viewport_init(&vp);

        vp.cam_x = 15.0;
        vp.cam_y = 25.0;
        vp.zoom = 1.5;

        double original_x = 100.0;
        double original_y = 200.0;

        int sx = world_to_screen_x(&vp, original_x);
        int sy = world_to_screen_y(&vp, original_y);

        double wx = screen_to_world_x(&vp, sx);
        double wy = screen_to_world_y(&vp, sy);

        ASSERT_NEAR(wx, original_x, 1.0, "Roundtrip X coordinate is close to original");
        ASSERT_NEAR(wy, original_y, 1.0, "Roundtrip Y coordinate is close to original");
    }

    TEST("Viewport panning") {
        Viewport vp;
        viewport_init(&vp);

        double initial_x = vp.cam_x;
        double initial_y = vp.cam_y;

        viewport_pan(&vp, 10.0, 5.0);

        ASSERT_NEAR(vp.cam_x, initial_x + 10.0, 0.01, "Pan X increases camera X");
        ASSERT_NEAR(vp.cam_y, initial_y + 5.0, 0.01, "Pan Y increases camera Y");

        viewport_pan(&vp, -5.0, -3.0);

        ASSERT_NEAR(vp.cam_x, initial_x + 5.0, 0.01, "Negative pan decreases camera position");
        ASSERT_NEAR(vp.cam_y, initial_y + 2.0, 0.01, "Negative pan decreases camera position");
    }

    TEST("Viewport panning with zoom") {
        Viewport vp;
        viewport_init(&vp);

        vp.zoom = 2.0;

        double initial_x = vp.cam_x;
        double initial_y = vp.cam_y;

        viewport_pan(&vp, 10.0, 5.0);

        // Pan is divided by zoom, so at 2x zoom, pan(10, 5) moves camera by (5, 2.5)
        ASSERT_NEAR(vp.cam_x, initial_x + 5.0, 0.01, "Pan with 2x zoom: 10 becomes 5");
        ASSERT_NEAR(vp.cam_y, initial_y + 2.5, 0.01, "Pan with 2x zoom: 5 becomes 2.5");
    }

    TEST("Viewport zooming") {
        Viewport vp;
        viewport_init(&vp);
        vp.term_width = 80;
        vp.term_height = 24;

        double initial_zoom = vp.zoom;

        viewport_zoom(&vp, 1.2);

        ASSERT_NEAR(vp.zoom, initial_zoom * 1.2, 0.01, "Zoom increases by factor");

        viewport_zoom(&vp, 1.0 / 1.2);

        ASSERT_NEAR(vp.zoom, initial_zoom, 0.01, "Zoom decreases back to original");
    }

    TEST("Zoom limits") {
        Viewport vp;
        viewport_init(&vp);
        vp.term_width = 80;
        vp.term_height = 24;

        // Try to zoom way in
        for (int i = 0; i < 20; i++) {
            viewport_zoom(&vp, 2.0);
        }

        ASSERT_NEAR(vp.zoom, 10.0, 0.01, "Zoom clamped to max (10.0)");

        // Try to zoom way out
        for (int i = 0; i < 20; i++) {
            viewport_zoom(&vp, 0.5);
        }

        ASSERT_NEAR(vp.zoom, 0.1, 0.01, "Zoom clamped to min (0.1)");
    }

    TEST("Visibility testing") {
        Viewport vp;
        viewport_init(&vp);
        vp.term_width = 80;
        vp.term_height = 24;

        // Point at origin should be visible
        int visible = is_visible(&vp, 0.0, 0.0);
        ASSERT(visible, "Point (0,0) is visible");

        // Point within viewport should be visible
        visible = is_visible(&vp, 40.0, 12.0);
        ASSERT(visible, "Point (40,12) within viewport is visible");

        // Point outside viewport should not be visible
        visible = is_visible(&vp, 100.0, 100.0);
        ASSERT(!visible, "Point (100,100) outside viewport is not visible");

        // Negative coordinates should not be visible
        visible = is_visible(&vp, -10.0, -10.0);
        ASSERT(!visible, "Point (-10,-10) is not visible");
    }

    TEST_END();
}
