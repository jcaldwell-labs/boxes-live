# Boxes-Live Demo and Testing Guide

## Promotional Video Script / Feature Showcase

This guide provides a step-by-step walkthrough of all features, designed for creating promotional videos or comprehensive user testing.

---

## Setup (Pre-Demo)

### Terminal Setup
```bash
# Ensure you have a modern terminal (xterm, gnome-terminal, alacritty, kitty)
# Recommended size: 120x40 or larger

# Navigate to project
cd /home/be-dev-agent/projects/boxes-live

# Build the application
make clean && make

# Optional: Build with debug logging
make clean
make DEBUG=1
```

### Equipment Checklist
- ✓ Terminal emulator with box-drawing support
- ✓ Keyboard (for text editing)
- ✓ Mouse (for drag and drop)
- ✓ Joystick/gamepad (optional, for full feature demo)

---

## Demo Script: "Welcome to Boxes-Live"

### Part 1: First Launch (0:00-0:30)

**Narration**: "Boxes-Live turns your terminal into an interactive canvas workspace, like Miro or Figma, but in pure ASCII."

**Actions**:
```bash
./boxes-live
```

**What You See**:
- Clean welcome box with basic instructions
- Status bar showing camera position
- Mode indicator: "VIEW"
- Joystick visualizer panel (if joystick connected)

**Demonstrate**:
- Point out the single welcome box
- Show the mode indicator in status bar
- Show the visualizer panel on the right

---

### Part 2: Navigation (0:30-1:00)

**Narration**: "Navigate the canvas smoothly with keyboard, mouse, or joystick."

**Keyboard**:
```
WASD or Arrow Keys → Pan around
+ or - → Zoom in/out
R → Reset view to origin
```

**Joystick** (if available):
```
Left Stick → Pan (analog control)
Button A → Zoom in
Button B → Zoom out
Button LB → Reset view
```

**Demonstrate**:
1. Pan up/down/left/right with WASD
2. Zoom in to see box details
3. Zoom out for overview
4. Reset view with R
5. (If joystick) Show analog stick control with visualizer
6. Point out camera position updates in status bar

---

### Part 3: The Mode System (1:00-1:30)

**Narration**: "Boxes-Live uses a clear 4-mode system. Each mode has a specific purpose."

**The 4 Modes**:
1. **VIEW** - Safe exploration (pan and zoom only)
2. **SELECT** - Choose and navigate boxes
3. **EDIT** - Modify selected box
4. **CONNECT** - Link boxes together (coming soon)

**Demonstrate**:
```
Keyboard: (modes triggered automatically)
Joystick: Button MENU to cycle through modes
```

**Show**:
1. Current mode: VIEW (shown in status bar)
2. Press Button MENU (or Y for joystick)
3. Watch mode change to SELECT
4. Point out how visualizer updates with new button mappings
5. Press MENU again → EDIT
6. Press MENU again → CONNECT
7. Press MENU again → Back to VIEW

**Key Point**: "Each mode shows exactly what each button does - no guessing!"

---

### Part 4: Creating Boxes (1:30-2:00)

**Narration**: "Creating boxes is instant and intuitive."

**Quick Create (VIEW Mode)**:
```
Keyboard: Press N
Joystick: Press Button X
```

**Demonstrate**:
1. Make sure in VIEW mode
2. Pan to empty area
3. Press N (or Button X)
4. **BOOM** - box appears instantly at cursor/viewport center
5. Create 2-3 more boxes in different locations
6. Show how quick and easy it is

**Default Box**:
- Size: 50x16 (readable)
- Title: "Welcome to Boxes-Live!"
- Position: At cursor or viewport center

---

### Part 5: Selecting and Moving Boxes (2:00-2:45)

**Narration**: "Select boxes with keyboard, mouse, or joystick."

**SELECT Mode**:

**Keyboard**:
```
Tab → Cycle through boxes
```

**Mouse**:
```
Left-Click on box → Select
Left-Click + Drag → Move box
Left-Click empty space → Deselect
```

