#!/usr/bin/env bash
# realm-watcher.sh - Watch realm JSON file and auto-sync to canvas
#
# Usage: realm-watcher.sh <realm-json-file> <canvas-file> [interval-seconds]
#
# This script monitors a realm JSON file for changes and automatically:
# 1. Regenerates the canvas using realm2canvas
# 2. Signals boxes-live to reload (SIGUSR1)
# 3. Provides visual feedback in terminal

set -e

REALM_FILE="${1:-realm.json}"
CANVAS_FILE="${2:-realm_canvas.txt}"
INTERVAL="${3:-2}"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Validate inputs
if [ ! -f "$REALM_FILE" ]; then
    echo -e "${RED}✗ Error: Realm file '$REALM_FILE' not found${NC}" >&2
    exit 1
fi

# Get absolute paths
REALM_FILE=$(readlink -f "$REALM_FILE")
CANVAS_FILE=$(readlink -f "$CANVAS_FILE" 2>/dev/null || echo "$(pwd)/$CANVAS_FILE")

echo -e "${GREEN}🔍 Realm Watcher Started${NC}"
echo -e "${BLUE}Watching: ${REALM_FILE}${NC}"
echo -e "${BLUE}Canvas:   ${CANVAS_FILE}${NC}"
echo -e "${BLUE}Interval: ${INTERVAL}s${NC}"
echo ""
echo -e "${YELLOW}Monitoring for changes... (Ctrl+C to stop)${NC}"
echo ""

# Store last modification time
LAST_MTIME=$(stat -c %Y "$REALM_FILE" 2>/dev/null || echo 0)

# Initial sync
echo -e "${BLUE}[$(date '+%H:%M:%S')]${NC} Initial sync..."
./connectors/realm2canvas "$REALM_FILE" "$CANVAS_FILE"

# Find boxes-live PID
sync_boxes_live() {
    # Find all boxes-live processes viewing this canvas
    PIDS=$(pgrep -f "boxes-live.*$(basename "$CANVAS_FILE")" || true)

    if [ -n "$PIDS" ]; then
        for PID in $PIDS; do
            kill -SIGUSR1 "$PID" 2>/dev/null && \
                echo -e "${GREEN}[$(date '+%H:%M:%S')]${NC} ✓ Synced to boxes-live (PID: $PID)"
        done
    else
        echo -e "${YELLOW}[$(date '+%H:%M:%S')]${NC} ⚠ No boxes-live process found (canvas updated)"
    fi
}

sync_boxes_live

SYNC_COUNT=1

# Main watch loop
while true; do
    sleep "$INTERVAL"

    # Check if file was modified
    CURRENT_MTIME=$(stat -c %Y "$REALM_FILE" 2>/dev/null || echo 0)

    if [ "$CURRENT_MTIME" -gt "$LAST_MTIME" ]; then
        echo -e "${BLUE}[$(date '+%H:%M:%S')]${NC} 🔄 Realm file changed, regenerating canvas..."

        # Regenerate canvas
        if ./connectors/realm2canvas "$REALM_FILE" "$CANVAS_FILE" 2>&1 | grep -q "✓"; then
            LAST_MTIME=$CURRENT_MTIME
            SYNC_COUNT=$((SYNC_COUNT + 1))

            # Sync to boxes-live
            sync_boxes_live

            # Show stats
            BOX_COUNT=$(grep -c "^BOX:" "$CANVAS_FILE" || echo 0)
            echo -e "${GREEN}[$(date '+%H:%M:%S')]${NC} 📊 Canvas: ${BOX_COUNT} boxes | Sync #${SYNC_COUNT}"
        else
            echo -e "${RED}[$(date '+%H:%M:%S')]${NC} ✗ Failed to regenerate canvas"
        fi
    fi
done
