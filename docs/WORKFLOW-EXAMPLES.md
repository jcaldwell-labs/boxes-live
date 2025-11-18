# Workflow Examples

**Real-World Use Cases for boxes-live Connectors**

This document demonstrates practical workflows using boxes-live connectors for various real-world scenarios.

## Table of Contents

- [System Administration](#system-administration)
- [DevOps and Monitoring](#devops-and-monitoring)
- [Software Development](#software-development)
- [Data Analysis](#data-analysis)
- [Project Management](#project-management)
- [Education and Learning](#education-and-learning)

## System Administration

### Workflow 1: System Process Monitoring

**Goal**: Monitor and visualize system processes in real-time.

**Steps:**

```bash
#!/bin/bash
# monitor-processes.sh - Live process monitoring

CANVAS="processes_live.txt"

# Generate initial canvas
pstree -p | ./connectors/pstree2canvas --max-depth 3 > "$CANVAS"

# Add system summary box
./connectors/boxes-cli add "$CANVAS" \
    --x 50 --y 50 \
    --width 40 --height 10 \
    --title "System Overview" \
    --color 6 \
    --content \
    "Hostname: $(hostname)" \
    "Uptime: $(uptime -p)" \
    "Load: $(uptime | awk -F'load average:' '{print $2}')" \
    "Memory: $(free -h | awk '/^Mem:/ {print $3 "/" $2}')" \
    "" \
    "Updated: $(date)"

# View interactively
./boxes-live  # F3 → processes_live.txt

# For continuous monitoring, run in loop:
# while true; do
#     pstree -p | ./connectors/pstree2canvas --max-depth 3 > "$CANVAS"
#     # Add summary box (as above)
#     pkill -SIGUSR1 boxes-live  # Trigger reload
#     sleep 10
# done
```

**Output**: Interactive canvas showing:
- Process hierarchy with PIDs
- Color-coded by type (system, user, daemons)
- System summary box with live stats
- Auto-refreshing every 10 seconds

### Workflow 2: Container Fleet Management

**Goal**: Visualize Docker container status across environment.

```bash
#!/bin/bash
# container-dashboard.sh - Docker container dashboard

CANVAS="containers.txt"

# Generate container canvas
docker ps -a | ./connectors/docker2canvas --layout status > "$CANVAS"

# Add environment banner
./connectors/boxes-cli add "$CANVAS" \
    --x 700 --y 50 \
    --width 50 --height 8 \
    --title "Production Environment" \
    --color 4 \
    --content \
    "Environment: PRODUCTION" \
    "Region: us-east-1" \
    "Containers: $(docker ps -q | wc -l) running" \
    "Images: $(docker images -q | wc -l) total" \
    "Last Check: $(date)"

# Count containers by status
running=$(docker ps -q | wc -l)
stopped=$(docker ps -aq | wc -l)
stopped=$((stopped - running))

./connectors/boxes-cli add "$CANVAS" \
    --x 1300 --y 50 \
    --width 35 --height 10 \
    --title "Status Summary" \
    --color 2 \
    --content \
    "Running: $running" \
    "Stopped: $stopped" \
    "" \
    "Health Checks:" \
    "$(docker ps --filter health=healthy -q | wc -l) healthy" \
    "$(docker ps --filter health=unhealthy -q | wc -l) unhealthy"

# Export to markdown report
./connectors/boxes-cli export "$CANVAS" \
    --format markdown -o "container-report-$(date +%Y%m%d).md"

echo "Dashboard saved to: $CANVAS"
echo "Report exported to: container-report-$(date +%Y%m%d).md"
```

**Usage**:
```bash
./container-dashboard.sh
./boxes-live  # View interactive dashboard
```

### Workflow 3: Log Analysis and Troubleshooting

**Goal**: Analyze application logs to identify issues.

```bash
#!/bin/bash
# analyze-logs.sh - Parse and visualize log files

LOG_FILE="/var/log/application.log"
CANVAS="log_analysis.txt"

# Parse logs into canvas
./connectors/log2canvas "$LOG_FILE" --max 100 > "$CANVAS"

# Find and highlight errors
error_count=$(./connectors/boxes-cli list "$CANVAS" --color 1 | wc -l)

# Add summary box
./connectors/boxes-cli add "$CANVAS" \
    --x 50 --y 50 \
    --width 45 --height 12 \
    --title "Log Analysis Summary" \
    --color 1 \
    --content \
    "Log File: $LOG_FILE" \
    "Total Entries: 100 (latest)" \
    "" \
    "By Severity:" \
    "  Errors: $error_count" \
    "  Warnings: $(./connectors/boxes-cli list "$CANVAS" --color 3 | wc -l)" \
    "  Info: $(./connectors/boxes-cli list "$CANVAS" --color 4 | wc -l)" \
    "" \
    "Analysis Time: $(date)"

# Search for specific error patterns
echo "Searching for common issues..."
./connectors/boxes-cli search "$CANVAS" "timeout" > errors_timeout.txt
./connectors/boxes-cli search "$CANVAS" "failed" > errors_failed.txt
./connectors/boxes-cli search "$CANVAS" "exception" > errors_exception.txt

# Export error summary
{
    echo "# Log Error Analysis - $(date)"
    echo ""
    echo "## Timeout Errors"
    cat errors_timeout.txt
    echo ""
    echo "## Failed Operations"
    cat errors_failed.txt
    echo ""
    echo "## Exceptions"
    cat errors_exception.txt
} > log_error_summary.md

echo "Canvas: $CANVAS"
echo "Error summary: log_error_summary.md"
```

## DevOps and Monitoring

### Workflow 4: CI/CD Pipeline Visualization

**Goal**: Visualize CI/CD pipeline stages and status.

```bash
#!/bin/bash
# cicd-pipeline.sh - Visualize pipeline status

CANVAS="pipeline.txt"

# Create base canvas
./connectors/boxes-cli create "$CANVAS" --width 3000 --height 1000

# Define pipeline stages
declare -a STAGES=(
    "Source|git|Running|2"
    "Build|docker|Success|2"
    "Test|pytest|Success|2"
    "Scan|trivy|Warning|3"
    "Deploy|kubectl|Pending|3"
)

X_POS=200
Y_POS=300

# Add stage boxes
for stage_data in "${STAGES[@]}"; do
    IFS='|' read -r name tool status color <<< "$stage_data"

    ./connectors/boxes-cli add "$CANVAS" \
        --x $X_POS --y $Y_POS \
        --width 35 --height 10 \
        --title "$name" \
        --color $color \
        --content \
        "Tool: $tool" \
        "Status: $status" \
        "" \
        "Duration: $(( RANDOM % 60 ))s" \
        "Timestamp: $(date +%H:%M:%S)"

    X_POS=$((X_POS + 400))
done

# Add pipeline header
./connectors/boxes-cli add "$CANVAS" \
    --x 800 --y 100 \
    --width 60 --height 8 \
    --title "CI/CD Pipeline - boxes-live v0.3.0" \
    --color 6 \
    --content \
    "Branch: main" \
    "Commit: abc123f - Add connector integration tests" \
    "Triggered by: GitHub Push" \
    "Started: $(date)"

# Add pipeline summary
./connectors/boxes-cli add "$CANVAS" \
    --x 200 --y 500 \
    --width 50 --height 12 \
    --title "Pipeline Summary" \
    --color 4 \
    --content \
    "✓ Source: Completed" \
    "✓ Build: Completed" \
    "✓ Test: Passed (245 tests)" \
    "⚠ Security Scan: 2 warnings" \
    "⏳ Deploy: In Progress" \
    "" \
    "Overall: In Progress" \
    "ETA: 2 minutes"

echo "Pipeline visualization: $CANVAS"
./boxes-live  # View pipeline
```

### Workflow 5: Infrastructure Monitoring

**Goal**: Monitor infrastructure components in real-time.

```bash
#!/bin/bash
# infrastructure-monitor.sh - Monitor all infrastructure

CANVAS="infrastructure.txt"

# Create canvas
./connectors/boxes-cli create "$CANVAS" --width 2500 --height 2000

# 1. Docker containers
echo "Adding Docker containers..."
docker ps -a | ./connectors/docker2canvas --layout grid > /tmp/docker.txt
# Merge into main canvas (simplified - would need merge tool)

# 2. Git repositories status
echo "Adding Git status..."
for repo in ~/projects/*; do
    if [ -d "$repo/.git" ]; then
        # Use subshell to avoid changing directory
        (
            cd "$repo" || continue
            repo_name=$(basename "$repo")
            branch=$(git branch --show-current)
            changes=$(git status --short | wc -l)
            commits=$(git log --oneline -n 3)

            # Use absolute path or return to original directory
            cd - > /dev/null || exit
            ./connectors/boxes-cli add "$CANVAS" \
                --x $((200 + RANDOM % 1000)) \
                --y $((500 + RANDOM % 500)) \
                --title "$repo_name" \
                --color 4 \
                --content \
                "Branch: $branch" \
                "Changes: $changes files" \
                "Recent commits:" \
                "$commits"
        )
    fi
done

# 3. System metrics
./connectors/boxes-cli add "$CANVAS" \
    --x 100 --y 100 \
    --width 50 --height 16 \
    --title "System Metrics" \
    --color 2 \
    --content \
    "CPU: $(top -bn1 | grep "Cpu(s)" | awk '{print $2}')%" \
    "Memory: $(free | awk '/Mem:/ {printf "%.1f%%", $3/$2 * 100}')" \
    "Disk: $(df -h / | awk 'NR==2 {print $5}')" \
    "Uptime: $(uptime -p)" \
    "" \
    "Network:" \
    "  RX: $(cat /sys/class/net/eth0/statistics/rx_bytes 2>/dev/null || echo 0)" \
    "  TX: $(cat /sys/class/net/eth0/statistics/tx_bytes 2>/dev/null || echo 0)" \
    "" \
    "Updated: $(date +%H:%M:%S)"

echo "Infrastructure dashboard: $CANVAS"
```

## Software Development

### Workflow 6: Git Repository Visualization

**Goal**: Visualize git commit history and branch structure.

```bash
#!/bin/bash
# git-history.sh - Visualize git repository

CANVAS="git_history.txt"

# Generate git history canvas
./connectors/git2canvas --max 30 > "$CANVAS"

# Add repository info
repo_name=$(basename $(git rev-parse --show-toplevel))
branch=$(git branch --show-current)
author=$(git config user.name)

./connectors/boxes-cli add "$CANVAS" \
    --x 100 --y 50 \
    --width 50 --height 14 \
    --title "Repository: $repo_name" \
    --color 6 \
    --content \
    "Current Branch: $branch" \
    "Author: $author" \
    "" \
    "Statistics:" \
    "  Commits: $(git rev-list --count HEAD)" \
    "  Contributors: $(git shortlog -s | wc -l)" \
    "  Branches: $(git branch -a | wc -l)" \
    "  Tags: $(git tag | wc -l)" \
    "" \
    "Last Commit: $(git log -1 --format=%cr)"

# Add branch comparison
current_branch=$(git branch --show-current)
main_branch="main"

if [ "$current_branch" != "$main_branch" ]; then
    ahead=$(git rev-list --count ${main_branch}..HEAD 2>/dev/null || echo "0")
    behind=$(git rev-list --count HEAD..${main_branch} 2>/dev/null || echo "0")

    ./connectors/boxes-cli add "$CANVAS" \
        --x 700 --y 50 \
        --width 40 --height 10 \
        --title "Branch Comparison" \
        --color 3 \
        --content \
        "Current: $current_branch" \
        "Base: $main_branch" \
        "" \
        "Ahead: $ahead commits" \
        "Behind: $behind commits" \
        "" \
        "Status: $([ $behind -gt 0 ] && echo 'Needs rebase' || echo 'Up to date')"
fi

# Export to markdown report
./connectors/boxes-cli export "$CANVAS" \
    --format markdown -o "git-history-report.md"

echo "Git history: $CANVAS"
echo "Report: git-history-report.md"
```

### Workflow 7: Codebase Structure Visualization

**Goal**: Visualize project directory structure and dependencies.

```bash
#!/bin/bash
# codebase-structure.sh - Visualize codebase

PROJECT_DIR="${1:-.}"
CANVAS="codebase_structure.txt"

# Generate directory tree
./connectors/tree2canvas "$PROJECT_DIR" --max-depth 3 > "$CANVAS"

# Add project statistics
total_files=$(find "$PROJECT_DIR" -type f | wc -l)
code_files=$(find "$PROJECT_DIR" -type f \( -name "*.c" -o -name "*.h" -o -name "*.py" \) | wc -l)
loc=$(find "$PROJECT_DIR" \( -name "*.c" -o -name "*.h" \) | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}')

./connectors/boxes-cli add "$CANVAS" \
    --x 100 --y 50 \
    --width 45 --height 14 \
    --title "Project Statistics" \
    --color 4 \
    --content \
    "Project: $(basename "$(pwd)")" \
    "Language: C, Python" \
    "" \
    "Files:" \
    "  Total: $total_files" \
    "  Code: $code_files" \
    "" \
    "Lines of Code: $loc" \
    "" \
    "Generated: $(date)"

# Add recent changes
./connectors/boxes-cli add "$CANVAS" \
    --x 600 --y 50 \
    --width 55 --height 12 \
    --title "Recent Changes" \
    --color 2 \
    --content \
    "Last 5 commits:" \
    "$(git log --oneline -n 5 | sed 's/^/  /')"

echo "Codebase structure: $CANVAS"
```

## Data Analysis

### Workflow 8: CSV Data Visualization

**Goal**: Visualize and explore CSV data interactively.

```bash
#!/bin/bash
# visualize-csv.sh - Convert CSV to interactive canvas

CSV_FILE="${1:-data.csv}"
CANVAS="data_visualization.txt"

# Convert CSV to canvas
./connectors/csv2canvas "$CSV_FILE" > "$CANVAS"

# Arrange in grid layout
./connectors/boxes-cli arrange "$CANVAS" --layout grid --spacing 60

# Add data summary
total_rows=$(./connectors/boxes-cli stats "$CANVAS" --json | jq '.total_boxes')

./connectors/boxes-cli add "$CANVAS" \
    --x 100 --y 50 \
    --width 50 --height 12 \
    --title "Dataset Overview" \
    --color 6 \
    --content \
    "Source: $(basename "$CSV_FILE")" \
    "Rows: $total_rows" \
    "" \
    "Color Legend:" \
    "  Red: High values" \
    "  Yellow: Medium values" \
    "  Green: Low values" \
    "" \
    "Created: $(date)"

# Export analysis
./connectors/boxes-cli export "$CANVAS" \
    --format markdown -o "data-analysis-$(date +%Y%m%d).md"

./connectors/boxes-cli export "$CANVAS" \
    --format json -o "data-analysis-$(date +%Y%m%d).json"

echo "Visualization: $CANVAS"
echo "Analysis exported to markdown and JSON"
```

### Workflow 9: JSON API Response Visualization

**Goal**: Visualize API responses for debugging.

```bash
#!/bin/bash
# visualize-api.sh - Visualize API responses

API_URL="https://api.github.com/repos/jcaldwell-labs/boxes-live"
CANVAS="api_response.txt"

# Fetch API data
echo "Fetching from $API_URL..."
curl -s "$API_URL" > /tmp/api_response.json

# Convert to canvas
./connectors/json2canvas /tmp/api_response.json > "$CANVAS"

# Add API metadata
status_code=$(curl -s -o /dev/null -w "%{http_code}" "$API_URL")

./connectors/boxes-cli add "$CANVAS" \
    --x 100 --y 50 \
    --width 50 --height 14 \
    --title "API Response" \
    --color 4 \
    --content \
    "Endpoint: $API_URL" \
    "Status: $status_code" \
    "Method: GET" \
    "" \
    "Response Time: $(date)" \
    "Content-Type: application/json" \
    "" \
    "Fields: $(jq 'keys | length' /tmp/api_response.json)" \
    "" \
    "View below for full response →"

# View interactively
./boxes-live  # F3 → $CANVAS

# Export for documentation
./connectors/boxes-cli export "$CANVAS" \
    --format markdown -o "api-documentation.md"
```

## Project Management

### Workflow 10: GitHub Organization Dashboard

**Goal**: Visualize GitHub organization structure and activity.

```bash
#!/bin/bash
# github-org-dashboard.sh - Visualize GitHub org

# Generate org canvas (uses jcaldwell-labs2canvas as template)
./connectors/jcaldwell-labs2canvas > "github_org.txt"

echo "GitHub Organization dashboard created: github_org.txt"
echo ""
echo "View with: ./boxes-live"
echo "Then press F3 and enter: github_org.txt"
echo ""
echo "The canvas shows:"
echo "  • All projects with status"
echo "  • Open PRs requiring review"
echo "  • Health metrics"
echo "  • Next steps and priorities"
```

### Workflow 11: Task Management Board

**Goal**: Create visual task board from CSV task list.

```bash
#!/bin/bash
# task-board.sh - Create visual task board

TASKS_CSV="tasks.csv"
CANVAS="task_board.txt"

# Create sample tasks file if it doesn't exist
if [ ! -f "$TASKS_CSV" ]; then
    cat > "$TASKS_CSV" <<EOF
id,title,status,priority,assignee
1,Implement connector tests,in_progress,high,alice
2,Write documentation,todo,medium,bob
3,Review PR #5,todo,high,alice
4,Deploy to production,blocked,high,charlie
5,Update dependencies,done,low,bob
EOF
fi

# Create canvas
./connectors/boxes-cli create "$CANVAS"

# Group tasks by status
statuses=("todo" "in_progress" "blocked" "done")
colors=(3 2 1 4)  # yellow, green, red, blue
x_positions=(200 600 1000 1400)

# Add status column headers
for i in ${!statuses[@]}; do
    status=${statuses[$i]}
    color=${colors[$i]}
    x=${x_positions[$i]}

    ./connectors/boxes-cli add "$CANVAS" \
        --x $x --y 100 \
        --width 35 --height 6 \
        --title "${status^^}" \
        --color $color \
        --content \
        "Tasks in this status" \
        "Drag to reorder"
done

# Add tasks to appropriate columns
y_pos=250

while IFS=',' read -r id title status priority assignee; do
    [ "$id" = "id" ] && continue  # Skip header

    # Determine column
    for i in ${!statuses[@]}; do
        if [ "$status" = "${statuses[$i]}" ]; then
            x=${x_positions[$i]}
            color=${colors[$i]}
            break
        fi
    done

    ./connectors/boxes-cli add "$CANVAS" \
        --x $x --y $y_pos \
        --width 35 --height 10 \
        --title "Task #$id: $title" \
        --color $color \
        --content \
        "Priority: $priority" \
        "Assignee: $assignee" \
        "Status: $status" \
        "" \
        "Created: $(date +%Y-%m-%d)"

    y_pos=$((y_pos + 130))

    # Reset Y position for new column
    if [ $y_pos -gt 800 ]; then
        y_pos=250
    fi
done < "$TASKS_CSV"

# Add board header
./connectors/boxes-cli add "$CANVAS" \
    --x 700 --y 20 \
    --width 50 --height 8 \
    --title "Task Board - Sprint 3" \
    --color 6 \
    --content \
    "Team: boxes-live" \
    "Sprint: Week of $(date +%Y-%m-%d)" \
    "Total Tasks: $(tail -n +2 $TASKS_CSV | wc -l)" \
    "Updated: $(date)"

echo "Task board created: $CANVAS"
echo "View with: ./boxes-live"
```

### Workflow 12: Sprint Planning

**Goal**: Plan and track sprint progress.

```bash
#!/bin/bash
# sprint-planning.sh - Sprint planning canvas

CANVAS="sprint_planning.txt"

./connectors/boxes-cli create "$CANVAS" --width 2500 --height 1500

# Sprint header
./connectors/boxes-cli add "$CANVAS" \
    --x 900 --y 50 \
    --width 60 --height 10 \
    --title "Sprint 5 Planning - boxes-live" \
    --color 6 \
    --content \
    "Duration: 2 weeks" \
    "Start: $(date +%Y-%m-%d)" \
    "Team: 3 developers" \
    "Capacity: 120 story points" \
    "" \
    "Goal: Complete connector integration testing"

# User stories
declare -a STORIES=(
    "Integration Tests|13|high|alice"
    "CONNECTOR-GUIDE.md|8|high|bob"
    "WORKFLOW-EXAMPLES.md|8|medium|charlie"
    "Update README|5|medium|alice"
    "Test jcaldwell-labs2canvas|5|low|bob"
)

y_pos=200
story_num=1

for story in "${STORIES[@]}"; do
    IFS='|' read -r title points priority assignee <<< "$story"

    color=2
    [ "$priority" = "high" ] && color=1
    [ "$priority" = "low" ] && color=4

    ./connectors/boxes-cli add "$CANVAS" \
        --x 200 --y $y_pos \
        --width 40 --height 12 \
        --title "Story $story_num: $title" \
        --color $color \
        --content \
        "Points: $points" \
        "Priority: $priority" \
        "Assignee: $assignee" \
        "" \
        "Acceptance Criteria:" \
        "  • Tests pass" \
        "  • Documentation complete" \
        "  • Code reviewed"

    y_pos=$((y_pos + 160))
    story_num=$((story_num + 1))
done

# Sprint metrics
total_points=39

./connectors/boxes-cli add "$CANVAS" \
    --x 800 --y 200 \
    --width 45 --height 14 \
    --title "Sprint Metrics" \
    --color 4 \
    --content \
    "Total Points: $total_points" \
    "Capacity: 120" \
    "Utilization: $(echo "scale=1; $total_points/120*100" | bc)%" \
    "" \
    "Distribution:" \
    "  High Priority: 21 pts" \
    "  Medium Priority: 13 pts" \
    "  Low Priority: 5 pts" \
    "" \
    "Status: Planning"

echo "Sprint planning canvas: $CANVAS"
```

## Education and Learning

### Workflow 13: Periodic Table of Elements

**Goal**: Interactive periodic table for chemistry education.

```bash
#!/bin/bash
# periodic-table.sh - Generate interactive periodic table

CANVAS="periodic_table.txt"

# Generate periodic table
./connectors/periodic2canvas > "$CANVAS"

# Add educational header
./connectors/boxes-cli add "$CANVAS" \
    --x 700 --y 30 \
    --width 60 --height 12 \
    --title "Interactive Periodic Table of Elements" \
    --color 6 \
    --content \
    "Navigate with arrow keys or WASD" \
    "Zoom with +/- or Z/X" \
    "Click boxes to select" \
    "" \
    "Color Legend:" \
    "  Different colors represent element categories" \
    "" \
    "boxes-live chemistry education tool"

# Add usage instructions
./connectors/boxes-cli add "$CANVAS" \
    --x 50 --y 800 \
    --width 50 --height 14 \
    --title "Instructions" \
    --color 4 \
    --content \
    "Search for elements:" \
    "  ./connectors/boxes-cli search $CANVAS 'Hydrogen'" \
    "" \
    "Filter by group:" \
    "  ./connectors/boxes-cli list $CANVAS --region 0 0 500 500" \
    "" \
    "Export to study guide:" \
    "  ./connectors/boxes-cli export $CANVAS --format markdown" \
    "" \
    "Happy learning!"

echo "Periodic table created: $CANVAS"
echo ""
echo "Try searching for an element:"
echo "  ./connectors/boxes-cli search $CANVAS 'Carbon'"
```

### Workflow 14: Learning Path Visualization

**Goal**: Visualize learning curriculum and progress.

```bash
#!/bin/bash
# learning-path.sh - Visualize learning curriculum

CANVAS="learning_path.txt"

./connectors/boxes-cli create "$CANVAS" --width 2000 --height 1500

# Learning path header
./connectors/boxes-cli add "$CANVAS" \
    --x 700 --y 50 \
    --width 55 --height 10 \
    --title "C Programming Learning Path" \
    --color 6 \
    --content \
    "From Beginner to Advanced" \
    "Estimated Duration: 12 weeks" \
    "" \
    "Progress: 45% Complete" \
    "Current Module: Pointers & Memory" \
    "Next Milestone: Data Structures"

# Learning modules
declare -a MODULES=(
    "Basics|Completed|2|Setup,Variables,Types|100"
    "Control Flow|Completed|2|If/Else,Loops,Functions|100"
    "Pointers|In Progress|2|References,Dereferencing,Arrays|60"
    "Memory|Not Started|3|malloc,free,Stack vs Heap|0"
    "Data Structures|Not Started|5|Lists,Trees,Hash Tables|0"
    "Advanced|Not Started|8|File I/O,Networking,Threads|0"
)

y_pos=200

for module_data in "${MODULES[@]}"; do
    IFS='|' read -r title status weeks topics progress <<< "$module_data"

    # Color based on status
    color=0
    [ "$status" = "Completed" ] && color=2
    [ "$status" = "In Progress" ] && color=3

    ./connectors/boxes-cli add "$CANVAS" \
        --x 200 --y $y_pos \
        --width 50 --height 14 \
        --title "$title" \
        --color $color \
        --content \
        "Status: $status" \
        "Duration: $weeks weeks" \
        "Progress: $progress%" \
        "" \
        "Topics:" \
        "$(echo $topics | tr ',' '\n' | sed 's/^/  • /')" \
        "" \
        "$([ "$status" = "Completed" ] && echo '✓ Module Complete' || echo '→ Continue Learning')"

    # Add progress boxes to the right
    ./connectors/boxes-cli add "$CANVAS" \
        --x 800 --y $y_pos \
        --width 60 --height 14 \
        --title "Resources for $title" \
        --color 4 \
        --content \
        "Learning Materials:" \
        "  • Video tutorials" \
        "  • Code examples" \
        "  • Practice exercises" \
        "  • Quizzes" \
        "" \
        "Time Invested: $((weeks * progress / 100)) weeks" \
        "Est. Remaining: $((weeks * (100 - progress) / 100)) weeks"

    y_pos=$((y_pos + 200))
done

echo "Learning path visualization: $CANVAS"
echo "View your progress with: ./boxes-live"
```

## Advanced Workflows

### Workflow 15: Multi-Source Dashboard

**Goal**: Combine multiple data sources into single dashboard.

```bash
#!/bin/bash
# multi-source-dashboard.sh - Unified monitoring dashboard

CANVAS="dashboard.txt"

# Create large canvas for dashboard
./connectors/boxes-cli create "$CANVAS" --width 3000 --height 2000

# Section 1: System processes (top-left)
echo "Adding system processes..."
pstree -p | ./connectors/pstree2canvas --max-depth 2 --layout grid > /tmp/processes.txt

# Parse and add boxes from processes canvas
# Skip header (3 lines), then parse each box entry
tail -n +4 /tmp/processes.txt > /tmp/processes_boxes.txt
# Note: In production, use a proper parser or merge tool
# For simplicity, this example shows the concept

# Section 2: Docker containers (top-right)
echo "Adding Docker containers..."
docker ps | ./connectors/docker2canvas --layout grid > /tmp/docker.txt

# Parse docker boxes and add with offset to position in top-right
# Read box data and add with X offset of +1000 for right positioning
# Note: Full implementation would parse the canvas format properly

# Section 3: Git repositories (bottom-left)
echo "Adding Git status..."
./connectors/git2canvas --max 10 > /tmp/git.txt

# Parse git boxes and add with Y offset of +800 for bottom positioning
# Note: Full implementation would use canvas merge functionality

# Section 4: Logs (bottom-right)
echo "Adding recent logs..."
tail -50 /var/log/syslog | ./connectors/log2canvas --layout grid > /tmp/logs.txt

# Parse log boxes and add with X+1000, Y+800 offset for bottom-right
# Note: A complete dashboard would use proper canvas merging tools

# Add dashboard title
./connectors/boxes-cli add "$CANVAS" \
    --x 1200 --y 50 \
    --width 60 --height 12 \
    --title "Unified System Dashboard" \
    --color 6 \
    --content \
    "Quadrant View:" \
    "  Top-Left: System Processes" \
    "  Top-Right: Docker Containers" \
    "  Bottom-Left: Git Repositories" \
    "  Bottom-Right: System Logs" \
    "" \
    "Last Updated: $(date)" \
    "Auto-refresh: Every 30s"

echo "Multi-source dashboard created: $CANVAS"
echo "Contains data from: processes, docker, git, and logs"
```

### Workflow 16: Automated Daily Report

**Goal**: Generate daily development report automatically.

```bash
#!/bin/bash
# daily-report.sh - Generate daily development report

REPORT_DATE=$(date +%Y-%m-%d)
CANVAS="daily_report_${REPORT_DATE}.txt"
MARKDOWN="daily_report_${REPORT_DATE}.md"

# Create canvas
./connectors/boxes-cli create "$CANVAS"

# Add report header
./connectors/boxes-cli add "$CANVAS" \
    --x 700 --y 50 \
    --width 60 --height 10 \
    --title "Daily Development Report" \
    --color 6 \
    --content \
    "Date: $REPORT_DATE" \
    "Team: boxes-live Development" \
    "" \
    "Generated automatically at $(date +%H:%M)" \
    "" \
    "Sections: Git Activity, Tests, Deployments"

# Git activity today
git_commits=$(git log --since="midnight" --oneline | wc -l)

./connectors/boxes-cli add "$CANVAS" \
    --x 200 --y 200 \
    --width 50 --height 14 \
    --title "Git Activity - Today" \
    --color 2 \
    --content \
    "Commits: $git_commits" \
    "" \
    "Recent commits:" \
    "$(git log --since='midnight' --oneline | head -5 | sed 's/^/  /')" \
    "" \
    "Contributors: $(git log --since='midnight' --format='%an' | sort -u | wc -l)"

# Test results (if available)
if [ -f "test_results.txt" ]; then
    ./connectors/boxes-cli add "$CANVAS" \
        --x 700 --y 200 \
        --width 50 --height 14 \
        --title "Test Results" \
        --color 4 \
        --content \
        "$(cat test_results.txt)"
fi

# Export to markdown
./connectors/boxes-cli export "$CANVAS" --format markdown -o "$MARKDOWN"

# Email report (if configured)
if command -v mail &> /dev/null; then
    mail -s "Daily Report - $REPORT_DATE" team@example.com < "$MARKDOWN"
    echo "Report emailed to team"
fi

echo "Daily report generated:"
echo "  Canvas: $CANVAS"
echo "  Markdown: $MARKDOWN"
```

## Tips and Tricks

### Quick Commands

```bash
# Quick process check
pstree -p | ./connectors/pstree2canvas > /tmp/processes.txt && ./boxes-live
# Then press F3 and load /tmp/processes.txt

# Quick log analysis
tail -100 /var/log/syslog | ./connectors/log2canvas > /tmp/logs.txt && ./boxes-live
# Then press F3 and load /tmp/logs.txt

# Quick container status
docker ps | ./connectors/docker2canvas > /tmp/containers.txt && ./boxes-live
# Then press F3 and load /tmp/containers.txt

# Quick git history
./connectors/git2canvas --max 20 > /tmp/git.txt && ./boxes-live
# Then press F3 and load /tmp/git.txt
```

### Automation with Cron

```bash
# Add to crontab: crontab -e

# Generate process monitor every hour
0 * * * * cd /path/to/boxes-live && pstree -p | ./connectors/pstree2canvas > /var/www/html/processes.txt

# Daily report at 8 AM
0 8 * * * cd /path/to/boxes-live && ./scripts/daily-report.sh

# Container status every 5 minutes
*/5 * * * * cd /path/to/boxes-live && docker ps | ./connectors/docker2canvas > /var/www/html/containers.txt
```

### Integration with Other Tools

```bash
# With watch for continuous updates
watch -n 10 'pstree -p | ./connectors/pstree2canvas > monitor.txt'

# With jq for API responses
curl -s https://api.github.com/users/jcaldwell-labs | \
    jq '.' | \
    ./connectors/json2canvas > github_user.txt

# With systemd for service monitoring
systemctl list-units --type=service --all | \
    ./connectors/text2canvas --lines > services.txt
```

## Conclusion

These workflows demonstrate the versatility of boxes-live connectors across various domains:

- **System Administration**: Process monitoring, container management, log analysis
- **DevOps**: CI/CD pipelines, infrastructure monitoring
- **Development**: Git visualization, codebase structure
- **Data Analysis**: CSV/JSON visualization, API debugging
- **Project Management**: Task boards, sprint planning, GitHub org dashboards
- **Education**: Periodic table, learning paths

The connector ecosystem enables you to transform any data source into an interactive visual canvas, making complex information accessible and actionable.

## Further Resources

- **Connector Guide**: [docs/CONNECTOR-GUIDE.md](CONNECTOR-GUIDE.md) - Comprehensive connector development guide
- **CLI Documentation**: [connectors/README.md](../connectors/README.md) - boxes-cli command reference
- **Testing Guide**: [TESTING.md](../TESTING.md) - How to test connectors
- **Main Documentation**: [README.md](../README.md) - Project overview

## Contributing Workflows

Have a creative workflow? We'd love to see it! Please:

1. Document your workflow with code examples
2. Test it thoroughly
3. Submit a pull request adding it to this guide

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.
