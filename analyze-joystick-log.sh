#!/bin/bash
# Analyze joystick debug log to identify issues
#
# Usage: ./analyze-joystick-log.sh [log-file]
#
# If no log file specified, uses /tmp/joystick_debug.log

LOG_FILE="${1:-/tmp/joystick_debug.log}"

if [ ! -f "$LOG_FILE" ]; then
    echo "ERROR: Log file not found: $LOG_FILE"
    echo ""
    echo "Run ./test-joystick.sh first to generate the debug log."
    exit 1
fi

echo "========================================"
echo "Joystick Debug Log Analysis"
echo "========================================"
echo "Log file: $LOG_FILE"
echo ""

# Basic stats
echo "--- Statistics ---"
echo "Total lines: $(wc -l < "$LOG_FILE")"
echo "EV_ABS events: $(grep -c "EV_ABS:" "$LOG_FILE")"
echo "EV_KEY events: $(grep -c "EV_KEY" "$LOG_FILE")"
echo "EV_SYN events: $(grep -c "EV_SYN" "$LOG_FILE")"
echo ""

# Check if joystick opened successfully
echo "--- Initialization ---"
grep -E "(Joystick opened|Failed to open)" "$LOG_FILE" | head -5
echo ""

# Show unique axis codes detected
echo "--- Axis Codes Detected ---"
grep "EV_ABS:" "$LOG_FILE" | sed 's/.*code=\([0-9]*\).*/\1/' | sort -n | uniq -c
echo ""

# Show unique button codes detected
echo "--- Button Codes Detected ---"
grep "EV_KEY" "$LOG_FILE" | sed 's/.*code=\([0-9]*\).*/\1/' | sort -n | uniq -c
echo ""

# Show button mappings
echo "--- Button Mappings ---"
grep "-> button=" "$LOG_FILE" | sed 's/.*-> button=\([0-9]*\).*/button \1/' | sort | uniq -c
echo ""

# Show sample axis values
echo "--- Sample Axis Values (first 10) ---"
grep "AXIS_X\|AXIS_Y" "$LOG_FILE" | head -10
echo ""

# Look for unknown events
UNKNOWN_COUNT=$(grep -c "Unknown ABS axis\|UNKNOWN" "$LOG_FILE")
if [ "$UNKNOWN_COUNT" -gt 0 ]; then
    echo "--- WARNING: Unknown Events Found ($UNKNOWN_COUNT) ---"
    grep "Unknown ABS axis\|UNKNOWN" "$LOG_FILE" | head -10
    echo ""
fi

# Show axis ranges
echo "--- Axis Value Ranges ---"
echo "X-axis values:"
grep "AXIS_X =" "$LOG_FILE" | sed 's/.*AXIS_X = \([0-9-]*\).*/\1/' | sort -n | head -1 | xargs -I {} echo "  Min: {}"
grep "AXIS_X =" "$LOG_FILE" | sed 's/.*AXIS_X = \([0-9-]*\).*/\1/' | sort -n | tail -1 | xargs -I {} echo "  Max: {}"
echo ""
echo "Y-axis values:"
grep "AXIS_Y =" "$LOG_FILE" | sed 's/.*AXIS_Y = \([0-9-]*\).*/\1/' | sort -n | head -1 | xargs -I {} echo "  Min: {}"
grep "AXIS_Y =" "$LOG_FILE" | sed 's/.*AXIS_Y = \([0-9-]*\).*/\1/' | sort -n | tail -1 | xargs -I {} echo "  Max: {}"
echo ""

echo "========================================"
echo "Analysis complete"
echo "========================================"
echo ""
echo "For detailed event codes, see:"
echo "  https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h"
echo ""
echo "Expected values:"
echo "  ABS_X = 0, ABS_Y = 1 (for left stick)"
echo "  ABS_RX = 3, ABS_RY = 4 (for right stick)"
echo "  ABS_Z = 2, ABS_RZ = 5 (for triggers)"
echo "  BTN_GAMEPAD (0x130) = Button A (button 0)"
echo "  BTN_JOYSTICK (0x120) starts at button 0"
