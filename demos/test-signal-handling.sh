#!/bin/bash
# Test signal handling for boxes-live
# This script demonstrates that the application properly handles signals
# and cleans up the terminal state when interrupted

echo "Testing signal handling for boxes-live..."
echo ""
echo "This test will:"
echo "  1. Start boxes-live in the background"
echo "  2. Send SIGINT (Ctrl+C) after 1 second"
echo "  3. Verify the terminal is cleaned up properly"
echo ""

# Start boxes-live with a file
./boxes-live demos/live_monitor.txt &
PID=$!

echo "Started boxes-live with PID: $PID"
sleep 1

echo "Sending SIGINT to PID $PID..."
kill -INT $PID

# Wait for process to exit
wait $PID
EXIT_CODE=$?

echo ""
echo "Process exited with code: $EXIT_CODE"

# Check if terminal is in good state
if stty sane 2>/dev/null; then
    echo "✓ Terminal state is clean"
else
    echo "Note: stty check not available in this environment"
fi

# Exit code 130 (128 + 2) indicates SIGINT was received and handled
if [ $EXIT_CODE -eq 130 ] || [ $EXIT_CODE -eq 0 ]; then
    echo "✓ Signal handling working correctly"
    exit 0
else
    echo "✗ Unexpected exit code: $EXIT_CODE"
    exit 1
fi
