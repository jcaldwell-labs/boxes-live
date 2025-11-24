# Joystick UX Redesign Proposal

## Current Problems
1. **No clear workflow** - Button mappings exist but don't form a coherent editing experience
2. **Clunky parameter editing** - Mode system is confusing, no clear apply/cancel
3. **No text editing** - Can't modify box content
4. **No box creation defaults** - Creating boxes is awkward
5. **No connections** - Can't link boxes together
6. **No grid** - Hard to align boxes precisely
7. **Training canvas overwhelms** - Too many boxes on startup

## Proposed Mode System

### Clear Mode Hierarchy
Replace current 3-mode system with 4 clear modes:

**1. VIEW MODE** (default on startup)
- Purpose: Navigate and observe canvas
- Left Stick: Pan viewport
- Button A/B: Zoom in/out
- Button X: Quick-add box at cursor
- Button Y: Enter SELECT mode
- Button MENU: Cycle to next mode
- No editing allowed - safe exploration

**2. SELECT MODE**
- Purpose: Choose and move boxes
- Left Stick: Move cursor (selects box under cursor)
- Button A: Select box / Confirm move
- Button B: Deselect / Back to VIEW
- Button X: Enter EDIT mode for selected box
- Button Y: Delete selected box (with confirmation)
- Button LB+Stick: Multi-select (hold LB, move cursor)
- Button RB: Quick-duplicate selected box
- Button MENU: Cycle to next mode

**3. EDIT MODE** (for selected box)
- Purpose: Modify box properties and content
- Left Stick: Resize box (width/height)
- Right Stick (if available): Fine-tune position
- Button A: Enter TEXT EDIT for content
- Button B: Apply changes & return to SELECT
- Button X: Cycle box style/color
- Button Y: Open PARAMETER panel
- Button LB: Decrease property value
- Button RB: Increase property value
- D-pad: Navigate parameter fields
- Button MENU: Cycle to next mode

**4. CONNECT MODE**
- Purpose: Link boxes together
- Left Stick: Move cursor
- Button A: Start connection from box / End connection at box
- Button B: Cancel connection / Back to VIEW
- Button X: Toggle connection style (solid/dashed/arrow)
- Button Y: Delete connection
- Button MENU: Cycle to next mode

### Mode Indicator Enhancement
- Large mode banner at top of screen
- Show available actions for current mode
- Color-coded: VIEW=blue, SELECT=green, EDIT=yellow, CONNECT=red
- Visualizer panel updates to show mode-specific actions

## Box Creation Workflow

### Default Canvas
Start with ONE example box showing the help text:
```
┌─ Welcome to Boxes-Live! ────┐
│                              │
│ MENU: Toggle mode            │
│ Current: VIEW                │
│                              │
│ Press X to create new box    │
│ Press Y to enter SELECT mode │
│                              │
└──────────────────────────────┘
```

### Quick Box Creation
- **Button X in VIEW mode**: Creates box at cursor with sensible defaults
  - Size: 20x5 (readable default)
  - Content: "New Box" with timestamp
  - Position: At cursor or viewport center
  - Automatically enter SELECT mode with new box selected

### Box Templates (future enhancement)
- Hold LB + press X: Open template menu
  - Small note (15x3)
  - Medium box (25x6)
  - Large box (40x10)
  - Title box (50x3, centered text)
  - Code block (60x15, monospace)

## Parameter Editor Redesign

### Current Problems
- Confusing mode transitions
- No clear "apply" or "cancel"
- Hard to see what you're editing

### New Parameter Panel (Button Y in EDIT mode)
```
┌─ Box Properties ──────────────┐
│                                │
│ [>] Width:  25    ◄  ►         │
│ [ ] Height: 10    ◄  ►         │
│ [ ] Color:  Blue  ◄  ►         │
│ [ ] Style:  Solid ◄  ►         │
│                                │
│ ┌─ Preview ──────────┐         │
│ │                    │         │
│ │   Preview Box      │         │
│ │                    │         │
│ └────────────────────┘         │
│                                │
│ [A] Apply  [B] Cancel          │
│                                │
└────────────────────────────────┘
```

**Controls:**
- Left Stick Up/Down: Navigate fields (arrow moves)
- Left Stick Left/Right: Adjust value (or LB/RB buttons)
- Button A: Apply and close
- Button B: Cancel and close
- Live preview shows changes in real-time

## Text Editing System

### Approach 1: Keyboard Required
- Button A in EDIT mode: Opens text editor
- Keyboard required for text input
- ESC or Button B: Close editor
- Show "Keyboard Required" message if not available

### Approach 2: Joystick Text Entry (fallback)
- On-screen keyboard with cursor
- Slow but functional
- Left stick: Move cursor
- Button A: Type character
- Button B: Backspace
- Button X: Space
- Button Y: Done

