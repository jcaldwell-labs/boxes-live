# Campaign Orchestration Guide

## Overview

boxes-live v1.1+ includes comprehensive campaign orchestration infrastructure for text-based adventure games. This system transforms boxes-live from a standalone canvas tool into foundational infrastructure for multi-user, narrative-driven campaigns orchestrated through tmux.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                 Campaign Session (tmux)                 │
├──────────────────────┬──────────────────────────────────┤
│                      │  Narrative Console (GM)          │
│  Realm Visualization │  • Edit narrative                │
│  (boxes-live)        │  • Issue GM commands             │
│                      │  • Trigger events                │
│  • Real-time canvas  ├──────────────────────────────────┤
│  • Entity positions  │  State Manager                   │
│  • Spatial layout    │  • Campaign commands             │
│  • Auto-sync         │  • Entity management             │
│                      │  • State queries                 │
└──────────────────────┴──────────────────────────────────┘
           ▲                      │
           │                      ▼
    SIGUSR1 (reload)      realm.json (state)
           │                      │
           └──────────────────────┘
              realm2canvas
```

## Core Components

### 1. Signal-Based Synchronization

**Enhanced Signal Handler** (`src/signal_handler.c`)

- **SIGUSR1**: Reload canvas (campaign state sync)
- **SIGUSR2**: Custom campaign events (extensible)

**Usage:**
```bash
# Reload canvas in running boxes-live
pkill -SIGUSR1 boxes-live

# Trigger custom campaign event
pkill -SIGUSR2 boxes-live
```

**Implementation:**
```c
// In main loop (src/main.c)
if (signal_should_reload()) {
    const char *current_file = persistence_get_current_file();
    if (current_file != NULL) {
        Canvas new_canvas;
        if (canvas_load(&new_canvas, current_file) == 0) {
            canvas_cleanup(&canvas);
            canvas = new_canvas;
        }
    }
}
```

### 2. realm2canvas Connector

Converts adventure-engine realm definitions (JSON) to boxes-live canvas format.

**Input Format** (realm.json):
```json
{
  "realm": "The Crystal Spire",
  "description": "A towering crystalline structure",
  "dimensions": {"width": 180, "height": 120},
  "entities": [
    {
      "id": "hero1",
      "name": "Kira Shadowstep",
      "type": "hero",
      "class": "assassin",
      "x": 15,
      "y": 20,
      "hp": 75,
      "max_hp": 85,
      "level": 7
    }
  ],
  "locations": [
    {
      "id": "loc1",
      "name": "Guardian's Sanctum",
      "x": 95,
      "y": 55,
      "type": "boss_room",
      "description": "Where the Guardian awaits"
    }
  ],
  "quests": [
    {
      "id": "quest1",
      "title": "Claim the Crystal",
      "status": "active"
    }
  ],
  "state": {
    "day": 7,
    "time": "midnight",
    "combat_active": false
  }
}
```

**Output Format** (realm_canvas.txt):
```
WIDTH: 180
HEIGHT: 120

BOX: 5.0 5.0 45.0 10.0
TITLE: The Crystal Spire
CONTENT:
A towering crystalline structure
Day: 7 | Time: midnight
END

BOX: 15 20 25.0 7.0
TITLE: Kira Shadowstep
COLOR: green
CONTENT:
Type: hero
HP: 75/85
END
```

**Usage:**
```bash
# Convert realm to canvas
./connectors/realm2canvas realm.json canvas.txt

# Create example realm (if file doesn't exist)
./connectors/realm2canvas nonexistent.json example.txt
```

**Features:**
- ✅ Automatic example generation
- ✅ Works with or without `jq` (fallback parser)
- ✅ Entity type color coding (heroes=green, enemies=red, npcs=white)
- ✅ Visual icons for locations (⚔ dungeons, 🏛 settlements)
- ✅ Quest log rendering
- ✅ Party status tracking

### 3. Campaign Session Launcher

**Script:** `demos/campaign-session.sh`

Creates a tmux session with optimized layout for campaign management.

**Usage:**
```bash
# Launch new campaign
./demos/campaign-session.sh my-campaign

# Attach to existing campaign
./demos/campaign-session.sh my-campaign  # Auto-detects and attaches
```

**Session Layout:**
```
┌─────────────────┬─────────────────┐
│                 │  Narrative      │
│  Realm Canvas   │  Console        │
│  (boxes-live)   │  (GM commands)  │
│                 ├─────────────────┤
│                 │  State Manager  │
│                 │  (CLI tools)    │
└─────────────────┴─────────────────┘
  60% width         40% width
