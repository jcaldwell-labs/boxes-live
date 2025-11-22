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
#include "persistence.h"
#include "signal_handler.h"
#include "joystick.h"


/* Print usage information */
static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [FILE]\n", program_name);
    printf("\nBoxes-Live: Terminal-based interactive canvas\n");
    printf("\nOPTIONS:\n");
    printf("  -h, --help     Show this help message and exit\n");
    printf("\nFILE:\n");
    printf("  Optional canvas file to load on startup (*.txt)\n");
    printf("  If not specified, starts with sample canvas\n");
    printf("\nCONTROLS:\n");
    printf("  Pan:           Arrow keys or WASD\n");
    printf("  Zoom:          +/- or Z/X\n");
    printf("  Reset view:    R or 0\n");
    printf("  New box:       N\n");
    printf("  Delete box:    D (when box selected)\n");
    printf("  Select box:    Click or Tab to cycle\n");
    printf("  Color box:     1-7 (when box selected)\n");
    printf("  Save canvas:   F2 (saves to canvas.txt)\n");
    printf("  Load/Reload:   F3\n");
    printf("  Quit:          Q or ESC\n");
    printf("\nEXAMPLES:\n");
    printf("  %s                          # Start with sample canvas\n", program_name);
    printf("  %s my_canvas.txt            # Load specific canvas file\n", program_name);
    printf("  %s demos/live_monitor.txt   # Load demo file\n", program_name);
}

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


int main(int argc, char *argv[]) {
    char *load_file = NULL;

    /* Parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            return 1;
        } else {
            /* Assume it's a file to load */
            if (load_file != NULL) {
                fprintf(stderr, "Error: Multiple files specified\n");
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return 1;
            }
            load_file = argv[i];
        }
    }

    /* Initialize terminal */
    if (terminal_init() != 0) {
        fprintf(stderr, "Failed to initialize terminal\n");
        return 1;
    }

    /* Initialize viewport */
    Viewport viewport;
    viewport_init(&viewport);
    terminal_update_size(&viewport);

    /* Initialize canvas */
    Canvas canvas;

    /* Load from file if specified, otherwise use sample canvas */
    if (load_file != NULL) {
        /* Try to load the specified file */
        if (canvas_load(&canvas, load_file) != 0) {
            terminal_cleanup();
            fprintf(stderr, "Error: Failed to load canvas from '%s'\n", load_file);
            fprintf(stderr, "Make sure the file exists and is in the correct format.\n");
            return 1;
        }
        /* Store the loaded file for F3 reload */
        persistence_set_current_file(load_file);
    } else {
        /* Initialize with sample boxes */
        init_sample_canvas(&canvas);
    }

    /* Initialize joystick (optional, degrades gracefully if not available) */
    JoystickState joystick;
    joystick_init(&joystick);
    /* Initialize cursor at viewport center */
    joystick.cursor_x = viewport.cam_x + (viewport.term_width / 2.0) / viewport.zoom;
    joystick.cursor_y = viewport.cam_y + (viewport.term_height / 2.0) / viewport.zoom;

    /* Main loop */
    int running = 1;
    while (running) {
        /* Check for termination signals (Ctrl+C, kill, etc.) */
        if (signal_should_quit()) {
            running = 0;
            break;
        }

        /* Check for window resize signal */
        if (signal_window_resized()) {
            terminal_update_size(&viewport);
        }

        /* Check for reload signal (SIGUSR1) */
        if (signal_should_reload()) {
            const char *current_file = persistence_get_current_file();
            if (current_file != NULL) {
                Canvas new_canvas;
                if (canvas_load(&new_canvas, current_file) == 0) {
                    canvas_cleanup(&canvas);
                    canvas = new_canvas;
                }
            }
        }

        /* Update terminal size (in case of resize) */
        terminal_update_size(&viewport);

        /* Clear screen */
        terminal_clear();

        /* Render canvas */
        render_canvas(&canvas, &viewport);

        /* Render joystick cursor (if in navigation mode) */
        if (joystick.available) {
            render_joystick_cursor(&joystick, &viewport);
        }

        /* Render status bar */
        render_status(&canvas, &viewport);

        /* Render joystick mode indicator */
        if (joystick.available) {
            render_joystick_mode(&joystick, &canvas);
        }

        /* Render parameter panel (if in parameter mode) */
        if (joystick.available && joystick.mode == MODE_PARAMETER) {
            Box *selected = canvas_get_selected(&canvas);
            if (selected) {
                render_parameter_panel(&joystick, selected);
            }
        }

        /* Refresh display */
        terminal_refresh();

        /* Handle keyboard input */
        if (handle_input(&canvas, &viewport)) {
            running = 0;
        }

        /* Handle joystick input */
        if (joystick.available) {
            joystick_poll(&joystick);
            if (handle_joystick_input(&canvas, &viewport, &joystick)) {
                running = 0;
            }
        } else {
            /* Try to reconnect joystick if disconnected */
            joystick_try_reconnect(&joystick);
        }

        /* Small delay to reduce CPU usage */
        struct timespec ts = {0, 16667000}; /* ~60 FPS (16.667ms) */
        nanosleep(&ts, NULL);
    }

    /* Cleanup */
    joystick_close(&joystick);
    canvas_cleanup(&canvas);
    terminal_cleanup();

    return 0;
}