**Joystick**:
```
Button Y (from VIEW) → Enter SELECT mode
Button A → Cycle through boxes
Left Stick → Move cursor (for selecting)
```

**Demonstrate**:
1. Press Tab to cycle through boxes
2. Watch selected box highlight in status bar
3. Switch to mouse
4. Click and drag a box to new position
5. (If joystick) Enter SELECT mode
6. Use Button A to cycle
7. Show visualizer updating

**Key Point**: "Selected box shown clearly in status bar"

---

### Part 6: Editing Box Properties (2:45-3:30)

**Narration**: "Edit properties with a clean, intuitive parameter editor."

**EDIT Mode Activation**:
```
Keyboard: Select a box with Tab
Joystick: Button X in SELECT mode
```

**Parameter Editor** (Button Y in EDIT mode):
```
Up/Down → Navigate fields
Left/Right → Adjust value
Button LB/RB → Large adjustments
Button A → Apply and close
Button B → Cancel and close
```

**Demonstrate**:
1. Select a box (Tab or Button A)
2. Enter EDIT mode
3. Press Button Y to open parameter editor
4. Show the 3 fields: Width, Height, Color
5. Navigate with stick up/down
6. Watch visual sliders update
7. Adjust width with stick left/right
8. Watch box resize in real-time (LIVE PREVIEW!)
9. Change to height field
10. Adjust height with Button LB/RB (big steps)
11. Change to color field
12. Cycle through colors (Red, Green, Blue, etc.)
13. Press Button A to apply

**Key Point**: "Live preview - you see changes immediately!"

---

### Part 7: Text Editing (3:30-4:15)

**Narration**: "Edit box titles with full keyboard support."

**Text Editor** (Button A in EDIT mode):
```
Type → Insert characters
Backspace → Delete
Arrow keys → Move cursor
Home/End → Jump to start/end
ESC or Button B → Save and close
```

**Demonstrate**:
1. With a box selected in EDIT mode
2. Press Button A (or just press A if using keyboard)
3. Text editor opens with current title
4. Show blinking cursor at end
5. Type new text: "My First Box"
6. Use arrows to move cursor back
7. Insert text in middle: "Amazing "
8. Result: "My Amazing First Box"
9. Press ESC to save
10. Watch box title update

**Key Point**: "Full text editing with visual cursor and scrolling support!"

---

### Part 8: The Visualizer Panel (4:15-4:45)

**Narration**: "Never wonder what a button does - the visualizer shows everything."

**Visualizer Features**:
- Current mode prominently displayed
- All 6 main buttons with live status
- Button highlights when pressed
- Action labels change per mode
- Left stick position indicator (5x5 grid)
- Numeric coordinates
- Global buttons (MENU, START, SELECT, BACK)

**Demonstrate**:
1. Point out visualizer on right side
2. Press buttons one by one
3. Watch them highlight in green
4. Change modes with MENU
5. Watch action labels update
6. Move left stick
7. Watch position indicator move in grid
8. Show numeric coordinates update
9. Press BACK to hide visualizer
10. Press BACK again to show

**Key Point**: "Real-time feedback - you always know what's happening!"

---

### Part 9: Complete Workflow Example (4:45-5:30)

**Narration**: "Let's create a complete diagram from scratch."

**Scenario**: Create a simple flowchart

**Steps**:
```
1. Start in VIEW mode
2. Press X to create "Start" box
3. Edit title: Button A → type "Start"
4. Press ESC to save

5. Pan right (WASD)
6. Press X to create "Process" box
7. Edit title: Button A → type "Process Data"
8. Edit properties: Button Y
   - Make it wider (width 40)
   - Change color to Blue
   - Apply with Button A

9. Pan right again
10. Create "End" box
11. Edit title: Button A → type "End"
12. Make it green with parameter editor

13. Pan to overview
14. Admire your creation!

15. Save canvas: Button START (or F2)
16. File saved to canvas.txt
```

**Demonstrate**:
- Smooth workflow from creation to editing
- Multiple modes working together
- Real-time updates
- Professional-looking result

---

