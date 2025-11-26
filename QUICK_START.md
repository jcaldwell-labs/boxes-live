# Boxes-Live Quick Start Guide

**Version**: 1.2  
**For**: First-time users and testers  
**Time**: 5 minutes to productivity

---

## 30-Second Workflow

```bash
# Build and run
make && ./boxes-live

# Create boxes: Press 'n'
# Move view: Arrow keys or WASD
# Zoom: +/- keys
# Save: F2
# Quit: Q or ESC
```

That's it! You're ready to create your first canvas.

---

## 5-Minute Tutorial

### Step 1: Launch Boxes-Live (30 seconds)

```bash
cd /path/to/boxes-live
make clean && make
./boxes-live
```

**You should see**: A welcome box in the center with instructions.

### Step 2: Navigate the Canvas (1 minute)

**Keyboard Navigation**:
- **Arrow Keys** or **WASD**: Pan (move the view)
- **+** or **-**: Zoom in/out
- **R**: Reset view to origin (0,0)

**Mouse Navigation** (if available):
- Click a box to select it
- Drag a box to move it

**Try it**: Pan around, zoom in, zoom out, then press **R** to reset.

### Step 3: Create Boxes (1 minute)

**Press 'n'** (lowercase) to create a new box at the current cursor position.

**Try it**: Create 3 boxes in different positions:
1. Press **n** → creates first box
2. Move with arrow keys
3. Press **n** → creates second box
4. Move again
5. Press **n** → creates third box

**Keyboard shortcuts for templates**:
- **n**: Square box (20x10) - good for notes
- **Shift+N**: Horizontal box (40x10) - good for headers
- **Ctrl+N**: Vertical box (20x20) - good for lists

### Step 4: Select and Color Boxes (1 minute)

**Select a box**:
- **Mouse**: Click on it
- **Keyboard**: Press **Tab** to cycle through boxes

**Change color** (1-7):
- **1**: Red
- **2**: Green
- **3**: Blue
- **4**: Yellow
- **5**: Magenta
- **6**: Cyan
- **7**: White
- **0**: Default (no color)

**Try it**: Select each box and give it a different color.

### Step 5: Save Your Work (30 seconds)

**Press F2** to save your canvas.

Default filename: `canvas.txt`

**Try it**: Press **F2** → Your canvas is saved!

### Step 6: Advanced Features (1 minute)

**Grid System**:
- **G**: Toggle grid visibility
- **Shift+G**: Toggle snap-to-grid

**Focus Mode** (read box content full-screen):
- **F**: Enter focus mode for selected box
- **ESC**: Exit focus mode
- **j/k** or **↑/↓**: Scroll content

**Delete a box**:
- Select it (click or Tab)
- Press **D** (uppercase)

**Load a saved canvas**:
- Press **F3**
- Loads the last saved file

---

## Common Tasks Reference

| Task | Keyboard | Mouse/Joystick |
|------|----------|----------------|
| Create box | `n` | Button Y (joystick) |
| Select box | `Tab` | Click box |
| Move box | Select + Arrow keys | Drag box |
| Color box | Select + `1-7` | - |
| Delete box | Select + `D` | - |
| Pan view | Arrow keys / WASD | - |
| Zoom in/out | `+` / `-` | Scroll wheel |
| Reset view | `R` | - |
| Save canvas | `F2` | Button Start |
| Load canvas | `F3` | - |
| Toggle grid | `G` | Button Y (NAV mode) |
| Focus mode | `F` | - |
| Quit | `Q` or `ESC` | Button Select |

---

## Joystick Mode (Optional)

If you have a gamepad/joystick connected:

**NAV Mode** (default):
- **Left Stick**: Pan viewport
- **Button A**: Zoom in
- **Button B**: Zoom out
- **Button X**: Create box
- **Button Y**: Toggle grid
- **Button LB**: Cycle modes (NAV → SELECTION → EDIT)

