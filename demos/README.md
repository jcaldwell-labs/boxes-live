# boxes-live Demos

This directory contains demonstration scripts showing boxes-live's capabilities for live data visualization and monitoring.

## Current Capabilities (Without Full Plugin System)

While the full plugin architecture (PLUGIN_ARCHITECTURE.md) is designed but not yet implemented, we can still achieve impressive live monitoring using:

1. **Save/Load Mechanism** (F2/F3) - Manual or scripted reload
2. **CLI Tools** (`boxes-cli`) - Programmatic canvas manipulation
3. **File Watching** (`inotifywait`) - Detect canvas changes
4. **Connectors** - Generate canvas from various sources

## Demos

### 1. Live System Monitor (`live-monitor.sh`)

**What it does:**
- Creates a canvas with system monitoring boxes
- Updates every 2 seconds with live system stats:
  - CPU usage
  - Memory usage
  - Disk usage
  - Network status
  - Top processes
- Uses CLI to update canvas file
- User manually reloads with F3 or sets up auto-reload

**Usage:**
```bash
cd demos
./live-monitor.sh
# In another terminal:
../boxes-live
# Press F3, select: live_monitor.txt
# Watch it update! Press F3 to reload when prompted
```

**How it works:**
```
live-monitor.sh → boxes-cli update → canvas.txt → boxes-live (F3 reload)
     └─ every 2s       └─ writes file     └─ user loads
```

### 2. Auto-Reload Watcher (`auto-reload-demo.sh`)

**What it does:**
- Uses `inotifywait` to watch canvas file for changes
- Notifies when updates are available
- Simulates plugin-like auto-reload behavior

**Usage:**
```bash
# Terminal 1: Start file watcher
cd demos
./auto-reload-demo.sh live_monitor.txt

# Terminal 2: Start live updates
./live-monitor.sh

# Terminal 3: Run boxes-live and load canvas
../boxes-live
# F3 → live_monitor.txt
# Press F3 when watcher notifies of changes
```

**Future:** Could auto-send reload signal to boxes-live process

### 3. Git Dashboard (`git-dashboard.sh`)

**What it does:**
- Live visualization of git repository state
- Updates every 5 seconds:
  - Repository info (name, branch, remote)
  - Recent commits (last 5)
  - All branches
  - Uncommitted changes
  - Repository statistics

**Usage:**
```bash
cd demos
./git-dashboard.sh /path/to/repo
# In another terminal:
../boxes-live
# F3 → git_dashboard.txt
```

**Use cases:**
- Monitor active development
- Track CI/CD status (combine with connector)
- Team awareness dashboard

## What's Possible Now

### ✅ Working Today (File-Based IPC)

1. **Live Monitoring**
   - System stats (CPU, memory, disk)
   - Process monitoring
   - Log file tailing (with text2canvas)
   - Git repository state

2. **Periodic Updates**
   - Cron-based updates
   - While-loop polling
   - Event-driven via inotifywait

3. **Data Import**
   - CSV → boxes (csv2canvas)
   - Text files → boxes (text2canvas)
   - Directory trees → boxes (tree2canvas)
   - Git history → boxes (git2canvas)

4. **Programmatic Control**
   - boxes-cli for CRUD operations
   - Bash scripts for automation
   - Pipeline composition

### 🚧 Future (Requires Plugin System Implementation)

1. **Automatic Reload**
   - Canvas auto-reloads on file change
   - No manual F3 press needed
   - Seamless live updates

2. **Named Pipe Support**
   - Boxes bound to FIFOs
   - Streaming updates
   - Low-latency refresh

3. **Plugin Manager**
   - Dynamic plugin loading
   - Plugin configuration
   - Dependency management

4. **Event System**
   - box_created, box_deleted hooks
   - Selection change events
   - Custom event triggers

## Creating Your Own Monitor

Template:
```bash
#!/bin/bash
CANVAS="my_monitor.txt"

# Create canvas
../connectors/boxes-cli create "$CANVAS"

# Add boxes
../connectors/boxes-cli add "$CANVAS" \
    --x 10 --y 5 --width 40 --height 10 \
    --title "My Data" --color 2 \
    --content "Initial content"

# Update loop
while true; do
    # Get your data
    data=$(your_command_here)

    # Update box (ID 1)
    ../connectors/boxes-cli update "$CANVAS" 1 \
        --content "$data"

    sleep 5
done
```

## Performance Considerations

**Current Limitations:**
- Manual reload (F3 press) adds latency
- File I/O overhead on each update
- Not suitable for <100ms update rates

**With Plugin System:**
- Sub-second updates possible
- Direct memory updates
- Event-driven refresh

## Tips

1. **Update Frequency**: 1-5 seconds works well for most monitors
2. **Box Count**: 5-10 boxes per canvas for readability
3. **Content Lines**: Keep <10 lines per box for visibility
4. **Colors**: Use colors to distinguish data types
5. **Layout**: Use consistent positions (grid layout)

## Examples of What You Can Build

### System Admin Dashboard
```
┌─────────────────┬─────────────────┬─────────────────┐
│  CPU/Memory     │   Disk Usage    │   Network       │
├─────────────────┼─────────────────┼─────────────────┤
│  Processes      │   Logs          │   Services      │
└─────────────────┴─────────────────┴─────────────────┘
```

### Development Dashboard
```
┌─────────────────┬─────────────────┬─────────────────┐
│  Git Status     │   CI/CD         │   Issues        │
├─────────────────┼─────────────────┼─────────────────┤
│  Test Results   │   Code Stats    │   TODOs         │
└─────────────────┴─────────────────┴─────────────────┘
```

### DevOps Monitor
```
┌─────────────────┬─────────────────┬─────────────────┐
│  Containers     │   Services      │   Metrics       │
├─────────────────┼─────────────────┼─────────────────┤
│  Logs           │   Alerts        │   Resources     │
└─────────────────┴─────────────────┴─────────────────┘
```

## See Also

- [PLUGIN_ARCHITECTURE.md](../PLUGIN_ARCHITECTURE.md) - Full plugin system design
- [CONNECTORS.md](../CONNECTORS.md) - Data import/export tools
- [MCP_SERVER.md](../MCP_SERVER.md) - Claude Desktop integration spec
- [examples/](../examples/) - Usage examples for connectors

## Contributing

Have a cool demo idea? Submit a PR with:
1. Script in `demos/`
2. Documentation in this README
3. Example screenshot or ASCII art output
