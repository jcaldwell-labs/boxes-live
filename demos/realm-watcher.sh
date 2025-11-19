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

# Find boxes-live root directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOXES_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

REALM_FILE="${1:-realm.json}"
CANVAS_FILE="${2:-realm_canvas.txt}"
INTERVAL="${3:-2}"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Cross-platform absolute path resolution
abspath() {
    if command -v readlink >/dev/null 2>&1 && readlink -f / >/dev/null 2>&1; then
        # GNU readlink (Linux)
        readlink -f "$1"
    elif command -v greadlink >/dev/null 2>&1 && greadlink -f / >/dev/null 2>&1; then
        # GNU readlink via coreutils on macOS
        greadlink -f "$1"
    elif command -v realpath >/dev/null 2>&1; then
        # realpath command (available on most systems)
        realpath "$1"
    else
        # Fallback: just prefix with $PWD
        case "$1" in
            /*) echo "$1" ;;
            *) echo "$(pwd)/$1" ;;
        esac
    fi
}

# Validate inputs
if [ ! -f "$REALM_FILE" ]; then
    echo -e "${RED}✗ Error: Realm file '$REALM_FILE' not found${NC}" >&2
    exit 1
fi

# Get absolute paths
REALM_FILE=$(abspath "$REALM_FILE")
CANVAS_FILE=$(abspath "$CANVAS_FILE")

echo -e "${GREEN}🔍 Realm Watcher Started${NC}"
echo -e "${BLUE}Watching: ${REALM_FILE}${NC}"
echo -e "${BLUE}Canvas:   ${CANVAS_FILE}${NC}"
echo -e "${BLUE}Interval: ${INTERVAL}s${NC}"
echo ""
echo -e "${YELLOW}Monitoring for changes... (Ctrl+C to stop)${NC}"
echo ""

# Cross-platform file modification time retrieval
get_mtime() {
    if stat -c %Y "$1" 2>/dev/null; then
        # GNU stat (Linux)
        return
    elif stat -f %m "$1" 2>/dev/null; then
        # BSD stat (macOS)
        return
    else
        # Fallback: use ls (less reliable)
        echo 0
    fi
}

# Store last modification time
LAST_MTIME=$(get_mtime "$REALM_FILE")

# Initial sync
echo -e "${BLUE}[$(date '+%H:%M:%S')]${NC} Initial sync..."
"$BOXES_ROOT/connectors/realm2canvas" "$REALM_FILE" "$CANVAS_FILE"

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
    CURRENT_MTIME=$(get_mtime "$REALM_FILE")

    if [ "$CURRENT_MTIME" -gt "$LAST_MTIME" ]; then
        echo -e "${BLUE}[$(date '+%H:%M:%S')]${NC} 🔄 Realm file changed, regenerating canvas..."

        # Regenerate canvas
        if "$BOXES_ROOT/connectors/realm2canvas" "$REALM_FILE" "$CANVAS_FILE" 2>&1 | grep -q "✓"; then
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
