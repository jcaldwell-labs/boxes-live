#include "test.h"
#include "export.h"
#include "canvas.h"
#include "viewport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test export functionality */
int main(void) {
    Canvas canvas;
    Viewport vp;
    
    TEST_START();
    
    TEST("Export viewport to file") {
        /* Initialize canvas */
        canvas_init(&canvas, 200.0, 100.0);
        
        /* Initialize viewport */
        vp.cam_x = 0.0;
        vp.cam_y = 0.0;
        vp.zoom = 1.0;
        vp.term_width = 80;
        vp.term_height = 24;
        
        /* Add some boxes */
        int box1 = canvas_add_box(&canvas, 10.0, 5.0, 25, 6, "API Spec");
        ASSERT(box1 >= 0, "Box 1 created");
        
        const char *content1[] = {"REST endpoints", "Authentication"};
        canvas_add_box_content(&canvas, box1, content1, 2);
        
        int box2 = canvas_add_box(&canvas, 50.0, 5.0, 25, 6, "Notes");
        ASSERT(box2 >= 0, "Box 2 created");
        
        const char *content2[] = {"TODO: Review", "Documentation"};
        canvas_add_box_content(&canvas, box2, content2, 2);
        
        int box3 = canvas_add_box(&canvas, 30.0, 15.0, 25, 6, "Build");
        ASSERT(box3 >= 0, "Box 3 created");
        
        const char *content3[] = {"CI/CD setup", "Tests passing"};
        canvas_add_box_content(&canvas, box3, content3, 2);
        
        /* Add some connections */
        canvas_add_connection(&canvas, box1, box2);
        canvas_add_connection(&canvas, box1, box3);
        
        /* Export to file */
        const char *test_file = "/tmp/test-export.txt";
        int result = export_viewport_to_file(&canvas, &vp, test_file);
        ASSERT_EQ(result, 0, "Export succeeded");
        
        /* Verify file exists and has content */
        FILE *fp = fopen(test_file, "r");
        ASSERT_NOT_NULL(fp, "Export file created");
        
        if (fp) {
            char line[256];
            int has_header = 0;
            int has_separator = 0;
            int has_footer = 0;
            int has_boxes = 0;
            
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "boxes-live canvas export")) {
                    has_header = 1;
                }
                if (strstr(line, "═══════")) {
                    has_separator = 1;
                }
                if (strstr(line, "Boxes:")) {
                    has_footer = 1;
                }
                if (strstr(line, "API Spec") || strstr(line, "Notes") || strstr(line, "Build")) {
                    has_boxes = 1;
                }
            }
            
            fclose(fp);
            
            ASSERT(has_header, "Export has header");
            ASSERT(has_separator, "Export has separator");
            ASSERT(has_footer, "Export has footer with metadata");
            ASSERT(has_boxes, "Export contains box titles");
        }
        
        /* Clean up test file */
        remove(test_file);
        
        /* Clean up canvas */
        canvas_cleanup(&canvas);
    }
    
    TEST_END();
}
