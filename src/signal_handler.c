#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include "signal_handler.h"
#include "terminal.h"

/* Volatile flags set by signal handlers */
static volatile sig_atomic_t quit_flag = 0;
static volatile sig_atomic_t resize_flag = 0;

/* Signal handler for termination signals (SIGINT, SIGTERM, SIGHUP) */
static void handle_termination(int sig) {
    (void)sig; /* Unused parameter */
    quit_flag = 1;
}

/* Signal handler for window resize (SIGWINCH) */
static void handle_resize(int sig) {
    (void)sig; /* Unused parameter */
    resize_flag = 1;
}

/* Initialize signal handlers */
int signal_handler_init(void) {
    struct sigaction sa_term;
    struct sigaction sa_resize;

    /* Setup termination signal handler */
    sa_term.sa_handler = handle_termination;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;

    /* Setup resize signal handler */
    sa_resize.sa_handler = handle_resize;
    sigemptyset(&sa_resize.sa_mask);
    sa_resize.sa_flags = 0;

    /* Register signal handlers */
    if (sigaction(SIGINT, &sa_term, NULL) == -1) {
        return -1;
    }

    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        return -1;
    }

    if (sigaction(SIGHUP, &sa_term, NULL) == -1) {
        return -1;
    }

    if (sigaction(SIGWINCH, &sa_resize, NULL) == -1) {
        return -1;
    }

    /* Ignore SIGPIPE (broken pipe) to prevent crashes if output is redirected */
    signal(SIGPIPE, SIG_IGN);

    return 0;
}

/* Check if a termination signal was received */
bool signal_should_quit(void) {
    return quit_flag != 0;
}

/* Get the terminal resize flag and reset it */
bool signal_window_resized(void) {
    if (resize_flag) {
        resize_flag = 0;
        return true;
    }
    return false;
}

/* Cleanup signal handlers - restore defaults */
void signal_handler_cleanup(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGWINCH, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
}
