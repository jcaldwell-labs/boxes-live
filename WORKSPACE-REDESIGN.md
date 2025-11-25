# Boxes-Live: Workspace Redesign - Practical Terminal Workspace

## The Vision

Transform boxes-live from a visual canvas into a **practical terminal workspace manager** where each box is a functional workspace component (file viewer, terminal session, command output, etc.).

Think: **tmux meets spatial organization meets specialized box types**

---

## Real Use Cases

### Use Case 1: Development Workspace
```
┌─ File Tree ────┐  ┌─ main.c ──────────────────┐  ┌─ Build Output ──┐
│ src/           │  │ #include <stdio.h>        │  │ gcc -o app       │
│ ├─ main.c      │  │ int main() {              │  │ Build: SUCCESS   │
│ ├─ utils.c     │  │   printf("Hello\n");      │  │ 0 errors         │
│ └─ config.h    │  │   return 0;               │  └──────────────────┘
│                │  │ }                         │
└────────────────┘  └───────────────────────────┘
```

### Use Case 2: Log Monitoring
```
┌─ Error Log ────────────┐  ┌─ Access Log ─────────┐
│ [ERROR] Connection     │  │ 200 GET /api/users   │
│ [ERROR] Timeout        │  │ 200 POST /api/data   │
│ [WARN] Retry...        │  │ 404 GET /missing     │
└────────────────────────┘  └──────────────────────┘

┌─ System Stats ─────────────────────────────────────┐
│ CPU: 45%  MEM: 2.1GB  DISK: 120GB                  │
└────────────────────────────────────────────────────┘
```

### Use Case 3: File Selection Workflow
```
┌─ Project Files (fuzzy search) ────────────────┐
│ > src/main.c                                   │
│   src/utils.c                                  │
│   src/config.h                                 │
│   include/types.h                              │
│   README.md                                    │
│                                                │
│ Type to filter: mai_                           │
└────────────────────────────────────────────────┘

Press Enter → File opens in new box or expands current box
```

### Use Case 4: Terminal Sessions
```
┌─ Terminal 1 ───────┐  ┌─ Terminal 2 ───────┐
│ $ npm run dev      │  │ $ git status       │
│ Server running...  │  │ On branch main     │
│                    │  │                    │
└────────────────────┘  └────────────────────┘
```

---

## Box Types System

### Core Concept
Each box has a **type** that determines its behavior and rendering.

```c
typedef enum {
    BOX_TYPE_TEXT,          // Static text (current behavior)
    BOX_TYPE_FILE_VIEWER,   // Read-only file display
    BOX_TYPE_FILE_EDITOR,   // Editable file content
    BOX_TYPE_TERMINAL,      // Embedded shell session
    BOX_TYPE_COMMAND,       // Command output (live or static)
    BOX_TYPE_FILE_LIST,     // File browser/selector
    BOX_TYPE_LOG_TAIL,      // Live log file tail -f
    BOX_TYPE_PROCESS,       // Process monitor (top/htop)
    BOX_TYPE_CUSTOM         // User-defined script/plugin
} BoxType;

typedef struct Box {
    // ... existing fields ...

    BoxType type;
    void *type_data;        // Type-specific data
    bool focused;           // Is this box in focus mode?
    bool interactive;       // Can this box receive input?

    // File integration
    char *file_path;        // Associated file (if any)
    time_t file_mtime;      // Last modified time (for auto-reload)

    // Process integration
    pid_t process_pid;      // Associated process (if any)
    int pty_fd;             // PTY file descriptor for terminal boxes

    // Scrolling
    int scroll_offset;      // Scroll position for content
    int scroll_max;         // Maximum scroll (calculated from content)
} Box;
```

---

## Box Expansion / Focus Mode

### Concept
Press a key to "expand" a box to full screen, interact with it, then collapse back to canvas view.

### Activation
```
From any mode:
  - Press ENTER or Button A (when box selected)
  - Box expands to fill terminal
  - Enter FOCUS mode
```

### FOCUS Mode
```
┌─────────────────────────────────────────────────────────────┐
│ FILE: src/main.c                    [FOCUS MODE - ESC=Exit] │
├─────────────────────────────────────────────────────────────┤
│ #include <stdio.h>                                          │
│ #include <stdlib.h>                                         │
│                                                             │
│ int main(int argc, char *argv[]) {                         │
│     printf("Hello, World!\n");                             │
│     return 0;                                              │
│ }                                                           │
│                                                             │
│                                                             │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ j/k=Scroll  /=Search  E=Edit  ESC=Back to Canvas           │
└─────────────────────────────────────────────────────────────┘
```

