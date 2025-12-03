#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/command_runner.h"

int main(void) {
    TEST_START();

    TEST("command_runner_set_command - Basic set") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        int result = command_runner_set_command(box, "echo hello");
        ASSERT_EQ(result, 0, "Set command should succeed");
        ASSERT_NOT_NULL(box->command, "Command should be set");
        ASSERT_STR_EQ(box->command, "echo hello", "Command matches");
        ASSERT_EQ(box->content_type, BOX_CONTENT_COMMAND, "Content type is COMMAND");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_set_command - NULL parameters") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        int result = command_runner_set_command(NULL, "echo hello");
        ASSERT_EQ(result, -1, "NULL box should fail");

        result = command_runner_set_command(box, NULL);
        ASSERT_EQ(result, -1, "NULL command should fail");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_execute - Simple echo") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        command_runner_set_command(box, "echo hello");
        int exit_code = command_runner_execute(box);

        ASSERT_EQ(exit_code, 0, "Echo should exit with 0");
        ASSERT_EQ(box->content_type, BOX_CONTENT_COMMAND, "Content type is COMMAND");
        ASSERT(box->content_lines > 0, "Should have output lines");

        /* First line should contain "hello" (may have trailing space on Windows) */
        if (box->content && box->content_lines > 0) {
            ASSERT(strstr(box->content[0], "hello") != NULL, "First line contains hello");
        }

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_execute - Exit code captured") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Command that exits with 0 */
        command_runner_set_command(box, "echo success");
        int exit_code = command_runner_execute(box);
        ASSERT_EQ(exit_code, 0, "Successful command exits with 0");

        int stored_code = command_runner_get_exit_code(box);
        ASSERT_EQ(stored_code, 0, "Stored exit code is 0");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_execute - NULL box") {
        int result = command_runner_execute(NULL);
        ASSERT_EQ(result, -1, "NULL box should return -1");
    }

    TEST("command_runner_execute - No command set") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Don't set a command */
        int result = command_runner_execute(box);
        ASSERT_EQ(result, -1, "No command should return -1");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_clear - Clears output") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        command_runner_set_command(box, "echo hello");
        command_runner_execute(box);

        ASSERT(box->content_lines > 0, "Has output before clear");

        command_runner_clear(box);

        ASSERT_EQ(box->content_lines, 0, "No content after clear");
        ASSERT_NULL(box->content, "Content is NULL after clear");
        /* Command should still be set */
        ASSERT_NOT_NULL(box->command, "Command preserved after clear");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_clear - NULL box is safe") {
        /* Should not crash */
        command_runner_clear(NULL);
        ASSERT(1, "NULL box handled safely");
    }

    TEST("command_runner_get_exit_code - Non-command box") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Box is TEXT type by default */
        int exit_code = command_runner_get_exit_code(box);
        ASSERT_EQ(exit_code, EXIT_CODE_UNKNOWN, "Non-command box returns unknown");

        canvas_cleanup(&canvas);
    }

    TEST("command_runner_validate - Safe commands") {
        ASSERT_EQ(command_runner_validate("echo hello"), 1, "echo hello is safe");
        ASSERT_EQ(command_runner_validate("ls -la"), 1, "ls -la is safe");
        ASSERT_EQ(command_runner_validate("make test"), 1, "make test is safe");
        ASSERT_EQ(command_runner_validate("git status"), 1, "git status is safe");
    }

    TEST("command_runner_validate - Dangerous commands") {
        ASSERT_EQ(command_runner_validate("echo $VAR"), 0, "$ is dangerous");
        ASSERT_EQ(command_runner_validate("echo `cmd`"), 0, "Backtick is dangerous");
        ASSERT_EQ(command_runner_validate("cmd1 | cmd2"), 0, "Pipe is dangerous");
        ASSERT_EQ(command_runner_validate("cmd1 && cmd2"), 0, "And is dangerous");
        ASSERT_EQ(command_runner_validate("cmd1 ; cmd2"), 0, "Semicolon is dangerous");
        ASSERT_EQ(command_runner_validate("cmd > file"), 0, "Redirect is dangerous");
    }

    TEST("command_runner_validate - NULL and empty") {
        ASSERT_EQ(command_runner_validate(NULL), 0, "NULL is invalid");
        ASSERT_EQ(command_runner_validate(""), 0, "Empty is invalid");
    }

    TEST("command_runner_execute - Re-run replaces content") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* First run */
        command_runner_set_command(box, "echo first");
        command_runner_execute(box);
        int first_lines = box->content_lines;
        ASSERT(first_lines > 0, "First run has output");

        /* Second run - should replace content */
        command_runner_execute(box);
        ASSERT(box->content_lines > 0, "Second run has output");

        /* Check first line contains "first" (command unchanged, may have trailing space) */
        if (box->content && box->content_lines > 0) {
            ASSERT(strstr(box->content[0], "first") != NULL, "Re-run output contains first");
        }

        canvas_cleanup(&canvas);
    }

    TEST("command_runner - Multi-line output") {
        Canvas canvas;
        canvas_init(&canvas, 1000.0, 1000.0);

        int box_id = canvas_add_box(&canvas, 10.0, 20.0, 30, 10, "Test Box");
        Box *box = canvas_get_box(&canvas, box_id);

        /* Command with multiple lines of output */
#ifdef _WIN32
        command_runner_set_command(box, "echo line1 & echo line2 & echo line3");
#else
        command_runner_set_command(box, "echo line1; echo line2; echo line3");
#endif
        command_runner_execute(box);

        /* Should have at least 3 output lines plus exit code line */
        ASSERT(box->content_lines >= 3, "Should have multiple output lines");

        canvas_cleanup(&canvas);
    }

    TEST_END();
}