### Proposed Implementation
- Try Approach 1 first (most users have keyboard)
- Show clear message: "Press A to edit text (keyboard required)"
- Implement Approach 2 later if needed

## Grid System

### Grid Display
- Toggle with LB+RB simultaneous press (or keyboard 'G')
- Visual: Faint dots or lines at regular intervals
- Configurable spacing: 5, 10, 20 world units
- Rendered behind all boxes

### Snap to Grid
- Toggle with Button START (hold for 1 second)
- When enabled:
  - Box positions snap to grid
  - Box sizes snap to grid increments
  - Visual indicator "SNAP" in status bar
  - Audible feedback (beep?) when snapping

### Grid Configuration
- In parameter mode, add "Grid" section
- Spacing: 5/10/20
- Snap strength: Off/Loose/Strict
- Grid color/style

## Box Connections

### Connection Model
```
typedef struct {
    int from_box_id;
    int to_box_id;
    ConnectionStyle style;  // SOLID, DASHED, ARROW
    int color;
    char *label;  // Optional text on connection
} Connection;
```

### Creating Connections (CONNECT mode)
1. Enter CONNECT mode (Button MENU from any mode)
2. Move cursor to source box, press A
3. Source box highlights in blue
4. Move cursor to destination box, press A
5. Connection created with default style
6. Press X to cycle connection styles before confirming

### Connection Rendering
- Draw lines between box centers (or edges)
- Support arrow heads for directional flow
- Avoid overlapping with boxes (route around?)
- Show connection labels if set

### Connection Management
- Button Y in CONNECT mode: Delete connection under cursor
- Connections stored in Canvas structure
- Save/load connections with canvas

## Visualizer Panel Updates

### Mode-Aware Display
Show relevant actions for current mode:

**VIEW Mode:**
```
Mode: VIEW (Explore)
─────────────────────
[A ] Zoom In
[B ] Zoom Out
[X ] Create Box
[Y ] → SELECT Mode
[LB] Show Grid
[RB] Toggle Snap
Left Stick: Pan
```

**SELECT Mode:**
```
Mode: SELECT (Choose)
─────────────────────
[A ] Select Box
[B ] → VIEW Mode
[X ] → EDIT Mode
[Y ] Delete Box
[LB] Multi-Select
[RB] Duplicate Box
Left Stick: Move Cursor
```

**EDIT Mode:**
```
Mode: EDIT (Modify)
─────────────────────
[A ] Edit Text
[B ] Apply & Exit
[X ] Cycle Color
[Y ] Parameters
[LB] Decrease Value
[RB] Increase Value
Left Stick: Resize Box
```

**CONNECT Mode:**
```
Mode: CONNECT (Link)
─────────────────────
[A ] Connect Boxes
[B ] → VIEW Mode
[X ] Cycle Style
[Y ] Delete Link
Left Stick: Move Cursor
```

## Implementation Priority

### Phase 1: Core Mode System (Implement First)
1. ✓ Implement 4-mode enum (VIEW, SELECT, EDIT, CONNECT)
2. ✓ Add mode toggle with Button MENU (7)
3. ✓ Update visualizer to show mode-specific actions
4. ✓ Simplified default canvas with one box
5. ✓ Quick box creation (Button X in VIEW)

### Phase 2: Parameter Editor
6. Redesign parameter panel with Apply/Cancel
7. Add live preview in parameter editor
8. Clean up field navigation with clear cursor

### Phase 3: Text Editing
9. Add text editing mode with keyboard input
10. Fallback to on-screen keyboard if needed

### Phase 4: Grid System
11. Implement grid rendering (toggle with LB+RB)
12. Add snap-to-grid functionality
13. Grid configuration in settings

### Phase 5: Connections
14. Add Connection data structure to Canvas
15. Implement CONNECT mode workflow
16. Connection rendering with styles
17. Save/load connections

## Open Questions for Discussion

1. **Mode cycling direction**: Always forward, or Button MENU cycles forward, Button BACK cycles backward?
2. **Default box size**: 20x5 good? Or larger like 30x8?
3. **Connection routing**: Simple straight lines, or smart routing around boxes?
4. **Grid spacing**: Default to 10 units? Configurable?
5. **Text editor**: Full-screen takeover, or inline editing in box?
6. **Multi-select**: Worth the complexity? Or focus on single-box operations?
7. **Undo/Redo**: Should we add Ctrl+Z support for keyboard users?

## Next Steps

Please review this proposal and provide feedback:
- Which features are highest priority?
- Any concerns about the mode system?
- Alternative approaches to consider?
- Which open questions need decisions?

Once approved, I'll implement Phase 1 first to establish the core mode system.
