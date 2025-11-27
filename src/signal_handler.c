#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include "signal_handler.h"
#include "terminal.h"

/* Volatile flags set by signal handlers */
static volatile sig_atomic_t quit_flag = 0;
static volatile sig_atomic_t resize_flag = 0;
static volatile sig_atomic_t reload_flag = 0;
static volatile sig_atomic_t sync_flag = 0;

#ifndef _WIN32
/* ========================================================================
 * UNIX IMPLEMENTATION - Full POSIX signal handling
 * ======================================================================== */

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

/* Signal handler for reload (SIGUSR1) */
static void handle_reload(int sig) {
    (void)sig; /* Unused parameter */
    reload_flag = 1;
}

/* Signal handler for sync (SIGUSR2) */
static void handle_sync(int sig) {
    (void)sig; /* Unused parameter */
    sync_flag = 1;
}

/* Initialize signal handlers */
int signal_handler_init(void) {
    struct sigaction sa_term;
    struct sigaction sa_resize;
    struct sigaction sa_reload;
    struct sigaction sa_sync;

    /* Setup termination signal handler */
    sa_term.sa_handler = handle_termination;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;

    /* Setup resize signal handler */
    sa_resize.sa_handler = handle_resize;
    sigemptyset(&sa_resize.sa_mask);
    sa_resize.sa_flags = 0;

    /* Setup reload signal handler */
    sa_reload.sa_handler = handle_reload;
    sigemptyset(&sa_reload.sa_mask);
    sa_reload.sa_flags = 0;

    /* Setup sync signal handler */
    sa_sync.sa_handler = handle_sync;
    sigemptyset(&sa_sync.sa_mask);
    sa_sync.sa_flags = 0;

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

    if (sigaction(SIGUSR1, &sa_reload, NULL) == -1) {
        return -1;
    }

    if (sigaction(SIGUSR2, &sa_sync, NULL) == -1) {
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

/* Get the reload flag and reset it */
bool signal_should_reload(void) {
    if (reload_flag) {
        reload_flag = 0;
        return true;
    }
    return false;
}

/* Get the sync flag and reset it */
bool signal_should_sync(void) {
    if (sync_flag) {
        sync_flag = 0;
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
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
}

#else
/* ========================================================================
 * WINDOWS IMPLEMENTATION - Minimal signal handling (SIGINT/SIGTERM only)
 * ======================================================================== */

/* Windows signal handler for Ctrl+C */
static void handle_termination(int sig) {
    (void)sig;
    quit_flag = 1;
}

/* Initialize signal handlers */
int signal_handler_init(void) {
    /* Windows only supports SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV */
    signal(SIGINT, handle_termination);
    signal(SIGTERM, handle_termination);
    return 0;
}

/* Check if quit signal received */
bool signal_should_quit(void) {
    return quit_flag != 0;
}

/* Get the terminal resize flag and reset it */
bool signal_window_resized(void) {
    /* Windows: resize detection handled by PDCurses, not signals */
    return false;
}

/* Get the resize flag and reset it */
bool signal_should_resize(void) {
    /* Windows: resize detection handled by PDCurses, not signals */
    return false;
}

/* Get the reload flag and reset it */
bool signal_should_reload(void) {
    /* Not supported on Windows */
    return false;
}

/* Get the sync flag and reset it */
bool signal_should_sync(void) {
    /* Not supported on Windows */
    return false;
}

/* Cleanup signal handlers */
void signal_handler_cleanup(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

#endif  /* _WIN32 */