### Part 10: Advanced Features (5:30-6:00)

**Narration**: "Boxes-Live includes advanced features for power users."

**Saving and Loading**:
```
Save: Button START (joystick) or F2 (keyboard)
Load: F3 (keyboard)
```

**Demonstrate**:
1. Create a few boxes
2. Press Button START to save
3. Quit with Q
4. Relaunch: ./boxes-live canvas.txt
5. Your work is back!

**Multi-Input Support**:
```
Keyboard: Full control, best for text
Mouse: Intuitive drag and drop
Joystick: Analog precision, hands-free
```

**Demonstrate**:
- Switch between input methods seamlessly
- Show how they all work together
- Mouse for quick selection
- Keyboard for precise input
- Joystick for smooth analog control

---

## Testing Checklist

### Basic Functionality
- [ ] Application launches without errors
- [ ] Welcome box displays correctly
- [ ] Can pan with WASD/arrows
- [ ] Can zoom with +/-
- [ ] Can reset view with R
- [ ] Can quit with Q or ESC

### Box Creation
- [ ] Can create box with N (keyboard)
- [ ] Can create box with Button X (joystick in VIEW)
- [ ] New box appears at correct location
- [ ] New box has default size and title

### Box Selection
- [ ] Can cycle boxes with Tab
- [ ] Can select box with mouse click
- [ ] Can select box with Button A (joystick in SELECT)
- [ ] Selected box shown in status bar
- [ ] Can deselect with click on empty space

### Box Movement
- [ ] Can drag box with mouse
- [ ] Can move box with stick in EDIT mode
- [ ] Box position updates smoothly

### Parameter Editing
- [ ] Can open parameter editor with Button Y in EDIT
- [ ] Can navigate fields with stick up/down
- [ ] Can adjust width with stick left/right
- [ ] Can adjust height with LB/RB buttons
- [ ] Can cycle colors
- [ ] Live preview shows changes immediately
- [ ] Button A applies changes
- [ ] Button B cancels and restores original values
- [ ] Visual sliders update correctly

### Text Editing
- [ ] Can open text editor with Button A in EDIT
- [ ] Cursor visible and positioned correctly
- [ ] Can type characters
- [ ] Can backspace
- [ ] Can move cursor with arrow keys
- [ ] Can jump to start/end with Home/End
- [ ] Text scrolls horizontally when too long
- [ ] ESC saves and closes
- [ ] Button B saves and closes
- [ ] Box title updates after save

### Visualizer
- [ ] Visualizer shows current mode
- [ ] Button labels update per mode
- [ ] Buttons highlight when pressed
- [ ] Stick indicator shows position
- [ ] Numeric coordinates update
- [ ] BACK button toggles visibility
- [ ] Global buttons shown

### Mode System
- [ ] Starts in VIEW mode
- [ ] MENU button cycles modes
- [ ] Mode shown in status bar
- [ ] Mode-specific controls work correctly
- [ ] Can cycle VIEW → SELECT → EDIT → CONNECT → VIEW

### Save/Load
- [ ] Can save with Button START or F2
- [ ] File saved to canvas.txt
- [ ] Can load with F3 or ./boxes-live canvas.txt
- [ ] All boxes restored correctly
- [ ] Box properties preserved

### Joystick Startup
- [ ] No "racing canvas" on startup
- [ ] Settling period prevents axis drift
- [ ] Cursor initialized at viewport center

---

## Known Issues / Future Enhancements

### Not Yet Implemented
- [ ] Grid display (Phase 4)
- [ ] Snap to grid (Phase 4)
- [ ] Box connections (Phase 5)
- [ ] Multi-line content editing (Phase 3b)
- [ ] Undo/redo
- [ ] Copy/paste boxes
- [ ] Box templates
- [ ] On-screen keyboard for joystick-only users

### Configuration System
- [ ] Configurable button mappings
- [ ] Configurable mode cycling behavior
- [ ] Adjustable parameter ranges
- [ ] Custom color schemes
- [ ] Keybinding preferences

---

## Promotional Video Outline (6 minutes)

