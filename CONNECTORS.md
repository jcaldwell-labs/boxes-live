# Unix Philosophy Connectors and Demonstration Projects

## Overview

This document outlines connector designs and demonstration projects that showcase boxes-live as a universal terminal-based smart canvas. Following the Unix philosophy, we design small, focused tools that integrate via text-based interfaces, enabling composition and reuse.

## Unix Philosophy Principles

Our connector design follows these core tenets:

1. **Small, Focused Tools**: Each connector does one thing well
2. **Text-Based I/O**: Use stdin/stdout for data flow
3. **Composability**: Chain connectors via pipes
4. **Plain Text Formats**: Human-readable, grep-able data
5. **Filter Design**: Read canvas, transform, write canvas

## Canvas Text Format

All connectors use a standard text format for canvas data:

```
BOXES_CANVAS_V1
<world_width> <world_height>
<box_count>
<id> <x> <y> <width> <height> <selected> <color>
<title>
<content_line_count>
<line1>
<line2>
...
```

Example:
```
BOXES_CANVAS_V1
2000.0 1500.0
2
1 100.0 100.0 30 8 0 1
Hydrogen
2
H
1.008
2 200.0 100.0 30 8 0 2
Helium
2
He
4.003
```

## Connector Architecture

```
┌──────────────┐
│ Data Source  │ (CSV, JSON, database, API)
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Generator   │ (data → canvas format)
│  Connector   │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ boxes-live   │ (interactive editing)
│   canvas     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Exporter    │ (canvas → output format)
│  Connector   │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Output     │ (Markdown, SVG, PDF)
└──────────────┘
```

## Demonstration Projects

### 1. Periodic Table of Elements

**Purpose**: Scientific data visualization and navigation

**Data Source**: CSV or JSON with element data
```csv
atomic_number,symbol,name,atomic_mass,category
1,H,Hydrogen,1.008,nonmetal
2,He,Helium,4.003,noble_gas
```

**Connector**: `periodic2canvas`

```bash
#!/bin/bash
# periodic2canvas - Generate periodic table canvas from CSV

cat <<EOF
BOXES_CANVAS_V1
2000.0 800.0
118
EOF

awk -F',' 'NR>1 {
    # Calculate position based on group and period
    x = $5 * 80  # group * spacing
    y = $6 * 60  # period * spacing

    # Map category to color
    color = category_to_color($7)

    printf "%d %.1f %.1f 12 6 0 %d\n", NR-1, x, y, color
    printf "%s\n", $3  # name
    printf "3\n"
    printf "%s\n", $2   # symbol
    printf "%s\n", $1   # atomic number
    printf "%.3f\n", $4 # atomic mass
}' elements.csv
```

**Usage**:
```bash
periodic2canvas < elements.csv > periodic.txt
boxes-live  # F3 to load periodic.txt
```

**Features**:
- Color-coded by element category
- Navigate with pan/zoom
- Click elements for selection
- Educational tool for chemistry

### 2. UML Class Diagram

**Purpose**: Software architecture visualization

**Data Source**: Code parsing or PlantUML text

**Connector**: `uml2canvas`

```bash
#!/bin/bash
# uml2canvas - Convert PlantUML to interactive canvas

parse_plantuml() {
    local file=$1
    local box_id=1
    local y_pos=100

    echo "BOXES_CANVAS_V1"
    echo "3000.0 2000.0"

    # Count classes
    class_count=$(grep -c "^class " "$file")
    echo "$class_count"

    # Parse each class
    grep "^class " "$file" | while read -r line; do
        class_name=$(echo "$line" | awk '{print $2}')

        # Calculate position (auto-layout in grid)
        x=$((100 + (box_id % 5) * 200))
        y=$((100 + (box_id / 5) * 150))

        echo "$box_id $x $y 40 12 0 3"  # Blue for classes
        echo "$class_name"

        # Extract methods and fields
        echo "5"
        echo "Class"
        echo "━━━━━━━━━━━━"
        echo "Fields: ..."
        echo "Methods: ..."
        echo ""

        ((box_id++))
    done
}

parse_plantuml "$1"
```

**Usage**:
```bash
uml2canvas diagram.puml > uml.txt
boxes-live  # Load and arrange classes
```

**Future Enhancement**: Parse relationships and draw connection lines

### 3. Mermaid Diagram Converter

**Purpose**: Flowchart and diagram visualization

**Data Source**: Mermaid markdown syntax

