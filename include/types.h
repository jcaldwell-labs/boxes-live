#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* Maximum number of boxes on the canvas */
#define MAX_BOXES 100

/* Box structure representing a rectangular region with content */
typedef struct {
    double x;           /* World X coordinate */
    double y;           /* World Y coordinate */
    int width;          /* Width in characters */
    int height;         /* Height in characters */
    char *title;        /* Box title */
    char **content;     /* Array of content lines */
    int content_lines;  /* Number of content lines */
} Box;

/* Viewport structure for camera/view control */
typedef struct {
    double cam_x;       /* Camera X position in world space */
    double cam_y;       /* Camera Y position in world space */
    double zoom;        /* Zoom level (1.0 = normal, >1.0 = zoomed in) */
    int term_width;     /* Terminal width in characters */
    int term_height;    /* Terminal height in characters */
} Viewport;

/* Canvas structure containing all boxes */
typedef struct {
    Box boxes[MAX_BOXES];
    int box_count;
    double world_width;
    double world_height;
} Canvas;

#endif /* TYPES_H */
