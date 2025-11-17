# Quick Demo Guide - Run This Now!

## TL;DR - Live System Monitor

**What it is:** A real-time system monitoring dashboard that updates every 2 seconds

**What you'll see:**
- CPU, memory, disk usage
- Network connections
- Top processes
- Live timestamps

## 3-Step Demo

### Terminal 1: Start the monitor
```bash
cd demos
./live-monitor.sh
```

This creates `live_monitor.txt` and updates it every 2 seconds.

### Terminal 2: Run boxes-live
```bash
./boxes-live
```

In the application:
1. Press **F3** (Load)
2. Type: `demos/live_monitor.txt`
3. Press Enter

You'll see the monitoring dashboard!

### Watching Live Updates

The monitor script prints:
```
[HH:MM:SS] Update #1
  ✓ Updated all boxes
```

Every time you see this, press **F3** in boxes-live to reload and see fresh data!

## What's Happening?

```
live-monitor.sh → boxes-cli update → live_monitor.txt → boxes-live (F3)
```

1. Script collects system stats
2. CLI tool updates canvas file
3. You press F3 to reload
4. Canvas shows new data!

## Try This: Watch It Change

1. Open a browser or run `stress-ng` to increase CPU
2. Wait for next update (2 seconds)
3. Press F3 in boxes-live
4. See CPU % increase!

## Other Quick Demos

### Git Dashboard (if in a git repo)
```bash
# Terminal 1
cd demos
./git-dashboard.sh ..

# Terminal 2
./boxes-live
# F3 → git_dashboard.txt
```

### Periodic Table
```bash
./connectors/periodic2canvas periodic.txt
./boxes-live
# F3 → periodic.txt
```

### Your README
```bash
./connectors/text2canvas README.md readme_canvas.txt --mode paragraph
./boxes-live
# F3 → readme_canvas.txt
```

## Understanding the Architecture

**Current (File-Based):**
- ✅ Works today
- Manual reload (F3)
- 1-5 second updates practical

**Future (Plugin System):**
- Auto-reload (no F3 needed)
- Named pipes for streaming
- Sub-second updates

See **DEMO_CAPABILITIES.md** for details!

## Troubleshooting

**"boxes-cli not found"**
```bash
# Make sure you're in the right directory
cd /path/to/boxes-live
./connectors/boxes-cli --version
```

**"Permission denied"**
```bash
chmod +x demos/*.sh connectors/*
```

**"Canvas not found"**
```bash
# Make sure the path is relative to where boxes-live runs
# Use: demos/live_monitor.txt
# Not: live_monitor.txt
```

## What's Next?

Once the plugin system is implemented (PLUGIN_ARCHITECTURE.md), this will:
- Auto-reload on changes (no F3)
- Support named pipes for streaming
- Enable real-time updates

But even now, it's pretty cool! 🎉
