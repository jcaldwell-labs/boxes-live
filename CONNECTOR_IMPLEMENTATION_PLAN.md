# Connector Implementation Plan

## Overview

This document outlines the implementation plan for new connector type todos, following the established patterns and specifications in the boxes-live project.

## Common Specification

All connectors follow the **BOXES_CANVAS_V1** format:

```
BOXES_CANVAS_V1
<world_width> <world_height>
<box_count>

<id> <x> <y> <width> <height> <selected> <color>
<title>
<content_line_count>
<content_line_1>
<content_line_2>
...
```

## Implementation Pattern

Each connector follows this structure:

### 1. **Connector Script Structure**
- Shebang line (#!/bin/bash or #!/usr/bin/env python3)
- Header comment with usage instructions
- Input validation
- Data parsing/transformation
- Auto-layout algorithm (grid, flow, tree, circle)
- Canvas format output generation

### 2. **Auto-Layout Algorithms**
- **Grid Layout**: `x = base_x + (index % cols) * (width + spacing)`
- **Flow Layout**: Left-to-right, wrapping at max width
- **Tree Layout**: Hierarchical with parent-child relationships
- **Circle Layout**: Radial arrangement around center

### 3. **Color Coding Strategy**
- Hash-based: `color = hash(category) % 7 + 1`
- Priority-based: Critical=1 (red), Warning=3 (yellow), Info=4 (blue)
- Category-based: Map categories to specific color codes (1-7)

### 4. **Box Dimensions**
- Default width: 30-35 characters
- Default height: 6-8 lines
- Adaptive height based on content length

## Test Approach

Each connector has a corresponding test script following the pattern in `tests/test_cli.sh`:

```bash
#!/bin/bash
set -e

# Test helper functions
test_start() { echo -e "\n[TEST] $1"; }
assert_success() { [ $? -eq 0 ] && echo "✓ $1" || echo "✗ $1"; }
assert_file_exists() { [ -f "$1" ] && echo "✓ File exists" || echo "✗ File missing"; }
assert_contains() { grep -q "$2" "$1" && echo "✓ Contains: $2" || echo "✗ Missing: $2"; }

# Test cases
test_start "Connector generates valid canvas"
./connectors/connector_name input.txt > output.txt
assert_success "Connector execution"
assert_contains output.txt "BOXES_CANVAS_V1"

test_start "Box count matches expected"
./connectors/boxes-cli stats output.txt --json | grep -q '"box_count": 5'
assert_success "Box count validation"
```

## Connectors to Implement

### Priority 1: Data Import Connectors

#### 1. log2canvas - Log File Analyzer
**Purpose**: Parse and visualize log patterns
**Input**: Log files (syslog, application logs, etc.)
**Layout**: Timeline (chronological flow)
**Color Coding**:
- ERROR: Red (1)
- WARNING: Yellow (3)
- INFO: Blue (4)
- DEBUG: Green (2)

**Features**:
- Parse common log formats (syslog, JSON logs, custom)
- Group by timestamp windows (e.g., every 5 minutes)
- Extract log level, message, source
- Timeline layout showing log flow

**Test Cases**:
- Parse syslog format
- Parse JSON logs
- Handle multi-line stack traces
- Color code by severity
- Verify chronological order

---

#### 2. json2canvas - JSON Data Visualizer
**Purpose**: Generic JSON to canvas converter
**Input**: JSON array of objects
**Layout**: Grid (5 columns)
**Color Coding**: Hash-based on object type/category

**Features**:
- Parse JSON array or object
- Auto-detect key fields (id, name, title)
- Convert nested objects to multi-line content
- Support custom field mapping via options

**Test Cases**:
- Parse flat JSON array
- Parse nested JSON objects
- Handle missing fields gracefully
- Verify grid layout positions
- Test color consistency

---

#### 3. docker2canvas - Docker Container Visualizer
**Purpose**: Visualize Docker containers and images
**Input**: `docker ps` or `docker images` output
**Layout**: Grid grouped by status
**Color Coding**:
- Running: Green (2)
- Stopped: Gray (0)
- Exited with error: Red (1)

**Features**:
- Parse `docker ps -a` output
- Show container ID, name, status, image
- Group by status (running, stopped, exited)
- Display resource usage if available

**Test Cases**:
- Parse docker ps output
- Handle containers with long names
- Group by status correctly
- Verify color coding by state
- Handle no containers case

---

#### 4. pstree2canvas - Process Tree Visualizer
**Purpose**: Visualize process hierarchy
**Input**: `pstree` or `ps` output
**Layout**: Tree (hierarchical)
**Color Coding**:
- Root processes: Blue (4)
- User processes: Green (2)
- System processes: Cyan (6)

**Features**:
- Parse pstree output or ps with parent PID
- Build hierarchical tree structure
- Show process name, PID, CPU/memory
- Indent child processes under parents

**Test Cases**:
- Parse pstree output
- Build correct parent-child relationships
- Verify tree layout (indentation)
- Handle deep hierarchies (>5 levels)
- Show process details in content

---

### Priority 2: Export Connectors (Future Work)

#### 5. canvas2dot - Graphviz Exporter
**Purpose**: Export canvas to Graphviz DOT format
**Input**: Canvas file
**Output**: DOT format for graph visualization

#### 6. canvas2svg - SVG Exporter
**Purpose**: Export canvas to SVG for web display
**Input**: Canvas file
**Output**: SVG with boxes and connections

---

## Development Workflow

For each connector:

1. **Implement Connector** (`connectors/connector_name`)
   - Follow template structure
   - Add usage examples in header
   - Implement input parsing
   - Implement layout algorithm
   - Generate BOXES_CANVAS_V1 output

2. **Create Test Suite** (`tests/test_connector_name.sh`)
   - Create sample input files
   - Test basic functionality
   - Test edge cases
   - Test format validation
   - Test integration with boxes-cli

3. **Update Documentation**
   - Add to `connectors/README.md`
   - Add usage example to `CONNECTORS.md`
   - Update main `README.md` if appropriate

4. **Manual Testing**
   - Run connector with real-world data
   - Load output in boxes-live
   - Verify visual appearance
   - Test with boxes-cli commands

5. **Commit and Push**
   - Commit connector + tests together
   - Clear commit message describing purpose
   - Push to feature branch

---

## Quality Checklist

Before considering a connector complete:

- [ ] Follows BOXES_CANVAS_V1 format exactly
- [ ] Has executable permissions (chmod +x)
- [ ] Has shebang line and usage documentation
- [ ] Handles invalid/malformed input gracefully
- [ ] Uses stderr for errors, stdout for canvas
- [ ] Provides sensible default layout
- [ ] Uses meaningful color coding
- [ ] Has comprehensive test suite (>5 test cases)
- [ ] Test suite passes all assertions
- [ ] Documented in README with examples
- [ ] Works with boxes-cli commands (list, search, export)
- [ ] Manually tested with boxes-live viewer

---

## Timeline

- **Phase 1**: log2canvas + tests (2-3 hours)
- **Phase 2**: json2canvas + tests (2-3 hours)
- **Phase 3**: docker2canvas + tests (2-3 hours)
- **Phase 4**: pstree2canvas + tests (3-4 hours)
- **Phase 5**: Documentation updates (1 hour)
- **Phase 6**: Integration testing (1 hour)

**Total Estimated Time**: 12-15 hours

---

## Success Criteria

Each connector is considered complete when:

1. Generates valid BOXES_CANVAS_V1 format
2. Implements appropriate auto-layout
3. Uses meaningful color coding
4. Has 100% passing test suite
5. Integrates with boxes-cli (stats, list, search work)
6. Can be loaded in boxes-live viewer
7. Has clear documentation with examples
8. Handles edge cases gracefully

---

## Notes

- All connectors are Unix philosophy tools (stdin/stdout)
- Focus on composability (pipe-able)
- Keep each connector focused on one task
- Prefer bash for simple connectors, Python for complex parsing
- Use existing connectors as reference implementations
- Test with real-world data when possible
