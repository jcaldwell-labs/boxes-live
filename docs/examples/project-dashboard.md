# Example: Project Dashboard Canvas

Create a task tracking dashboard using boxes-live.

## Overview

This example shows how to create a Kanban-style project dashboard with tasks organized by status.

## Manual Creation

### 1. Create Column Headers

Create three boxes for your columns:

```
Position: (50, 50)    Title: "TODO"      Color: Yellow (3)
Position: (200, 50)   Title: "DOING"     Color: Cyan (6)
Position: (350, 50)   Title: "DONE"      Color: Green (2)
```

### 2. Add Task Boxes

Create task boxes under each column:

```
Position: (50, 120)   Title: "Task 1"    Type: TASK
Position: (50, 200)   Title: "Task 2"    Type: TASK
Position: (200, 120)  Title: "Task 3"    Type: TASK
Position: (350, 120)  Title: "Task 4"    Type: TASK
```

### 3. Save

Press F2 and save as `dashboard.txt`.

## Using boxes-cli

Create the same dashboard programmatically:

```bash
# Create the canvas
./connectors/boxes-cli create dashboard.txt

# Add column headers
./connectors/boxes-cli add dashboard.txt --x 50 --y 50 --title "TODO" --color 3 --width 25
./connectors/boxes-cli add dashboard.txt --x 200 --y 50 --title "DOING" --color 6 --width 25
./connectors/boxes-cli add dashboard.txt --x 350 --y 50 --title "DONE" --color 2 --width 25

# Add tasks
./connectors/boxes-cli add dashboard.txt --x 50 --y 120 --title "Task 1" --content "Implement feature A"
./connectors/boxes-cli add dashboard.txt --x 50 --y 200 --title "Task 2" --content "Write tests"
./connectors/boxes-cli add dashboard.txt --x 200 --y 120 --title "Task 3" --content "Code review"
./connectors/boxes-cli add dashboard.txt --x 350 --y 120 --title "Task 4" --content "Deploy v1.0"

# View the result
./boxes-live  # Press F3, load dashboard.txt
```

## Canvas File Format

The resulting `dashboard.txt` looks like:

```
BOX
x: 50
y: 50
width: 25
height: 8
title: TODO
color: 3
content:
END

BOX
x: 200
y: 50
width: 25
height: 8
title: DOING
color: 6
content:
END

BOX
x: 350
y: 50
width: 25
height: 8
title: DONE
color: 2
content:
END

BOX
x: 50
y: 120
width: 20
height: 8
title: Task 1
color: 0
content: Implement feature A
END
```

## Workflow Integration

### Update from CI/CD

```bash
# Mark a task as done by moving it
./connectors/boxes-cli move dashboard.txt --title "Task 3" --x 350 --y 200
```

### Export to Markdown

```bash
./connectors/boxes-cli export dashboard.txt --format markdown > status.md
```

### Generate from issue tracker

```bash
# Custom script to pull from GitHub Issues
gh issue list --json title,state | \
  jq -r '.[] | "BOX\nx: \(if .state == "OPEN" then 50 else 350 end)\ny: \(.number * 80)\ntitle: \(.title)\nEND"' \
  > issues.txt
```

## Tips

- Use colors consistently for status (green=done, yellow=todo, etc.)
- Keep columns at fixed X positions for visual alignment
- Use box types (TASK) for items that have completion status
- Export regularly for backup and sharing