**In FOCUS mode**:
- Box content fills entire terminal
- Full keyboard control of box content
- Box-type-specific commands
- ESC returns to canvas view
- Box position/size remembered

---

## File Integration

### File Viewer Box (BOX_TYPE_FILE_VIEWER)

**Creation**:
```
:file /path/to/file.txt     # Command mode
Or: Press F in SELECT mode → file picker appears
```

**Features**:
- Syntax highlighting (based on file extension)
- Line numbers
- Search (/ like vim)
- Scroll with j/k or stick
- Auto-reload on file change

**In Focus**:
- Full file content visible
- vim-like navigation (j/k/gg/G)
- Press E to convert to FILE_EDITOR

### File Editor Box (BOX_TYPE_FILE_EDITOR)

**Features**:
- Full text editing (like our current text editor)
- Save writes to file
- Undo/redo
- Syntax highlighting
- Line-based editing

**In Focus**:
- Acts like nano/micro editor
- ESC or :wq saves and returns to canvas

### File List Box (BOX_TYPE_FILE_LIST)

**Features**:
- Shows files matching glob pattern
- Fuzzy search/filter
- Up/Down to navigate
- Enter to open file in new viewer box

**Example**:
```
Box shows: "src/**/*.c"
Contents:
  > src/main.c
    src/render.c
    src/input.c
    ...
```

Press Enter → Creates new FILE_VIEWER box with selected file

---

## Terminal Integration

### Terminal Box (BOX_TYPE_TERMINAL)

**Uses PTY** (pseudo-terminal):
- Embedded shell session
- Full terminal emulation (using existing terminal libs)
- Runs bash/zsh/fish
- Can run interactive commands

**In Canvas View**:
- Shows last N lines of output (like preview)
- Title shows current command or "Shell 1"

**In Focus**:
- Full terminal interaction
- Type commands, see output
- Ctrl+C works
- ESC returns to canvas (shell keeps running)

### Command Output Box (BOX_TYPE_COMMAND)

**One-time command execution**:
```
:run ls -la          # Runs command, captures output
:run npm test        # Runs tests, shows results
:watch df -h         # Runs command every N seconds
```

**Features**:
- Captures stdout/stderr
- Shows exit code
- Option to re-run
- Option to convert to live tail

---

## Better Keybinding System

### Current Problem
- Hardcoded button mappings
- Different controls per mode
- Hard to customize
- Not obvious to users

### Solution: Configuration File

**~/.config/boxes-live/keys.conf**:
```ini
[global]
quit = q, ESC
save = F2, :w
mode_cycle = MENU

[view_mode]
pan_up = w, UP, STICK_Y_NEG
pan_down = s, DOWN, STICK_Y_POS
pan_left = a, LEFT, STICK_X_NEG
pan_right = d, RIGHT, STICK_X_POS
zoom_in = +, z, BUTTON_A
zoom_out = -, x, BUTTON_B
create_box = n, BUTTON_X
enter_select = TAB, BUTTON_Y
toggle_grid = g, BUTTON_LB
toggle_snap = G, BUTTON_RB

[select_mode]
cycle_box = TAB, j, k, BUTTON_A, STICK_Y
enter_edit = ENTER, BUTTON_X
delete_box = d, BUTTON_Y
back_to_view = ESC, BUTTON_B
focus_box = SPACE, ENTER, BUTTON_A

[edit_mode]
move_box = h, j, k, l, STICK
resize_box = H, J, K, L, SHIFT+STICK
edit_text = i, a, BUTTON_A
parameters = p, BUTTON_Y
cycle_color = c, BUTTON_X
back_to_select = ESC, BUTTON_B

[focus_mode]
scroll_up = k, UP, STICK_Y_NEG
scroll_down = j, DOWN, STICK_Y_POS
search = /
edit_mode = e, i
exit_focus = ESC, q, BUTTON_B
```

### Dynamic Key Binding
- Load from config file
- Fall back to defaults
- Show active bindings in help (F1)
- Remappable per-user

---

## Command Mode (Like vim :command)

