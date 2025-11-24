# Boxes-Live Usage Guide

## Building

```bash
make          # Build the application
make clean    # Clean build artifacts
make run      # Build and run the application
```

## Running

```bash
./boxes-live
```

## Controls

The application supports **keyboard**, **mouse**, and **joystick** input through a unified input layer that provides 1:1 intent mapping across all devices.

### Keyboard

#### Navigation
- **Arrow Keys** or **W/S**: Pan up/down
- **A/H**: Pan left
- **L**: Pan right
- **+** or **=** or **Z**: Zoom in
- **-** or **_** or **X**: Zoom out
- **R** or **0**: Reset viewport to origin with 1.0x zoom

#### Box Management
- **N**: Create new box at center of viewport
- **D**: Delete currently selected box
- **Tab**: Cycle through boxes (select next)
- **1-7**: Apply color to selected box (red, green, blue, yellow, magenta, cyan, white)
- **0**: Reset selected box color to default (or reset view if no selection)

#### File Operations
- **F2**: Save canvas to file (`canvas.txt`)
- **F3**: Load canvas from file (`canvas.txt`)

#### Exit
- **Q** or **ESC**: Quit the application

### Mouse

- **Left Click**: Select box at cursor position
- **Left Click Drag**: Move selected box
- **Scroll Wheel Up**: Zoom in (if supported)
- **Scroll Wheel Down**: Zoom out (if supported)

### Joystick (Gamepad)

**Navigation Mode** (Default):
- **Left Stick**: Pan viewport (smooth analog control)
- **Button A**: Zoom in
- **Button B**: Zoom out
- **Button X**: Cycle to next box (select/focus)
- **Button Y**: Create new box at cursor
- **Start**: Save canvas
- **Select**: Load canvas

**Edit Mode** (When box selected):
- **Left Stick**: Move selected box
- **Button A**: Enter parameter mode
- **Button B**: Return to navigation
- **Button X**: Cycle box color
- **Button Y**: Delete box

See [JOYSTICK-GUIDE.md](JOYSTICK-GUIDE.md) for complete joystick documentation.

## Features

- **Unified Input**: Consistent user experience across keyboard, mouse, and joystick
- **Pan and Zoom**: Navigate through a large 2D canvas
- **Dynamic Canvas**: Unlimited boxes (grows automatically)
- **Interactive Box Management**: Create, delete, and select boxes
- **Mouse Support**: Click to select boxes, drag to move them
- **Joystick Support**: Full gamepad control with three input modes
- **Box Selection**: Visual indication of selected box (highlighted border)
- **Save/Load**: Persist your canvas to disk and reload later
- **Box Rendering**: Displays boxes with titles and content
- **Coordinate Transformation**: Smooth world-to-screen coordinate mapping
- **Status Bar**: Shows current camera position, zoom level, box count, and selected box
- **Responsive**: Handles terminal resizing

## Canvas Layout

The default canvas contains several sample boxes demonstrating:
- Welcome/instructions box
- Project information
- Feature list
- Technical details
- Future enhancement ideas
- Credits

Explore the canvas by panning around with arrow keys and try zooming in/out!

## Requirements

- ncurses library (`libncurses-dev` on Debian/Ubuntu)
- GCC compiler
- Terminal with box-drawing character support

## Using Save/Load

### Saving Your Canvas
1. Press **F2** to save the current canvas state to `canvas.txt`
2. All boxes, their positions, content, and selection state are saved
3. The file is saved in the current working directory

### Loading a Canvas
1. Press **F3** to load a previously saved canvas from `canvas.txt`
2. The current canvas will be replaced with the loaded one
3. If the file doesn't exist or is invalid, the current canvas remains unchanged

### Canvas File Format
The canvas is saved in a human-readable text format that includes:
- World dimensions
- Box count
- For each box: ID, position, size, title, content, and selection state

## Tips

1. Start by reading the Welcome box at the origin (5, 5)
2. Pan around to discover other boxes
3. Try zooming in to see box details more clearly
4. Try zooming out to see more of the canvas at once
5. Use the status bar to track your current position
6. Click boxes with the mouse to select them
7. Press **N** to create new boxes at the center of your viewport
8. Use **Tab** to cycle through boxes if you can't find them
9. Save your work with **F2** before quitting
10. Deleted boxes cannot be recovered unless you reload from a saved file