**SELECTION Mode**:
- **Button A**: Cycle to next box
- **Button B**: Back to NAV
- **Button X**: Enter EDIT mode
- **Button Y**: Delete selected box

**EDIT Mode**:
- **Button A**: Edit text
- **Button B**: Back to SELECTION
- **Button X**: Cycle color
- **Button Y**: Open parameter editor

See [JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md) for complete joystick documentation.

---

## Troubleshooting

### Application won't start
**Problem**: `Error: Terminal type 'dumb' does not support cursor movement`  
**Solution**: Use a modern terminal (xterm, gnome-terminal, alacritty, kitty)

### Boxes look weird
**Problem**: Box-drawing characters display as `???` or `+--+`  
**Solution**: Your terminal doesn't support Unicode. Use a UTF-8 compatible terminal.

### Grid not visible
**Problem**: Pressed `G` but no grid appears  
**Solution**: 
1. Grid color is dim gray - may be hard to see on some backgrounds
2. Try zooming in (grid becomes more visible)
3. Check grid is enabled: Status bar should show `[GRID:10]`

### Can't save file
**Problem**: `Failed to save canvas`  
**Solution**: Check write permissions in current directory

### Joystick not working
**Problem**: No joystick visualizer appears  
**Solution**: 
1. Check joystick is connected: `ls /dev/input/js*`
2. Check permissions: `sudo chmod a+r /dev/input/js0`
3. See [JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md) for troubleshooting

---

## Example Workflows

### Creating a Simple Flowchart

1. Create boxes for each step: Press **n** (5 times)
2. Position them vertically: Use arrow keys
3. Color by priority:
   - Red: Critical steps
   - Green: Completed
   - Yellow: In progress
4. Add text content (future: text editing)
5. Save: **F2**

### Organizing a Project Board

1. Create columns:
   - **Shift+N** → "TODO" (horizontal box)
   - Move right → **Shift+N** → "In Progress"
   - Move right → **Shift+N** → "Done"
2. Create task boxes: **n** (multiple times)
3. Arrange under columns
4. Color by priority
5. Enable grid for alignment: **G**
6. Enable snap: **Shift+G**
7. Save: **F2**

### Reading a Log File (Future Feature)

1. Create a box: **n**
2. Edit text: Add `:file /var/log/syslog`
3. Enter focus mode: **F**
4. Scroll with **j/k**
5. Exit: **ESC**

---

## Next Steps

Once you're comfortable with basics:

1. **Read full documentation**:
   - [README.md](README.md) - Complete feature list
   - [ARCHITECTURE.md](ARCHITECTURE.md) - System design
   - [JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md) - Gamepad controls

2. **Try example canvases**:
   ```bash
   ./boxes-live
   # Press F3, load: test-canvases/flowchart-example.txt
   ```

3. **Customize your setup**:
   - Copy `config.ini.example` to `~/.config/boxes-live/config.ini`
   - Edit button mappings, grid settings, templates

4. **Explore connectors**:
   ```bash
   # Visualize processes
   pstree -p | ./connectors/pstree2canvas > processes.txt
   ./boxes-live  # F3 to load
   ```

---

## Getting Help

- **Documentation**: See `docs/` directory
- **Issues**: https://github.com/jcaldwell-labs/boxes-live/issues
- **Examples**: See `examples/` and `test-canvases/`
- **Testing Guide**: [TESTING.md](TESTING.md)

---

## Tips for Productivity

1. **Use grid + snap** for aligned layouts (G + Shift+G)
2. **Use templates** for consistent sizing (n vs Shift+N vs Ctrl+N)
3. **Color code** your boxes for quick visual scanning
4. **Save often** (F2) - no auto-save yet
5. **Focus mode** (F) for reading long content
6. **Tab cycling** for quick box selection

---

**Ready to start?** Run `./boxes-live` and press `n` to create your first box!

For testing scenarios, see [UAT_TESTING_GUIDE.md](UAT_TESTING_GUIDE.md).
