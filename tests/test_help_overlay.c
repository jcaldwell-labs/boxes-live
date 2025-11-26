/**
 * Test suite for help overlay functionality (Issue #34)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/types.h"
#include "../include/canvas.h"

/* Simple test framework macros */
#define TEST(name) \
    printf("\n[TEST] %s\n", name); \
    test_count++;

#define ASSERT(condition, message) \
    do { \
        assertion_count++; \
        if (!(condition)) { \
            printf("  ✗ FAILED: %s\n", message); \
            failed_count++; \
        } else { \
            printf("  ✓ %s\n", message); \
            passed_count++; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        assertion_count++; \
        if ((expected) != (actual)) { \
            printf("  ✗ FAILED: %s (expected: %d, got: %d)\n", message, (int)(expected), (int)(actual)); \
            failed_count++; \
        } else { \
            printf("  ✓ %s (expected: %d, got: %d)\n", message, (int)(expected), (int)(actual)); \
            passed_count++; \
        } \
    } while(0)

/* Global test counters */
static int test_count = 0;
static int assertion_count = 0;
static int passed_count = 0;
static int failed_count = 0;

/* Test help overlay initialization */
void test_help_overlay_init(void) {
    TEST("Help overlay initialization");
    
    Canvas canvas;
    int result = canvas_init(&canvas, 100.0, 100.0);
    
    ASSERT_EQ(0, result, "Canvas initialization should succeed");
    ASSERT_EQ(0, canvas.help.visible, "Help overlay should be hidden by default");
    
    canvas_cleanup(&canvas);
}

/* Test help overlay toggle */
void test_help_overlay_toggle(void) {
    TEST("Help overlay toggle");
    
    Canvas canvas;
    canvas_init(&canvas, 100.0, 100.0);
    
    /* Initially hidden */
    ASSERT_EQ(0, canvas.help.visible, "Help overlay initially hidden");
    
    /* Toggle visible */
    canvas.help.visible = true;
    ASSERT_EQ(1, canvas.help.visible, "Help overlay should be visible after toggle");
    
    /* Toggle hidden */
    canvas.help.visible = false;
    ASSERT_EQ(0, canvas.help.visible, "Help overlay should be hidden after toggle");
    
    canvas_cleanup(&canvas);
}

/* Test that help overlay state is preserved across operations */
void test_help_overlay_state_preservation(void) {
    TEST("Help overlay state preservation during canvas operations");
    
    Canvas canvas;
    canvas_init(&canvas, 100.0, 100.0);
    
    /* Add a box */
    int box_id = canvas_add_box(&canvas, 10, 10, 20, 5, "Test Box");
    ASSERT(box_id >= 0, "Box added successfully");
    
    /* Toggle help overlay */
    canvas.help.visible = true;
    ASSERT_EQ(1, canvas.help.visible, "Help overlay visible");
    
    /* Select the box */
    canvas_select_box(&canvas, box_id);
    ASSERT_EQ(1, canvas.help.visible, "Help overlay still visible after selecting box");
    
    /* Add another box */
    int box_id2 = canvas_add_box(&canvas, 30, 30, 20, 5, "Test Box 2");
    ASSERT(box_id2 >= 0, "Second box added successfully");
    ASSERT_EQ(1, canvas.help.visible, "Help overlay still visible after adding box");
    
    /* Delete a box */
    canvas_remove_box(&canvas, box_id);
    ASSERT_EQ(1, canvas.help.visible, "Help overlay still visible after deleting box");
    
    /* Hide overlay */
    canvas.help.visible = false;
    ASSERT_EQ(0, canvas.help.visible, "Help overlay hidden");
    
    canvas_cleanup(&canvas);
}

int main(void) {
    printf("===========================================\n");
    printf("Help Overlay Test Suite (Issue #34)\n");
    printf("===========================================\n");
    
    test_help_overlay_init();
    test_help_overlay_toggle();
    test_help_overlay_state_preservation();
    
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", test_count);
    printf("Assertions: %d\n", assertion_count);
    printf("Passed: %d\n", passed_count);
    printf("Failed: %d\n", failed_count);
    
    if (failed_count > 0) {
        printf("\n❌ TESTS FAILED\n");
        return 1;
    }
    
    printf("\n✅ ALL TESTS PASSED\n");
    return 0;
}