### Activation
Press `:` (colon) → Enter command mode

### Commands
```
:file /path/to/file         # Open file in new viewer box
:glob src/**/*.c            # Open file list box
:run make                   # Run command, show output in box
:terminal                   # Create new terminal box
:help                       # Show help box
:config                     # Open configuration editor
:save canvas.txt            # Save canvas
:load canvas.txt            # Load canvas
:mode select                # Switch to select mode
:grid on                    # Enable grid
:snap on                    # Enable snap
:box rename "New Title"     # Rename selected box
:box type terminal          # Change box type
:quit                       # Quit application
```

### Command Parser
- Simple split on whitespace
- Arguments after command name
- Tab completion for file paths
- History (up/down arrows)

---

## Refined Mode System

### Rethinking Modes

**BROWSE Mode** (was VIEW)
- Navigate canvas spatially
- Preview box contents
- Quick actions (create, delete)
- File operations from command mode

**FOCUS Mode** (new, replaces SELECT/EDIT)
- Selected box takes full screen
- Type-specific interaction
- Deep work on one box
- ESC returns to BROWSE

**COMMAND Mode** (new)
- Vim-like : commands
- Create boxes with specific types
- Execute commands
- Configure settings

### Simplified Workflow

```
1. Start in BROWSE mode
   - See all boxes spatially
   - Pan and zoom
   - Tab to select boxes

2. Press ENTER on selected box
   → Enter FOCUS mode
   - Box expands to full screen
   - Interact with box content
   - Type-specific controls

3. Press ESC
   → Return to BROWSE mode
   - See full canvas
   - Select different box

4. Press :
   → Enter COMMAND mode
   - Type command (:file /path/to/file)
   - Create new boxes
   - Execute actions
```

---

## Practical Configuration System

### Config File Structure

**~/.config/boxes-live/config.toml**:
```toml
[general]
default_canvas = "~/.boxes-live/workspaces/default.canvas"
auto_save = true
auto_save_interval = 60  # seconds

[grid]
default_visible = false
default_spacing = 10
snap_enabled = false
dot_character = "."
origin_marker = "+"

[keybindings]
# See keys.conf for detailed bindings
config_file = "~/.config/boxes-live/keys.conf"

[box_defaults]
text_box_width = 50
text_box_height = 16
file_viewer_width = 80
file_viewer_height = 30
terminal_width = 80
terminal_height = 24

[colors]
grid_color = "gray"
selected_box_border = "green"
focused_box_border = "yellow"
error_box_border = "red"

[file_integration]
auto_reload_on_change = true
syntax_highlighting = true
line_numbers = true
tab_width = 4

[modes]
# Which modes are enabled
enable_browse = true
enable_focus = true
enable_command = true
enable_connect = false  # Future feature

# Mode cycling order
mode_cycle_order = ["browse", "focus", "command"]

[joystick]
# Which joystick buttons are configurable
button_layout = "xbox"  # xbox, playstation, generic
analog_sensitivity = 1.0
deadzone = 0.15
settling_frames = 30
show_visualizer = true

# Per-mode button mappings (overrides keys.conf)
[joystick.browse_mode]
button_a = "zoom_in"
button_b = "zoom_out"
button_x = "create_box"
button_y = "focus_box"  # Changed from "enter_select"
button_lb = "toggle_grid"
button_rb = "toggle_snap"
button_menu = "command_mode"
button_start = "save_canvas"
button_select = "quit"

[joystick.focus_mode]
button_a = "action"  # Type-specific (edit text, run command, etc.)
button_b = "exit_focus"
button_x = "scroll_page_down"
button_y = "scroll_page_up"
button_lb = "scroll_up"
button_rb = "scroll_down"
left_stick = "scroll"  # Vertical scroll in focus mode
```

---

## Box Type Implementations

### BOX_TYPE_FILE_VIEWER

```c
typedef struct {
    char **lines;           // File content (array of lines)
    int num_lines;
    int scroll_offset;      // Current scroll position
    char *syntax_type;      // "c", "python", "markdown", etc.
    bool show_line_numbers;
} FileViewerData;
```

**Operations**:
- Open file → Parse into lines
- Scroll with j/k or stick
- Search with /
- Follow mode (tail -f)
- Reload on file change (inotify)

### BOX_TYPE_FILE_EDITOR

