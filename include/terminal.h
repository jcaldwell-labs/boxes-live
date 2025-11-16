#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

/* Initialize ncurses and terminal settings */
int terminal_init(void);

/* Cleanup and restore terminal */
void terminal_cleanup(void);

/* Update viewport with current terminal dimensions */
void terminal_update_size(Viewport *vp);

/* Clear the screen */
void terminal_clear(void);

/* Refresh the display */
void terminal_refresh(void);

#endif /* TERMINAL_H */
