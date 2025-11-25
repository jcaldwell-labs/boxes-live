# Phase 5b: Focus Mode - Full-Screen Box Interaction

## Vision

Allow any selected box to expand to full terminal size for deep interaction, then collapse back to canvas view. Essential foundation for file viewers, terminals, and interactive boxes.

## Activation

**From any mode with a selected box**:
```
Keyboard: Press ENTER or SPACE
Joystick: Press Button A (when in SELECT mode)
Mouse: Double-click on box
```

## Focus Mode Behavior

### Screen Transition

**Before** (Canvas view):
```
┌─ Files ─┐  ┌─ main.c ────┐  ┌─ Output ─┐
│ src/     │  │ code here   │  │ results  │
│ test/    │  │             │  │          │
└──────────┘  └─────────────┘  └──────────┘
```

Press ENTER on "main.c" box
↓

**After** (Focus view):
```
┌────────────────────────────────────────────────────────────┐
│ main.c                              [FOCUS - ESC to exit] │
├────────────────────────────────────────────────────────────┤
│                                                            │
│   Full box content fills the entire terminal              │
│   All content visible with scrolling                      │
│   Box-specific interactions available                     │
│                                                            │
│                                                            │
├────────────────────────────────────────────────────────────┤
│ j/k: Scroll  i: Edit  ESC: Back to canvas                │
└────────────────────────────────────────────────────────────┘
```

Press ESC
↓

Back to canvas with box in original position

### Key Characteristics

1. **Full terminal usage** - Box content fills COLS x LINES
2. **Preserve context** - Remember box position/size on canvas
3. **Scrolling** - View more content than fits in box
4. **Box-specific controls** - Depends on box type
5. **Easy exit** - ESC or Button B always returns to canvas

## Implementation

### State Management

```c
typedef struct {
    bool focus_active;          // Is any box in focus mode?
    int focused_box_id;         // Which box is focused (-1 if none)
    int scroll_offset;          // Scroll position in focused box
    int scroll_max;             // Maximum scroll (calculated)
} FocusState;
```

Store in Canvas or as global state.

### Rendering

```c
void render_focused_box(const Canvas *canvas, const FocusState *focus) {
    Box *box = canvas_get_box(canvas, focus->focused_box_id);
    if (!box) return;

    /* Clear screen */
    clear();

    /* Title bar */
    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 0, " %s ", box->title);
    int hint_x = COLS - 25;
    if (hint_x > 0) {
        mvprintw(0, hint_x, " [FOCUS - ESC to exit] ");
    }
    /* Fill rest of title bar */
    for (int x = strlen(box->title) + 2; x < hint_x; x++) {
        mvaddch(0, x, ' ');
    }
    attroff(A_REVERSE | A_BOLD);

    /* Content area (full terminal minus title and status) */
    int content_start_y = 2;
    int content_height = LINES - 3;  /* Title + border + status */

    /* Border after title */
    for (int x = 0; x < COLS; x++) {
        mvaddch(1, x, ACS_HLINE);
    }

    /* Render box content with scrolling */
    if (box->content) {
        for (int i = 0; i < content_height && i < box->content_lines; i++) {
            int line_idx = focus->scroll_offset + i;
            if (line_idx >= 0 && line_idx < box->content_lines) {
                /* Line numbers */
                attron(COLOR_PAIR(8));
                mvprintw(content_start_y + i, 2, "%4d ", line_idx + 1);
                attroff(COLOR_PAIR(8));

                /* Content */
                mvprintw(content_start_y + i, 8, "%s", box->content[line_idx]);
            }
        }
    }

    /* Status bar */
    int status_y = LINES - 1;
    mvaddch(status_y - 1, 0, ACS_HLINE);
    for (int x = 1; x < COLS; x++) {
        mvaddch(status_y - 1, x, ACS_HLINE);
    }

    attron(A_REVERSE);
    char status[256];
    snprintf(status, sizeof(status),
             " j/k: Scroll | i: Edit | ESC: Back to canvas | Line %d/%d ",
             focus->scroll_offset + 1, box->content_lines);
    mvprintw(status_y, 0, "%s", status);
    for (int x = strlen(status); x < COLS; x++) {
        mvaddch(status_y, x, ' ');
    }
    attroff(A_REVERSE);
}
```

