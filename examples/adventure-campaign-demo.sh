#!/usr/bin/env bash
# adventure-campaign-demo.sh - Complete demo of tmux campaign orchestration
#
# This demo showcases the full campaign orchestration system:
# - tmux multi-pane session layout
# - realm2canvas connector for adventure-engine integration
# - Signal-based synchronization (SIGUSR1/SIGUSR2)
# - Real-time realm state updates
# - Inter-pane command coordination

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${MAGENTA}"
cat << 'BANNER'
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║        🎲  Adventure Campaign Orchestration Demo  🎲        ║
║                                                              ║
║              boxes-live Campaign System v1.1                 ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
BANNER
echo -e "${NC}"

echo -e "${CYAN}This demo showcases:${NC}"
echo "  ✓ tmux multi-pane campaign sessions"
echo "  ✓ realm2canvas connector (adventure-engine integration)"
echo "  ✓ Signal-based state synchronization"
echo "  ✓ Real-time realm visualization"
echo "  ✓ Inter-pane command coordination"
echo ""

# Find boxes-live root before changing directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOXES_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Setup demo environment
DEMO_DIR="/tmp/boxes-live-campaign-demo"
mkdir -p "$DEMO_DIR"
cd "$DEMO_DIR"

echo -e "${BLUE}📁 Setting up demo environment: $DEMO_DIR${NC}"

# Create example realm definition
echo -e "${BLUE}📝 Creating example realm (adventure-engine format)...${NC}"

cat > realm.json << 'EOF'
{
  "realm": "The Crystal Spire",
  "description": "A towering crystalline structure emanating magical energy",
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
      "level": 7,
      "status": "ready",
      "abilities": ["Shadow Strike", "Smoke Bomb", "Backstab"]
    },
    {
      "id": "hero2",
      "name": "Aldric Stormcaller",
      "type": "hero",
      "class": "mage",
      "x": 20,
      "y": 20,
      "hp": 55,
      "max_hp": 65,
      "level": 8,
      "status": "ready",
      "abilities": ["Lightning Bolt", "Teleport", "Shield"]
    },
    {
      "id": "hero3",
      "name": "Borin Ironheart",
      "type": "hero",
      "class": "paladin",
      "x": 18,
      "y": 23,
      "hp": 120,
      "max_hp": 120,
      "level": 6,
      "status": "defending",
      "abilities": ["Divine Shield", "Heal", "Smite"]
    },
    {
      "id": "npc1",
      "name": "Mystic Oracle",
      "type": "npc",
      "x": 90,
      "y": 15,
      "role": "quest_giver",
      "dialogue": "The Crystal holds ancient power... but beware the Guardian!"
    },
    {
      "id": "enemy1",
      "name": "Crystal Guardian",
      "type": "enemy",
      "x": 95,
      "y": 60,
      "hp": 250,
      "max_hp": 250,
      "threat_level": "boss",
      "abilities": ["Crystal Shards", "Energy Beam", "Regenerate"]
    },
    {
      "id": "enemy2",
      "name": "Shadow Wraith",
      "type": "enemy",
      "x": 50,
      "y": 75,
      "hp": 80,
      "max_hp": 80,
      "threat_level": "high"
    }
  ],
  "locations": [
    {
      "id": "loc1",
      "name": "Entrance Chamber",
      "x": 15,
      "y": 15,
      "type": "safe_zone",
      "description": "The grand entrance to the Crystal Spire",
      "explored": true
    },
    {
      "id": "loc2",
      "name": "Chamber of Echoes",
      "x": 55,
      "y": 30,
      "type": "puzzle",
      "description": "A room filled with magical mirrors and echoing voices",
      "explored": false
    },
    {
      "id": "loc3",
      "name": "Guardian's Sanctum",
      "x": 95,
      "y": 55,
      "type": "boss_room",
      "description": "The heart of the Spire where the Crystal Guardian awaits",
      "explored": false
    },
    {
      "id": "loc4",
      "name": "Crystal Chamber",
      "x": 100,
      "y": 15,
      "type": "treasure",
      "description": "The legendary Crystal of Eternity rests here",
      "explored": false
    }
  ],
  "quests": [
    {
      "id": "quest1",
      "title": "Claim the Crystal of Eternity",
      "status": "active",
      "description": "Retrieve the Crystal from the Spire's summit",
      "reward": "10,000 gold, Crystal of Eternity (Artifact)",
      "objectives": [
        "Explore the Spire (in progress)",
        "Defeat the Crystal Guardian (pending)",
        "Claim the Crystal (pending)"
      ]
    }
  ],
  "state": {
    "day": 7,
    "time": "midnight",
    "weather": "clear",
    "mood": "epic",
    "turn": 0,
    "combat_active": false
  }
}
EOF

echo -e "${GREEN}✓ Realm definition created${NC}"

# Convert realm to canvas
echo -e "${BLUE}🎨 Converting realm to canvas format...${NC}"
"$BOXES_ROOT/connectors/realm2canvas" realm.json realm_canvas.txt

echo -e "${GREEN}✓ Canvas generated${NC}"

