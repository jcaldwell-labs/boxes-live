#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_runner.h"

/* Internal: Store exit code as metadata line */
static void store_exit_code(Box *box, int exit_code) {
    if (!box || !box->content) return;

    /* Add exit code as last line */
    char exit_line[64];
    snprintf(exit_line, sizeof(exit_line), "[Exit: %d]", exit_code);

    /* Reallocate content array for one more line */
    char **new_content = realloc(box->content, sizeof(char *) * (box->content_lines + 1));
    if (new_content) {
        box->content = new_content;
        box->content[box->content_lines] = strdup(exit_line);
        if (box->content[box->content_lines]) {
            box->content_lines++;
        }
    }
}

/* Execute command and capture output */
int command_runner_execute(Box *box) {
    if (!box || !box->command || box->command[0] == '\0') {
        return -1;
    }

    /* Clear any existing output */
    command_runner_clear(box);

    /* Build command with stderr redirect */
    char cmd_with_redirect[1024];
    snprintf(cmd_with_redirect, sizeof(cmd_with_redirect), "%s 2>&1", box->command);

    /* Execute command */
    FILE *pipe = popen(cmd_with_redirect, "r");
    if (!pipe) {
        return -1;
    }

    /* Read output lines */
    char **lines = NULL;
    int line_count = 0;
    int capacity = 64;

    lines = malloc(sizeof(char *) * capacity);
    if (!lines) {
        pclose(pipe);
        return -1;
    }

    char buffer[MAX_COMMAND_OUTPUT];
    size_t total_bytes = 0;

    while (fgets(buffer, sizeof(buffer), pipe) != NULL && line_count < MAX_COMMAND_LINES) {
        size_t len = strlen(buffer);
        total_bytes += len;

        if (total_bytes > MAX_COMMAND_OUTPUT) {
            break;  /* Output limit reached */
        }

        /* Remove trailing newline */
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            /* Also handle Windows CRLF */
            if (len > 1 && buffer[len - 2] == '\r') {
                buffer[len - 2] = '\0';
            }
        }

        /* Grow array if needed */
        if (line_count >= capacity) {
            capacity *= 2;
            char **new_lines = realloc(lines, sizeof(char *) * capacity);
            if (!new_lines) {
                break;
            }
            lines = new_lines;
        }

        lines[line_count] = strdup(buffer);
        if (!lines[line_count]) {
            break;
        }
        line_count++;
    }

    /* Get exit status */
    int status = pclose(pipe);
    int exit_code = 0;

#ifdef _WIN32
    exit_code = status;
#else
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else {
        exit_code = -1;
    }
#endif

    /* If no output, add a placeholder */
    if (line_count == 0) {
        lines[0] = strdup("(no output)");
        if (lines[0]) {
            line_count = 1;
        }
    }

    /* Assign to box */
    box->content = lines;
    box->content_lines = line_count;
    box->content_type = BOX_CONTENT_COMMAND;

    /* Store exit code as metadata */
    store_exit_code(box, exit_code);

    return exit_code;
}

/* Set command for a box */
int command_runner_set_command(Box *box, const char *command) {
    if (!box || !command) {
        return -1;
    }

    /* Free existing command */
    if (box->command) {
        free(box->command);
    }

    box->command = strdup(command);
    if (!box->command) {
        return -1;
    }

    box->content_type = BOX_CONTENT_COMMAND;
    return 0;
}

/* Get exit code from command box */
int command_runner_get_exit_code(const Box *box) {
    if (!box || box->content_type != BOX_CONTENT_COMMAND) {
        return EXIT_CODE_UNKNOWN;
    }

    if (!box->content || box->content_lines == 0) {
        return EXIT_CODE_UNKNOWN;
    }

    /* Exit code is in the last line */
    const char *last_line = box->content[box->content_lines - 1];
    if (!last_line) {
        return EXIT_CODE_UNKNOWN;
    }

    /* Parse "[Exit: N]" format */
    int exit_code;
    if (sscanf(last_line, "[Exit: %d]", &exit_code) == 1) {
        return exit_code;
    }

    return EXIT_CODE_UNKNOWN;
}

/* Clear command output */
void command_runner_clear(Box *box) {
    if (!box) {
        return;
    }

    if (box->content) {
        for (int i = 0; i < box->content_lines; i++) {
            if (box->content[i]) {
                free(box->content[i]);
            }
        }
        free(box->content);
        box->content = NULL;
    }
    box->content_lines = 0;
}

/* Basic command validation */
int command_runner_validate(const char *command) {
    if (!command || command[0] == '\0') {
        return 0;
    }

    /* Reject commands with dangerous shell metacharacters for injection */
    /* Note: This is basic protection, not comprehensive security */
    const char *dangerous = "`$;|&><";
    for (const char *p = command; *p; p++) {
        for (const char *d = dangerous; *d; d++) {
            if (*p == *d) {
                return 0;  /* Potentially dangerous */
            }
        }
    }

    return 1;  /* Appears safe */
}