### Input Handling

```c
int handle_focus_mode_input(Canvas *canvas, FocusState *focus) {
    int ch = getch();
    if (ch == ERR) return 0;

    Box *box = canvas_get_box(canvas, focus->focused_box_id);
    if (!box) {
        /* Box deleted, exit focus */
        focus->focus_active = false;
        return 0;
    }

    switch (ch) {
        case 27:  /* ESC */
        case 'q':
            /* Exit focus mode */
            focus->focus_active = false;
            focus->focused_box_id = -1;
            return 0;

        case 'j':
        case KEY_DOWN:
            /* Scroll down */
            if (focus->scroll_offset < focus->scroll_max) {
                focus->scroll_offset++;
            }
            return 0;

        case 'k':
        case KEY_UP:
            /* Scroll up */
            if (focus->scroll_offset > 0) {
                focus->scroll_offset--;
            }
            return 0;

        case 'g':
            /* Jump to top */
            focus->scroll_offset = 0;
            return 0;

        case 'G':
            /* Jump to bottom */
            focus->scroll_offset = focus->scroll_max;
            return 0;

        case 'i':
        case 'a':
            /* Edit mode (future: depends on box type) */
            return 0;

        default:
            return 0;
    }
}
```

### Main Loop Integration

```c
/* In main loop: */
if (focus_state.focus_active) {
    /* Render focused box */
    render_focused_box(&canvas, &focus_state);

    /* Handle focus mode input */
    if (handle_focus_mode_input(&canvas, &focus_state)) {
        running = 0;  /* Quit */
    }
} else {
    /* Normal canvas rendering */
    render_canvas(&canvas, &viewport);
    /* ... etc */
}
```

## Controls in Focus Mode

### Universal (All Box Types)
- **ESC** or **q**: Exit focus, return to canvas
- **j** or **Down**: Scroll down
- **k** or **Up**: Scroll up
- **g**: Jump to top
- **G**: Jump to bottom
- **Button B** (joystick): Exit focus

### Type-Specific (Future)

**TEXT box**:
- Same as universal

**FILE_VIEWER box**:
- **/**: Search
- **n**: Next search result
- **e**: Convert to editor

**TERMINAL box**:
- All keys pass through to shell
- ESC ESC (double-tap) to exit

**FILE_LIST box**:
- **j/k**: Navigate files
- **ENTER**: Open selected file
- **/**: Filter/search

## Integration with Existing Modes

### Mode Simplification Option

Could simplify modes:
- **BROWSE** mode (was VIEW + SELECT)
- **FOCUS** mode (new - full screen box)
- **COMMAND** mode (future - : commands)

Or keep current 4-mode system and add FOCUS as 5th mode.

### Recommended: Keep Current Modes

- **VIEW**: Navigate canvas
- **SELECT**: Choose box
- **EDIT**: Quick edits (color, move, params)
- **FOCUS**: Deep interaction (new, activated with ENTER)
- **CONNECT**: Link boxes (future)

FOCUS is orthogonal - can enter from any mode when box is selected.

## Scroll Management

### Calculate scroll_max
```c
int calculate_scroll_max(const Box *box) {
    int content_height = LINES - 3;  /* Available space */
    if (!box || !box->content) return 0;

    int max_scroll = box->content_lines - content_height;
    return (max_scroll > 0) ? max_scroll : 0;
}
```

### Scroll Clamping
- Always clamp to 0 <= scroll_offset <= scroll_max
- Update scroll_max when terminal resizes
- Handle boxes with less content than screen (don't scroll)

## Future Enhancements

### Smooth Transitions
- Animate box expanding (not this phase)
- Fade between views

### Split Focus
- Multiple boxes in focus simultaneously
- Like tmux panes but spatial

### Focus History
- Remember last focused box
- Quick-switch with hotkey

### Focus Profiles
- Save focus layouts
- Workspace configurations

## Implementation Steps

1. Add FocusState to Canvas
2. Add focus rendering function
3. Add focus input handler
4. Add ENTER key mapping
5. Integrate into main loop
6. Test with text boxes
7. (Future) Extend for other box types

Let's implement!
