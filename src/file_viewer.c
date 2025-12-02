#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "file_viewer.h"

/* Load file contents into a box */
int file_viewer_load(Box *box, const char *filepath) {
    if (box == NULL || filepath == NULL) {
        return -1;
    }

    /* Check file exists and get size */
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return -1;
    }

    /* Check file size limit */
    if (st.st_size > MAX_FILE_SIZE) {
        return -1;
    }

    /* Open file for reading */
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        return -1;
    }

    /* Clear existing content */
    file_viewer_clear(box);

    /* Count lines first */
    int line_count = 0;
    char line_buf[MAX_FILE_LINE_LENGTH];
    while (fgets(line_buf, sizeof(line_buf), f) != NULL) {
        line_count++;
    }

    /* Rewind to start */
    rewind(f);

    /* Allocate content array */
    if (line_count > 0) {
        box->content = malloc(sizeof(char *) * line_count);
        if (box->content == NULL) {
            fclose(f);
            return -1;
        }

        /* Read lines into content array */
        int i = 0;
        while (fgets(line_buf, sizeof(line_buf), f) != NULL && i < line_count) {
            /* Remove trailing newline */
            size_t len = strlen(line_buf);
            if (len > 0 && line_buf[len - 1] == '\n') {
                line_buf[len - 1] = '\0';
                /* Also handle Windows CRLF */
                if (len > 1 && line_buf[len - 2] == '\r') {
                    line_buf[len - 2] = '\0';
                }
            }
            box->content[i] = strdup(line_buf);
            if (box->content[i] == NULL) {
                /* Cleanup on allocation failure */
                for (int j = 0; j < i; j++) {
                    free(box->content[j]);
                }
                free(box->content);
                box->content = NULL;
                fclose(f);
                return -1;
            }
            i++;
        }
        box->content_lines = i;
    } else {
        box->content = NULL;
        box->content_lines = 0;
    }

    fclose(f);

    /* Set file path and content type */
    if (box->file_path != NULL) {
        free(box->file_path);
    }
    box->file_path = strdup(filepath);
    if (box->file_path == NULL) {
        file_viewer_clear(box);
        return -1;
    }
    box->content_type = BOX_CONTENT_FILE;

    return 0;
}

/* Reload file contents for a file-type box */
int file_viewer_reload(Box *box) {
    if (box == NULL) {
        return -1;
    }

    if (box->content_type != BOX_CONTENT_FILE || box->file_path == NULL) {
        return -1;
    }

    /* Save file path before clear */
    char *saved_path = strdup(box->file_path);
    if (saved_path == NULL) {
        return -1;
    }

    /* Clear and reload */
    file_viewer_clear(box);

    int result = file_viewer_load(box, saved_path);
    free(saved_path);

    return result;
}

/* Free file contents from a box */
void file_viewer_clear(Box *box) {
    if (box == NULL) {
        return;
    }

    /* Free content lines */
    if (box->content != NULL) {
        for (int i = 0; i < box->content_lines; i++) {
            if (box->content[i] != NULL) {
                free(box->content[i]);
            }
        }
        free(box->content);
        box->content = NULL;
    }
    box->content_lines = 0;
}

/* Check if file exists and is readable */
int file_viewer_file_exists(const char *filepath) {
    if (filepath == NULL) {
        return 0;
    }

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        return 0;
    }
    fclose(f);
    return 1;
}

/* Get basename from path */
const char *file_viewer_basename(const char *filepath) {
    if (filepath == NULL) {
        return NULL;
    }

    /* Find last path separator (handle both Unix and Windows) */
    const char *last_sep = NULL;
    const char *p = filepath;

    while (*p != '\0') {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
        p++;
    }

    if (last_sep != NULL) {
        return last_sep + 1;
    }

    return filepath;
}
