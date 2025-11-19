#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <stdbool.h>

/* Initialize signal handlers for proper terminal cleanup */
/* Returns 0 on success, -1 on error */
int signal_handler_init(void);

/* Check if a termination signal was received */
bool signal_should_quit(void);

/* Get the terminal resize flag and reset it */
bool signal_window_resized(void);

/* Check if reload signal (SIGUSR1) was received and reset it */
bool signal_should_reload(void);

/* Check if sync signal (SIGUSR2) was received and reset it */
bool signal_should_sync(void);

/* Cleanup signal handlers */
void signal_handler_cleanup(void);

#endif /* SIGNAL_HANDLER_H */
