#!/bin/bash
# cli_demo.sh - Demonstration of boxes-cli capabilities
#
# This script creates a sample project planning canvas showing
# how to use the CLI for programmatic canvas creation

set -e

CLI="./connectors/boxes-cli"
CANVAS="examples/project_canvas.txt"

echo "=========================================="
echo "boxes-live CLI Demonstration"
echo "=========================================="
echo

# Create canvas
echo "1. Creating project canvas..."
$CLI create "$CANVAS" --width 2000 --height 1200
echo "   ✓ Canvas created: $CANVAS"
echo

# Add project phases
echo "2. Adding project phases..."
$CLI add "$CANVAS" --x 100 --y 100 --title "Phase 1: Planning" \
    --content "Requirements gathering" "Architecture design" "Tech stack selection" \
    --color 3  # Blue
echo "   ✓ Added Phase 1"

$CLI add "$CANVAS" --x 400 --y 100 --title "Phase 2: Development" \
    --content "Backend API" "Frontend UI" "Database schema" \
    --color 2  # Green
echo "   ✓ Added Phase 2"

$CLI add "$CANVAS" --x 700 --y 100 --title "Phase 3: Testing" \
    --content "Unit tests" "Integration tests" "E2E tests" \
    --color 4  # Yellow
echo "   ✓ Added Phase 3"

$CLI add "$CANVAS" --x 1000 --y 100 --title "Phase 4: Deployment" \
    --content "CI/CD setup" "Production deploy" "Monitoring" \
    --color 1  # Red
echo "   ✓ Added Phase 4"
echo

# Add tasks for Phase 1
echo "3. Adding detailed tasks..."
$CLI add "$CANVAS" --x 150 --y 300 --title "User Stories" \
    --content "Feature A" "Feature B" "Feature C" \
    --color 6  # Cyan
echo "   ✓ Added tasks"

$CLI add "$CANVAS" --x 150 --y 450 --title "Technical Specs" \
    --content "API design" "Data models" "System architecture" \
    --color 6
echo "   ✓ Added specs"
echo

# Show statistics
echo "4. Canvas statistics:"
$CLI stats "$CANVAS"
echo

# List all boxes
echo "5. Box listing:"
$CLI list "$CANVAS"
echo

# Search example
echo "6. Search for 'test' related boxes:"
$CLI search "$CANVAS" "test"
echo

# Export to different formats
echo "7. Exporting to multiple formats..."
$CLI export "$CANVAS" --format markdown -o examples/project.md
echo "   ✓ Exported to Markdown: examples/project.md"

$CLI export "$CANVAS" --format json -o examples/project.json
echo "   ✓ Exported to JSON: examples/project.json"

$CLI export "$CANVAS" --format csv -o examples/project.csv
echo "   ✓ Exported to CSV: examples/project.csv"
echo

# JSON API example
echo "8. JSON API usage (for agent integration):"
echo "   Getting box details as JSON:"
$CLI get "$CANVAS" 1 --json | python3 -m json.tool | head -10
echo "   ..."
echo

echo "=========================================="
echo "Demo complete!"
echo "=========================================="
echo
echo "To view the canvas interactively:"
echo "  boxes-live"
echo "  Press F3, then type: $CANVAS"
echo
echo "Files created:"
echo "  - $CANVAS (interactive canvas)"
echo "  - examples/project.md (Markdown export)"
echo "  - examples/project.json (JSON export)"
echo "  - examples/project.csv (CSV export)"
echo
