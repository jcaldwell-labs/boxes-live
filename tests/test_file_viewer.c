#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/file_viewer.h"

#define TEST_FILE "test_file_viewer_temp.txt"

/* Helper function to create a test file with content */
static int create_test_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

int main(void) {
    TEST_START();

    TEST("file_viewer_file_exists - existing file") {
        create_test_file(TEST_FILE, "test content\n");

        int result = file_viewer_file_exists(TEST_FILE);
        ASSERT_EQ(result, 1, "Existing file should return 1");

        unlink(TEST_FILE);
    }

    TEST("file_viewer_file_exists - non-existent file") {
        int result = file_viewer_file_exists("nonexistent_file_xyz123.txt");
        ASSERT_EQ(result, 0, "Non-existent file should return 0");
    }

    TEST("file_viewer_file_exists - NULL path") {
        int result = file_viewer_file_exists(NULL);
        ASSERT_EQ(result, 0, "NULL path should return 0");
    }

    TEST("file_viewer_basename - Unix paths") {
        const char *result;

        result = file_viewer_basename("/path/to/file.txt");
        ASSERT_STR_EQ(result, "file.txt", "Unix path basename");

        result = file_viewer_basename("/file.txt");
        ASSERT_STR_EQ(result, "file.txt", "Root path basename");

        result = file_viewer_basename("file.txt");
        ASSERT_STR_EQ(result, "file.txt", "Bare filename");
    }

    TEST("file_viewer_basename - Windows paths") {
        const char *result;

        result = file_viewer_basename("C:\\path\\to\\file.txt");
        ASSERT_STR_EQ(result, "file.txt", "Windows path basename");

        result = file_viewer_basename("C:\\file.txt");
        ASSERT_STR_EQ(result, "file.txt", "Root Windows path basename");
    }

    TEST("file_viewer_basename - NULL path") {
        const char *result = file_viewer_basename(NULL);
        ASSERT_NULL(result, "NULL path should return NULL");
    }

    TEST("file_viewer_load - Simple file") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "Line 1\nLine 2\nLine 3\n");

        int result = file_viewer_load(box, TEST_FILE);
        ASSERT_EQ(result, 0, "Load should succeed");

        ASSERT_EQ(box->content_type, BOX_CONTENT_FILE, "Content type should be FILE");
        ASSERT_NOT_NULL(box->file_path, "file_path should be set");
        ASSERT_STR_EQ(box->file_path, TEST_FILE, "file_path matches");

        ASSERT_EQ(box->content_lines, 3, "Should have 3 lines");
        ASSERT_NOT_NULL(box->content, "Content array should be allocated");

        if (box->content && box->content_lines >= 3) {
            ASSERT_STR_EQ(box->content[0], "Line 1", "First line content");
            ASSERT_STR_EQ(box->content[1], "Line 2", "Second line content");
            ASSERT_STR_EQ(box->content[2], "Line 3", "Third line content");
        }

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_load - Windows line endings") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "Line 1\r\nLine 2\r\n");

        int result = file_viewer_load(box, TEST_FILE);
        ASSERT_EQ(result, 0, "Load should succeed with CRLF");

        ASSERT_EQ(box->content_lines, 2, "Should have 2 lines");
        if (box->content && box->content_lines >= 2) {
            ASSERT_STR_EQ(box->content[0], "Line 1", "First line without CR");
            ASSERT_STR_EQ(box->content[1], "Line 2", "Second line without CR");
        }

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_load - Empty file") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "");

        int result = file_viewer_load(box, TEST_FILE);
        ASSERT_EQ(result, 0, "Load should succeed for empty file");

        ASSERT_EQ(box->content_type, BOX_CONTENT_FILE, "Content type should be FILE");
        ASSERT_EQ(box->content_lines, 0, "Should have 0 lines");

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_load - Non-existent file") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        int result = file_viewer_load(box, "nonexistent_file_xyz123.txt");
        ASSERT_EQ(result, -1, "Load should fail for non-existent file");

        canvas_cleanup(&canvas);
    }

    TEST("file_viewer_load - NULL parameters") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "content\n");

        int result = file_viewer_load(NULL, TEST_FILE);
        ASSERT_EQ(result, -1, "NULL box should return -1");

        result = file_viewer_load(box, NULL);
        ASSERT_EQ(result, -1, "NULL path should return -1");

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_reload - Basic reload") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "Original\n");
        file_viewer_load(box, TEST_FILE);

        ASSERT_EQ(box->content_lines, 1, "Initially 1 line");

        /* Modify file */
        create_test_file(TEST_FILE, "Updated\nNew line\n");

        int result = file_viewer_reload(box);
        ASSERT_EQ(result, 0, "Reload should succeed");

        ASSERT_EQ(box->content_lines, 2, "After reload 2 lines");
        if (box->content && box->content_lines >= 2) {
            ASSERT_STR_EQ(box->content[0], "Updated", "Updated first line");
            ASSERT_STR_EQ(box->content[1], "New line", "New second line");
        }

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_reload - Non-file box") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Box is TEXT type by default, not FILE */
        int result = file_viewer_reload(box);
        ASSERT_EQ(result, -1, "Reload should fail for non-file box");

        canvas_cleanup(&canvas);
    }

    TEST("file_viewer_clear - Clears content but preserves path") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        create_test_file(TEST_FILE, "Line 1\nLine 2\n");
        file_viewer_load(box, TEST_FILE);

        ASSERT_EQ(box->content_lines, 2, "Has 2 lines before clear");
        ASSERT_NOT_NULL(box->content, "Content allocated before clear");

        file_viewer_clear(box);

        ASSERT_EQ(box->content_lines, 0, "Has 0 lines after clear");
        ASSERT_NULL(box->content, "Content is NULL after clear");
        /* Note: file_path is NOT cleared by file_viewer_clear */

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    TEST("file_viewer_clear - NULL box is safe") {
        /* Should not crash */
        file_viewer_clear(NULL);
        ASSERT(1, "NULL box handled safely");
    }

    TEST("file_viewer_load - Replaces existing content") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Add initial content */
        const char *initial[] = {"Old line 1", "Old line 2", "Old line 3"};
        canvas_add_box_content(&canvas, box_id, initial, 3);

        ASSERT_EQ(box->content_lines, 3, "Initially 3 lines");

        /* Load file - should replace content */
        create_test_file(TEST_FILE, "New line\n");
        int result = file_viewer_load(box, TEST_FILE);

        ASSERT_EQ(result, 0, "Load should succeed");
        ASSERT_EQ(box->content_lines, 1, "Now has 1 line");
        if (box->content && box->content_lines >= 1) {
            ASSERT_STR_EQ(box->content[0], "New line", "Content replaced");
        }

        canvas_cleanup(&canvas);
        unlink(TEST_FILE);
    }

    /* Cleanup test file */
    unlink(TEST_FILE);

    TEST_END();
}
