#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

/* Initial capacity for dynamic box array */
#define INITIAL_BOX_CAPACITY 16

/* Box structure representing a rectangular region with content */
typedef struct {
    double x;           /* World X coordinate */
    double y;           /* World Y coordinate */
    int width;          /* Width in characters */
    int height;         /* Height in characters */
    char *title;        /* Box title */
    char **content;     /* Array of content lines */
    int content_lines;  /* Number of content lines */
    bool selected;      /* Is this box currently selected? */
    int id;             /* Unique box identifier */
} Box;

/* Viewport structure for camera/view control */
typedef struct {
    double cam_x;       /* Camera X position in world space */
    double cam_y;       /* Camera Y position in world space */
    double zoom;        /* Zoom level (1.0 = normal, >1.0 = zoomed in) */
    int term_width;     /* Terminal width in characters */
    int term_height;    /* Terminal height in characters */
} Viewport;

/* Canvas structure containing all boxes (dynamic array) */
typedef struct {
    Box *boxes;         /* Dynamic array of boxes */
    int box_count;      /* Number of boxes currently in use */
    int box_capacity;   /* Allocated capacity */
    double world_width;
    double world_height;
    int next_id;        /* Next unique ID to assign */
    int selected_index; /* Index of selected box, -1 if none */
} Canvas;

#endif /* TYPES_H */
