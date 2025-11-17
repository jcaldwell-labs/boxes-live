# boxes-live Demo Capabilities - What We Can Build Now

## Plugin System Status

### 📋 Designed (PLUGIN_ARCHITECTURE.md)
- ✅ Named pipe support
- ✅ File watcher integration
- ✅ Event system
- ✅ Plugin manager
- ✅ Hook points

### 🚧 Not Yet Implemented
The core plugin system code is **not in src/** yet. The architecture is fully designed but requires:
1. Adding `pipe_path` field to Box struct
2. Background reader threads/processes
3. Automatic canvas reload on file change
4. Plugin configuration system

### ✅ What Works TODAY

Even without the full plugin system, we can build impressive live monitoring using:

1. **Save/Load (F2/F3)** - Built-in persistence
2. **boxes-cli** - Python CLI for canvas manipulation
3. **Connectors** - 6 import/export tools
4. **File watching** - inotifywait for change detection
5. **Bash scripting** - Automation and updates

## What You Can Build Right Now

### 1. Live System Monitor ⭐ RECOMMENDED DEMO

**Demo:** `demos/live-monitor.sh`

Creates a dashboard that updates every 2 seconds with:
- System uptime and hostname
- CPU usage percentage
- Memory usage
- Disk usage
- Network connections
- Top 5 processes by memory

**How to run:**
```bash
cd demos
./live-monitor.sh &          # Start background updater
cd ..
./boxes-live                  # Run application
# Press F3
# Type: demos/live_monitor.txt
# Press F3 again every 2s to see updates
```

**Architecture:**
```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ live-monitor │────>│  boxes-cli   │────>│ canvas.txt   │
│   .sh        │     │  (update)    │     │              │
│ (every 2s)   │     └──────────────┘     └──────────────┘
└──────────────┘                                  │
                                                  │
                                                  v
                                          ┌──────────────┐
                                          │ boxes-live   │
                                          │ (F3 reload)  │
                                          └──────────────┘
```

### 2. Git Repository Dashboard

**Demo:** `demos/git-dashboard.sh`

Live view of repository state:
- Current branch and remote
- Recent 5 commits
- All branches
- Uncommitted changes
- Repository statistics

**How to run:**
```bash
cd demos
./git-dashboard.sh /path/to/repo &
cd ..
./boxes-live  # F3 → git_dashboard.txt
```

### 3. CSV Data Visualization

**Connector:** `connectors/csv2canvas`

Convert spreadsheet data to visual canvas:
```bash
# Create sample CSV
cat > data.csv << EOF
Name,Status,Priority
Task 1,In Progress,High
Task 2,Done,Medium
Task 3,Blocked,High
EOF

# Convert to canvas
./connectors/csv2canvas data.csv tasks.txt

# View in boxes-live
./boxes-live  # F3 → tasks.txt
```

### 4. Directory Tree Visualization

**Connector:** `connectors/tree2canvas`

Visual file system exploration:
```bash
./connectors/tree2canvas /path/to/dir tree.txt
./boxes-live  # F3 → tree.txt
```

### 5. Text File to Canvas

**Connector:** `connectors/text2canvas`

Convert any text file to boxes:
```bash
./connectors/text2canvas README.md readme_canvas.txt --mode paragraph
./boxes-live  # F3 → readme_canvas.txt
```

### 6. Periodic Table Demo

**Connector:** `connectors/periodic2canvas`

Color-coded chemical elements:
```bash
./connectors/periodic2canvas periodic.txt
./boxes-live  # F3 → periodic.txt
```

## Capability Matrix

| Feature | Status | How It Works |
|---------|--------|-------------|
| **Live Updates** | ✅ Manual | Script updates file, user presses F3 |
| **Auto-Reload** | 🚧 Designed | Needs plugin system implementation |
| **Named Pipes** | 🚧 Designed | Architecture documented |
| **File Import** | ✅ Full | 6 connectors available |
| **CLI Control** | ✅ Full | boxes-cli (CRUD operations) |
| **System Monitoring** | ✅ Demo | live-monitor.sh |
| **Git Integration** | ✅ Demo | git-dashboard.sh + git2canvas |
| **CSV Import** | ✅ Full | csv2canvas connector |
| **Event Hooks** | 🚧 Designed | Plugin system needed |
| **Plugin Manager** | 🚧 Designed | Implementation pending |

## Performance Characteristics

### Current (File-Based Updates)

**Pros:**
- ✅ No code changes to boxes-live needed
- ✅ Works with existing save/load
- ✅ Simple to implement (bash scripts)
- ✅ Portable (standard Unix tools)

**Cons:**
- ⚠️ Manual reload (F3 press)
- ⚠️ File I/O overhead
- ⚠️ 1-5 second update rate practical limit
- ⚠️ Not suitable for high-frequency updates (<1s)

### With Plugin System (Future)

**Benefits:**
- ✅ Automatic reload (no F3)
- ✅ Sub-second updates
- ✅ Lower latency
- ✅ Named pipe support
- ✅ Event-driven architecture

**Trade-offs:**
- Requires C code changes
- More complex implementation
- Potential thread safety considerations

## Bridging the Gap: Semi-Automatic Reload

You can simulate auto-reload using `inotifywait`:

```bash
# Terminal 1: Monitor for changes
inotifywait -m -e modify canvas.txt | while read; do
    echo "Canvas updated! Press F3 in boxes-live"
    # Future: Could send SIGUSR1 to boxes-live process
done

# Terminal 2: Run updater
./demos/live-monitor.sh

# Terminal 3: Run boxes-live
./boxes-live
```

## Building Your Own Monitor

**Template:**
```bash
#!/bin/bash
CANVAS="my_monitor.txt"

# Create
../connectors/boxes-cli create "$CANVAS"

# Add boxes
../connectors/boxes-cli add "$CANVAS" \
    --x 10 --y 5 --width 40 --height 10 \
    --title "Data" --color 2

# Update loop
while true; do
    data=$(your_data_source)
    ../connectors/boxes-cli update "$CANVAS" 1 \
        --content "$data"
    sleep 2
done
```

## Real-World Use Cases

### 1. DevOps Dashboard
```bash
# Monitor Docker containers
while true; do
    boxes-cli update dashboard.txt 1 \
        --content "$(docker ps --format 'table {{.Names}}\t{{.Status}}')"
    sleep 5
done
```

### 2. Build Monitor
```bash
# Track CI/CD pipeline
while true; do
    status=$(curl -s https://api.ci.com/status)
    boxes-cli update dashboard.txt 2 --content "$status"
    sleep 30
done
```

### 3. Log Aggregator
```bash
# Tail multiple log files
tail -f /var/log/app.log | while read line; do
    boxes-cli update dashboard.txt 3 --content "$line"
done
```

### 4. Service Health
```bash
# Check service status
while true; do
    services=$(systemctl list-units --state=running | head -10)
    boxes-cli update dashboard.txt 4 --content "$services"
    sleep 10
done
```

## Migration Path to Full Plugins

When the plugin system is implemented:

**Phase 1** (Current):
```bash
# External script updates file
script → boxes-cli → canvas.txt → boxes-live (F3)
```

**Phase 2** (File watching):
```bash
# Auto-reload on change
script → boxes-cli → canvas.txt → boxes-live (inotify → reload)
```

**Phase 3** (Named pipes):
```c
// Box bound to pipe
box.pipe_path = "/tmp/monitor.pipe"
// Direct streaming updates
echo "data" > /tmp/monitor.pipe
```

**Phase 4** (Full plugins):
```c
// C plugin with hooks
plugin_register("monitor", monitor_plugin_init);
// Real-time updates, no file I/O
```

## Next Steps

### To Demo Now:
1. Run `demos/live-monitor.sh` in one terminal
2. Run `./boxes-live` in another
3. Press F3, load `demos/live_monitor.txt`
4. Press F3 every 2 seconds to see updates

### To Implement Full Plugins:
1. Add pipe support to Box struct (types.h)
2. Implement background reader threads (new plugin.c module)
3. Add auto-reload on file change (inotify integration)
4. Create plugin configuration system
5. Implement event hooks
6. Build plugin manager CLI

### Contributing:
See [PLUGIN_ARCHITECTURE.md](PLUGIN_ARCHITECTURE.md) for implementation details.

## Summary

**What we have:** Solid foundation for live monitoring using file-based IPC
**What we're building toward:** Full plugin system with named pipes and auto-reload
**What you can do today:** Build impressive monitoring dashboards with existing tools

The demos prove the concept works. The architecture is designed. Now it's a matter of implementing the C code to make it seamless!
