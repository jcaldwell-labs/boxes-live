#ifndef COMMAND_RUNNER_H
#define COMMAND_RUNNER_H

#include "types.h"

/* Maximum command output size (in bytes) */
#define MAX_COMMAND_OUTPUT (64 * 1024)  /* 64 KB */

/* Maximum output lines to store */
#define MAX_COMMAND_LINES 1000

/* Exit code field - stored in box content as metadata */
#define EXIT_CODE_UNKNOWN -999

/**
 * Execute a command and capture its output into a box.
 * Sets box->content_type to BOX_CONTENT_COMMAND and populates box->content.
 * Captures both stdout and stderr (combined).
 *
 * @param box The box to store command output (must have box->command set)
 * @return Exit code of the command, or -1 on execution error
 */
int command_runner_execute(Box *box);

/**
 * Set the command for a box (copies the string).
 * Does not execute - call command_runner_execute() after.
 *
 * @param box The box to set command on
 * @param command The command string to copy
 * @return 0 on success, -1 on error
 */
int command_runner_set_command(Box *box, const char *command);

/**
 * Get the last exit code for a command box.
 * Exit code is stored as metadata in the first line of content.
 *
 * @param box The command box to check
 * @return Exit code, or EXIT_CODE_UNKNOWN if not available
 */
int command_runner_get_exit_code(const Box *box);

/**
 * Clear command output from a box.
 * Clears box->content but preserves box->command for re-run.
 *
 * @param box The box to clear
 */
void command_runner_clear(Box *box);

/**
 * Check if a command string looks safe (basic validation).
 * This is NOT a security guarantee - just catches obvious issues.
 *
 * @param command The command string to validate
 * @return 1 if appears safe, 0 if suspicious
 */
int command_runner_validate(const char *command);

#endif /* COMMAND_RUNNER_H */
