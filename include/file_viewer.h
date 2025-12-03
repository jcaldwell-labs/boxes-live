#ifndef FILE_VIEWER_H
#define FILE_VIEWER_H

#include "types.h"

/* Maximum file size to load (in bytes) */
#define MAX_FILE_SIZE (1024 * 1024)  /* 1 MB */

/* Maximum line length for file content */
#define MAX_FILE_LINE_LENGTH 1024

/**
 * Load file contents into a box.
 * Sets box->content_type to BOX_CONTENT_FILE and populates box->content.
 *
 * @param box The box to load file contents into
 * @param filepath Path to the file to load
 * @return 0 on success, -1 on error (file not found, permission denied, etc.)
 */
int file_viewer_load(Box *box, const char *filepath);

/**
 * Reload file contents for a file-type box.
 * Only works if box->content_type == BOX_CONTENT_FILE and box->file_path is set.
 *
 * @param box The box to reload
 * @return 0 on success, -1 on error
 */
int file_viewer_reload(Box *box);

/**
 * Free file contents from a box.
 * Clears box->content but preserves file_path for potential reload.
 *
 * @param box The box to clear
 */
void file_viewer_clear(Box *box);

/**
 * Check if a file exists and is readable.
 *
 * @param filepath Path to check
 * @return 1 if file exists and is readable, 0 otherwise
 */
int file_viewer_file_exists(const char *filepath);

/**
 * Get the basename (filename without directory) from a path.
 *
 * @param filepath Full file path
 * @return Pointer to basename within filepath (do not free)
 */
const char *file_viewer_basename(const char *filepath);

#endif /* FILE_VIEWER_H */