### 0:00-0:15 - Hook
"What if you could build visual diagrams, mind maps, and flowcharts right in your terminal?"

### 0:15-0:30 - Introduction
"Meet Boxes-Live - an interactive canvas for your terminal."

### 0:30-1:00 - Navigation Demo
Show smooth panning and zooming

### 1:00-1:30 - Mode System
Explain the 4 modes with visualizer

### 1:30-2:00 - Quick Box Creation
Create boxes instantly

### 2:00-2:45 - Mouse and Keyboard
Show multiple input methods

### 2:45-3:30 - Parameter Editor
Live preview feature showcase

### 3:30-4:15 - Text Editing
Full keyboard support demo

### 4:15-4:45 - Visualizer
Real-time feedback system

### 4:45-5:30 - Complete Workflow
Build a flowchart from scratch

### 5:30-6:00 - Save/Load & Closing
Persistence and multi-session work

---

## Pro Tips for Demo

### Visual Appeal
- Use a terminal with good font rendering
- Consider terminal themes with high contrast
- Record at 120x40 or larger
- Use color terminal (not monochrome)

### Pacing
- Start slow to show basics
- Speed up for advanced features
- Pause on impressive features (live preview!)
- Leave time for viewer to read box text

### Narration Tips
- "Watch how smooth the panning is"
- "Notice the real-time updates"
- "All in your terminal - no GUI required"
- "Keyboard, mouse, AND joystick support"
- "The visualizer tells you exactly what each button does"

### Feature Highlights
- **Live preview** in parameter editor
- **Analog stick** precision control
- **Modal overlays** for focused editing
- **Multi-input** support (keyboard/mouse/joystick)
- **Persistent** canvases with save/load
- **Clean interface** with clear mode indicators

---

## Quick Start for New Users

### 30-Second Quick Start

```bash
# Launch
./boxes-live

# Create a box
Press N (or Button X with joystick)

# Edit the title
1. Press Tab to select box
2. Press Button A (or just start typing if keyboard-only)
3. Type "My First Box"
4. Press ESC to save

# Change the color
1. Press Button Y (opens parameter editor)
2. Press Down arrow twice (select Color field)
3. Press Right arrow to cycle colors
4. Press Button A to apply

# Save your work
Press F2 (or Button START)

# Quit
Press Q or ESC
```

### 5-Minute Tutorial

**Step 1: Explore** (1 minute)
- Pan around with WASD
- Zoom with +/-
- Read the welcome box
- Press BACK to see visualizer (if joystick)

**Step 2: Create** (1 minute)
- Press N to create 3 boxes
- Notice they appear at viewport center
- Pan between them

**Step 3: Select** (1 minute)
- Press Tab to cycle boxes
- Watch status bar show selected box
- Try mouse click to select
- Try joystick Button A in SELECT mode

**Step 4: Edit Properties** (1 minute)
- Select a box
- Press Button Y (or enter EDIT mode first)
- Open parameter editor
- Change width to 30
- Change color to Blue
- Press A to apply

**Step 5: Edit Text** (1 minute)
- In EDIT mode, press Button A
- Type a new title
- Press ESC to save
- Admire your edited box!

---

## Troubleshooting

### Terminal Display Issues
```bash
# If boxes don't render correctly:
echo $TERM  # Should be xterm-256color or similar

# Try different terminal
alacritty  # Recommended
kitty      # Also good
gnome-terminal  # Usually works
```

### Joystick Not Detected
```bash
# Check joystick device
ls -l /dev/input/event*

# Test with evtest (if available)
sudo evtest /dev/input/event0

# May need permissions
sudo chmod a+r /dev/input/event0
```

### Canvas Races Away on Startup
- **Fixed in Phase 1** with settling period
- Joystick now ignores axis for first 30 frames (~0.5 seconds)
- Allows device to stabilize before accepting input

### Text Editor Not Working
- Text editing **requires keyboard**
- Joystick can open/close, but keyboard needed for typing
- Future: On-screen keyboard for joystick-only users

---

## Command Reference Card

