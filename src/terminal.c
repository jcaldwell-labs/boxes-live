#include <ncurses.h>
#include <stdlib.h>
#include "terminal.h"

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
    mousemask(ALL_MOUSE_EVENTS, NULL);
    #endif

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
