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

/* Initialize canvas with training boxes */
static void init_sample_canvas(Canvas *canvas) {
    canvas_init(canvas, 200.0, 100.0);

    /* Create training boxes for learning controls */
    int box_id;

    /* Box 1: Welcome and Overview */
    box_id = canvas_add_box(canvas, 5, 5, 40, 10, "Welcome to Boxes-Live!");
    const char *welcome[] = {
        "Interactive terminal canvas",
        "",
        "This training canvas teaches you",
        "keyboard, mouse, and joystick controls",
        "",
        "Pan around to explore all boxes",
        "Try all the controls below!"
    };
    canvas_add_box_content(canvas, box_id, welcome, 7);

    /* Box 2: Keyboard Controls - Navigation */
    box_id = canvas_add_box(canvas, 50, 5, 35, 12, "Keyboard: Navigation");
    const char *kb_nav[] = {
        "Pan:     Arrow keys or WASD",
        "Zoom In: + or Z",
        "Zoom Out: - or X",
        "Reset:   R or 0",
        "",
        "Cycle:   Tab (select next box)",
        "Create:  N (new box at center)",
        "Delete:  D (when box selected)",
        "Quit:    Q or ESC"
    };
    canvas_add_box_content(canvas, box_id, kb_nav, 9);

    /* Box 3: Mouse Controls */
    box_id = canvas_add_box(canvas, 90, 5, 35, 10, "Mouse Controls");
    const char *mouse[] = {
        "Select:    Left-Click on box",
        "Deselect:  Left-Click empty area",
        "Move Box:  Left-Click + Drag",
        "",
        "Zoom In:   Scroll Wheel Up",
        "Zoom Out:  Scroll Wheel Down",
        "",
        "Create:    Ctrl + Left-Click"
    };
    canvas_add_box_content(canvas, box_id, mouse, 8);

    /* Box 4: Joystick Controls - Navigation */
    box_id = canvas_add_box(canvas, 5, 20, 40, 16, "Joystick: Navigation Mode");
    const char *joy_nav[] = {
        "Pan:       Left Stick (analog)",
        "",
        "Button A:  Zoom In",
        "Button B:  Zoom Out",
        "Button X:  Cycle Box (Tab)",
        "Button Y:  Create Box",
        "",
        "Button LB: Reset View",
        "Button RB: Deselect Box",
        "Button BACK: Toggle Visualizer",
        "",
        "Start:     Save Canvas",
        "Select:    Quit",
        "",
        "Visualizer shows button states!"
    };
    canvas_add_box_content(canvas, box_id, joy_nav, 14);

    /* Box 5: Joystick Controls - Edit Mode */
    box_id = canvas_add_box(canvas, 50, 20, 40, 14, "Joystick: Edit Mode");
    const char *joy_edit[] = {
        "After selecting a box:",
        "",
        "Move:      Left Stick (analog)",
        "",
        "Button A:  Enter Param Mode",
        "Button B:  Exit to Navigation",
        "Button X:  Cycle Color",
        "LB + Y:    Delete Box (combo!)",
        "",
        "Edit mode lets you manipulate",
        "the selected box directly",
        "",
        "Hold LB while pressing Y to delete"
    };
    canvas_add_box_content(canvas, box_id, joy_edit, 12);

    /* Box 6: Practice Area */
    box_id = canvas_add_box(canvas, 5, 40, 40, 8, "Practice: Pan Here");
    const char *practice[] = {
        "Use WASD or Left Stick",
        "to pan the viewport",
        "",
        "Notice the camera position",
        "in the status bar at top"
    };
    canvas_add_box_content(canvas, box_id, practice, 5);

    /* Box 7: Zoom Practice */
    box_id = canvas_add_box(canvas, 50, 40, 30, 6, "Practice: Zoom");
    const char *zoom_practice[] = {
        "Press + / - (keyboard)",
        "or Button A / B (joystick)",
        "to zoom in and out"
    };
    canvas_add_box_content(canvas, box_id, zoom_practice, 3);

    /* Box 8: Selection Practice */
    box_id = canvas_add_box(canvas, 85, 40, 35, 8, "Practice: Selection");
    const char *select_practice[] = {
        "Press Tab or Button X",
        "to cycle through boxes",
        "",
        "Selected box shown in status",
        "Use D or Button Y to delete"
    };
    canvas_add_box_content(canvas, box_id, select_practice, 5);

    /* Box 9: Creation Practice */
    box_id = canvas_add_box(canvas, 5, 55, 40, 7, "Practice: Create Box");
    const char *create_practice[] = {
        "Press N (keyboard)",
        "or Button Y (joystick)",
        "",
        "Creates box at viewport center"
    };
    canvas_add_box_content(canvas, box_id, create_practice, 4);

    /* Box 10: Far away exploration box */
    box_id = canvas_add_box(canvas, 130, 60, 35, 9, "Exploration Area");
    const char *explore[] = {
        "You found the far box!",
        "",
        "Good job navigating!",
        "",
        "Press R or Button LB",
        "to reset the view",
        "and return to origin"
    };
    canvas_add_box_content(canvas, box_id, explore, 7);

    /* Box 11: Quick Reference */
    box_id = canvas_add_box(canvas, 95, 20, 30, 10, "Quick Reference");
    const char *quick_ref[] = {
        "Most Common Actions:",
        "",
        "Pan:   WASD / Arrows",
        "Zoom:  +/- or Z/X",
        "New:   N",
        "Cycle: Tab",
        "Reset: R",
        "Quit:  Q or ESC"
    };
    canvas_add_box_content(canvas, box_id, quick_ref, 8);
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

        /* Render parameter panel (if in parameter mode) */
        if (joystick.available && joystick.mode == MODE_PARAMETER) {
            Box *selected = canvas_get_selected(&canvas);
            if (selected) {
                render_parameter_panel(&joystick, selected);
            }
        }

        /* Render joystick visualizer (if enabled) */
        if (joystick.available) {
            render_joystick_visualizer(&joystick, &viewport);
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