```c
typedef struct {
    char **lines;
    int num_lines;
    int cursor_line;
    int cursor_col;
    bool modified;
    bool auto_save;
} FileEditorData;
```

**Operations**:
- Full text editing
- Line-based operations
- Save to file
- Undo/redo
- Syntax highlighting

### BOX_TYPE_TERMINAL

```c
typedef struct {
    int pty_master;         // PTY master fd
    int pty_slave;          // PTY slave fd
    pid_t shell_pid;        // Shell process
    char **scrollback;      // Scrollback buffer
    int scrollback_lines;
} TerminalData;
```

**Operations**:
- Create PTY pair
- Fork shell process
- Capture output
- Send input when focused
- Maintain scrollback buffer

### BOX_TYPE_FILE_LIST

```c
typedef struct {
    char **file_paths;      // Matched files
    int num_files;
    int selected_index;     // Current selection
    char *filter_text;      // Fuzzy search filter
    char *glob_pattern;     // Original pattern
} FileListData;
```

**Operations**:
- Glob file paths
- Fuzzy filter by name
- Navigate with j/k
- Enter creates viewer box
- Live update as files change

### BOX_TYPE_COMMAND

```c
typedef struct {
    char *command;          // Command string
    char **output_lines;    // Captured output
    int exit_code;
    bool is_running;
    bool auto_refresh;      // Re-run periodically
    int refresh_interval;   // Seconds
} CommandData;
```

**Operations**:
- Execute command
- Capture stdout/stderr
- Show exit code
- Option to re-run
- Option to watch (refresh)

---

## Focus Mode Redesign

### Full-Screen Box Interaction

When box is focused (ENTER or Button A when selected):

```
Previous: Canvas with multiple boxes
Press ENTER on selected box
↓
┌────────────────────────────────────────────────────────────┐
│ [FOCUS] src/main.c                              [ESC=Exit] │
├────────────────────────────────────────────────────────────┤
│     1  #include <stdio.h>                                  │
│     2  #include <stdlib.h>                                 │
│     3                                                       │
│     4  int main(int argc, char *argv[]) {                  │
│     5      printf("Hello, World!\n");                      │
│     6      return 0;                                        │
│     7  }                                                    │
│     8                                                       │
│   ...                                                       │
│                                                             │
├────────────────────────────────────────────────────────────┤
│ Type-specific controls shown here                          │
│ j/k=Scroll  /=Search  E=Edit  :=Command  ESC=Back          │
└────────────────────────────────────────────────────────────┘

Press ESC
↓
Back to canvas view with box in original position/size
```

### Type-Specific Focus Behavior

**FILE_VIEWER**: Vim-like navigation
**FILE_EDITOR**: Full editing mode
**TERMINAL**: Interactive shell
**FILE_LIST**: Navigate and select files
**COMMAND**: Show full output, option to re-run

---

## Cleaner Integration Patterns

### Pattern 1: File → Box Pipeline

```bash
# Command mode creates boxes from files
:file src/main.c                    # Creates FILE_VIEWER box
:edit config.h                      # Creates FILE_EDITOR box
:glob src/**/*.c                    # Creates FILE_LIST box
:tail -f /var/log/app.log          # Creates LOG_TAIL box
```

### Pattern 2: Box → Terminal Integration

```bash
# Boxes can execute commands
:run make                           # Creates COMMAND box with output
:terminal                           # Creates TERMINAL box with shell
:watch "git status"                 # Creates auto-refreshing COMMAND box
```

### Pattern 3: Box Linking (Phase 5)

```
File List Box → Connected to → File Viewer Box

When you select file in list:
  - Viewer box automatically updates to show selected file
  - Connection line shows data flow
```

---

## Configuration System Implementation

### Phase 1: TOML Config Parser
```c
// Use a lightweight TOML parser library
// Or write simple ini-style parser
typedef struct {
    char *key;
    char *value;
} ConfigEntry;

typedef struct {
    ConfigEntry *entries;
    int num_entries;
} Config;

Config* load_config(const char *path);
const char* config_get(Config *cfg, const char *section, const char *key, const char *default_val);
int config_get_int(Config *cfg, const char *section, const char *key, int default_val);
bool config_get_bool(Config *cfg, const char *section, const char *key, bool default_val);
```