```

**Pane Responsibilities:**

1. **Left Pane (Realm Visualization)**:
   - Runs `boxes-live` with campaign canvas
   - Real-time entity positions
   - Auto-syncs on SIGUSR1

2. **Right-Top (Narrative Console)**:
   - GM narration area
   - Campaign command reference
   - Quick sync instructions

3. **Right-Bottom (State Manager)**:
   - boxes-cli for entity management
   - Campaign statistics
   - Quick action commands

**Helper Commands:**

The session creates `~/.boxes-live/campaigns/<name>/commands.sh`:

```bash
# Source helper in any pane
source $CAMPAIGN_HELPER

# Available commands
sync              # Sync canvas to visualization (SIGUSR1)
event             # Trigger campaign event (SIGUSR2)
add NAME [X Y]    # Add entity to realm
move ID X Y       # Move entity
export            # Export campaign state (JSON)
```

### 4. Realm Watcher

**Script:** `demos/realm-watcher.sh`

Monitors realm JSON file and auto-syncs changes to canvas.

**Usage:**
```bash
# Watch realm file and auto-sync
./demos/realm-watcher.sh realm.json realm_canvas.txt [interval-seconds]

# Example with 2-second polling
./demos/realm-watcher.sh campaign/realm.json campaign/canvas.txt 2
```

**Features:**
- ✅ Detects realm.json modifications
- ✅ Auto-regenerates canvas via realm2canvas
- ✅ Signals running boxes-live instances (SIGUSR1)
- ✅ Colored output with sync statistics
- ✅ Turn counter and event logging

**Output Example:**
```
🔍 Realm Watcher Started
Watching: /path/to/realm.json
Canvas:   /path/to/canvas.txt
Interval: 2s

[04:20:15] Initial sync...
[04:20:15] ✓ Synced to boxes-live (PID: 12345)
[04:20:23] 🔄 Realm file changed, regenerating canvas...
[04:20:23] ✓ Synced to boxes-live (PID: 12345)
[04:20:23] 📊 Canvas: 15 boxes | Sync #2
```

## Complete Workflow Example

### Step 1: Create Realm Definition

```bash
# Create realm JSON
cat > my_realm.json << 'EOF'
{
  "realm": "Dragon's Keep",
  "entities": [
    {
      "id": "hero1",
      "name": "Brave Knight",
      "type": "hero",
      "x": 10,
      "y": 10,
      "hp": 100,
      "max_hp": 100
    }
  ],
  "locations": [
    {
      "name": "Dragon's Lair",
      "x": 80,
      "y": 50,
      "type": "boss_room"
    }
  ],
  "state": {"day": 1, "time": "dawn"}
}
EOF
```

### Step 2: Convert to Canvas

```bash
./connectors/realm2canvas my_realm.json my_canvas.txt
```

### Step 3: Launch Campaign Session

Terminal 1:
```bash
# Start campaign session
./demos/campaign-session.sh dragons-keep
```

This opens tmux with:
- Left: boxes-live displaying realm
- Right-top: Narrative console
- Right-bottom: Command center

### Step 4: Enable Auto-Sync

Terminal 2:
```bash
# Watch for changes and auto-sync
./demos/realm-watcher.sh my_realm.json my_canvas.txt
```

### Step 5: Update Campaign State

Terminal 2 or from tmux pane:
```bash
# Edit realm (use your favorite editor)
vim my_realm.json

# Update hero position
jq '.entities[0].x = 20' my_realm.json > tmp && mv tmp my_realm.json

# realm-watcher automatically:
# 1. Detects change
# 2. Regenerates canvas
# 3. Signals boxes-live (SIGUSR1)
# 4. Canvas updates in real-time!
```

### Step 6: Campaign Progression

Use boxes-cli for entity management:
```bash
# List all entities
./connectors/boxes-cli list my_canvas.txt

# Add new entity
./connectors/boxes-cli add my_canvas.txt \
  --x 50 --y 30 --width 25 --height 8 \
  --title "Fire Drake" \
  --content "HP: 500/500\nThreat: Boss"

