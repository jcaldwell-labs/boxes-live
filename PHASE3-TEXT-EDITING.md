# Phase 3: Text Editing System

## Goals
1. **Keyboard-based** - Primary input method (most users have keyboard)
2. **Edit title and content** - Modify box text easily
3. **Clear workflow** - Obvious what you're editing and how to save/cancel
4. **Configurable** - Easy to add on-screen keyboard fallback later

## Activation
From EDIT mode, press **Button A** → Opens text editor (modal overlay)

## Text Editor Layout

```
┌─ Edit Text ──────────────────────────────────────────┐
│                                                       │
│  Box Title (press Tab to switch):                    │
│  ┌─────────────────────────────────────────────────┐ │
│  │ Welcome to Boxes-Live!_                         │ │
│  └─────────────────────────────────────────────────┘ │
│                                                       │
│  Box Content (line 1 of 5):                          │
│  ┌─────────────────────────────────────────────────┐ │
│  │ Interactive terminal canvas_                     │ │
│  └─────────────────────────────────────────────────┘ │
│                                                       │
│  Navigation:                                          │
│    Tab: Switch between Title/Content                 │
│    Up/Down: Navigate content lines                   │
│    Enter: New line (in content)                      │
│    Backspace: Delete character                       │
│    Esc or Button B: Save & Close                     │
│                                                       │
│  Keyboard Required - Joystick for navigation only    │
└───────────────────────────────────────────────────────┘
```

## Controls

### Text Input (Keyboard Required)
- **Any printable key**: Insert character at cursor
- **Backspace**: Delete character before cursor
- **Delete**: Delete character at cursor
- **Enter**: New line (content only, not title)
- **Left/Right Arrow**: Move cursor within line
- **Home/End**: Jump to start/end of line

### Field Navigation
- **Tab**: Switch between title and content fields
- **Up/Down Arrow**: Navigate content lines (when in content field)
- **Button B** or **ESC**: Save changes and close editor

### Joystick Support (Limited)
- Joystick buttons don't input text (keyboard required)
- Button B: Save and close editor
- Stick: No function (could move cursor in future)

## Editor State

```c
typedef enum {
    TEXT_FIELD_TITLE,     // Editing box title
    TEXT_FIELD_CONTENT    // Editing box content
} TextEditField;

typedef struct {
    bool active;              // Is text editor open?
    TextEditField field;      // Which field is active
    int content_line;         // Current line in content (0-based)
    int cursor_pos;           // Cursor position within current line

    // Backup for cancel (not implemented yet - ESC always saves)
    char *backup_title;
    char **backup_content;
    int backup_content_lines;
} TextEditorState;
```

Stored in `JoystickState` or as global state.

## Implementation Approach

### Phase 3a: Basic Title Editing
Start with simplest case:
1. Button A in EDIT mode opens editor
2. Editor shows only title field
3. Keyboard input modifies title
4. ESC or Button B saves and closes
5. No content editing yet

### Phase 3b: Content Editing
Add content editing:
1. Tab switches to content field
2. Up/Down navigates lines
3. Enter creates new lines
4. Edit multiple lines

### Phase 3c: Enhanced Features (Future)
1. Cancel without saving (Button X)
2. Undo/Redo
3. Copy/Paste
4. On-screen keyboard fallback for joystick-only users
5. Syntax highlighting for code boxes

## Technical Implementation

### Text Storage in Box
Current Box structure (from types.h):
```c
typedef struct {
    int id;
    double x, y;
    int width, height;
    char *title;
    char **content;        // Array of strings
    int content_lines;
    int color;
} Box;
```

Box already has dynamic title and content - just need to modify them.