### Phase 2: Key Binding Parser
```c
typedef struct {
    char *action_name;      // "zoom_in"
    int num_bindings;
    struct {
        InputType type;     // KEYBOARD, JOYSTICK, MOUSE
        int code;           // Key code or button number
        int modifiers;      // Shift, Ctrl, Alt
    } bindings[8];          // Max 8 bindings per action
} KeyBinding;

// Load from keys.conf
KeyBinding* load_keybindings(const char *path);
KeyBinding* find_binding(KeyBinding *bindings, const char *action);
```

### Phase 3: Runtime Reconfiguration
```
:config                     # Opens config editor
:bind g toggle_grid         # Rebind on the fly
:unbind BUTTON_X            # Remove binding
:reload_config              # Reload config file
```

---

## Practical Workflow Examples

### Workflow 1: Code Review

```
1. Start boxes-live in project directory
2. :glob src/**/*.c                  # File list box
3. Select file with j/k, press ENTER
4. File viewer box created and focused
5. Press E to edit
6. Make changes
7. ESC back to canvas
8. :run make                         # Build in command box
9. See results
10. :save code-review-workspace
```

### Workflow 2: Log Analysis

```
1. :tail -f /var/log/error.log       # Live error log
2. :tail -f /var/log/access.log      # Live access log
3. :watch "df -h"                    # Disk usage monitor
4. Arrange boxes spatially
5. Monitor all three simultaneously
6. Focus on error log when needed
7. ESC back to overview
```

### Workflow 3: Git Workflow

```
1. :run git status                   # Status box
2. :file .git/COMMIT_EDITMSG         # Last commit message
3. :glob src/**/*.{c,h}              # Changed files
4. Select file → ENTER → View changes
5. :terminal                         # Open shell for git commands
6. Save workspace: :save git-workflow
```

---

## Migration Strategy

### Keep Existing Features

Don't break what works:
- ✓ 4-mode system (VIEW→BROWSE, SELECT→FOCUS)
- ✓ Joystick visualizer
- ✓ Parameter editor
- ✓ Text editor (becomes foundation for FILE_EDITOR)
- ✓ Grid system

### Add New Systems

Layer on top:
1. Box type system
2. Focus mode (full-screen boxes)
3. Command mode (: commands)
4. File integration
5. Terminal integration
6. Configuration system

### Incremental Implementation

**Phase 5a**: Box types and focus mode
**Phase 5b**: File viewer boxes
**Phase 5c**: Configuration system
**Phase 5d**: Terminal boxes
**Phase 5e**: Command mode
**Phase 5f**: File list/glob boxes

---

## Immediate Next Steps

### What to Implement First?

**Option A: Focus Mode** (Most Impact)
- Add FOCUS mode (box expansion)
- ESC returns to canvas
- Makes existing boxes more useful immediately

**Option B: Box Types** (Foundation)
- Add BoxType enum
- Add FILE_VIEWER type
- Command to create from file (:file path)
- Foundation for all other features

**Option C: Configuration System** (User Request)
- Load config file
- Parse keybindings
- Make joystick configurable
- Addresses immediate pain point

**Recommendation**: Start with **Option C** (Config System), then **Option A** (Focus Mode), then **Option B** (Box Types)

This addresses your immediate needs (configurable joystick), enables better testing of the mode system, and sets up for the practical workspace features.

---

## Questions for You

1. **Which implementation order?** Config → Focus → Types, or different priority?

2. **Box types most useful?** Which types do you want first?
   - File viewer?
   - Terminal?
   - Command output?
   - File list?

3. **Configuration format?** TOML, INI, or simple key=value?

4. **Command mode activation?** Colon (:) like vim, or different key?

5. **Focus mode activation?** ENTER, SPACE, or specific button?

6. **Keep 4 modes or simplify to 2-3?** BROWSE + FOCUS might be enough?

---

## Design Philosophy

Going forward:
- **Practical first** - Features must solve real problems
- **Configurable by default** - Everything in config file
- **Type-specific behavior** - Boxes adapt to content type
- **Clean integration** - Files, terminals, commands feel natural
- **Spatial organization** - Keep the canvas metaphor for navigation
- **Keyboard-first** - Joystick is enhancement, not requirement

This transforms boxes-live from "terminal canvas toy" into "spatial terminal workspace manager" - a real tool for real work.

Shall I proceed with configuration system implementation?