# Update and sync
pkill -SIGUSR1 boxes-live
```

## Adventure Demo

Run the complete demonstration:

```bash
./examples/adventure-campaign-demo.sh
```

This creates:
- Example realm (Crystal Spire adventure)
- Multi-entity scenario (3 heroes, 2 enemies, 4 locations)
- Campaign simulation script
- Complete documentation

**Demo showcases:**
- ✅ tmux multi-pane layout
- ✅ Real-time realm visualization
- ✅ Signal-based synchronization
- ✅ Turn-based progression
- ✅ Combat simulation
- ✅ State management

## Testing

Run the campaign orchestration test suite:

```bash
./tests/test_campaign.sh
```

**Tests verify:**
1. Signal handler compilation (SIGUSR1/SIGUSR2)
2. realm2canvas existence and execution
3. Example realm generation
4. Realm → canvas conversion
5. Canvas structure validation
6. Watcher script availability
7. Campaign launcher availability
8. Demo script availability
9. boxes-live binary availability
10. Canvas syntax validation
11. Fallback mode (no jq)
12. Demo file creation

## Integration with adventure-engine

While boxes-live provides the visualization and tmux orchestration layer, it's designed to integrate with adventure-engine (or similar) for:

- **Narrative Management**: Story progression, dialogue trees
- **Game Rules**: Combat systems, skill checks, inventory
- **State Persistence**: Database-backed campaign state
- **Multiplayer**: Player session management

**Integration Pattern:**

```
adventure-engine (Backend)
    │
    │ Generates realm.json
    ▼
realm.json ─────► realm2canvas ─────► canvas.txt
    │                                     │
    │ (modifications)                     │ (SIGUSR1)
    ▼                                     ▼
realm-watcher ──────────────────► boxes-live (tmux pane)
```

## Performance Considerations

### Large Campaigns

For campaigns with 100+ entities:

1. **Chunking**: Consider spatial partitioning
2. **Selective Sync**: Only sync visible entities
3. **Debouncing**: realm-watcher includes built-in polling interval
4. **Canvas Optimization**: Use viewport culling (already implemented)

### Multi-Player

For multi-player campaigns:

1. **Separate Canvases**: One per player + 1 GM canvas
2. **State Synchronization**: Use realm.json as single source of truth
3. **tmux Sessions**: One session per player
4. **Broadcast Updates**: Update realm.json, watcher syncs all sessions

## Roadmap

### v1.2.0 (Near-term)
- [ ] Connection lines between entities (show relationships)
- [ ] Fog of war (unexplored areas)
- [ ] Turn indicator and initiative tracker
- [ ] Audio cues on events (SIGUSR2 triggers)

### v1.3.0 (Mid-term)
- [ ] Real-time collaboration (multiple GMs)
- [ ] Chat frame integration
- [ ] Dice roller integration
- [ ] Session logging and replay

### v2.0.0 (Long-term)
- [ ] WebSocket support (remote players)
- [ ] AI-assisted narrative generation
- [ ] VTT-style combat grid mode
- [ ] Plugin marketplace for campaign types

## Troubleshooting

### Canvas not updating after SIGUSR1

**Check:**
1. Is boxes-live running? `pgrep boxes-live`
2. Did you save the canvas file?
3. Is the file path correct in `persistence_get_current_file()`?

**Solution:**
```bash
# Find boxes-live PID
ps aux | grep boxes-live

# Send signal manually
kill -SIGUSR1 <PID>

# Check if file changed
ls -l canvas.txt
```

### realm2canvas fails

**Check:**
1. Is realm.json valid JSON? `jq . realm.json`
2. Is connector executable? `ls -la connectors/realm2canvas`

**Solution:**
```bash
# Validate JSON
jq . realm.json || echo "Invalid JSON!"

# Regenerate with fallback
PATH=/tmp realm2canvas realm.json output.txt
```

### tmux session issues

**Check:**
1. Is tmux installed? `tmux -V`
2. Session name conflicts? `tmux ls`

**Solution:**
```bash
# Kill conflicting session
tmux kill-session -t my-campaign

# List all sessions
tmux ls

# Attach to existing
tmux attach -t my-campaign
```

## Examples

See working examples in:
- `examples/adventure-campaign-demo.sh` - Complete demo
- `demos/campaign-session.sh` - Session launcher
- `demos/realm-watcher.sh` - Auto-sync watcher
- `connectors/realm2canvas` - Realm converter

## Contributing

To add campaign features:

1. **New Signal**: Add to `signal_handler.c` and update handler
2. **New Connector**: Create in `connectors/` following realm2canvas pattern
3. **Tests**: Add to `tests/test_campaign.sh`
4. **Documentation**: Update this file and examples

## License

Same as boxes-live (see LICENSE file)

---

**Version**: 1.1.0
**Last Updated**: 2025-11-19
**Status**: Production-ready
