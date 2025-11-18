# Connector Development Guide

**boxes-live Connector Ecosystem**

This guide provides comprehensive documentation for using existing connectors and developing new ones for boxes-live.

## Table of Contents

- [Overview](#overview)
- [Connector Architecture](#connector-architecture)
- [Using Existing Connectors](#using-existing-connectors)
- [Developing Custom Connectors](#developing-custom-connectors)
- [Canvas File Format](#canvas-file-format)
- [Best Practices](#best-practices)
- [Testing Connectors](#testing-connectors)
- [Connector Reference](#connector-reference)

## Overview

Connectors are the bridge between external data sources and the boxes-live interactive canvas. They transform various data formats into the boxes-live canvas format, enabling visualization of:

- **System Data**: Processes (pstree), containers (docker), logs, git history
- **Structured Data**: CSV, JSON, configuration files
- **Custom Data**: GitHub organizations, project structures, any custom source

### Philosophy

Connectors follow the Unix philosophy:
- **Do one thing well**: Convert one data format to canvas format
- **Composable**: Can be chained with other tools via pipes
- **Text-based**: Human-readable intermediate formats
- **Testable**: Easy to verify output correctness

## Connector Architecture

### Data Flow

```
Data Source → Connector → Canvas File → boxes-live (interactive view)
                        ↓
                    boxes-cli (manipulation)
                        ↓
                    Export (markdown/json/csv)
```

### Types of Connectors

1. **Data Parsers**: Parse external data (pstree2canvas, log2canvas, docker2canvas)
2. **Generators**: Generate canvases from templates (periodic2canvas, jcaldwell-labs2canvas)
3. **Format Converters**: Convert between formats (json2canvas, csv2canvas)
4. **System Inspectors**: Inspect live system state (pstree, docker, git)

## Using Existing Connectors

### Quick Start Examples

#### Visualize Process Tree

```bash
# Generate from live system
pstree -p | ./connectors/pstree2canvas > processes.txt

# Or from saved output
pstree -p > pstree.txt
./connectors/pstree2canvas pstree.txt > processes.txt

# View interactively
./boxes-live  # Press F3, load processes.txt
```

#### Visualize Docker Containers

```bash
# From live Docker
docker ps | ./connectors/docker2canvas > containers.txt

# From JSON output
docker ps --format json | ./connectors/docker2canvas --format json > containers.txt

# View and manipulate
./boxes-live  # F3 → containers.txt
```

#### Visualize Logs

```bash
# Parse application logs
./connectors/log2canvas /var/log/syslog > logs.txt

# Filter by severity
./connectors/boxes-cli list logs.txt --color 1  # Red = errors only
./connectors/boxes-cli search logs.txt "ERROR"
```

#### Visualize CSV Data

```bash
# Convert CSV to canvas
./connectors/csv2canvas data.csv > data_canvas.txt

# Arrange in grid
./connectors/boxes-cli arrange data_canvas.txt --layout grid
```

### Common Workflow Patterns

#### Pattern 1: Generate → View

```bash
# Simple: generate and view
./connectors/periodic2canvas > periodic.txt
./boxes-live  # F3 → periodic.txt
```

#### Pattern 2: Generate → Manipulate → View

```bash
# Generate base canvas
./connectors/git2canvas --max 20 > git_history.txt

# Add custom annotations
./connectors/boxes-cli add git_history.txt \
    --x 500 --y 100 \
    --title "RELEASE v1.0" \
    --content "Major milestone" \
    --color 1

# View result
./boxes-live  # F3 → git_history.txt
```

#### Pattern 3: Generate → Manipulate → Export

```bash
# Generate from data source
./connectors/docker2canvas > containers.txt

# Add summary box
./connectors/boxes-cli add containers.txt \
    --x 100 --y 50 \
    --title "Container Overview" \
    --content "Production Environment" "Last Updated: $(date)"

# Export to markdown report
./connectors/boxes-cli export containers.txt \
    --format markdown -o container_report.md
```

#### Pattern 4: Continuous Monitoring

```bash
#!/bin/bash
# Live monitoring with auto-refresh

CANVAS="live_monitor.txt"

while true; do
    # Generate fresh canvas
    ./connectors/pstree2canvas > "$CANVAS"

    # Add timestamp
    ./connectors/boxes-cli add "$CANVAS" \
        --x 50 --y 50 \
        --title "System Monitor" \
        --content "Updated: $(date)"

    # Auto-reload in boxes-live (send SIGUSR1 signal)
    pkill -SIGUSR1 boxes-live

    sleep 10
done
```

## Developing Custom Connectors

### Connector Template (Bash)

```bash
#!/bin/bash
# myconnector - Convert my data format to boxes-live canvas
#
# Usage: myconnector [input_file] > output.txt
#        cat data | myconnector > output.txt

set -euo pipefail

# Configuration
WORLD_WIDTH=2000
WORLD_HEIGHT=1500
BOX_WIDTH=30
BOX_HEIGHT=8

# Read input
INPUT="${1:--}"
if [ "$INPUT" = "-" ]; then
    LINES=$(cat)
else
    LINES=$(cat "$INPUT")
fi

# Count items
ITEM_COUNT=$(echo "$LINES" | wc -l)

# Output canvas header
echo "BOXES_CANVAS_V1"
echo "$WORLD_WIDTH $WORLD_HEIGHT"
echo "$ITEM_COUNT"

# Process each line and generate boxes
BOX_ID=1
Y_POS=100

# Use process substitution to avoid subshell issues
while read -r line; do
    # Extract data from line (customize for your format)
    TITLE=$(echo "$line" | cut -d',' -f1)
    CONTENT=$(echo "$line" | cut -d',' -f2)

    # Calculate position
    X_POS=$((100 + (BOX_ID % 5) * 200))

    # Choose color based on content (customize logic)
    COLOR=2  # Default green

    # Output box in canvas format
    echo "$BOX_ID $X_POS $Y_POS $BOX_WIDTH $BOX_HEIGHT 0 $COLOR"
    echo "$TITLE"
    echo "1"  # Number of content lines
    echo "$CONTENT"

    BOX_ID=$((BOX_ID + 1))
    Y_POS=$((Y_POS + 100))
done < <(echo "$LINES")
```

### Connector Template (Python)

```python
#!/usr/bin/env python3
"""
myconnector - Convert my data format to boxes-live canvas

Usage:
    myconnector [input_file] > output.txt
    cat data.json | myconnector > output.txt
"""

import sys
import json
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description='Convert data to canvas')
    parser.add_argument('input', nargs='?', default='-',
                       help='Input file (or stdin)')
    parser.add_argument('--layout', choices=['grid', 'tree'],
                       default='grid', help='Layout algorithm')
    parser.add_argument('--color', type=int, default=2,
                       help='Default color (0-7)')
    return parser.parse_args()

def read_input(filename):
    """Read and parse input data"""
    if filename == '-':
        data = json.load(sys.stdin)
    else:
        with open(filename, 'r') as f:
            data = json.load(f)
    return data

def calculate_position(index, layout='grid'):
    """Calculate box position based on layout"""
    if layout == 'grid':
        col = index % 5
        row = index // 5
        x = 100 + col * 200
        y = 100 + row * 120
    else:  # tree
        level = index // 3
        pos_in_level = index % 3
        x = 100 + pos_in_level * 300
        y = 100 + level * 150

    return x, y

def choose_color(item):
    """Choose color based on item properties"""
    # Customize this logic for your data
    if 'priority' in item:
        priority = item['priority'].lower()
        if priority == 'high':
            return 1  # Red
        elif priority == 'medium':
            return 3  # Yellow
        else:
            return 2  # Green
    return 2  # Default green

def generate_canvas(data, args):
    """Generate canvas from data"""

    # Header
    print("BOXES_CANVAS_V1")

    # World dimensions
    world_width = 2000
    world_height = 1500
    print(f"{world_width} {world_height}")

    # Box count
    print(len(data))

    # Generate boxes
    for idx, item in enumerate(data, 1):
        x, y = calculate_position(idx - 1, args.layout)

        # Box properties
        width = 30
        height = 8
        selected = 0
        color = args.color if args.color is not None else choose_color(item)

        # Title and content
        title = item.get('title', f'Item {idx}')[:50]
        content_lines = []

        # Add content based on item structure
        for key, value in item.items():
            if key != 'title':
                content_lines.append(f"{key}: {value}")

        # Output box
        print(f"{idx} {x} {y} {width} {height} {selected} {color}")
        print(title)
        print(len(content_lines))
        for line in content_lines:
            print(line[:60])

def main():
    args = parse_args()

    try:
        data = read_input(args.input)
        generate_canvas(data, args)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
```

### Advanced Techniques

#### Dynamic Layouts

```python
def layout_hierarchical(items, parent_map):
    """Arrange items in hierarchical tree layout"""
    levels = {}

    def assign_level(item_id, level=0):
        if item_id not in levels:
            levels[item_id] = level
            children = parent_map.get(item_id, [])
            for child in children:
                assign_level(child, level + 1)

    # Start from root items
    roots = [item_id for item_id in items if item_id not in parent_map]
    for root in roots:
        assign_level(root)

    # Calculate positions
    positions = {}
    level_counts = {}

    for item_id, level in levels.items():
        count = level_counts.get(level, 0)
        x = 100 + level * 300
        y = 100 + count * 120
        positions[item_id] = (x, y)
        level_counts[level] = count + 1

    return positions
```

#### Smart Color Coding

```python
def get_semantic_color(item):
    """Choose color based on semantic meaning"""

    # Status-based
    status = item.get('status', '').lower()
    if status in ['error', 'failed', 'critical']:
        return 1  # Red
    elif status in ['warning', 'pending']:
        return 3  # Yellow
    elif status in ['success', 'running', 'active']:
        return 2  # Green
    elif status in ['info', 'completed']:
        return 4  # Blue

    # Type-based
    item_type = item.get('type', '').lower()
    if item_type in ['system', 'core']:
        return 4  # Blue
    elif item_type in ['user', 'custom']:
        return 2  # Green
    elif item_type in ['daemon', 'service']:
        return 6  # Cyan

    return 0  # Default
```

#### Content Formatting

```python
def format_content_lines(item, max_lines=10):
    """Format item data as content lines"""
    lines = []

    # Add metadata
    if 'created' in item:
        lines.append(f"Created: {item['created']}")
    if 'updated' in item:
        lines.append(f"Updated: {item['updated']}")

    # Add description (word-wrapped)
    if 'description' in item:
        desc = item['description']
        words = desc.split()
        current_line = []
        for word in words:
            if len(' '.join(current_line + [word])) > 60:
                lines.append(' '.join(current_line))
                current_line = [word]
            else:
                current_line.append(word)
        if current_line:
            lines.append(' '.join(current_line))

    # Truncate if too long
    if len(lines) > max_lines:
        lines = lines[:max_lines-1] + ['...']

    return lines
```

## Canvas File Format

### Format Specification

```
BOXES_CANVAS_V1
<world_width> <world_height>
<box_count>
<box_1_data>
<box_2_data>
...
```

### Box Format

Each box consists of 3+ lines:

```
<id> <x> <y> <width> <height> <selected> <color>
<title>
<content_line_count>
<content_line_1>
<content_line_2>
...
```

**Field Descriptions:**

- `id`: Unique integer identifier (1-based)
- `x`, `y`: Position in world coordinates (float)
- `width`, `height`: Box dimensions in characters (int)
- `selected`: Selection state (0 or 1)
- `color`: Color code (0-7)
  - 0: Default/white
  - 1: Red
  - 2: Green
  - 3: Yellow
  - 4: Blue
  - 5: Magenta
  - 6: Cyan
  - 7: White
- `title`: Single-line title (max 50 chars recommended)
- `content_line_count`: Number of content lines
- `content_line_N`: Content lines (max 60 chars recommended)

### Example Canvas

```
BOXES_CANVAS_V1
2000.0 1500.0
3
1 100.0 100.0 30 8 0 2
Web Server
3
Status: Running
Port: 8080
Uptime: 24h
2 350.0 100.0 30 8 0 4
Database
2
Status: Active
Connections: 42
3 600.0 100.0 30 8 0 1
Cache
2
Status: Error
Error: Connection timeout
```

## Best Practices

### Design Principles

1. **Single Responsibility**: Each connector does one thing well
2. **Fail Fast**: Exit with error on invalid input
3. **Pipe-Friendly**: Accept stdin, output to stdout
4. **Error to Stderr**: Errors and warnings to stderr only
5. **Help Text**: Provide `--help` option
6. **Format Detection**: Auto-detect input format when possible

### Error Handling

```bash
# Bash error handling
set -euo pipefail  # Exit on error, undefined vars, pipe failures

# Check for required tools
if ! command -v jq &> /dev/null; then
    echo "Error: jq is required but not installed" >&2
    exit 1
fi

# Validate input
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File not found: $INPUT_FILE" >&2
    exit 1
fi

# Check for empty input
if [ ! -s "$INPUT_FILE" ]; then
    echo "Error: Input file is empty" >&2
    exit 1
fi
```

```python
# Python error handling
import sys

def main():
    try:
        data = parse_input()
        if not data:
            raise ValueError("No data to process")

        generate_canvas(data)

    except FileNotFoundError as e:
        print(f"Error: File not found: {e}", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid data: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
```

### Layout Guidelines

**Grid Layout**: Best for unrelated items
```
Item1  Item2  Item3  Item4  Item5
Item6  Item7  Item8  Item9  Item10
```

**Tree Layout**: Best for hierarchical data
```
        Root
         |
    +---------+
    |    |    |
  Child1 Child2 Child3
```

**Flow Layout**: Best for sequential processes
```
Step1 → Step2 → Step3 → Step4
```

### Color Guidelines

Use colors consistently:

- **Red (1)**: Errors, critical items, high priority
- **Green (2)**: Success, running, normal state, user items
- **Yellow (3)**: Warnings, pending, medium priority
- **Blue (4)**: Info, system items, stable state
- **Magenta (5)**: Special, custom, misc
- **Cyan (6)**: Services, daemons, background
- **White (7)**: Default, neutral

### Performance Considerations

- **Limit Box Count**: Default to reasonable limits (100-500 boxes)
- **Provide `--max` option**: Allow users to limit output
- **Stream Processing**: Process line-by-line for large inputs
- **Lazy Evaluation**: Don't load entire dataset if not needed

Example:
```python
def generate_canvas_streaming(input_stream, max_boxes=100):
    """Process input line-by-line to handle large files"""
    boxes = []

    for idx, line in enumerate(input_stream):
        if idx >= max_boxes:
            break
        boxes.append(parse_line(line))

    output_canvas(boxes)
```

## Testing Connectors

### Test Template

Create `tests/test_myconnector.sh`:

```bash
#!/bin/bash
# test_myconnector.sh - Test suite for myconnector

set -e

CONNECTOR="./connectors/myconnector"
CLI="./connectors/boxes-cli"
FAIL_COUNT=0
PASS_COUNT=0

# Test helpers (same as other test files)
test_start() { echo -e "\n[TEST] $1"; }
assert_success() {
    if [ $? -eq 0 ]; then
        echo "  ✓ $1"; ((PASS_COUNT++))
    else
        echo "  ✗ $1"; ((FAIL_COUNT++))
    fi
}

# Cleanup
cleanup() {
    rm -f test_myconnector_*.txt
}
trap cleanup EXIT

# Test 1: Basic execution
test_start "Basic connector execution"
echo "test,data" | $CONNECTOR > test_myconnector_output.txt
assert_success "Connector runs successfully"

# Test 2: Valid canvas format
test_start "Generates valid canvas format"
head -1 test_myconnector_output.txt | grep -q "BOXES_CANVAS_V1"
assert_success "Canvas header present"

# Test 3: CLI integration
test_start "boxes-cli can read output"
$CLI stats test_myconnector_output.txt > /dev/null
assert_success "boxes-cli stats works"

# Summary
echo ""
echo "Passed: $PASS_COUNT"
echo "Failed: $FAIL_COUNT"
[ $FAIL_COUNT -eq 0 ] && exit 0 || exit 1
```

### Testing Checklist

- [ ] Runs without errors on valid input
- [ ] Generates valid canvas format (BOXES_CANVAS_V1 header)
- [ ] Handles empty input gracefully (error, not crash)
- [ ] Handles invalid input gracefully
- [ ] Provides `--help` text
- [ ] boxes-cli can read generated canvas
- [ ] Box count matches expected
- [ ] Colors are applied correctly
- [ ] Layout is reasonable
- [ ] Performance is acceptable for typical data

## Connector Reference

### pstree2canvas

Convert process tree to visual canvas.

```bash
# From live system
pstree -p | ./connectors/pstree2canvas > processes.txt

# With options
pstree -p | ./connectors/pstree2canvas \
    --layout tree \
    --max-depth 3 \
    > processes.txt
```

**Options:**
- `--format [auto|pstree|ps]`: Input format
- `--layout [tree|grid]`: Layout style
- `--max-depth N`: Maximum tree depth
- `--spacing N`: Box spacing

**Colors:**
- Blue: Root processes (init, systemd)
- Cyan: Daemons (sshd, nginx)
- Yellow: Shells (bash, zsh)
- Green: User processes
- Magenta: Background workers

### log2canvas

Parse log files into visual timeline.

```bash
# Generic logs
./connectors/log2canvas /var/log/syslog > logs.txt

# JSON logs
./connectors/log2canvas app.log --format json > logs.txt

# Limit entries
./connectors/log2canvas app.log --max 50 > logs.txt
```

**Options:**
- `--format [auto|generic|json|syslog]`: Log format
- `--layout [timeline|grid]`: Layout style
- `--max N`: Maximum entries

**Colors:**
- Red: ERROR, CRITICAL
- Yellow: WARNING
- Blue: INFO
- Green: DEBUG

### docker2canvas

Visualize Docker containers.

```bash
# From docker ps
docker ps | ./connectors/docker2canvas > containers.txt

# From JSON
docker ps --format json | ./connectors/docker2canvas --format json > containers.txt
```

**Options:**
- `--format [auto|table|json]`: Input format
- `--layout [status|grid]`: Layout style
- `--show-ports`: Include port mappings

**Colors:**
- Green: Running containers
- Yellow: Clean exits (code 0)
- Red: Error exits (code != 0)

### boxes-cli

Command-line canvas manipulation tool.

```bash
# Create canvas
./connectors/boxes-cli create canvas.txt

# Add box
./connectors/boxes-cli add canvas.txt \
    --x 100 --y 100 \
    --title "Task" \
    --content "Description" \
    --color 2

# List boxes
./connectors/boxes-cli list canvas.txt
./connectors/boxes-cli list canvas.txt --color 1  # Filter by color
./connectors/boxes-cli list canvas.txt --json     # JSON output

# Search
./connectors/boxes-cli search canvas.txt "keyword"

# Export
./connectors/boxes-cli export canvas.txt --format markdown -o output.md
./connectors/boxes-cli export canvas.txt --format json -o data.json
./connectors/boxes-cli export canvas.txt --format csv -o data.csv

# Statistics
./connectors/boxes-cli stats canvas.txt
./connectors/boxes-cli stats canvas.txt --json
```

See `./connectors/boxes-cli --help` for full options.

### Other Connectors

- **csv2canvas**: Convert CSV files to canvas
- **json2canvas**: Convert JSON data to canvas
- **git2canvas**: Visualize git commit history
- **tree2canvas**: Visualize directory structure
- **text2canvas**: Convert text files to boxes
- **periodic2canvas**: Generate periodic table

Each supports `--help` for detailed usage.

## Advanced Topics

### Creating MCP Connectors

For integration with Claude and other AI tools via Model Context Protocol (MCP):

```python
#!/usr/bin/env python3
"""
MCP connector for boxes-live
Exposes canvas operations via MCP protocol
"""

from mcp import MCPServer, Tool

class BoxesLiveMCP(MCPServer):
    def __init__(self):
        super().__init__("boxes-live")

        self.register_tool(Tool(
            name="create_canvas",
            description="Create new canvas visualization",
            parameters={
                "data": "Data to visualize",
                "layout": "Layout algorithm (grid/tree/flow)"
            },
            handler=self.create_canvas
        ))

    def create_canvas(self, data, layout="grid"):
        # Generate canvas using connector
        # Return path to canvas file
        pass
```

See [MCP_SERVER.md](../MCP_SERVER.md) for complete MCP integration guide.

### Plugin Architecture

For live-updating connectors that refresh automatically:

```bash
#!/bin/bash
# Live-updating connector plugin

CANVAS="monitor.txt"
UPDATE_INTERVAL=5

while true; do
    ./connectors/myconnector > "$CANVAS"

    # Signal boxes-live to reload
    pkill -SIGUSR1 boxes-live

    sleep $UPDATE_INTERVAL
done
```

See [PLUGIN_ARCHITECTURE.md](../PLUGIN_ARCHITECTURE.md) for details.

## Getting Help

- **Documentation**: See [connectors/README.md](../connectors/README.md)
- **Examples**: See [docs/WORKFLOW-EXAMPLES.md](WORKFLOW-EXAMPLES.md)
- **Testing Guide**: See [TESTING.md](../TESTING.md)
- **Issues**: https://github.com/jcaldwell1066/boxes-live/issues

## Contributing

We welcome new connectors! To contribute:

1. Follow the connector template above
2. Add comprehensive tests
3. Document usage in connector header
4. Update this guide with your connector
5. Submit pull request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.
