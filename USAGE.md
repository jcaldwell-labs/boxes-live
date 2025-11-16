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

### Navigation
- **Arrow Keys** or **WASD**: Pan the viewport around the canvas
- **+** or **=** or **Z**: Zoom in
- **-** or **_** or **X**: Zoom out
- **R** or **0**: Reset viewport to origin with 1.0x zoom

### Exit
- **Q** or **ESC**: Quit the application

## Features

- **Pan and Zoom**: Navigate through a large 2D canvas
- **Box Rendering**: Displays boxes with titles and content
- **Coordinate Transformation**: Smooth world-to-screen coordinate mapping
- **Status Bar**: Shows current camera position and zoom level
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

## Tips

1. Start by reading the Welcome box at the origin (5, 5)
2. Pan around to discover other boxes
3. Try zooming in to see box details more clearly
4. Try zooming out to see more of the canvas at once
5. Use the status bar to track your current position
