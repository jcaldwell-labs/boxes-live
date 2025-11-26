# Test Canvas Files

This directory contains example canvas files for testing and demonstration purposes.

## Files

### empty.txt
- **Purpose**: Minimal empty canvas for testing canvas initialization
- **Boxes**: 0
- **Connections**: 0
- **Grid**: Disabled (default settings)
- **Use case**: Starting point for new users, testing empty state handling

### flowchart-example.txt
- **Purpose**: Demonstrates a simple decision flowchart
- **Boxes**: 5 (Start Process, Decision Point, Yes Path, No Path, End Process)
- **Connections**: 4 (connecting the flowchart steps)
- **Grid**: Visible and snap enabled (spacing: 10)
- **Use case**: Example of process flow visualization, testing connections

### workspace-layout.txt
- **Purpose**: Example project management workspace
- **Boxes**: 8 (TODO, In Progress, Completed, Notes, Architecture, etc.)
- **Connections**: 3 (showing relationships between components)
- **Grid**: Visible only, snap disabled (spacing: 10)
- **Use case**: Kanban-style board, project planning, testing complex layouts

### stress-test.txt
- **Purpose**: Performance testing with large number of boxes
- **Boxes**: 120 (in 12×10 grid pattern)
- **Connections**: 0
- **Grid**: Visible and snap enabled (spacing: 20)
- **Use case**: Performance testing, scroll testing, memory testing
- **Notes**: Tests rendering performance, viewport culling, memory usage

## Usage

### Loading Test Canvases

**In boxes-live**:
1. Start application: `./boxes-live`
2. Press **F3** to load
3. Enter path: `test-canvases/flowchart-example.txt`

**Command line** (future):
```bash
./boxes-live --load test-canvases/workspace-layout.txt
```

### Using in UAT

These canvases are used in automated UAT scenarios:
```bash
# Test all scenarios including canvas loading
./test-uat.sh all

# Test just canvas loading
./test-uat.sh scenario3
```

### Creating Custom Test Canvases

Use the boxes-live application or create manually following the format:

```
BOXES_CANVAS_V1
<world_width> <world_height>
<box_count>
<box1_id> <x> <y> <width> <height> <selected> <color>
"<title>"
<content_line_count>
<content_line_1>
...
<next_id> <selected_index>
CONNECTIONS
<connection_count>
<conn_id> <source_id> <dest_id> <color>
...
<next_conn_id>
GRID
<visible> <snap_enabled> <spacing>
```

See [TESTING.md](../TESTING.md) for more details on canvas file format.

## Grid Settings Explained

| Canvas | Visible | Snap | Spacing | Purpose |
|--------|---------|------|---------|---------|
| empty.txt | No | No | 10 | Clean slate |
| flowchart-example.txt | Yes | Yes | 10 | Aligned flowchart |
| workspace-layout.txt | Yes | No | 10 | Free positioning with guide |
| stress-test.txt | Yes | Yes | 20 | Large grid for many boxes |

### Grid Parameters

- **visible**: 1 = grid dots shown, 0 = hidden
- **snap_enabled**: 1 = boxes snap to grid, 0 = free positioning
- **spacing**: Grid spacing in world units (5, 10, 15, 20 common)

## Testing Guidelines

### Scenario 1: First-Time User
- Use: `empty.txt`
- Task: Create 3 boxes, apply colors, save

### Scenario 2: Grid Functionality
- Use: `flowchart-example.txt`
- Task: Verify grid visible, snap working, can toggle

### Scenario 3: Productivity Workflow
- Use: `workspace-layout.txt`
- Task: Organize workspace, use grid for alignment

### Scenario 5: Performance Testing
- Use: `stress-test.txt`
- Task: Pan/zoom with 120 boxes, measure lag

## File Format Notes

### Version 1 (Current)
- Magic: `BOXES_CANVAS_V1`
- Includes: Boxes, connections, grid settings
- Backward compatible: Grid section optional

### Adding New Test Cases

When adding test canvases:
1. Name descriptively: `feature-name-test.txt`
2. Include GRID section (for consistency)
3. Add comment in this README
4. Update UAT scenarios if relevant

## Known Issues

### Grid Persistence (FIXED)
- ✅ Grid settings now save/load correctly
- Previously: Grid state was lost on save/load
- Fix: Added GRID section to persistence format

### Connection Validation
- Connections to non-existent boxes are skipped during load
- This prevents crashes but may silently drop invalid connections

## Related Documentation

- [UAT_TESTING_GUIDE.md](../UAT_TESTING_GUIDE.md) - Testing scenarios
- [QUICK_START.md](../QUICK_START.md) - User guide
- [TESTING.md](../TESTING.md) - Automated testing
- [FEEDBACK_FORM.md](../FEEDBACK_FORM.md) - User feedback template

## Contributing

To add test canvases:
1. Create meaningful scenarios
2. Test they load correctly: `./boxes-live` → F3 → load file
3. Verify grid settings appropriate for use case
4. Add to this README with description
5. Update UAT scenarios if applicable
