# boxes-live Connectors and CLI

This directory contains the boxes-live CLI tool and connector scripts that enable Unix-philosophy integration with the canvas.

## Quick Start

### Using the CLI

```bash
# Create a new canvas
./connectors/boxes-cli create my_canvas.txt

# Add a box
./connectors/boxes-cli add my_canvas.txt \
    --x 100 --y 100 \
    --title "Welcome" \
    --content "First line" "Second line" \
    --color 3

# List all boxes
./connectors/boxes-cli list my_canvas.txt

# Search boxes
./connectors/boxes-cli search my_canvas.txt "welcome"

# Export to Markdown
./connectors/boxes-cli export my_canvas.txt --format markdown -o output.md

# View statistics
./connectors/boxes-cli stats my_canvas.txt
```

### Using Connectors

```bash
# Generate periodic table
./connectors/periodic2canvas > periodic.txt

# Load in boxes-live (press F3, type filename)
boxes-live  # Then F3 → periodic.txt
```

## CLI Tool: boxes-cli

The `boxes-cli` tool provides complete programmatic access to canvas files.

### Commands

#### create - Create new canvas
```bash
boxes-cli create <canvas.txt> [--width W] [--height H]
```

#### add - Add box to canvas
```bash
boxes-cli add <canvas.txt> \
    --x X --y Y \
    --title "Title" \
    [--content "Line1" "Line2" ...] \
    [--width W] [--height H] \
    [--color C]
```

#### list - List all boxes
```bash
boxes-cli list <canvas.txt> \
    [--color C] \
    [--search QUERY] \
    [--region X1 Y1 X2 Y2] \
    [--json]
```

#### get - Get box details
```bash
boxes-cli get <canvas.txt> <box_id> [--json]
```

#### update - Update box properties
```bash
boxes-cli update <canvas.txt> <box_id> \
    [--x X] [--y Y] \
    [--title "New Title"] \
    [--color C] \
    [--content "Line1" "Line2" ...]
```

#### delete - Delete box
```bash
boxes-cli delete <canvas.txt> <box_id>
```

#### search - Full-text search
```bash
boxes-cli search <canvas.txt> <query> [--case-sensitive] [--json]
```

#### arrange - Auto-arrange boxes
```bash
boxes-cli arrange <canvas.txt> \
    --layout [grid|tree|circle|flow] \
    [--spacing S]
```

#### export - Export canvas
```bash
boxes-cli export <canvas.txt> \
    --format [markdown|json|svg|csv] \
    [-o OUTPUT]
```

#### stats - Show statistics
```bash
boxes-cli stats <canvas.txt> [--json]
```

## Connectors

### periodic2canvas

Generate an interactive periodic table of elements.

```bash
./connectors/periodic2canvas > periodic_table.txt
boxes-live  # F3 to load periodic_table.txt
```

Features:
- Color-coded by element category (metals, nonmetals, etc.)
- Positioned by group and period
- Shows symbol, atomic number, name, and atomic mass
- Navigate with pan/zoom in boxes-live

## CLI + Testing Integration

The CLI enables test-driven development workflows:

### Automated Testing
```bash
# Run CLI integration tests
./tests/test_cli.sh

# Use CLI in test scripts
assert_box_count() {
    local count=$(boxes-cli list "$1" | wc -l)
    [ "$count" -eq "$2" ] || exit 1
}

# Test workflow
boxes-cli create test.txt
boxes-cli add test.txt --title "Test" --x 100 --y 100
assert_box_count test.txt 1
```

### Snapshot Testing
```bash
# Create expected output
boxes-cli create expected.txt
boxes-cli add expected.txt --x 100 --y 100 --title "Expected"

# Generate actual output (via your workflow)
your_generator > actual.txt

# Compare
diff expected.txt actual.txt
```

### CI/CD Integration
```bash
#!/bin/bash
# .github/workflows/test.yml

# Test canvas generation
./connectors/periodic2canvas > periodic.txt
boxes-cli stats periodic.txt --json > stats.json

# Verify box count
box_count=$(jq '.total_boxes' stats.json)
[ "$box_count" -eq 20 ] || exit 1

# Verify export formats
boxes-cli export periodic.txt --format markdown -o periodic.md
boxes-cli export periodic.txt --format json -o periodic.json
```

## Agent Integration Patterns

The CLI is designed for AI agent workflows with minimal context usage:

### Pattern 1: Create and Populate
```bash
# Agent creates canvas programmatically
boxes-cli create project.txt
boxes-cli add project.txt --x 100 --y 100 --title "Backend API" --color 3
boxes-cli add project.txt --x 300 --y 100 --title "Frontend UI" --color 2
boxes-cli add project.txt --x 200 --y 200 --title "Database" --color 1
```

### Pattern 2: Query and Analyze
```bash
# Agent searches existing canvas
boxes-cli search canvas.txt "API" --json | jq '.[].id'
boxes-cli stats canvas.txt --json | jq '.color_distribution'
```

### Pattern 3: Transform and Export
```bash
# Agent converts canvas to different formats
boxes-cli export canvas.txt --format markdown > report.md
boxes-cli export canvas.txt --format json > data.json
boxes-cli export canvas.txt --format csv > spreadsheet.csv
```

