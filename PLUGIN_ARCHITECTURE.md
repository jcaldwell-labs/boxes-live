# boxes-live Plugin Architecture

## Overview

This document outlines the plugin and extensibility system for boxes-live, enabling live-updating content, external integrations, and dynamic data sources. Unlike cloud-based tools like Miro, boxes-live leverages Unix mechanisms (named pipes, file watchers, cron) for a local-first, composable plugin architecture.

## Design Philosophy

1. **Unix Integration**: Use existing Unix tools (pipes, inotify, cron, systemd)
2. **Local-First**: No cloud dependencies, works offline
3. **Text-Based**: All data flows through readable text formats
4. **Process-Based**: Each plugin is a separate process (isolation, security)
5. **Declarative**: Plugin configuration in simple text files
6. **Composable**: Chain plugins via pipes and filters

## Plugin Categories

Based on Miro's 2024 plugin ecosystem, we identify key categories for boxes-live:

### 1. Live Data Sources (Real-Time Updates)
- System monitoring (CPU, memory, network)
- Log file tailing (application logs, system logs)
- Command output streaming (top, netstat, docker ps)
- File watchers (code changes, document updates)
- RSS/Atom feeds
- API polling (GitHub notifications, weather, stock prices)

### 2. Two-Way Sync Integrations
- GitHub Issues ↔ boxes (like Miro's GitHub integration)
- Jira Tasks ↔ boxes
- Trello Cards ↔ boxes
- Git commits → boxes (read-only)
- Database queries → boxes

### 3. Code Intelligence
- Code structure visualization (ctags, LSP)
- Git history graph
- Dependency trees
- Call graphs
- Test coverage visualization

### 4. Documentation & Diagrams
- Mermaid diagrams (live preview)
- PlantUML rendering
- Markdown preview
- LaTeX equations
- DOT graphs

### 5. Automation & Workflows
- Scheduled updates (cron-based)
- Event-driven triggers (inotify, webhooks)
- Transformation pipelines
- Export automation

## Core Plugin Mechanisms

### Mechanism 1: Named Pipes (FIFOs) - Live Streaming

**Concept**: A box can be bound to a named pipe that continuously updates its content.

**Implementation**:
```c
// Box structure extension
typedef struct {
    // ... existing fields ...
    char *pipe_path;          // Path to named pipe (if live)
    pid_t reader_pid;         // Background reader process
    time_t last_update;       // Last content update timestamp
    int refresh_interval_ms;  // Polling interval (default: 1000ms)
} Box;
```

**Example - Live Git Status**:
```bash
# Create named pipe
mkfifo /tmp/boxes/git_status.pipe

# Start content producer
while true; do
    git status --short > /tmp/boxes/git_status.pipe
    sleep 5
done &

# Create box bound to pipe
boxes-cli add canvas.txt \
    --x 100 --y 100 \
    --title "Git Status" \
    --pipe /tmp/boxes/git_status.pipe \
    --refresh 5000  # 5 second polling
```

**Canvas Format Extension**:
```
# Standard box
1 100.0 100.0 30 8 0 2
Title
2
Line 1
Line 2

# Live-updating box with pipe
2 200.0 100.0 30 8 0 3
Git Status
PIPE:/tmp/boxes/git_status.pipe:5000
0
```

### Mechanism 2: File Watchers (inotify) - Change Detection

**Concept**: Auto-reload box content when source file changes.

**Implementation**:
```c
// Watch for file changes using inotify
int setup_file_watch(Box *box, const char *file_path) {
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, file_path, IN_MODIFY);

    // Non-blocking read in main loop
    // On change: reload file → update box content
    return fd;
}
```

**Example - Live Markdown Preview**:
```bash
# Create box that watches a file
boxes-cli add canvas.txt \
    --title "README Preview" \
    --watch README.md \
    --filter "head -n 20"  # Optional processing
```

**Use Cases**:
- Code files (auto-reload on save)
- Configuration files
- Log files (tail -f behavior)
- Documentation
- Data files (CSV, JSON)

### Mechanism 3: Scheduled Updates (Cron-like)

**Concept**: Execute command periodically and update box with output.

**Implementation**:
```bash
# Box with scheduled command execution
boxes-cli add canvas.txt \
    --title "System Load" \
    --command "uptime" \
    --schedule "*/5 * * * *"  # Every 5 minutes (cron syntax)
```

**Canvas Format**:
```
3 300.0 100.0 30 6 0 4
System Load
CMD:uptime:300000
0
```

**Example Plugins**:
- System stats (uptime, df, free)
- GitHub notifications (gh api /notifications)
- Weather updates (curl wttr.in)
- Stock prices (curl API)
- Build status (CI/CD queries)

### Mechanism 4: Two-Way Sync (Bidirectional)

**Concept**: Box changes propagate to external system and vice versa.

**Implementation**:
```python
#!/usr/bin/env python3
# github_sync.py - Two-way GitHub issue sync

import subprocess
import json
import time

def sync_github_to_canvas(repo, canvas_file):
    """Pull GitHub issues → update canvas boxes"""
    issues = subprocess.check_output(
        ['gh', 'issue', 'list', '--repo', repo, '--json', 'number,title,state'],
        text=True
    )

    for issue in json.loads(issues):
        # Find or create box for this issue
        box_id = find_box_by_metadata(canvas_file, f"github:issue:{issue['number']}")

        if not box_id:
            # Create new box
            subprocess.run([
                'boxes-cli', 'add', canvas_file,
                '--title', f"#{issue['number']}: {issue['title']}",
                '--metadata', f"github:issue:{issue['number']}",
                '--color', '2' if issue['state'] == 'open' else '7'
            ])
        else:
            # Update existing box
            subprocess.run([
                'boxes-cli', 'update', canvas_file, str(box_id),
                '--title', f"#{issue['number']}: {issue['title']}",
                '--color', '2' if issue['state'] == 'open' else '7'
            ])

def sync_canvas_to_github(repo, canvas_file):
    """Push canvas changes → update GitHub issues"""
    # Detect box changes (new content, moved to different color)
    # Update GitHub issue accordingly
    pass

# Run bidirectional sync loop
while True:
    sync_github_to_canvas('user/repo', 'project.txt')
    sync_canvas_to_github('user/repo', 'project.txt')
    time.sleep(60)  # Sync every minute
```

**Usage**:
```bash
# Start background sync daemon
./plugins/github_sync.py --repo jcaldwell1066/boxes-live --canvas project.txt &

# Or as systemd service
systemctl --user start boxes-github-sync
```

### Mechanism 5: Event-Driven Webhooks

**Concept**: Receive HTTP webhooks and update canvas in real-time.

**Implementation**:
```python
#!/usr/bin/env python3
# webhook_server.py - Receive webhooks and update canvas

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import subprocess

class WebhookHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        payload = json.loads(body)

        # GitHub webhook example
        if 'issue' in payload:
            issue = payload['issue']
            subprocess.run([
                'boxes-cli', 'add', 'canvas.txt',
                '--title', f"New Issue: {issue['title']}",
                '--content', issue['body'][:100],
                '--color', '1'  # Red for new issues
            ])

        self.send_response(200)
        self.end_headers()

httpd = HTTPServer(('localhost', 8080), WebhookHandler)
print("Webhook server running on port 8080")
httpd.serve_forever()
```

**Usage with ngrok for external webhooks**:
```bash
# Start webhook server
./plugins/webhook_server.py --canvas canvas.txt &

# Expose to internet
ngrok http 8080

# Configure GitHub webhook to point to ngrok URL
# Now GitHub events instantly appear on canvas!
```

## Plugin Configuration Format

### Plugin Manifest (plugin.toml)

```toml
[plugin]
name = "github-issues"
version = "1.0.0"
description = "Two-way sync with GitHub Issues"
author = "Your Name"

[plugin.requirements]
commands = ["gh", "jq"]  # Required CLI tools
python_packages = []

[plugin.capabilities]
two_way_sync = true
live_updates = true
webhooks = true

[plugin.config]
# User-configurable options
repo = { type = "string", required = true, description = "GitHub repository (owner/repo)" }
poll_interval = { type = "int", default = 60, description = "Poll interval in seconds" }
webhook_port = { type = "int", default = 8080 }

[plugin.box_types]
# Custom box type for this plugin
[plugin.box_types.github_issue]
color = 3  # Blue
icon = "🔵"
metadata_fields = ["issue_number", "state", "assignee"]
```

### Plugin Directory Structure

```
plugins/
├── github-issues/
│   ├── plugin.toml          # Manifest
│   ├── sync.py              # Main sync script
│   ├── webhook.py           # Webhook server
│   ├── install.sh           # Setup script
│   └── README.md
├── live-logs/
│   ├── plugin.toml
│   ├── tail_logs.sh
│   └── README.md
├── system-monitor/
│   ├── plugin.toml
│   ├── monitor.sh
│   └── dashboard.template
└── ...
```

## Polling Interval System

### Smart Polling Strategy

**Problem**: Balance between responsiveness and resource usage.

**Solution**: Adaptive polling with exponential backoff.

```c
typedef struct {
    int base_interval_ms;      // Default: 1000ms
    int min_interval_ms;       // Minimum: 100ms
    int max_interval_ms;       // Maximum: 60000ms (1 min)
    int current_interval_ms;   // Current adaptive interval
    bool content_changed;      // Did content change last poll?
    int unchanged_count;       // Consecutive unchanged polls
} PollConfig;

// Adaptive polling algorithm
void update_poll_interval(PollConfig *config, bool changed) {
    if (changed) {
        // Content changed: increase responsiveness
        config->unchanged_count = 0;
        config->current_interval_ms = config->base_interval_ms;
    } else {
        // Content stable: decrease poll frequency
        config->unchanged_count++;

        if (config->unchanged_count > 5) {
            // Exponential backoff
            config->current_interval_ms =
                MIN(config->current_interval_ms * 2, config->max_interval_ms);
        }
    }
}
```

**Example Behavior**:
- Active updates: Poll every 1s
- After 5 unchanged polls: Poll every 2s
- After 10 unchanged polls: Poll every 4s
- Maximum backoff: Poll every 60s

**User Configuration**:
```bash
# Set polling interval per box
boxes-cli update canvas.txt 1 \
    --poll-interval 5000 \    # Base interval: 5s
    --poll-min 1000 \         # Fastest: 1s
    --poll-max 30000          # Slowest: 30s
```

## Canvas Format Extensions

### Extended Box Metadata

```
# Format: BOX_ID X Y W H SELECTED COLOR [METADATA]
1 100.0 100.0 30 8 0 2 PIPE:/tmp/git.pipe:5000
2 200.0 100.0 30 8 0 3 WATCH:/path/to/file:1000
3 300.0 100.0 30 8 0 4 CMD:uptime:60000
4 400.0 100.0 30 8 0 1 WEBHOOK:github:12345
```

**Metadata Types**:
- `PIPE:<path>:<interval>` - Named pipe source
- `WATCH:<file>:<interval>` - File watcher
- `CMD:<command>:<interval>` - Scheduled command
- `WEBHOOK:<source>:<id>` - Webhook receiver
- `SYNC:<plugin>:<id>` - Two-way sync

## Example Plugins Inspired by Miro

### 1. GitHub Issues Plugin (Like Miro's GitHub Integration)

```bash
#!/bin/bash
# plugins/github-issues/sync.sh

REPO=$1
CANVAS=$2
INTERVAL=${3:-60}

while true; do
    # Fetch issues
    gh issue list --repo "$REPO" --json number,title,state,labels | \
    jq -r '.[] | "\(.number)|\(.title)|\(.state)|\(.labels | map(.name) | join(","))"' | \
    while IFS='|' read -r num title state labels; do

        # Find existing box or create new
        box_id=$(boxes-cli list "$CANVAS" --search "GH-$num" --json | jq -r '.[0].id // empty')

        # Determine color by state
        color=3  # Blue for open
        [[ "$state" == "closed" ]] && color=7  # White for closed

        if [ -z "$box_id" ]; then
            # Create new box
            boxes-cli add "$CANVAS" \
                --title "GH-$num: $title" \
                --content "State: $state" "Labels: $labels" \
                --color "$color" \
                --metadata "github:issue:$num"
        else
            # Update existing
            boxes-cli update "$CANVAS" "$box_id" \
                --title "GH-$num: $title" \
                --color "$color"
        fi
    done

    sleep "$INTERVAL"
done
```

**Usage**:
```bash
./plugins/github-issues/sync.sh jcaldwell1066/boxes-live project.txt 30
```

### 2. AWS Cloud View (Like Miro's AWS Integration)

```python
#!/usr/bin/env python3
# plugins/aws-cloud-view/visualize.py

import boto3
import subprocess

def visualize_aws_architecture(profile, canvas_file):
    """Import AWS services as boxes"""
    session = boto3.Session(profile_name=profile)

    # EC2 instances
    ec2 = session.client('ec2')
    instances = ec2.describe_instances()

    x, y = 100, 100
    for reservation in instances['Reservations']:
        for instance in reservation['Instances']:
            subprocess.run([
                'boxes-cli', 'add', canvas_file,
                '--x', str(x), '--y', str(y),
                '--title', f"EC2: {instance['InstanceId']}",
                '--content',
                    f"Type: {instance['InstanceType']}",
                    f"State: {instance['State']['Name']}",
                    f"IP: {instance.get('PublicIpAddress', 'N/A')}",
                '--color', '3'  # Blue for EC2
            ])
            x += 150
            if x > 1000:
                x = 100
                y += 120

    # RDS databases
    rds = session.client('rds')
    databases = rds.describe_db_instances()

    for db in databases['DBInstances']:
        subprocess.run([
            'boxes-cli', 'add', canvas_file,
            '--x', str(x), '--y', str(y),
            '--title', f"RDS: {db['DBInstanceIdentifier']}",
            '--content',
                f"Engine: {db['Engine']}",
                f"Status: {db['DBInstanceStatus']}",
            '--color', '2'  # Green for RDS
        ])
        x += 150

if __name__ == '__main__':
    import sys
    visualize_aws_architecture(sys.argv[1], sys.argv[2])
```

### 3. Live System Monitor (Real-Time Dashboard)

```bash
#!/bin/bash
# plugins/system-monitor/dashboard.sh

CANVAS=$1

# Create monitoring boxes with live updates

# CPU Usage (named pipe)
mkfifo /tmp/boxes/cpu.pipe
while true; do
    top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1
    sleep 1
done > /tmp/boxes/cpu.pipe &

boxes-cli add "$CANVAS" \
    --title "CPU Usage" \
    --pipe /tmp/boxes/cpu.pipe \
    --refresh 1000 \
    --x 100 --y 100

# Memory Usage
mkfifo /tmp/boxes/mem.pipe
while true; do
    free -h | awk '/^Mem:/ {print $3 "/" $2}'
    sleep 2
done > /tmp/boxes/mem.pipe &

boxes-cli add "$CANVAS" \
    --title "Memory" \
    --pipe /tmp/boxes/mem.pipe \
    --refresh 2000 \
    --x 250 --y 100

# Disk Usage
boxes-cli add "$CANVAS" \
    --title "Disk Usage" \
    --command "df -h / | tail -1 | awk '{print \$3 \"/\" \$2 \" (\" \$5 \")\"}'" \
    --schedule 10000 \
    --x 400 --y 100

# Active Connections
mkfifo /tmp/boxes/netstat.pipe
while true; do
    netstat -an | grep ESTABLISHED | wc -l
    sleep 5
done > /tmp/boxes/netstat.pipe &

boxes-cli add "$CANVAS" \
    --title "Active Connections" \
    --pipe /tmp/boxes/netstat.pipe \
    --refresh 5000 \
    --x 550 --y 100
```

### 4. Live Log Viewer

```bash
#!/bin/bash
# plugins/live-logs/tail_log.sh

LOG_FILE=$1
CANVAS=$2
BOX_TITLE=$3

# Create named pipe for log tailing
PIPE="/tmp/boxes/log_$(basename "$LOG_FILE").pipe"
mkfifo "$PIPE"

# Tail log file to pipe (last 10 lines, updating)
tail -f -n 10 "$LOG_FILE" > "$PIPE" &

# Create box bound to pipe
boxes-cli add "$CANVAS" \
    --title "$BOX_TITLE" \
    --pipe "$PIPE" \
    --refresh 500 \  # Fast refresh for logs
    --height 15 \    # Taller box for log lines
    --color 4        # Yellow for logs
```

**Usage**:
```bash
./plugins/live-logs/tail_log.sh /var/log/nginx/access.log canvas.txt "Nginx Access Log"
./plugins/live-logs/tail_log.sh /var/log/app.log canvas.txt "Application Log"
```

### 5. Docker Container Monitor

```python
#!/usr/bin/env python3
# plugins/docker-monitor/containers.py

import docker
import subprocess
import time

def update_container_boxes(canvas_file):
    client = docker.from_env()
    containers = client.containers.list()

    x, y = 100, 100
    for container in containers:
        stats = container.stats(stream=False)

        # Calculate CPU percentage
        cpu_delta = stats['cpu_stats']['cpu_usage']['total_usage'] - \
                   stats['precpu_stats']['cpu_usage']['total_usage']
        system_delta = stats['cpu_stats']['system_cpu_usage'] - \
                      stats['precpu_stats']['system_cpu_usage']
        cpu_percent = (cpu_delta / system_delta) * 100 if system_delta > 0 else 0

        # Memory usage
        mem_usage = stats['memory_stats']['usage'] / (1024 ** 2)  # MB

        subprocess.run([
            'boxes-cli', 'add', canvas_file,
            '--x', str(x), '--y', str(y),
            '--title', f"🐳 {container.name}",
            '--content',
                f"Image: {container.image.tags[0] if container.image.tags else 'N/A'}",
                f"Status: {container.status}",
                f"CPU: {cpu_percent:.1f}%",
                f"Memory: {mem_usage:.0f}MB",
            '--color', '2' if container.status == 'running' else '1',
            '--metadata', f"docker:container:{container.id[:12]}"
        ])

        x += 180
        if x > 900:
            x = 100
            y += 140

# Update loop
while True:
    update_container_boxes('docker_dashboard.txt')
    time.sleep(5)
```

## Plugin Manager CLI

```bash
#!/bin/bash
# boxes-plugin - Plugin management CLI

case "$1" in
    list)
        # List installed plugins
        ls -1 plugins/
        ;;

    install)
        # Install plugin from directory or URL
        plugin=$2
        if [[ "$plugin" =~ ^https:// ]]; then
            git clone "$plugin" "plugins/$(basename "$plugin" .git)"
        else
            cp -r "$plugin" plugins/
        fi

        # Run install script
        cd "plugins/$(basename "$plugin")" && ./install.sh
        ;;

    enable)
        # Enable plugin for a canvas
        plugin=$2
        canvas=$3
        echo "$plugin" >> "$(dirname "$canvas")/.plugins"
        ;;

    start)
        # Start plugin daemon
        plugin=$2
        canvas=$3
        "./plugins/$plugin/start.sh" "$canvas" &
        echo $! > "/tmp/boxes-plugin-$plugin.pid"
        ;;

    stop)
        # Stop plugin daemon
        plugin=$2
        kill "$(cat "/tmp/boxes-plugin-$plugin.pid")"
        ;;
esac
```

**Usage**:
```bash
# Install plugin
boxes-plugin install ./plugins/github-issues

# Start plugin for canvas
boxes-plugin start github-issues project.txt

# Stop plugin
boxes-plugin stop github-issues
```

## Implementation Roadmap

### Phase 1: Core Live Update Support (Week 1-2)
- [ ] Add `pipe_path` and `refresh_interval_ms` to Box structure
- [ ] Implement named pipe reading in main loop
- [ ] Add `--pipe` and `--refresh` flags to boxes-cli
- [ ] Test with simple live sources (date, uptime)

### Phase 2: File Watchers (Week 3)
- [ ] Implement inotify-based file watching
- [ ] Add `--watch` flag to boxes-cli
- [ ] Create file watcher example (live Markdown preview)

### Phase 3: Scheduled Commands (Week 4)
- [ ] Implement command execution at intervals
- [ ] Add `--command` and `--schedule` flags
- [ ] Create system monitor dashboard example

### Phase 4: Plugin Framework (Week 5-6)
- [ ] Design plugin manifest format (plugin.toml)
- [ ] Implement plugin manager CLI
- [ ] Create plugin directory structure
- [ ] Documentation for plugin developers

### Phase 5: Example Plugins (Week 7-8)
- [ ] GitHub Issues sync plugin
- [ ] Live log viewer plugin
- [ ] System monitor plugin
- [ ] Docker container monitor plugin

### Phase 6: Advanced Features (Week 9-10)
- [ ] Webhook server support
- [ ] Two-way sync framework
- [ ] Adaptive polling intervals
- [ ] Plugin marketplace/repository

## Comparison: boxes-live vs Miro Plugins

| Feature | Miro | boxes-live |
|---------|------|------------|
| **Architecture** | Cloud API, OAuth | Local processes, Unix pipes |
| **Setup** | API keys, webhooks, cloud | Install script, start daemon |
| **Latency** | 100-500ms (HTTP) | <1ms (local IPC) |
| **Offline** | Requires internet | Fully functional |
| **Resources** | Bandwidth, API limits | Local CPU/memory only |
| **Privacy** | Cloud-hosted | Local-only, no data leaves machine |
| **Cost** | Rate limits, paid tiers | Free, unlimited |
| **Extensibility** | REST API, JavaScript SDK | Shell scripts, any language |
| **Real-Time** | Webhooks (complex setup) | Named pipes (native Unix) |
| **Integration** | 100+ official plugins | Infinite (any CLI tool) |

## Conclusion

The boxes-live plugin architecture combines Unix philosophy with modern integration needs. Named pipes provide real-time updates, file watchers enable reactive updates, and scheduled commands handle periodic data. This creates a powerful, local-first alternative to cloud-based smart canvas tools while maintaining simplicity and composability.

The key innovation: **every Unix command becomes a potential plugin**. No SDK required—just pipes, files, and text.
