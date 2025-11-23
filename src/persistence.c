#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "persistence.h"
#include "canvas.h"

#define FILE_MAGIC "BOXES_CANVAS_V1"
#define MAX_LINE_LENGTH 1024

/* Current file for reload functionality */
static const char *current_file = NULL;

/* Set the current file name */
void persistence_set_current_file(const char *filename) {
    current_file = filename;
}

/* Get the current file name */
const char *persistence_get_current_file(void) {
    return current_file;
}

/* Save canvas to file */
int canvas_save(const Canvas *canvas, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return -1;
    }

    /* Write header */
    fprintf(f, "%s\n", FILE_MAGIC);
    fprintf(f, "%.2f %.2f\n", canvas->world_width, canvas->world_height);
    fprintf(f, "%d\n", canvas->box_count);

    /* Write each box */
    for (int i = 0; i < canvas->box_count; i++) {
        const Box *box = &canvas->boxes[i];

        /* Write box properties */
        fprintf(f, "%d %.2f %.2f %d %d %d %d\n",
                box->id, box->x, box->y, box->width, box->height,
                box->selected ? 1 : 0, box->color);

        /* Write title */
        if (box->title) {
            fprintf(f, "%s\n", box->title);
        } else {
            fprintf(f, "NULL\n");
        }

        /* Write content */
        fprintf(f, "%d\n", box->content_lines);
        for (int j = 0; j < box->content_lines; j++) {
            fprintf(f, "%s\n", box->content[j]);
        }
    }

    /* Write next_id and selected_index */
    fprintf(f, "%d %d\n", canvas->next_id, canvas->selected_index);

    fclose(f);
    return 0;
}

/* Load canvas from file */
int canvas_load(Canvas *canvas, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return -1;
    }

    char magic[MAX_LINE_LENGTH];
    if (fgets(magic, sizeof(magic), f) == NULL) {
        fclose(f);
        return -1;
    }

    /* Remove newline */
    magic[strcspn(magic, "\n")] = 0;

    /* Check magic number */
    if (strcmp(magic, FILE_MAGIC) != 0) {
        fclose(f);
        return -1;
    }

    /* Read world size */
    double world_width, world_height;
    if (fscanf(f, "%lf %lf\n", &world_width, &world_height) != 2) {
        fclose(f);
        return -1;
    }

    /* Cleanup existing canvas data before reinitializing
     * Only call cleanup if canvas appears to be initialized (boxes pointer is not NULL)
     */
    if (canvas->boxes != NULL) {
        canvas_cleanup(canvas);
    }

    /* Initialize canvas */
    if (canvas_init(canvas, world_width, world_height) != 0) {
        fclose(f);
        return -1;
    }

    /* Read box count */
    int box_count;
    if (fscanf(f, "%d\n", &box_count) != 1) {
        canvas_cleanup(canvas);
        fclose(f);
        return -1;
    }

    /* Read each box */
    for (int i = 0; i < box_count; i++) {
        int id, width, height, selected_flag, color;
        double x, y;

        /* Read box properties */
        if (fscanf(f, "%d %lf %lf %d %d %d %d\n",
                   &id, &x, &y, &width, &height, &selected_flag, &color) != 7) {
            canvas_cleanup(canvas);
            fclose(f);
            return -1;
        }

        /* Read title */
        char title_line[MAX_LINE_LENGTH];
        if (fgets(title_line, sizeof(title_line), f) == NULL) {
            canvas_cleanup(canvas);
            fclose(f);
            return -1;
        }
        title_line[strcspn(title_line, "\n")] = 0;

        char *title = NULL;
        if (strcmp(title_line, "NULL") != 0) {
            title = title_line;
        }

        /* Add box to canvas */
        int new_box_id = canvas_add_box(canvas, x, y, width, height, title);
        if (new_box_id < 0) {
            canvas_cleanup(canvas);
            fclose(f);
            return -1;
        }

        /* Update box ID to match saved ID */
        Box *box = canvas_get_box(canvas, new_box_id);
        if (box) {
            box->id = id;
            box->selected = selected_flag ? true : false;
            box->color = color;
        }

        /* Read content lines */
        int content_lines;
        if (fscanf(f, "%d\n", &content_lines) != 1) {
            canvas_cleanup(canvas);
            fclose(f);
            return -1;
        }

        if (content_lines > 0) {
            char **content = malloc(sizeof(char *) * content_lines);
            if (content == NULL) {
                canvas_cleanup(canvas);
                fclose(f);
                return -1;
            }

            for (int j = 0; j < content_lines; j++) {
                char line[MAX_LINE_LENGTH];
                if (fgets(line, sizeof(line), f) == NULL) {
                    for (int k = 0; k < j; k++) {
                        free(content[k]);
                    }
                    free(content);
                    canvas_cleanup(canvas);
                    fclose(f);
                    return -1;
                }
                line[strcspn(line, "\n")] = 0;
                content[j] = strdup(line);
            }

            /* Set box content directly */
            if (box) {
                box->content = content;
                box->content_lines = content_lines;
            }
        }
    }

    /* Read next_id and selected_index */
    if (fscanf(f, "%d %d\n", &canvas->next_id, &canvas->selected_index) != 2) {
        /* Not fatal, just use defaults */
        canvas->next_id = canvas->box_count + 1;
        canvas->selected_index = -1;
    }

    fclose(f);
    return 0;
}