**Connector**: `mermaid2canvas`

```bash
#!/usr/bin/env python3
# mermaid2canvas - Convert Mermaid flowcharts to canvas

import sys
import re

def parse_mermaid(content):
    boxes = []
    connections = []

    # Parse node definitions: A[Text in box]
    for match in re.finditer(r'(\w+)\[([^\]]+)\]', content):
        node_id = match.group(1)
        text = match.group(2)
        boxes.append({
            'id': node_id,
            'text': text,
            'type': 'rect'
        })

    # Parse connections: A --> B
    for match in re.finditer(r'(\w+)\s*-->\s*(\w+)', content):
        connections.append((match.group(1), match.group(2)))

    return boxes, connections

def auto_layout(boxes):
    """Simple top-to-bottom layout"""
    for i, box in enumerate(boxes):
        box['x'] = 150 + (i % 3) * 200
        box['y'] = 100 + (i // 3) * 120

def generate_canvas(boxes, connections):
    print("BOXES_CANVAS_V1")
    print("2000.0 1500.0")
    print(len(boxes))

    for i, box in enumerate(boxes, 1):
        print(f"{i} {box['x']} {box['y']} 35 8 0 4")  # Yellow
        print(box['text'])
        print("1")
        print(box['id'])

if __name__ == '__main__':
    content = sys.stdin.read()
    boxes, connections = parse_mermaid(content)
    auto_layout(boxes)
    generate_canvas(boxes, connections)
```

**Usage**:
```bash
cat flowchart.md | mermaid2canvas > flow.txt
boxes-live  # Load and refine layout
```

### 4. Patat Presentation Connector

**Purpose**: Convert slide presentations to navigable canvas

**Data Source**: Patat markdown presentations

**Connector**: `patat2canvas`

```bash
#!/usr/bin/env ruby
# patat2canvas - Convert Patat slides to canvas boxes

require 'yaml'

slides = []
current_slide = []

ARGF.each_line do |line|
  if line =~ /^---+$/  # Slide separator
    slides << current_slide unless current_slide.empty?
    current_slide = []
  else
    current_slide << line.chomp
  end
end
slides << current_slide unless current_slide.empty?

puts "BOXES_CANVAS_V1"
puts "#{slides.length * 150} 800.0"
puts slides.length

slides.each_with_index do |slide, i|
  x = 100 + (i * 140)
  y = 100

  # Extract title (first line) and content
  title = slide.first || "Slide #{i+1}"
  content = slide[1..-1] || []

  puts "#{i+1} #{x} #{y} 30 #{[content.length + 2, 6].max} 0 #{(i % 7) + 1}"
  puts title
  puts content.length
  content.each { |line| puts line }
end
```

**Usage**:
```bash
patat2canvas presentation.md > slides.txt
boxes-live  # Navigate slides spatially
```

**Interactive Features**:
- Linear slide flow becomes 2D spatial layout
- Rearrange slides by dragging
- Add speaker notes as new boxes
- Export back to presentation format

### 5. Unix Process Tree Visualizer

**Purpose**: System monitoring and process visualization

**Connector**: `pstree2canvas`

```bash
#!/bin/bash
# pstree2canvas - Visualize process tree on canvas

generate_process_boxes() {
    local depth=0
    local box_id=1

    echo "BOXES_CANVAS_V1"
    echo "2000.0 1500.0"

    # Count processes
    ps_count=$(ps axo pid,ppid,comm | wc -l)
    echo $((ps_count - 1))

    # Generate boxes for each process
    ps axo pid,ppid,comm | tail -n +2 | while read pid ppid comm; do
        # Calculate position based on parent-child relationship
        depth=$(get_process_depth $pid)
        x=$((100 + depth * 150))
        y=$((100 + (box_id % 10) * 80))

        # Color by process type
        color=2  # Default green
        [[ "$comm" =~ bash|zsh|sh ]] && color=3  # Blue for shells
        [[ "$comm" =~ systemd|init ]] && color=1  # Red for system

        echo "$box_id $x $y 25 6 0 $color"
        echo "$comm"
        echo "2"
        echo "PID: $pid"
        echo "PPID: $ppid"

        ((box_id++))
    done
}

generate_process_boxes
```

**Usage**:
```bash
pstree2canvas > processes.txt
boxes-live  # Explore process hierarchy
```

### 6. Git History Visualizer

**Purpose**: Navigate git commit history spatially

