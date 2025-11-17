#!/bin/bash
# Live Monitor Demo - Shows real-time updates using file-based IPC
#
# This demonstrates boxes-live's capability for live data visualization
# using the existing save/load mechanism + CLI tools

set -e

# Get script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$PROJECT_ROOT/connectors/boxes-cli"

CANVAS="$SCRIPT_DIR/live_monitor.txt"
INTERVAL=2  # Update interval in seconds

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "========================================="
echo "boxes-live Live Monitor Demo"
echo "========================================="
echo ""
echo "This demo shows real-time system monitoring"
echo "using boxes-live's save/load + CLI tools"
echo ""
echo "Canvas file: $CANVAS"
echo "Update interval: ${INTERVAL}s"
echo ""
echo -e "${GREEN}Starting in 3 seconds...${NC}"
sleep 3

# Create initial canvas
echo -e "${BLUE}Creating canvas...${NC}"
$CLI create --width 200 --height 100 "$CANVAS"

# Function to update a box with new content
update_box() {
    local box_id=$1
    shift
    local content=("$@")

    # Build content args
    local content_args=""
    for line in "${content[@]}"; do
        content_args="$content_args --content \"$line\""
    done

    # Update the box
    eval $CLI update "$CANVAS" "$box_id" $content_args 2>/dev/null || true
}

# Create initial boxes
echo -e "${BLUE}Creating monitor boxes...${NC}"

# Box 1: System Info (ID 1)
$CLI add "$CANVAS" \
    --x 10 --y 5 \
    --width 35 --height 12 \
    --title "System Monitor" \
    --color 2 \
    --content "Initializing..." \
    --content "" \
    --content "Updates every ${INTERVAL}s"

# Box 2: CPU/Memory (ID 2)
$CLI add "$CANVAS" \
    --x 55 --y 5 \
    --width 35 --height 12 \
    --title "Resources" \
    --color 3 \
    --content "Loading..."

# Box 3: Disk Usage (ID 3)
$CLI add "$CANVAS" \
    --x 10 --y 22 \
    --width 35 --height 10 \
    --title "Disk Usage" \
    --color 4 \
    --content "Loading..."

# Box 4: Network (ID 4)
$CLI add "$CANVAS" \
    --x 55 --y 22 \
    --width 35 --height 10 \
    --title "Network" \
    --color 6 \
    --content "Loading..."

# Box 5: Processes (ID 5)
$CLI add "$CANVAS" \
    --x 100 --y 5 \
    --width 40 --height 20 \
    --title "Top Processes" \
    --color 5 \
    --content "Loading..."

# Box 6: Instructions (ID 6)
$CLI add "$CANVAS" \
    --x 10 --y 37 \
    --width 80 --height 8 \
    --title "Live Monitor Demo" \
    --color 1 \
    --content "This canvas updates every ${INTERVAL}s" \
    --content "Open in boxes-live and press F3 to reload" \
    --content "" \
    --content "Press Ctrl+C in this terminal to stop updates"

echo -e "${GREEN}✓ Canvas created${NC}"
echo ""
echo -e "${BLUE}Starting live updates...${NC}"
echo -e "${GREEN}Open the canvas in boxes-live:${NC}"
echo "  $PROJECT_ROOT/boxes-live"
echo "  Press F3 to load: $CANVAS"
echo ""
echo "Then watch as it updates every ${INTERVAL}s!"
echo "Press Ctrl+C here to stop"
echo ""

# Update loop
update_count=0
while true; do
    update_count=$((update_count + 1))
    timestamp=$(date "+%H:%M:%S")

    echo -e "${BLUE}[$timestamp] Update #$update_count${NC}"

    # Update Box 1: System Info
    uptime_info=$(uptime | sed 's/^.*up //' | sed 's/, *[0-9]* user.*//')
    hostname_info=$(hostname)
    update_box 1 \
        "Host: $hostname_info" \
        "Uptime: $uptime_info" \
        "Time: $timestamp" \
        "Updates: $update_count"

    # Update Box 2: CPU/Memory
    cpu_usage=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1"%"}')
    mem_info=$(free -h | awk '/^Mem:/ {print $3 " / " $2}')
    load_avg=$(uptime | awk -F'load average:' '{print $2}')
    update_box 2 \
        "CPU Usage: $cpu_usage" \
        "Memory: $mem_info" \
        "Load:$load_avg"

    # Update Box 3: Disk Usage
    disk_root=$(df -h / | awk 'NR==2 {print $3 " / " $2 " (" $5 ")"}')
    disk_home=$(df -h ~ | awk 'NR==2 {print $3 " / " $2 " (" $5 ")"}')
    update_box 3 \
        "Root: $disk_root" \
        "Home: $disk_home"

    # Update Box 4: Network
    if command -v ifconfig &> /dev/null; then
        ip_addr=$(ifconfig | grep "inet " | grep -v 127.0.0.1 | head -1 | awk '{print $2}')
    else
        ip_addr=$(ip addr | grep "inet " | grep -v 127.0.0.1 | head -1 | awk '{print $2}' | cut -d/ -f1)
    fi
    connections=$(netstat -an 2>/dev/null | grep ESTABLISHED | wc -l)
    update_box 4 \
        "IP: ${ip_addr:-N/A}" \
        "Connections: $connections"

    # Update Box 5: Top Processes
    processes=$(ps aux --sort=-%mem | head -6 | tail -5 | awk '{printf "%-12s %5s%%\n", $11, $4}')
    update_box 5 \
        "Process          MEM" \
        "$processes"

    echo -e "${GREEN}  ✓ Updated all boxes${NC}"

    sleep $INTERVAL
done
