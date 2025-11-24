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
#include "input_unified.h"
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

/* Initialize canvas with simple welcome box */
static void init_sample_canvas(Canvas *canvas) {
    canvas_init(canvas, 200.0, 100.0);

    /* Create single welcome box */
    int box_id;

    /* Single welcome box with essential info */
    box_id = canvas_add_box(canvas, 10, 8, 50, 16, "Welcome to Boxes-Live!");
    const char *welcome[] = {
        "Interactive terminal canvas workspace",
        "",
        "Current Mode: VIEW (safe exploration)",
        "",
        "Joystick Controls:",
        "  MENU button: Cycle through modes",
        "  A/B: Zoom in/out",
        "  X: Quick-create box",
        "  Y: Enter SELECT mode",
        "  BACK: Toggle visualizer panel",
        "",
        "Keyboard: WASD=pan  +/-=zoom  N=create",
        "          Tab=cycle  Q=quit",
        "",
        "Press X to create your first box!"
    };
    canvas_add_box_content(canvas, box_id, welcome, 14);
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

    /* Initialize unified input system */
    input_unified_init();

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

        /* Render parameter panel if active (Phase 2) */
        if (joystick.available && joystick.param_editor_active) {
            Box *selected = canvas_get_box(&canvas, joystick.selected_box_id);
            if (selected) {
                render_parameter_panel(&joystick, selected);
            }
        }

        /* Render text editor if active (Phase 3) */
        if (joystick.available && joystick.text_editor_active) {
            Box *selected = canvas_get_box(&canvas, joystick.selected_box_id);
            if (selected) {
                render_text_editor(&joystick, selected);
            }
        }

        /* Render joystick visualizer (if enabled) */
        if (joystick.available) {
            render_joystick_visualizer(&joystick, &viewport);
        }

        /* Refresh display */
        terminal_refresh();

        /* Handle keyboard input */
        if (handle_input(&canvas, &viewport, &joystick)) {
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
