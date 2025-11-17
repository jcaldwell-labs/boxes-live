#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "terminal.h"
#include "types.h"
#include "signal_handler.h"

/* Check if terminal type is compatible */
static int check_terminal_type(void) {
    const char *term = getenv("TERM");

    /* Check if TERM is set */
    if (term == NULL) {
        fprintf(stderr, "Error: TERM environment variable not set\n");
        fprintf(stderr, "Please set TERM to your terminal type (e.g., xterm, gnome, alacritty)\n");
        return -1;
    }

    /* Warn about potentially incompatible terminals */
    if (strcmp(term, "dumb") == 0) {
        fprintf(stderr, "Error: Terminal type '%s' does not support cursor movement\n", term);
        fprintf(stderr, "Please use a terminal emulator that supports ANSI escape sequences\n");
        return -1;
    }

    /* Log terminal type for debugging (optional) */
    /* fprintf(stderr, "Terminal type: %s\n", term); */

    return 0;
}

int terminal_init(void) {
    /* Check terminal type before initializing ncurses */
    if (check_terminal_type() != 0) {
        return -1;
    }

    /* Initialize signal handlers before ncurses */
    if (signal_handler_init() != 0) {
        fprintf(stderr, "Warning: Failed to initialize signal handlers\n");
        /* Continue anyway - not fatal */
    }

    /* Initialize ncurses */
    if (initscr() == NULL) {
        signal_handler_cleanup();
        return -1;
    }

    /* Disable line buffering */
    cbreak();

    /* Disable echo of typed characters */
    noecho();

    /* Enable keypad for arrow keys */
    keypad(stdscr, TRUE);

    /* Hide cursor */
    curs_set(0);

    /* Non-blocking input */
    nodelay(stdscr, TRUE);

    /* Enable mouse events if available */
    #ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    #endif

    /* Initialize colors if supported */
    if (has_colors()) {
        start_color();
        use_default_colors();

        /* Initialize color pairs (using ncurses COLOR_* macros for colors) */
        init_pair(BOX_COLOR_RED, COLOR_RED, -1);
        init_pair(BOX_COLOR_GREEN, COLOR_GREEN, -1);
        init_pair(BOX_COLOR_BLUE, COLOR_BLUE, -1);
        init_pair(BOX_COLOR_YELLOW, COLOR_YELLOW, -1);
        init_pair(BOX_COLOR_MAGENTA, COLOR_MAGENTA, -1);
        init_pair(BOX_COLOR_CYAN, COLOR_CYAN, -1);
        init_pair(BOX_COLOR_WHITE, COLOR_WHITE, -1);
    }

    return 0;
}

void terminal_cleanup(void) {
    /* Restore terminal to normal state */
    endwin();

    /* Clean up signal handlers */
    signal_handler_cleanup();
}

void terminal_update_size(Viewport *vp) {
    getmaxyx(stdscr, vp->term_height, vp->term_width);
}

void terminal_clear(void) {
    clear();
}

void terminal_refresh(void) {
    refresh();
}
