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
#include "config.h"

/* Print usage information */
static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [FILE]\n", program_name);
    printf("\nBoxes-Live: Terminal-based interactive canvas workspace\n");
    printf("\nOPTIONS:\n");
    printf("  -h, --help     Show this help message and exit\n");
    printf("\nFILE:\n");
    printf("  Optional canvas file to load on startup (*.txt)\n");
    printf("  If not specified, starts with empty canvas\n");
    printf("\nCONFIGURATION:\n");
    printf("  Config file: ~/.config/boxes-live/config.ini\n");
    printf("  See config.ini.example for all available settings\n");
    printf("  Customize joystick buttons, grid, and more!\n");
    printf("\nCONTROLS:\n");
    printf("  Help:          F1 (show keyboard shortcuts)\n");
    printf("  Pan:           Arrow keys or WASD\n");
    printf("  Zoom:          +/- or Z/X\n");
    printf("  Reset view:    R or 0\n");
    printf("  New box:       N\n");
    printf("  Delete box:    D (when box selected)\n");
    printf("  Select box:    Click to select\n");
    printf("  Box type:      T to cycle (NOTE/TASK/CODE/STICKY)\n");
    printf("  Display mode:  Tab to cycle (Full/Compact/Preview)\n");
    printf("  Color box:     1-7 (when box selected)\n");
    printf("  Save canvas:   F2 (saves to canvas.txt)\n");
    printf("  Load/Reload:   F3\n");
    printf("  Quit:          Q or ESC\n");
    printf("\nEXAMPLES:\n");
    printf("  %s                          # Start with sample canvas\n", program_name);
    printf("  %s my_canvas.txt            # Load specific canvas file\n", program_name);
    printf("  %s demos/live_monitor.txt   # Load demo file\n", program_name);
}

/* Initialize empty canvas (Issue #47 - default behavior) */
static void init_empty_canvas(Canvas *canvas) {
    canvas_init(canvas, 200.0, 100.0);
    /* Canvas starts empty - status bar will show onboarding hint */
}

/* Initialize canvas with welcome box (legacy behavior, opt-in via config) */
static void init_welcome_canvas(Canvas *canvas) {
    canvas_init(canvas, 200.0, 100.0);

    /* Create single welcome box */
    int box_id;

    /* Single welcome box with essential info */
    box_id = canvas_add_box(canvas, 10, 8, 60, 20, "Welcome to Boxes-Live!");
    const char *welcome[] = {
        "Interactive terminal canvas workspace",
        "",
        "Getting Started:",
        "  Press F1 to see full help",
        "  Press N to create your first box",
        "  Arrow keys or WASD to pan around",
        "  +/- or Z/X to zoom in/out",
        "",
        "After Creating a Box:",
        "  Box is automatically selected (highlighted)",
        "  Arrow keys still pan the viewport",
        "  Press Space/Enter to read box content (Focus Mode)",
        "  Press ESC to exit any mode",
        "",
        "More Features:",
        "  Click boxes to select them",
        "  Drag boxes to move them",
        "  Press G to toggle grid",
        "  Press F2 to save, F3 to load",
        "",
        "Press N to create your first box!"
    };
    canvas_add_box_content(canvas, box_id, welcome, 21);
}


int main(int argc, char *argv[]) {
    char *load_file = NULL;

    /* Load configuration (Phase 5a) */
    AppConfig app_config;
    char *config_path = config_get_default_path();
    if (config_path) {
        config_load(&app_config, config_path);
        free(config_path);
    } else {
        /* Fallback to defaults if path allocation failed */
        config_init_defaults(&app_config);
    }

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
    canvas.boxes = NULL;  /* Mark as uninitialized for canvas_load */

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

        /* Store filename in canvas for status bar display */
        canvas.filename = strdup(load_file);

        /* Center viewport on loaded content */
        if (canvas.box_count > 0) {
            /* Find bounding box of all boxes */
            double min_x = canvas.boxes[0].x;
            double min_y = canvas.boxes[0].y;
            double max_x = canvas.boxes[0].x + canvas.boxes[0].width;
            double max_y = canvas.boxes[0].y + canvas.boxes[0].height;

            for (int i = 1; i < canvas.box_count; i++) {
                if (canvas.boxes[i].x < min_x) min_x = canvas.boxes[i].x;
                if (canvas.boxes[i].y < min_y) min_y = canvas.boxes[i].y;
                if (canvas.boxes[i].x + canvas.boxes[i].width > max_x)
                    max_x = canvas.boxes[i].x + canvas.boxes[i].width;
                if (canvas.boxes[i].y + canvas.boxes[i].height > max_y)
                    max_y = canvas.boxes[i].y + canvas.boxes[i].height;
            }

            /* Center viewport on content */
            double center_x = (min_x + max_x) / 2.0;
            double center_y = (min_y + max_y) / 2.0;
            viewport.cam_x = center_x - (viewport.term_width / 2.0) / viewport.zoom;
            viewport.cam_y = center_y - (viewport.term_height / 2.0) / viewport.zoom;
        }
    } else {
        /* Initialize canvas based on config (Issue #47) */
        if (app_config.show_welcome_box) {
            init_welcome_canvas(&canvas);
        } else {
            init_empty_canvas(&canvas);
        }
    }

    /* Apply config to grid defaults (Phase 5a) */
    canvas.grid.visible = app_config.grid_visible_default;
    canvas.grid.snap_enabled = app_config.grid_snap_default;
    canvas.grid.spacing = app_config.grid_spacing;

    /* Initialize joystick (optional, degrades gracefully if not available) */
    JoystickState joystick;
    joystick_init(&joystick);

    /* Apply config to joystick settings (Phase 5a) */
    joystick.show_visualizer = app_config.show_visualizer;

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

        /* Focus mode rendering (Phase 5b) - takes over entire screen */
        if (canvas.focus.active) {
            render_focused_box(&canvas);
        } else {
            /* Normal canvas rendering */

            /* Render grid (Phase 4 - background layer) */
            render_grid(&canvas, &viewport);

            /* Render connections between boxes (Issue #20 - behind boxes) */
            render_connections(&canvas, &viewport);

            /* Render canvas */
            render_canvas(&canvas, &viewport, &app_config);

            /* Render sidebar (Issue #35 - overlays canvas) */
            render_sidebar(&canvas, &viewport);

            /* Render connection mode indicator (Issue #20) */
            render_connection_mode(&canvas, &viewport);

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
        }

        /* Render help overlay if visible (Issue #34) - after all other elements */
        if (canvas.help.visible) {
            render_help_overlay();
        }

        /* Render command line if active (Issue #55) - after status bar */
        render_command_line(&canvas);

        /* Refresh display */
        terminal_refresh();

        /* Handle keyboard input */
        if (handle_input(&canvas, &viewport, &joystick, &app_config)) {
            running = 0;
        }

        /* Handle joystick input */
        if (joystick.available) {
            joystick_poll(&joystick);
            if (handle_joystick_input(&canvas, &viewport, &joystick, &app_config)) {
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
