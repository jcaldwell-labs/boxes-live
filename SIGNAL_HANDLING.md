# Signal Handling in boxes-live

## Overview

The application now properly handles standard POSIX signals, ensuring the terminal is always cleaned up correctly, even when interrupted via Ctrl+C or terminated by the system.

## Supported Signals

### Termination Signals (Clean Exit)
- **SIGINT** (Ctrl+C) - User interruption
- **SIGTERM** - Termination request (e.g., `kill <pid>`)
- **SIGHUP** - Hang up (terminal closed)

These signals trigger graceful shutdown:
1. Signal handler sets a flag
2. Main loop detects the flag
3. Canvas and terminal are cleaned up properly
4. Application exits with appropriate code

### Resize Signal
- **SIGWINCH** - Window size changed
  - Automatically updates viewport dimensions
  - Ensures proper rendering after terminal resize

### Ignored Signals
- **SIGPIPE** - Broken pipe
  - Ignored to prevent crashes if output is redirected

## Terminal Type Detection

The application checks the `TERM` environment variable on startup:

```c
const char *term = getenv("TERM");
```

### Compatible Terminals
The application works with most modern terminals:
- xterm / xterm-256color
- gnome-terminal
- konsole
- alacritty
- kitty
- tmux
- screen
- And many others that support ANSI escape sequences

### Incompatible Terminals
The application will refuse to start if:
- `TERM` is not set
- `TERM` is set to "dumb" (no cursor movement support)

Error messages guide users to fix their environment.

## Implementation Details

### Signal Handler Module
**Files:** `src/signal_handler.c`, `include/signal_handler.h`

- Uses `sigaction()` for reliable signal handling
- Uses `sig_atomic_t` volatile flags for thread safety
- Signal handlers are async-signal-safe (no unsafe operations)

### Terminal Initialization
**File:** `src/terminal.c`

```c
int terminal_init(void) {
    check_terminal_type();      // Validate TERM variable
    signal_handler_init();       // Register signal handlers
    initscr();                   // Initialize ncurses
    // ... configure ncurses ...
}
```

### Terminal Cleanup
**File:** `src/terminal.c`

```c
void terminal_cleanup(void) {
    endwin();                    // Restore terminal
    signal_handler_cleanup();    // Restore default signal handlers
}
```

### Main Loop Integration
**File:** `src/main.c`

```c
while (running) {
    if (signal_should_quit()) {
        running = 0;
        break;
    }

    if (signal_window_resized()) {
        terminal_update_size(&viewport);
    }

    // ... render and handle input ...
}

// Cleanup is guaranteed to run
canvas_cleanup(&canvas);
terminal_cleanup();
```

## Testing Signal Handling

### Manual Test
```bash
# Start the application
./boxes-live demos/live_monitor.txt

# Press Ctrl+C
# Terminal should be restored to normal state
```

### Automated Test
```bash
# Run the signal handling test script
./demos/test-signal-handling.sh
```

### Verify Terminal State
After interrupting with Ctrl+C, verify the terminal is working:
```bash
# Should work normally
echo "Test"
ls
```

If the terminal is corrupted, try:
```bash
reset
# or
stty sane
```

## Benefits

1. **Clean Exits**: Ctrl+C always leaves terminal in good state
2. **Proper Cleanup**: Resources are freed on all exit paths
3. **Window Resize**: Automatically adapts to terminal size changes
4. **Standards Compliance**: Follows POSIX signal handling conventions
5. **Better UX**: Users don't need to manually reset their terminal

## Signal Handling Best Practices

The implementation follows these best practices:

1. **Async-Signal-Safety**: Signal handlers only set flags, no unsafe operations
2. **Cleanup Guarantee**: `terminal_cleanup()` always runs before exit
3. **Minimal Handler Logic**: Handlers are simple and fast
4. **Standard Exit Codes**: Exit code 130 for SIGINT (128 + signal number)
5. **Terminal Validation**: Check `TERM` before starting ncurses

## Debugging Signal Issues

If signal handling isn't working:

1. **Check TERM variable**:
   ```bash
   echo $TERM
   # Should show a valid terminal type like "xterm-256color"
   ```

2. **Verify signal handlers are registered**:
   The application logs warnings if signal registration fails.

3. **Test with different signals**:
   ```bash
   # Start in background
   ./boxes-live &
   PID=$!

   # Test different signals
   kill -INT $PID   # SIGINT
   kill -TERM $PID  # SIGTERM
   kill -HUP $PID   # SIGHUP
   ```

4. **Check exit codes**:
   - Exit code 0: Normal quit (Q or ESC)
   - Exit code 130: SIGINT (Ctrl+C)
   - Exit code 143: SIGTERM

## Future Enhancements

Potential improvements:
- [ ] Add SIGUSR1/SIGUSR2 for custom actions (e.g., save canvas)
- [ ] Implement crash handler with backtrace (SIGSEGV, SIGABRT)
- [ ] Add signal-based IPC for external control
- [ ] Support SIGTSTP (Ctrl+Z) with proper suspend/resume

## Related Files

- `src/signal_handler.c` - Signal handler implementation
- `include/signal_handler.h` - Signal handler API
- `src/terminal.c` - Terminal initialization with signal setup
- `src/main.c` - Main loop with signal checking
- `demos/test-signal-handling.sh` - Automated test script