### Global Controls (Any Mode)
| Action | Keyboard | Joystick | Mouse |
|--------|----------|----------|-------|
| Quit | Q or ESC | Button SELECT | - |
| Save | F2 | Button START | - |
| Load | F3 | - | - |
| Toggle Visualizer | - | Button BACK | - |
| Cycle Mode | Auto | Button MENU | - |

### VIEW Mode (Safe Exploration)
| Action | Keyboard | Joystick | Mouse |
|--------|----------|----------|-------|
| Pan | WASD/Arrows | Left Stick | - |
| Zoom In | + or Z | Button A | Wheel Up |
| Zoom Out | - or X | Button B | Wheel Down |
| Create Box | N | Button X | Ctrl+Click |
| Enter SELECT | Tab | Button Y | Click box |
| Reset View | R | Button LB | - |

### SELECT Mode (Choose Boxes)
| Action | Keyboard | Joystick | Mouse |
|--------|----------|----------|-------|
| Cycle Boxes | Tab | Button A | Click box |
| Deselect | Click empty | Button B | Click empty |
| Enter EDIT | Auto | Button X | Double-click |
| Delete Box | D | Button Y | - |
| Move Cursor | - | Left Stick | Move mouse |

### EDIT Mode (Modify Box)
| Action | Keyboard | Joystick | Mouse |
|--------|----------|----------|-------|
| Edit Text | Type | Button A | - |
| Parameters | - | Button Y | - |
| Cycle Color | 1-7 keys | Button X | - |
| Move Box | - | Left Stick | Drag |
| Return to SELECT | - | Button B | - |

### Parameter Editor
| Action | Keyboard | Joystick |
|--------|----------|----------|
| Navigate Fields | Up/Down | Stick Up/Down |
| Adjust Value | Left/Right | Stick Left/Right |
| Large Step | - | LB/RB |
| Apply | - | Button A |
| Cancel | - | Button B |

### Text Editor
| Action | Keyboard | Joystick |
|--------|----------|----------|
| Type | Any key | (not supported) |
| Move Cursor | Arrows | (not supported) |
| Delete | Backspace | (not supported) |
| Jump to Start | Home | (not supported) |
| Jump to End | End | (not supported) |
| Save & Close | ESC | Button B |

---

## Promotional Taglines

- "Boxes-Live: Your terminal, your canvas"
- "Visual thinking meets the command line"
- "Build diagrams without leaving your terminal"
- "Keyboard, mouse, AND joystick - your choice"
- "Real-time collaboration with your terminal"
- "ncurses-powered interactive workspace"
- "Like Miro, but make it terminal"

---

## Next Steps After Demo

### For Users
1. Load example canvases: `./boxes-live demos/live_monitor.txt`
2. Create your own diagrams
3. Save and share canvas files
4. Provide feedback on GitHub

### For Developers
1. Phase 4: Grid system (coming next)
2. Phase 5: Box connections
3. Configuration system
4. Plugin architecture
5. Network collaboration?

---

## Recording Tips

### Screen Recording
```bash
# Use asciinema for terminal recording
asciinema rec demo.cast

# Or use OBS Studio with terminal window
# Or use SimpleScreenRecorder

# Convert to GIF
agg demo.cast demo.gif
```

### Terminal Settings for Recording
```bash
# Use a good font
# Recommended: FiraCode, JetBrains Mono, Cascadia Code

# Set terminal size
resize -s 40 120

# Use good color scheme
# Recommend: Gruvbox, Nord, Solarized Dark
```

### What Makes a Good Demo
- Smooth panning (don't jerk around)
- Deliberate actions (not too fast)
- Show mistakes and corrections (relatable)
- Feature focus (one thing at a time)
- Clear narration (explain what you're doing)
- Visual indicators (cursor, highlights, etc.)

---

## Feedback Collection

After demo/testing, collect:
- What was intuitive?
- What was confusing?
- Which modes felt natural?
- Which button mappings worked well?
- What should be configurable?
- Missing features?
- Bug reports?

Use this to inform Phase 4+ development and configuration system design.
