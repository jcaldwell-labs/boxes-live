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
#include "canvas.h"

/* Initialize canvas with sample boxes */
static void init_sample_canvas(Canvas *canvas) {
    canvas_init(canvas, 200.0, 100.0);

    /* Create various boxes in different positions */
    int box_id;

    /* Box 1: Welcome box at origin */
    box_id = canvas_add_box(canvas, 5, 5, 30, 8, "Welcome to Boxes-Live!");
    const char *welcome[] = {
        "Pan: Arrow keys or WASD",
        "Zoom: +/- or Z/X",
        "Reset: R or 0",
        "Quit: Q or ESC",
        "Mouse: Click to select",
        "N: New box, D: Delete"
    };
    canvas_add_box_content(canvas, box_id, welcome, 6);

    /* Box 2: Project Info */
    box_id = canvas_add_box(canvas, 45, 5, 28, 7, "Project");
    const char *project[] = {
        "Terminal-based canvas",
        "Pan and zoom interface",
        "Like Miro for terminal"
    };
    canvas_add_box_content(canvas, box_id, project, 3);

    /* Box 3: Features */
    box_id = canvas_add_box(canvas, 5, 20, 25, 10, "Features");
    const char *features[] = {
        "- Box drawing",
        "- Smooth panning",
        "- Zoom control",
        "- Large canvas",
        "- Terminal UI",
        "- ncurses based",
        "- Dynamic boxes"
    };
    canvas_add_box_content(canvas, box_id, features, 7);

    /* Box 4: Technical */
    box_id = canvas_add_box(canvas, 40, 20, 35, 9, "Technical Details");
    const char *tech[] = {
        "Language: C",
        "Library: ncurses",
        "Coordinates: World -> Screen",
        "Viewport: Camera + Zoom",
        "Rendering: On-demand",
        "Memory: Dynamic allocation"
    };
    canvas_add_box_content(canvas, box_id, tech, 6);

    /* Box 5: Future Ideas */
    box_id = canvas_add_box(canvas, 85, 5, 32, 12, "Progress");
    const char *future[] = {
        "[x] Box creation/deletion",
        "[x] Dynamic canvas",
        "[x] Box selection",
        "[x] Mouse support",
        "[ ] Save/load canvas",
        "[ ] Connection lines",
        "[ ] Multiple colors",
        "[ ] Copy/paste",
        "[ ] Search boxes"
    };
    canvas_add_box_content(canvas, box_id, future, 9);

    /* Box 6: Credits */
    box_id = canvas_add_box(canvas, 85, 25, 28, 5, "Credits");
    const char *credits[] = {
        "Built with Claude Code",
        "2025"
    };
    canvas_add_box_content(canvas, box_id, credits, 2);

    /* Box 7: Small box */
    box_id = canvas_add_box(canvas, 25, 35, 15, 4, "Note");
    const char *note[] = {
        "Try zooming!"
    };
    canvas_add_box_content(canvas, box_id, note, 1);

    /* Box 8: Far away box to test scrolling */
    box_id = canvas_add_box(canvas, 120, 40, 25, 6, "Far Away Box");
    const char *far[] = {
        "You found me!",
        "Scroll around to",
        "explore the canvas"
    };
    canvas_add_box_content(canvas, box_id, far, 3);
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
        render_status(&canvas, &viewport);

        /* Refresh display */
        terminal_refresh();

        /* Handle input */
        if (handle_input(&canvas, &viewport)) {
            running = 0;
        }

        /* Small delay to reduce CPU usage */
        struct timespec ts = {0, 16667000}; /* ~60 FPS (16.667ms) */
        nanosleep(&ts, NULL);
    }

    /* Cleanup */
    canvas_cleanup(&canvas);
    terminal_cleanup();

    return 0;
}