**Connector**: `git2canvas`

```bash
#!/bin/bash
# git2canvas - Git commit graph on canvas

git log --all --pretty=format:"%H|%P|%s|%an|%ar" --date=short | \
awk -F'|' '
BEGIN {
    print "BOXES_CANVAS_V1"
    print "5000.0 2000.0"
    print NR
}
{
    hash = $1
    parent = $2
    message = $3
    author = $4
    date = $5

    # Layout: chronological left-to-right
    x = 100 + (NR * 180)
    y = 200

    # Color by author (hash author name to color)
    color = ((length(author) % 7) + 1)

    print NR, x, y, 35, 8, 0, color
    print substr(message, 1, 30)
    print 3
    print substr(hash, 1, 7)
    print author
    print date
}'
```

**Usage**:
```bash
cd /path/to/repo
git2canvas > commits.txt
boxes-live  # Navigate commit history
```

## Export Connectors

### Canvas to Markdown

```bash
#!/bin/bash
# canvas2md - Export canvas to Markdown document

canvas_file=$1

# Skip header
tail -n +4 "$canvas_file" | while read line; do
    if [[ "$line" =~ ^[0-9]+ ]]; then
        # Box metadata line
        read title
        read content_count

        echo "## $title"
        echo

        for ((i=0; i<content_count; i++)); do
            read content_line
            echo "$content_line"
        done
        echo
    fi
done
```

### Canvas to DOT (Graphviz)

```bash
#!/bin/bash
# canvas2dot - Export to Graphviz format

echo "digraph Canvas {"
echo "  node [shape=box];"

# Parse boxes and create nodes
awk '/^[0-9]+ / {
    getline title
    print "  node" $1 " [label=\"" title "\"];"
}' canvas.txt

echo "}"
```

### Canvas to SVG

```python
#!/usr/bin/env python3
# canvas2svg - Export canvas to SVG

import sys

def parse_canvas(filename):
    with open(filename) as f:
        lines = f.readlines()

    # Parse header
    header = lines[0].strip()
    world_w, world_h = map(float, lines[1].split())
    box_count = int(lines[2])

    boxes = []
    i = 3
    while i < len(lines) and len(boxes) < box_count:
        parts = lines[i].split()
        box = {
            'id': int(parts[0]),
            'x': float(parts[1]),
            'y': float(parts[2]),
            'width': int(parts[3]),
            'height': int(parts[4]),
            'color': int(parts[6]),
            'title': lines[i+1].strip(),
            'content': []
        }

        content_lines = int(lines[i+2])
        for j in range(content_lines):
            box['content'].append(lines[i+3+j].strip())

        boxes.append(box)
        i += 3 + content_lines + 1

    return world_w, world_h, boxes

def generate_svg(world_w, world_h, boxes):
    colors = ['#666', '#f88', '#8f8', '#88f', '#ff8', '#f8f', '#8ff', '#fff']

    print(f'<svg width="{world_w}" height="{world_h}" xmlns="http://www.w3.org/2000/svg">')

    for box in boxes:
        x, y = box['x'], box['y']
        w, h = box['width'] * 8, box['height'] * 16  # Character size
        color = colors[box['color']]

        print(f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" ')
        print(f'        stroke="{color}" fill="none" stroke-width="2"/>')
        print(f'  <text x="{x+5}" y="{y+20}" font-family="monospace" font-size="14">')
        print(f'    {box["title"]}</text>')

    print('</svg>')

if __name__ == '__main__':
    world_w, world_h, boxes = parse_canvas(sys.argv[1])
    generate_svg(world_w, world_h, boxes)
```

## Bi-Directional Connectors

### CSV Sync

```bash
#!/bin/bash
# csv-canvas - Bi-directional CSV ↔ Canvas sync

csv2canvas() {
    local csv=$1
    echo "BOXES_CANVAS_V1"
    echo "2000.0 1000.0"

    line_count=$(($(wc -l < "$csv") - 1))
    echo "$line_count"

    tail -n +2 "$csv" | nl | while read num fields; do
        IFS=',' read -ra COLS <<< "$fields"

        x=$((100 + (num % 10) * 150))
        y=$((100 + (num / 10) * 100))

        echo "$num $x $y 25 6 0 2"
        echo "${COLS[0]}"  # First column as title
        echo "$((${#COLS[@]} - 1))"

        for ((i=1; i<${#COLS[@]}; i++)); do
            echo "${COLS[i]}"
        done
    done
}

canvas2csv() {
    local canvas=$1
    # Extract boxes and convert to CSV rows
    # (Implementation here)
}

case "$1" in
    to-canvas)   csv2canvas "$2" ;;
    from-canvas) canvas2csv "$2" ;;
esac
```