### Input Handling Flow
```
1. User presses Button A in EDIT mode
   → joystick_open_text_editor(js, box)
   → Backup title/content
   → Set text_editor_active = true

2. While text_editor_active:
   → Intercept all keyboard input
   → Handle printable chars (insert into title/content)
   → Handle special keys (backspace, enter, arrows)
   → Handle Tab (switch fields)
   → Handle ESC/Button B (save and close)

3. User presses ESC or Button B:
   → joystick_close_text_editor(js, box)
   → Save changes to box
   → Set text_editor_active = false
   → Return to EDIT mode
```

### Cursor Management
- Track cursor position in current line
- Visual cursor: underscore character or reverse video
- Clamp cursor to valid positions (0 to strlen)
- Handle cursor at line boundaries

### Line Management (Content)
- content is array of strings (char **)
- Each line is null-terminated string
- Use realloc to add/remove lines
- Max lines: configurable (start with 50?)
- Max line length: configurable (start with 200?)

## Rendering

### Modal Overlay
- Darkened background (optional)
- Centered panel (or full-width for more text)
- Box border with title "Edit Text"
- Clear field labels
- Instruction hints at bottom

### Cursor Visualization
- Blinking cursor (using frame counter)
- Or simple underscore character
- Or reverse video on current character

### Multi-line Content Display
- Show current line being edited
- Show context (lines above/below)
- Line number indicator "Line 3 of 10"

## Configuration Points

These can be easily adjusted:
- Keyboard shortcuts (Tab, ESC, etc.)
- Button assignments (Button A = edit, Button B = save)
- Max line length
- Max number of lines
- Cursor blink rate
- Panel size and position
- Whether to backup for cancel (vs always save)

## Error Handling

### No Keyboard Available
- Show message: "Keyboard Required for Text Editing"
- Don't open editor if keyboard not detected (how to detect?)
- Suggest: "Press Button B to cancel"

### Text Too Long
- Limit title to box width - 4 characters
- Limit content lines to reasonable number (50?)
- Limit line length to prevent overflow
- Truncate or warn user

### Memory Allocation Failures
- Check malloc/realloc returns
- Fall back to not adding line if allocation fails
- Close editor gracefully

## Future Enhancements

### On-Screen Keyboard (Phase 3d)
For joystick-only users:
```
┌─ On-Screen Keyboard ────────────────────┐
│                                          │
│  Q W E R T Y U I O P  [Del]             │
│   A S D F G H J K L   [Enter]           │
│    Z X C V B N M     [Space] [Cancel]   │
│                                          │
│  Current: Hello_                         │
└──────────────────────────────────────────┘
```
- Grid of characters
- Stick moves cursor over keyboard
- Button A types character
- Button B deletes
- Button X spaces
- Button Y saves and closes

### Rich Text Features
- Bold/italic/underline (using ncurses attributes)
- Color text
- Bullet points
- Code blocks with syntax highlighting

## Testing Workflow

1. Create a box (Button X in VIEW mode)
2. Select box (Button Y to enter SELECT, Button A to cycle)
3. Enter EDIT mode (Button X in SELECT)
4. Press Button A to edit text
5. Type new title with keyboard
6. Press Tab to switch to content
7. Type content text
8. Press ESC or Button B to save and close
9. Verify box shows new text

## Implementation Plan

### Step 1: Basic Title Editing
- Add TextEditorState to JoystickState
- Add open/close functions
- Intercept keyboard when editor active
- Render simple editor panel
- Handle basic text input (printable chars, backspace)
- Save on ESC/Button B

### Step 2: Content Editing
- Add Tab to switch fields
- Add Up/Down for line navigation
- Add Enter for new lines
- Display multiple lines

### Step 3: Polish
- Add cursor blinking
- Show current line number
- Add max length limits
- Handle edge cases

### Step 4: On-Screen Keyboard (Optional)
- Only if user specifically needs joystick-only input
- Can be added later based on feedback

## Next Steps

Implement Step 1 (Basic Title Editing):
1. Add text editor state
2. Button A opens editor
3. Keyboard input modifies title
4. ESC saves and closes
5. Simple UI showing title field