# Show generated canvas stats
BOX_COUNT=$(grep -c "^BOX:" realm_canvas.txt || echo 0)
echo -e "${CYAN}📊 Canvas Statistics:${NC}"
echo "  Boxes: $BOX_COUNT"
echo "  Heroes: 3 (Kira, Aldric, Borin)"
echo "  Enemies: 2 (Guardian, Wraith)"
echo "  Locations: 4"
echo ""

# Create demo script for campaign simulation
cat > simulate_campaign.sh << 'SIM_SCRIPT'
#!/usr/bin/env bash
# Campaign simulation - demonstrates real-time updates

REALM_FILE="realm.json"
CANVAS_FILE="realm_canvas.txt"
TURN=1

echo "🎮 Campaign Simulation Running..."
echo "This script will simulate campaign progression"
echo ""

while true; do
    sleep 5

    # Simulate turn progression
    TURN=$((TURN + 1))

    # Update state in realm.json
    TMP_FILE=$(mktemp)

    jq --arg turn "$TURN" \
       '.state.turn = ($turn | tonumber)' \
       "$REALM_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$REALM_FILE"

    echo "[Turn $TURN] State updated"

    # Simulate party movement every 3 turns
    if [ $((TURN % 3)) -eq 0 ]; then
        # Move heroes forward
        jq '.entities |= map(
            if .type == "hero" then
                .x += 5
            else
                .
            end
        )' "$REALM_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$REALM_FILE"

        echo "[Turn $TURN] Party advanced!"
    fi

    # Simulate combat at turn 10
    if [ $TURN -eq 10 ]; then
        jq '.state.combat_active = true |
            .state.mood = "intense" |
            .entities |= map(
                if .id == "enemy2" then
                    .hp = 40
                else
                    .
                end
            )' "$REALM_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$REALM_FILE"

        echo "[Turn $TURN] ⚔️  Combat initiated with Shadow Wraith!"
    fi

    # End combat at turn 15
    if [ $TURN -eq 15 ]; then
        jq '.state.combat_active = false |
            .state.mood = "victorious" |
            .entities |= map(select(.id != "enemy2"))' \
            "$REALM_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$REALM_FILE"

        echo "[Turn $TURN] 🎉 Shadow Wraith defeated!"
    fi

    # Boss encounter at turn 20
    if [ $TURN -eq 20 ]; then
        jq '.state.combat_active = true |
            .state.mood = "desperate" |
            .entities |= map(
                if .id == "enemy1" then
                    .hp = 200
                else
                    .
                end
            )' "$REALM_FILE" > "$TMP_FILE" && mv "$TMP_FILE" "$REALM_FILE"

        echo "[Turn $TURN] 🐲 Boss Battle: Crystal Guardian engaged!"
    fi

    # Check for end condition
    if [ $TURN -ge 30 ]; then
        echo "[Turn $TURN] Demo complete! Realm saved."
        break
    fi
done
SIM_SCRIPT

chmod +x simulate_campaign.sh

echo -e "${YELLOW}📖 Demo Instructions:${NC}"
echo ""
echo "The campaign-session.sh script will launch a tmux session with:"
echo "  • LEFT PANE: Real-time realm visualization (boxes-live)"
echo "  • RIGHT-TOP: Narrative console / GM commands"
echo "  • RIGHT-BOTTOM: State manager / campaign commands"
echo ""
echo -e "${YELLOW}To try it manually:${NC}"
echo "  1. Start campaign session:"
echo "     ${CYAN}./demos/campaign-session.sh crystal-spire-demo${NC}"
echo ""
echo "  2. In another terminal, watch realm updates:"
echo "     ${CYAN}./demos/realm-watcher.sh $DEMO_DIR/realm.json $DEMO_DIR/realm_canvas.txt${NC}"
echo ""
echo "  3. Simulate campaign progression:"
echo "     ${CYAN}cd $DEMO_DIR && ./simulate_campaign.sh${NC}"
echo ""
echo "  4. Manually update realm and sync:"
echo "     ${CYAN}vim $DEMO_DIR/realm.json${NC}"
echo "     ${CYAN}pkill -SIGUSR1 boxes-live${NC}"
echo ""

echo -e "${GREEN}✓ Demo setup complete!${NC}"
echo -e "${BLUE}Demo files created in: $DEMO_DIR${NC}"
echo ""

echo -e "${MAGENTA}Key Features Demonstrated:${NC}"
echo "  🎯 Signal-based sync: SIGUSR1 reloads canvas in real-time"
echo "  🔄 Auto-conversion: realm.json → canvas.txt via realm2canvas"
echo "  🎮 Multi-pane: tmux orchestrates visualization + narrative + commands"
echo "  📊 Live updates: Changes to realm instantly visible in boxes-live"
echo "  ⚔️  Campaign flow: Turn-based progression with state management"
echo ""

echo -e "${CYAN}🎓 Learning Resources:${NC}"
echo "  • Campaign launcher: demos/campaign-session.sh"
echo "  • Realm connector: connectors/realm2canvas"
echo "  • State watcher: demos/realm-watcher.sh"
echo "  • Signal handler: src/signal_handler.c"
echo ""

echo -e "${GREEN}Demo ready! Check $DEMO_DIR for files.${NC}"
