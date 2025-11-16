#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "types.h"
#include "terminal.h"
#include "viewport.h"
#include "render.h"
#include "input.h"

/* Helper to create a box with title */
static void create_box(Box *box, double x, double y, int width, int height, const char *title) {
    box->x = x;
    box->y = y;
    box->width = width;
    box->height = height;
    box->title = title ? strdup(title) : NULL;
    box->content = NULL;
    box->content_lines = 0;
}

/* Helper to add content to a box */
static void add_box_content(Box *box, const char **lines, int count) {
    box->content = malloc(sizeof(char *) * count);
    if (box->content == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        box->content[i] = strdup(lines[i]);
    }
    box->content_lines = count;
}

/* Initialize canvas with sample boxes */
static void init_sample_canvas(Canvas *canvas) {
    canvas->box_count = 0;
    canvas->world_width = 200.0;
    canvas->world_height = 100.0;

    /* Create various boxes in different positions */

    /* Box 1: Welcome box at origin */
    create_box(&canvas->boxes[canvas->box_count], 5, 5, 30, 8, "Welcome to Boxes-Live!");
    const char *welcome[] = {
        "Pan: Arrow keys or WASD",
        "Zoom: +/- or Z/X",
        "Reset: R or 0",
        "Quit: Q or ESC"
    };
    add_box_content(&canvas->boxes[canvas->box_count], welcome, 4);
    canvas->box_count++;

    /* Box 2: Project Info */
    create_box(&canvas->boxes[canvas->box_count], 45, 5, 28, 7, "Project");
    const char *project[] = {
        "Terminal-based canvas",
        "Pan and zoom interface",
        "Like Miro for terminal"
    };
    add_box_content(&canvas->boxes[canvas->box_count], project, 3);
    canvas->box_count++;

    /* Box 3: Features */
    create_box(&canvas->boxes[canvas->box_count], 5, 20, 25, 10, "Features");
    const char *features[] = {
        "- Box drawing",
        "- Smooth panning",
        "- Zoom control",
        "- Large canvas",
        "- Terminal UI",
        "- ncurses based"
    };
    add_box_content(&canvas->boxes[canvas->box_count], features, 6);
    canvas->box_count++;

    /* Box 4: Technical */
    create_box(&canvas->boxes[canvas->box_count], 40, 20, 35, 9, "Technical Details");
    const char *tech[] = {
        "Language: C",
        "Library: ncurses",
        "Coordinates: World -> Screen",
        "Viewport: Camera + Zoom",
        "Rendering: On-demand"
    };
    add_box_content(&canvas->boxes[canvas->box_count], tech, 5);
    canvas->box_count++;

    /* Box 5: Future Ideas */
    create_box(&canvas->boxes[canvas->box_count], 85, 5, 32, 12, "Future Enhancements");
    const char *future[] = {
        "[ ] Box creation/editing",
        "[ ] Save/load canvas",
        "[ ] Connection lines",
        "[ ] Multiple colors",
        "[ ] Box selection",
        "[ ] Copy/paste",
        "[ ] Search boxes",
        "[x] Basic rendering"
    };
    add_box_content(&canvas->boxes[canvas->box_count], future, 8);
    canvas->box_count++;

    /* Box 6: Credits */
    create_box(&canvas->boxes[canvas->box_count], 85, 25, 28, 5, "Credits");
    const char *credits[] = {
        "Built with Claude Code",
        "2025"
    };
    add_box_content(&canvas->boxes[canvas->box_count], credits, 2);
    canvas->box_count++;

    /* Box 7: Small box */
    create_box(&canvas->boxes[canvas->box_count], 25, 35, 15, 4, "Note");
    const char *note[] = {
        "Try zooming!"
    };
    add_box_content(&canvas->boxes[canvas->box_count], note, 1);
    canvas->box_count++;

    /* Box 8: Far away box to test scrolling */
    create_box(&canvas->boxes[canvas->box_count], 120, 40, 25, 6, "Far Away Box");
    const char *far[] = {
        "You found me!",
        "Scroll around to",
        "explore the canvas"
    };
    add_box_content(&canvas->boxes[canvas->box_count], far, 3);
    canvas->box_count++;
}

/* Free canvas memory */
static void cleanup_canvas(Canvas *canvas) {
    for (int i = 0; i < canvas->box_count; i++) {
        Box *box = &canvas->boxes[i];
        if (box->title) {
            free(box->title);
        }
        if (box->content) {
            for (int j = 0; j < box->content_lines; j++) {
                free(box->content[j]);
            }
            free(box->content);
        }
    }
}

int main(void) {
    /* Initialize terminal */
    if (terminal_init() != 0) {
        fprintf(stderr, "Failed to initialize terminal\n");
        return 1;
    }

    /* Initialize viewport */
    Viewport viewport;
    viewport_init(&viewport);
    terminal_update_size(&viewport);

    /* Initialize canvas with sample boxes */
    Canvas canvas;
    init_sample_canvas(&canvas);

    /* Main loop */
    int running = 1;
    while (running) {
        /* Update terminal size (in case of resize) */
        terminal_update_size(&viewport);

        /* Clear screen */
        terminal_clear();

        /* Render canvas */
        render_canvas(&canvas, &viewport);

        /* Render status bar */
        render_status(&viewport);

        /* Refresh display */
        terminal_refresh();

        /* Handle input */
        if (handle_input(&viewport)) {
            running = 0;
        }

        /* Small delay to reduce CPU usage */
        struct timespec ts = {0, 16667000}; /* ~60 FPS (16.667ms) */
        nanosleep(&ts, NULL);
    }

    /* Cleanup */
    cleanup_canvas(&canvas);
    terminal_cleanup();

    return 0;
}
