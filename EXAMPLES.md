# boxes-live Usage Examples and Demonstrations

This document provides practical examples and ASCII demonstrations of boxes-live functionality.

## Table of Contents

- [Quick Start Examples](#quick-start-examples)
- [Visual Demonstrations](#visual-demonstrations)
- [Workflow Examples](#workflow-examples)
- [Advanced Use Cases](#advanced-use-cases)
- [Integration Examples](#integration-examples)

## Quick Start Examples

### Example 1: Basic Usage

```bash
# Start with sample canvas
./boxes-live

# Start with a specific file
./boxes-live my_project.txt

# Create a new canvas interactively
./boxes-live
# Press 'N' to create boxes
# Press 'F2' to save
```

### Example 2: Creating a Project Planning Board

```bash
# Start boxes-live
./boxes-live project_planning.txt

# Interactive steps:
1. Press 'N' to create "To Do" box
2. Click and drag it to position (10, 10)
3. Press '1' to color it RED

4. Press 'N' to create "In Progress" box
5. Drag to (50, 10)
6. Press '2' to color it GREEN

7. Press 'N' to create "Done" box
8. Drag to (90, 10)
9. Press '3' to color it BLUE

10. Press 'F2' to save
```

### Example 3: Loading Demo Files

```bash
# Load a pre-made demo
./boxes-live demos/process_tree.txt

# Explore using:
# - Arrow keys to pan
# - +/- to zoom in/out
# - Tab to cycle through boxes
```

## Visual Demonstrations

### Demo 1: Canvas View at Different Zoom Levels

**Zoom 1.0 (Normal View):**
```
┌──────────────────────────────────────────────────────────────────────┐
│                                                                      │
│  ┌───────────────────┐        ┌───────────────────┐                │
│  │  Project A        │        │  Project B        │                │
│  │                   │        │                   │                │
│  │  - Task 1         │        │  - Feature X      │                │
│  │  - Task 2         │        │  - Feature Y      │                │
│  │  - Task 3         │        │                   │                │
│  └───────────────────┘        └───────────────────┘                │
│                                                                      │
│       ┌─────────────────────────────┐                               │
│       │  Notes                      │                               │
│       │                             │                               │
│       │  Meeting on Friday          │                               │
│       │  Review design docs         │                               │
│       └─────────────────────────────┘                               │
│                                                                      │
│  Pos: (0.0, 0.0) | Zoom: 1.00x | Boxes: 3 | [N]ew [D]el [F2]Save  │
└──────────────────────────────────────────────────────────────────────┘
```

**Zoom 2.0 (Zoomed In):**
```
┌──────────────────────────────────────────────────────────────────────┐
│                                                                      │
│                                                                      │
│  ┌─────────────────────────────────────────────────┐               │
│  │  Project A                                       │               │
│  │                                                  │               │
│  │                                                  │               │
│  │  - Task 1                                        │               │
│  │                                                  │               │
│  │  - Task 2                                        │               │
│  │                                                  │               │
│  │  - Task 3                                        │               │
│  │                                                  │               │
│  └─────────────────────────────────────────────────┘               │
│                                                                      │
│                                                                      │
│  Pos: (0.0, 0.0) | Zoom: 2.00x | Boxes: 3 | [N]ew [D]el [F2]Save  │
└──────────────────────────────────────────────────────────────────────┘
```

**Zoom 0.5 (Zoomed Out - Bird's Eye View):**
```
┌──────────────────────────────────────────────────────────────────────┐
│                                                                      │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐   │
│  │ Prj │  │ Prj │  │ Prj │  │ Prj │  │ Prj │  │ Prj │  │ Prj │   │
│  │  A  │  │  B  │  │  C  │  │  D  │  │  E  │  │  F  │  │  G  │   │
│  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘   │
│                                                                      │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐                                │
│  │ Tsk │  │ Tsk │  │ Tsk │  │Note │                                │
│  │  1  │  │  2  │  │  3  │  │     │                                │
│  └─────┘  └─────┘  └─────┘  └─────┘                                │
│                                                                      │
│  ┌──────────────┐  ┌──────────────┐                                │
│  │   Archive    │  │   Ideas      │                                │
│  └──────────────┘  └──────────────┘                                │
│                                                                      │
│  Pos: (0.0, 0.0) | Zoom: 0.50x | Boxes: 13 | [N]ew [D]el [F2]Save │
└──────────────────────────────────────────────────────────────────────┘
```

### Demo 2: Box Selection and Colors

**Before Coloring:**
```
┌─────────────────────────────────────────────────┐
│  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │ To Do    │  │ Progress │  │ Done     │     │
│  │          │  │          │  │          │     │
│  │ - Fix #1 │  │ - Impl X │  │ - Test Y │     │
│  │ - Add #2 │  │          │  │          │     │
│  └──────────┘  └──────────┘  └──────────┘     │
│                                                 │
│ Select a box and press 1-7 to color it         │
└─────────────────────────────────────────────────┘
```

**After Coloring (represented with visual markers):**
```
┌─────────────────────────────────────────────────┐
│  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │ To Do    │  │ Progress │  │ Done     │     │
│  │ [RED]    │  │ [YELLOW] │  │ [GREEN]  │     │
│  │          │  │          │  │          │     │
│  │ - Fix #1 │  │ - Impl X │  │ - Test Y │     │
│  │ - Add #2 │  │          │  │          │     │
│  └──────────┘  └──────────┘  └──────────┘     │
│                                                 │
│ Colors help organize by status/priority        │
└─────────────────────────────────────────────────┘
```

### Demo 3: Mouse Drag and Drop

**Step 1: Click on box**
```
┌─────────────────────────────────────────────────┐
│                                                 │
│  ┌──────────┐                                   │
│  │ Task A   │ ← Clicked                         │
│  │ [SELECTED]                                   │
│  └──────────┘                                   │
│                                                 │
│                      Drag ─────►                │
│                                                 │
└─────────────────────────────────────────────────┘
```

**Step 2: Drag to new position**
```
┌─────────────────────────────────────────────────┐
│                                                 │
│                                                 │
│                                                 │
│                        ┌──────────┐             │
│                        │ Task A   │ ← Dropped   │
│                        │          │             │
│                        └──────────┘             │
│                                                 │
└─────────────────────────────────────────────────┘
```

## Workflow Examples

### Workflow 1: Agile Sprint Planning

```bash
# Create sprint board
./boxes-live sprint_1.txt

# Create columns
1. Create "Backlog" box (color: white)
2. Create "Sprint" box (color: yellow)
3. Create "In Progress" box (color: blue)
4. Create "Review" box (color: magenta)
5. Create "Done" box (color: green)

# Add stories
6. Create task boxes for each user story
7. Place them in "Backlog" column
8. Color-code by priority (red=high, yellow=medium, white=low)

# During sprint
9. Drag tasks from Backlog → Sprint
10. Move tasks through workflow as they progress
11. Save frequently with F2

# Sprint review
12. Zoom out to see full board
13. Export final state for documentation
```

### Workflow 2: System Architecture Diagram

```bash
./boxes-live architecture.txt

# Create layers
1. Top: "Frontend" box (color: blue)
2. Middle: "API Layer" box (color: green)
3. Bottom: "Database" box (color: red)

# Add components within layers
4. Create "React App" box inside Frontend area
5. Create "REST API" box in API Layer
6. Create "PostgreSQL" box in Database area

# Add supporting services
7. Create "Redis Cache" box (color: yellow)
8. Create "Message Queue" box (color: magenta)

# Document connections (in box content)
9. Edit boxes to note connections:
   "React App" → "Connects to: REST API"
   "REST API" → "Reads from: PostgreSQL, Redis"
```

### Workflow 3: Mind Mapping / Brainstorming

```bash
./boxes-live brainstorm.txt

# Central idea
1. Create central "Main Idea" box in center (50, 50)
2. Color it cyan
3. Make it larger (40x10 characters)

# Branch ideas
4. Create sub-topic boxes around main idea
5. Position them in a radial pattern:
   - North: "Technical Approach"
   - East: "Business Value"
   - South: "Risks & Challenges"
   - West: "Resources Needed"

# Sub-branches
6. For each sub-topic, add detail boxes
7. Use colors to indicate:
   - Green: Decided/confirmed
   - Yellow: Under discussion
   - Red: Blocked/issues

# Iterate
8. Add new boxes as ideas emerge
9. Rearrange by dragging
10. Delete bad ideas with 'D'
```

## Advanced Use Cases

### Use Case 1: Code Review Board

```bash
# Create review tracking canvas
./boxes-live code_review.txt

# File structure
1. Create boxes for each file under review
2. Color by status:
   - Red: Not reviewed
   - Yellow: In review
   - Green: Approved
   - Blue: Changes requested

# Review process
3. Add comments in box content
4. Move boxes through states by changing color
5. Track reviewer assignments in box titles

# Example box:
┌──────────────────────┐
│ auth.c [GREEN]       │
│                      │
│ Reviewer: Alice      │
│ Status: Approved     │
│ Comments:            │
│ - Good error handling│
│ - Tests passing      │
└──────────────────────┘
```

### Use Case 2: Network Topology Visualization

```bash
# Visualize server infrastructure
./boxes-live network_topology.txt

# Create network segments
1. Top row: "DMZ" boxes (web servers)
2. Middle row: "App Tier" boxes (application servers)
3. Bottom row: "Data Tier" boxes (databases)

# Add devices
4. Color-code by function:
   - Red: Firewalls
   - Blue: Web servers
   - Green: App servers
   - Yellow: Databases
   - Magenta: Load balancers

# Document connections in content
5. Each box lists its connections
6. Use position to show network flow
```

### Use Case 3: Meeting Notes / Agenda

```bash
./boxes-live team_meeting.txt

# Meeting structure
1. Top: "Agenda" box with topics
2. Left column: "Action Items" (red)
3. Right column: "Decisions" (green)
4. Bottom: "Parking Lot" (yellow)

# During meeting
5. Create new boxes for each discussion point
6. Move items to appropriate columns
7. Add owner names to action items
8. Save at end of meeting

# Post-meeting
9. Export action items to task tracker
10. Archive in meeting notes folder
```

## Integration Examples

### Integration 1: With Git Workflow

```bash
# Track git branches visually
git branch | ./connectors/git2canvas > git_status.txt
./boxes-live git_status.txt

# Manual workflow:
1. Create box for each branch
2. Position:
   - main branch: left
   - feature branches: middle
   - hotfix branches: right
3. Color by status:
   - Green: Merged
   - Yellow: Active
   - Red: Conflicts
```

### Integration 2: With Process Monitoring

```bash
# Visualize running processes
pstree -p | ./connectors/pstree2canvas > processes.txt
./boxes-live processes.txt

# Interactive monitoring:
1. Reload with F3 to refresh
2. Zoom to specific process subtrees
3. Add notes about high-CPU processes
4. Save snapshot for later analysis
```

### Integration 3: With Docker Containers

```bash
# Visualize container landscape
docker ps | ./connectors/docker2canvas > containers.txt
./boxes-live containers.txt

# Container management view:
1. Each container is a box
2. Color by status:
   - Green: Running
   - Red: Stopped
   - Yellow: Restarting
3. Content shows:
   - Image name
   - Ports
   - Resource usage
```

### Integration 4: Campaign Orchestration (tmux)

```bash
# Multi-pane game master session
./demos/launch_campaign.sh adventure_campaign

# Creates tmux layout:
┌─────────────────────────────────────────────┐
│ GM View (boxes-live)    │ Player View       │
│ - Full campaign map     │ - Current location│
│ - All entities          │ - Visible entities│
├─────────────────────────┼───────────────────┤
│ Realm Editor            │ Event Log         │
│ - Edit realms           │ - Combat log      │
│ - Update quests         │ - Narrative feed  │
└─────────────────────────┴───────────────────┘

# Workflow:
1. GM updates realm JSON files
2. realm2canvas connector transforms to canvas
3. Auto-sync via SIGUSR1 signal
4. All views update in real-time
```

## Keyboard Shortcuts Reference

### Navigation
```
Arrow Keys / WASD   Pan viewport
+/- or Z/X          Zoom in/out
R or 0              Reset view to origin
```

### Box Operations
```
N                   Create new box at viewport center
D                   Delete selected box
Tab                 Cycle through boxes
Mouse Click         Select box
Mouse Drag          Move box
```

### Box Styling
```
1                   Color box RED
2                   Color box GREEN
3                   Color box BLUE
4                   Color box YELLOW
5                   Color box MAGENTA
6                   Color box CYAN
7                   Color box WHITE
0                   Reset color to default (or reset view if nothing selected)
```

### File Operations
```
F2                  Save canvas to file
F3                  Load/Reload canvas from file
```

### Application
```
Q or ESC            Quit application
Ctrl+C              Force quit (signal)
```

## Tips and Tricks

### Tip 1: Organize Large Canvases

```
Use regions of the canvas for different purposes:

    (0,0)           (100,0)         (200,0)
      │               │               │
      ▼               ▼               ▼
  ┌─────────┐   ┌─────────┐   ┌─────────┐
  │ Current │   │ Future  │   │ Archive │
  │  Work   │   │  Ideas  │   │         │
  └─────────┘   └─────────┘   └─────────┘
```

### Tip 2: Color Coding System

```
Develop a consistent color scheme:
- Red (1): Urgent/Blocked
- Green (2): Complete/Approved
- Blue (3): In Progress
- Yellow (4): Needs Review
- Magenta (5): External Dependency
- Cyan (6): Reference/Documentation
- White (7): Low Priority
```

### Tip 3: Naming Convention

```
Use prefixes in box titles for easy scanning:
- [BUG] Critical login issue
- [FEAT] Add export feature
- [DOC] API documentation
- [TEST] Integration test suite
- [IDEA] Potential improvement
```

### Tip 4: Spatial Organization

```
Use vertical columns for workflow states:

    ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐
    │Todo │  │Doing│  │Done │  │Hold │
    ├─────┤  ├─────┤  ├─────┤  ├─────┤
    │ #1  │  │ #5  │  │ #9  │  │ #3  │
    │ #2  │  │ #6  │  │ #10 │  │ #4  │
    │ #7  │  │     │  │     │  │     │
    │ #8  │  │     │  │     │  │     │
    └─────┘  └─────┘  └─────┘  └─────┘
```

---

## See Also

- [README.md](README.md) - General project overview
- [USAGE.md](USAGE.md) - Detailed usage instructions
- [CONNECTORS.md](CONNECTORS.md) - Data connector ecosystem
- [CAMPAIGN_ORCHESTRATION.md](CAMPAIGN_ORCHESTRATION.md) - Game campaign setup

**Last Updated:** 2024-06-10