### Pattern 4: Pipeline Composition
```bash
# Agent chains operations
cat data.csv | csv2canvas | \
    boxes-cli arrange --layout grid | \
    boxes-cli export --format markdown > output.md
```

## JSON Output Mode

All commands support `--json` for structured output:

```bash
# Get structured data
boxes-cli list canvas.txt --json | jq '.[0].title'
boxes-cli stats canvas.txt --json | jq '.total_boxes'
boxes-cli search canvas.txt "query" --json | jq 'map(.id)'
```

This enables agent integration with minimal parsing overhead.

## Context Efficiency Comparison

### Miro API Response (JSON)
```json
{
  "type": "sticky_note",
  "id": "3074457346543033864",
  "title": "Task 1",
  "content": "Do something",
  "position": {"x": 100, "y": 200},
  "geometry": {"width": 150, "height": 150},
  "style": {"fillColor": "#fff9b1"},
  "createdAt": "2024-01-01T00:00:00Z",
  "modifiedAt": "2024-01-01T00:00:00Z",
  "createdBy": {"id": "...", "name": "..."},
  ...
}
```
**Size**: ~450 bytes per box

### boxes-live CLI Output (Text)
```
1 100.0 200.0 30 8 0 4
Task 1
1
Do something
```
**Size**: ~35 bytes per box

**Context Savings**: ~13x reduction

### boxes-live JSON Output
```json
{
  "id": 1,
  "x": 100.0,
  "y": 200.0,
  "width": 30,
  "height": 8,
  "color": 4,
  "title": "Task 1",
  "content": ["Do something"]
}
```
**Size**: ~110 bytes per box

**Context Savings**: ~4x reduction vs Miro, with structured data

## Unix Philosophy Benefits

1. **Composability**: Pipe commands together
2. **Text-based**: Grep, sed, awk all work
3. **Version Control**: Git-friendly text format
4. **Streaming**: Handle large datasets
5. **Testability**: Easy to write tests
6. **Debuggability**: Human-readable intermediate outputs
7. **Extensibility**: Add connectors without modifying core
8. **Simplicity**: Standard input/output conventions

## Writing Custom Connectors

Template for new connectors:

```bash
#!/bin/bash
# myconnector - Convert my data to canvas format
#
# Usage: myconnector < input > canvas.txt

set -euo pipefail

# Parse input
while read -r line; do
    # Extract fields
    title=$(echo "$line" | cut -d',' -f1)
    content=$(echo "$line" | cut -d',' -f2)

    # Calculate position (your logic here)
    x=$((100 + id * 150))
    y=100

    # Generate canvas format
    echo "$id $x $y 30 8 0 2"
    echo "$title"
    echo "1"
    echo "$content"

    ((id++))
done
```

Or in Python:

```python
#!/usr/bin/env python3
import sys
import json

# Read input
data = json.load(sys.stdin)

# Write canvas header
print("BOXES_CANVAS_V1")
print("2000.0 1500.0")
print(len(data))

# Write boxes
for i, item in enumerate(data, 1):
    x = 100 + (i % 5) * 200
    y = 100 + (i // 5) * 120

    print(f"{i} {x} {y} 30 8 0 3")
    print(item['title'])
    print(len(item['content']))
    for line in item['content']:
        print(line)
```

## Examples

### Project Planning
```bash
# Fetch GitHub issues
gh issue list --json number,title,body --jq '.' > issues.json

# Convert to canvas (custom connector)
cat issues.json | issues2canvas > project.txt

# Arrange by priority
boxes-cli arrange project.txt --layout grid

# Export roadmap
boxes-cli export project.txt --format markdown > ROADMAP.md
```

### Data Visualization
```bash
# Load CSV data
cat sales_data.csv | csv2canvas > sales.txt

# Color code by region
for region in "North" "South" "East" "West"; do
    boxes-cli list sales.txt --search "$region" --json | \
    jq -r '.[].id' | \
    while read id; do
        boxes-cli update sales.txt $id --color $color
    done
    ((color++))
done

# View interactively
boxes-live  # F3 → sales.txt
```

### Documentation Generation
```bash
# Extract code structure
ctags -x *.c | awk '{print $1, $2}' | while read name kind; do
    boxes-cli add docs.txt \
        --title "$name" \
        --content "Type: $kind" \
        --x $((RANDOM % 1000 + 100)) \
        --y $((RANDOM % 1000 + 100))
done

# Export to Markdown documentation
boxes-cli export docs.txt --format markdown > API.md
```

## Future Connectors

Ideas for additional connectors:

- `git2canvas` - Git commit history visualization
- `docker2canvas` - Container/image visualization
- `kubectl2canvas` - Kubernetes cluster state
- `jira2canvas` - Issue tracker integration
- `notion2canvas` - Notion database import
- `mermaid2canvas` - Mermaid diagram converter
- `plantuml2canvas` - UML diagram converter
- `org2canvas` - Org-mode outline import
- `markdown2canvas` - Markdown document structure
- `sql2canvas` - Database query results

See [CONNECTORS.md](../CONNECTORS.md) for detailed connector designs and examples.

## Support

For bugs or feature requests:
- File an issue: https://github.com/jcaldwell1066/boxes-live/issues
- See documentation: [CLAUDE.md](../CLAUDE.md)
- Run tests: `./tests/test_cli.sh`
