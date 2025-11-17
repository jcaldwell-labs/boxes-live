#!/bin/bash
# Auto-Reload Demo - Simulates plugin-like behavior with inotify
#
# This watches canvas.txt and automatically tells boxes-live to reload
# when changes are detected (simulates live plugin updates)

set -e

CANVAS="${1:-demo_canvas.txt}"

if ! command -v inotifywait &> /dev/null; then
    echo "Error: inotifywait not found"
    echo "Install: sudo apt-get install inotify-tools"
    exit 1
fi

echo "========================================="
echo "Auto-Reload Demo"
echo "========================================="
echo ""
echo "This simulates plugin-like live updates by:"
echo "1. Watching $CANVAS for changes"
echo "2. Automatically signaling boxes-live to reload"
echo ""
echo "Setup:"
echo "  1. Run boxes-live in another terminal"
echo "  2. Press F3 to load $CANVAS"
echo "  3. This script will auto-reload on changes"
echo ""
echo "Try: ./demos/live-monitor.sh"
echo "     (in another terminal to generate updates)"
echo ""
echo "Watching for changes to $CANVAS..."
echo "Press Ctrl+C to stop"
echo ""

# Watch for file modifications
inotifywait -m -e modify,close_write "$CANVAS" | while read -r directory event filename; do
    timestamp=$(date "+%H:%M:%S")
    echo "[$timestamp] Detected change: $event"
    echo "  → Canvas updated, reload with F3 in boxes-live"

    # Future: Could send signal to boxes-live process to auto-reload
    # For now, user manually presses F3

    # Could also play a sound or show notification
    # command -v paplay &>/dev/null && paplay /usr/share/sounds/freedesktop/stereo/message.oga &
done
