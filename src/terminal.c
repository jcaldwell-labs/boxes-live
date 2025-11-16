#include <ncurses.h>
#include <stdlib.h>
#include "terminal.h"
#include "types.h"

int terminal_init(void) {
    /* Initialize ncurses */
    if (initscr() == NULL) {
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
    endwin();
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