### JSON API Bridge

```python
#!/usr/bin/env python3
# api2canvas - Fetch JSON API data and create canvas

import sys
import json
import requests

def fetch_and_convert(api_url):
    response = requests.get(api_url)
    data = response.json()

    print("BOXES_CANVAS_V1")
    print("2000.0 1500.0")
    print(len(data))

    for i, item in enumerate(data):
        x = 100 + (i % 5) * 200
        y = 100 + (i // 5) * 120

        print(f"{i+1} {x} {y} 30 8 0 3")
        print(item.get('title', 'Untitled'))

        # Convert all fields to content lines
        fields = [f"{k}: {v}" for k, v in item.items() if k != 'title']
        print(len(fields))
        for field in fields:
            print(field)

if __name__ == '__main__':
    fetch_and_convert(sys.argv[1])
```

## Workflow Examples

### Research Paper Organization

```bash
# Extract citations from BibTeX
bibtex2canvas papers.bib > papers.txt

# Load and arrange papers spatially by topic
boxes-live  # Organize papers, add notes

# Export organized structure to Markdown
canvas2md papers.txt > research_outline.md
```

### Project Planning

```bash
# Import tasks from issue tracker
gh issue list --json title,body | jq2canvas > tasks.txt

# Organize on canvas (group by milestone)
boxes-live  # Drag tasks into swim lanes

# Export to Markdown roadmap
canvas2md tasks.txt > ROADMAP.md
```

### Data Exploration

```bash
# Load dataset
csv2canvas data.csv > viz.txt

# Explore interactively (filter, group, annotate)
boxes-live

# Export findings
canvas2md viz.txt > analysis.md
canvas2svg viz.txt > dashboard.svg
```

## Future Connector Ideas

1. **Database Query Results**: `sql2canvas` - Visualize query results
2. **Log File Analyzer**: `log2canvas` - Parse and visualize log patterns
3. **Network Topology**: `netstat2canvas` - Visualize network connections
4. **File System Navigator**: `tree2canvas` - Directory structure visualization
5. **Calendar/Schedule**: `cal2canvas` - Timeline visualization
6. **Mind Map**: `mindmap2canvas` - Hierarchical thought organization
7. **Code Call Graph**: `callgraph2canvas` - Function relationships
8. **Docker Containers**: `docker2canvas` - Container/image visualization
9. **Kubernetes Pods**: `kubectl2canvas` - Cluster state visualization
10. **Ansible Playbooks**: `ansible2canvas` - Infrastructure as canvas

## Connector Development Guidelines

When creating new connectors:

1. **Input Validation**: Handle malformed data gracefully
2. **Auto-Layout**: Provide sensible default positioning
3. **Color Coding**: Use colors meaningfully (categories, priorities, types)
4. **Metadata Preservation**: Round-trip without data loss
5. **Documentation**: Include usage examples in connector header
6. **Error Handling**: Clear error messages to stderr
7. **Streaming**: Support large datasets via streaming
8. **Composability**: Design for pipeline integration

## Standard Connector Template

```bash
#!/bin/bash
# connector-name - Brief description
#
# Usage: connector-name [options] < input > output
# Example: connector-name data.csv > canvas.txt

set -euo pipefail

show_help() {
    grep '^#' "$0" | tail -n +2 | cut -c3-
}

parse_input() {
    # Your parsing logic here
    :
}

generate_canvas() {
    echo "BOXES_CANVAS_V1"
    echo "2000.0 1500.0"
    # Your box generation here
}

main() {
    case "${1:-}" in
        -h|--help) show_help; exit 0 ;;
        *) parse_input | generate_canvas ;;
    esac
}

main "$@"
```

## Conclusion

These Unix philosophy-based connectors transform boxes-live from an interactive canvas into a universal visualization and organization tool. By following simple text-based interfaces and composable design, we create an ecosystem of tools that integrate seamlessly with existing Unix workflows.

The demonstration projects showcase real-world applications across domains: education (periodic table), software engineering (UML), documentation (Mermaid), presentations (Patat), and system administration (process trees). Each connector adheres to the Unix philosophy while extending the smart canvas's reach.
