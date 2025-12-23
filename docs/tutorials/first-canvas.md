# Tutorial: Create Your First Canvas

Learn how to create, navigate, and save a canvas in boxes-live.

## Prerequisites

- boxes-live built and ready (`make`)
- Terminal with box-drawing character support

## Step 1: Launch boxes-live

```bash
./boxes-live
```

You'll see an empty canvas with a status bar at the bottom showing your position and controls.

## Step 2: Create a Box

1. Press **N** to create a new box
2. The box appears at your cursor position
3. It's automatically selected (highlighted border)

## Step 3: Navigate

| Key | Action |
|-----|--------|
| Arrow Keys | Pan the viewport |
| + or Z | Zoom in |
| - or X | Zoom out |
| R | Reset view to origin |

Try zooming out to see more of the canvas, then zoom back in.

## Step 4: Create More Boxes

1. Pan to a new location
2. Press **N** to create another box
3. Repeat to create 3-4 boxes

## Step 5: Select and Move Boxes

1. **Click** on a box to select it
2. **Drag** to move it to a new position
3. The selected box has a highlighted border

## Step 6: Color Your Boxes

With a box selected:
- Press **1** through **7** to apply colors
- Press **0** to reset to default white

| Key | Color |
|-----|-------|
| 1 | Red |
| 2 | Green |
| 3 | Yellow |
| 4 | Blue |
| 5 | Magenta |
| 6 | Cyan |
| 7 | White |

## Step 7: Change Box Types

With a box selected, press **T** to cycle through types:
- NOTE (default)
- TASK
- CODE
- STICKY

Each type has a different icon in the title bar.

## Step 8: Save Your Canvas

1. Press **F2** to save
2. Enter a filename (e.g., `my-first-canvas.txt`)
3. Press Enter to confirm

## Step 9: Load a Canvas

1. Press **F3** to load
2. Enter the filename
3. Your canvas is restored

## Step 10: Exit

Press **Q** or **ESC** to quit.

## Next Steps

- Explore [Connector Guide](../CONNECTOR-GUIDE.md) to import data
- Try [Workflow Examples](../WORKFLOW-EXAMPLES.md) for real-world use cases
- Read [USAGE.md](../../USAGE.md) for complete controls

## Troubleshooting

**Box-drawing characters look wrong?**
Your terminal may not support Unicode. Try a modern terminal like kitty, alacritty, or gnome-terminal.

**Mouse not working?**
Ensure your terminal has mouse support enabled. Most modern terminals do by default.

**Colors not showing?**
Your terminal must support 256 colors. Try `echo $TERM` - it should show something like `xterm-256color`.
