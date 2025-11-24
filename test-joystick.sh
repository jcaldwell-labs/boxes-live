#!/bin/bash
# Joystick Debug Testing Script
#
# This script runs boxes-live with the joystick test canvas and captures debug output.
# Run this in a SEPARATE terminal to avoid corrupting the Claude Code session.
#
# Usage:
#   ./test-joystick.sh
#
# The debug log will be written to: /tmp/joystick_debug.log
# After testing, view the log with: cat /tmp/joystick_debug.log

# Clear any old debug log
rm -f /tmp/joystick_debug.log

echo "========================================"
echo "Joystick Debug Test"
echo "========================================"
echo ""
echo "Starting boxes-live with joystick test canvas..."
echo "Debug log will be written to: /tmp/joystick_debug.log"
echo ""
echo "INSTRUCTIONS:"
echo "1. Test navigation mode (left stick should pan)"
echo "2. Test zoom (A/B buttons)"
echo "3. Try creating a box (Y button)"
echo "4. Try cycling boxes (X button)"
echo "5. If you select a box, try moving it (left stick in edit mode)"
echo "6. Press Q to quit when done"
echo ""
echo "Press Enter to start..."
read

# Run boxes-live with default sample canvas (no file loading to avoid crash)
./boxes-live

echo ""
echo "========================================"
echo "Test complete!"
echo "========================================"
echo ""

# Show summary of debug log
if [ -f /tmp/joystick_debug.log ]; then
    echo "Debug log captured. Summary:"
    echo ""
    echo "Total events logged:"
    wc -l /tmp/joystick_debug.log
    echo ""
    echo "First 20 lines:"
    head -20 /tmp/joystick_debug.log
    echo ""
    echo "View full log with: cat /tmp/joystick_debug.log"
else
    echo "WARNING: No debug log found at /tmp/joystick_debug.log"
    echo "This could mean:"
    echo "  - Joystick was not detected"
    echo "  - Permissions issue"
    echo "  - DEBUG flag not compiled in"
fi
