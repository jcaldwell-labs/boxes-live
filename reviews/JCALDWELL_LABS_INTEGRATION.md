# jcaldwell-labs Ecosystem Integration Recommendations

**Document Version**: 1.0
**Date**: 2025-11-18
**Scope**: boxes-live integration across jcaldwell-labs organization
**Reviewer**: Claude (Sonnet 4.5)

---

## Executive Summary

This document provides comprehensive recommendations for integrating boxes-live with the broader jcaldwell-labs ecosystem. Based on review of PRs #3 and #4, and understanding of the organization's Unix philosophy approach, these recommendations aim to create a cohesive, composable toolset.

**Key Opportunities Identified**:
1. my-context + boxes-live: Visual work tracking
2. Cross-project visualization connectors
3. Unified dashboards for all 8 projects
4. Organizational health metrics canvas
5. Developer workflow integration

---

## Organization Overview

### Current Projects (8 Total)

**Production Ready**:
1. **my-context** (Go) - Agent journal CLI, PostgreSQL backend

**MVP Complete**:
2. **boxes-live** (C/ncurses) - Interactive canvas (Miro clone)

**Phase 1 MVP**:
3. **fintrack** (Go) - Financial tracking, account CRUD complete

**Active Development**:
4. **terminal-stars** (C/ncurses) - Starfield visualization
5. **tario** (C/ANSI) - Side-scrolling platformer
6. **adventure-engine-v2** (C) - Text adventure engine DSL

**Decision Point**:
7. **smartterm-prototype** (C/ncurses) - Terminal UI library POC

**Infrastructure**:
8. **.github** - Organization-wide workflows

### Open PRs (4 Total)

1. PR #5: terminal-stars - Add horizon and disable star field (skeet mode)
2. PR #3: fintrack - Write a CLAUDE.md
3. PR #2: smartterm-prototype - Write a CLAUDE.md
4. PR #1: .github - Improve session persistence

---

## Vision: Unix Philosophy Mapping

### Enterprise → Terminal Alternative Strategy

**Philosophy**:
- Do one thing well
- Compose via pipes/files
- Text/data streams
- SSH-friendly
- No GUI dependencies

**Current Mappings**:
- Miro → boxes-live ✓
- Jira → my-context ✓
- Excel → fintrack (Phase 1) ⚙️

**Future Opportunities**:
- Web Browser → tmux-based (Concept)
- X-Ray → Active docs (Planning)
- IDE → vim/neovim integration

---

## Integration Scenarios

### 1. my-context + boxes-live

**Status**: 🟢 HIGH PRIORITY

**Vision**: Visualize my-context journal data as interactive canvases

#### 1.1 Context Visualization Connector

**New File**: `connectors/context2canvas`

**Capabilities**:
```bash
# Timeline view of recent work
my-context export --format json --from "2025-11-01" | \
    context2canvas --layout timeline --group-by project > work.txt

# Project interconnections
context2canvas --partition "db:jcaldwell_labs" \
               --show-links \
               --depth 2 > project-map.txt

# Daily work summary
my-context export --date today | \
    context2canvas --layout compact > today.txt

# Weekly review canvas
context2canvas --week --summary > week-review.txt
```

**Implementation**:
```bash
#!/bin/bash
# connectors/context2canvas

# Read JSON from my-context export
# Parse sessions, notes, metadata
# Generate boxes for each work session
# Color-code by project/context
# Add connections for related work
# Output canvas format
```

**Box Content Example**:
```
┌─────────────────────────────────┐
│ boxes-live: PR Review          │
│ 2025-11-18 14:00 - 16:30       │
│                                 │
│ • Reviewed PRs #3 and #4       │
│ • Comprehensive testing         │
│ • Feature planning doc          │
│                                 │
│ Context: db:boxes_live_dev      │
│ Duration: 2.5 hours             │
│ Status: ✓ Complete              │
└─────────────────────────────────┘
```

#### 1.2 Bidirectional Integration

**my-context → boxes-live** (Read):
- Export journal entries to canvas
- Visualize work patterns
- Generate weekly reviews

**boxes-live → my-context** (Write):
- Add canvas planning sessions to journal
- Log time spent in boxes-live
- Track canvas iterations

**Implementation**:
```bash
# Log canvas session to my-context
boxes-live my-canvas.txt
# On exit: auto-log to my-context

export MY_CONTEXT_AUTO_LOG=true
export MY_CONTEXT_PARTITION=db:boxes_live_dev

# my-context receives:
# - Start time
# - End time
# - Canvas file
# - Changes made
```

#### 1.3 Unified Dashboard

**File**: `~/dashboards/jcaldwell-labs-work.txt`

**Contents**:
- Box 1: Today's my-context entries
- Box 2: Active canvases
- Box 3: Project status (all 8 projects)
- Box 4: Open PRs
- Box 5: Health metrics
- Box 6: Next actions

**Update Mechanism**:
```bash
# Regenerate dashboard
~/bin/update-dashboard.sh

# Auto-regenerate on cron
*/15 * * * * ~/bin/update-dashboard.sh

# Or use periodic2canvas with my-context data
periodic2canvas --command "context2canvas --today" \
                --interval 900 \
                --canvas ~/dashboards/work.txt &
```

---

### 2. fintrack + boxes-live

**Status**: 🟡 MEDIUM PRIORITY (Wait for fintrack Phase 2)

**Vision**: Financial data visualization

#### 2.1 Account Hierarchy Connector

**New File**: `connectors/fintrack2canvas`

**Capabilities**:
```bash
# Account structure
fintrack accounts list --format json | \
    fintrack2canvas --layout tree > accounts.txt

# Budget breakdown
fintrack budget --month 2025-11 | \
    fintrack2canvas --layout pie-grid > budget.txt

# Transaction flow (Phase 2+)
fintrack transactions --from 2025-11-01 --to 2025-11-30 | \
    fintrack2canvas --layout sankey > cashflow.txt
```

**Box Content Example**:
```
┌──────────────────────────────┐
│ Checking Account            │
│ Wells Fargo ****1234        │
│                              │
│ Balance: $5,432.10          │
│ Available: $5,432.10        │
│                              │
│ Last Updated: 2025-11-18    │
│ Status: ✓ Active            │
└──────────────────────────────┘
```

#### 2.2 Financial Dashboard

**Use Cases**:
- Monthly budget tracking
- Account overview
- Net worth visualization
- Spending patterns

**Canvas Layout**:
```
Top Row:     Net worth summary
Second Row:  Account balances (grid)
Third Row:   Budget categories (current month)
Bottom Row:  Recent transactions, alerts
```

---

### 3. Cross-Project Visualization

**Status**: 🟢 HIGH PRIORITY

**Vision**: Unified view of all jcaldwell-labs projects

#### 3.1 Organization Dashboard (PR #4 Concept)

**File**: `connectors/jcaldwell-labs2canvas` (already exists in PR #4)

**Enhancements Needed**:
1. **Dynamic Data**: Query GitHub API instead of hardcoded
2. **Real-time PRs**: Fetch actual PR status
3. **Commit Activity**: Show recent activity
4. **Health Metrics**: Calculate from real data

**Enhanced Implementation**:
```bash
#!/bin/bash
# connectors/jcaldwell-labs2canvas (v2)

# Require GitHub token
if [ -z "$GITHUB_TOKEN" ]; then
    echo "Error: GITHUB_TOKEN required" >&2
    exit 1
fi

# Fetch org data
gh api orgs/jcaldwell-labs/repos --paginate > /tmp/repos.json

# Fetch open PRs across all repos
gh pr list --search "org:jcaldwell-labs is:open" --json title,url,repo > /tmp/prs.json

# Fetch recent commits (last 7 days)
for repo in $(jq -r '.[].name' /tmp/repos.json); do
    gh api "repos/jcaldwell-labs/$repo/commits?since=$(date -d '7 days ago' -I)" >> /tmp/commits.json
done

# Generate canvas with real data
# ... (rest of implementation)
```

#### 3.2 Per-Project Connectors

**Create connectors for each major project**:

**my-context**:
```bash
# connectors/mycontext2canvas
mycontext2canvas --partition all \
                 --group-by project \
                 --time-range week > my-context-work.txt
```

**fintrack**:
```bash
# connectors/fintrack2canvas
fintrack2canvas --accounts \
                --transactions \
                --layout hierarchy > fintrack-overview.txt
```

**terminal-stars**:
```bash
# connectors/game-stats2canvas
game-stats2canvas --game terminal-stars \
                  --scores \
                  --achievements > game-stats.txt
```

---

### 4. Development Workflow Integration

**Status**: 🟢 HIGH PRIORITY

**Vision**: boxes-live as part of daily development workflow

#### 4.1 tmux Integration

**Setup** (`~/.tmux.conf`):
```bash
# Quick dashboard access
bind-key M-d split-window -h "boxes-live ~/dashboards/jcaldwell-labs.txt"

# Project-specific dashboards
bind-key M-b new-window -n "boxes-live" "boxes-live ~/dashboards/boxes-live-dev.txt"
bind-key M-m new-window -n "my-context" "boxes-live ~/dashboards/my-context-dev.txt"

# Quick canvas
bind-key M-c split-window -v "boxes-live ~/quick-notes.txt"

# Git dashboard for current repo
bind-key M-g split-window -h "git2canvas --max 20 | boxes-live"
```

**Use Cases**:
- Persistent project dashboard in tmux pane
- Quick note-taking canvas
- Git history visualization
- System monitoring

#### 4.2 Git Hooks Integration

**Pre-Commit Hook**:
```bash
#!/bin/bash
# .git/hooks/pre-commit

# Update project canvas with commit info
if [ -f ~/dashboards/$(basename $(pwd))-dev.txt ]; then
    git diff --staged --stat | \
        commit2canvas --append ~/dashboards/$(basename $(pwd))-dev.txt
fi
```

**Post-Merge Hook**:
```bash
#!/bin/bash
# .git/hooks/post-merge

# Regenerate git history canvas
git2canvas --max 30 > ~/dashboards/$(basename $(pwd))-git.txt

# Notify via boxes-live (if running)
pkill -SIGUSR1 boxes-live
```

#### 4.3 CI/CD Integration

**GitHub Actions Workflow**:
```yaml
# .github/workflows/visualize.yml
name: Generate Project Visualization

on:
  push:
    branches: [main]
  schedule:
    - cron: '0 0 * * *'  # Daily

jobs:
  visualize:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install boxes-live
        run: |
          curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/boxes-live/main/install.sh | bash
      - name: Generate canvas
        run: |
          ./connectors/codebase2canvas --depth 3 > architecture.txt
      - name: Upload artifact
        uses: actions/upload-artifact@v3
        with:
          name: project-canvas
          path: architecture.txt
```

---

### 5. Unified Connector Ecosystem

**Status**: 🟡 MEDIUM PRIORITY

**Vision**: Consistent connector patterns across all projects

#### 5.1 Standard Connector Library

**New File**: `connectors/lib/connector-base.sh`

**Purpose**: Shared functions for all connectors

**Contents**:
```bash
#!/bin/bash
# Connector framework library

# Initialize canvas
connector_init() {
    local title="$1"
    local width="${2:-2000}"
    local height="${3:-1500}"

    CONNECTOR_CANVAS=$(mktemp)
    trap "rm -f $CONNECTOR_CANVAS" EXIT

    boxes-cli create "$CONNECTOR_CANVAS" --width "$width" --height "$height"

    # Add title box
    boxes-cli add "$CONNECTOR_CANVAS" \
        --x $(( width / 2 - 100 )) --y 50 \
        --width 40 --height 6 \
        --title "$title" \
        --color 6
}

# Add box with auto-positioning
connector_add_box() {
    local title="$1"
    shift
    local content=("$@")

    # Auto-calculate position based on current count
    local box_count=$(boxes-cli stats "$CONNECTOR_CANVAS" | grep "Total boxes" | awk '{print $3}')
    local col=$(( box_count % 4 ))
    local row=$(( box_count / 4 ))
    local x=$(( 200 + col * 400 ))
    local y=$(( 200 + row * 250 ))

    boxes-cli add "$CONNECTOR_CANVAS" \
        --x $x --y $y \
        --width 35 --height 12 \
        --title "$title" \
        --color $(( (box_count % 6) + 1 )) \
        "${content[@]}"
}

# Finalize and output
connector_output() {
    cat "$CONNECTOR_CANVAS"
}
```

**Usage in Connectors**:
```bash
#!/bin/bash
# connectors/example2canvas

source "$(dirname "$0")/lib/connector-base.sh"

connector_init "Example Canvas"

# Add boxes
connector_add_box "Item 1" --content "Content A" --content "Content B"
connector_add_box "Item 2" --content "Content C"
connector_add_box "Item 3" --content "Content D"

connector_output
```

#### 5.2 Per-Project Connector Standards

**Naming Convention**:
- `{project}2canvas` - Project-specific connector
- `{datatype}2canvas` - Generic data connector
- `{source}2canvas` - External source connector

**Examples**:
```
connectors/
├── lib/
│   └── connector-base.sh
├── jcaldwell-labs2canvas  # Org overview
├── mycontext2canvas       # my-context data
├── fintrack2canvas        # fintrack data
├── github2canvas          # Generic GitHub
├── git2canvas             # Git history
├── csv2canvas             # CSV import
├── json2canvas            # JSON import
├── database2canvas        # SQL queries
└── logs2canvas            # Log aggregation
```

---

### 6. Documentation Coordination

**Status**: 🟡 MEDIUM PRIORITY

**Vision**: Consistent documentation across all projects

#### 6.1 CLAUDE.md Standards

**Current Status**:
- boxes-live: ✓ Comprehensive CLAUDE.md
- fintrack: PR #3 (pending)
- smartterm-prototype: PR #2 (pending)
- Others: Missing

**Recommended Structure** (based on boxes-live CLAUDE.md):
```markdown
# CLAUDE.md

## Project Overview
Brief description, key concepts

## Build System
Commands, dependencies

## Development Commands
Building, running, testing, debugging

## Architecture
Directory structure, core components, data flow

## Code Style and Conventions
Standards, patterns, conventions
```

**Action**: Create CLAUDE.md for remaining 5 projects using boxes-live as template

#### 6.2 Cross-Project Documentation Canvas

**New File**: `~/dashboards/jcaldwell-labs-docs.txt`

**Contents**:
- Box per project with:
  - Documentation status
  - Test coverage
  - Last updated
  - Contributors
  - Links to key docs

**Auto-Generated**:
```bash
# connectors/docs-status2canvas
docs-status2canvas --org jcaldwell-labs \
                   --check README.md \
                   --check CLAUDE.md \
                   --check TESTING.md \
                   --check CONTRIBUTING.md > docs-status.txt
```

---

### 7. Health Metrics Dashboard

**Status**: 🟢 HIGH PRIORITY

**Vision**: Real-time organization health monitoring

#### 7.1 Metrics to Track

**Per-Project**:
- Last commit date (freshness)
- Open issues count
- Open PRs count
- Test pass rate
- Code coverage
- Active contributors

**Organization-Wide**:
- Total projects
- Production projects
- Active development projects
- Total open PRs
- Average PR age
- Days since last commit (any project)

#### 7.2 Health Dashboard Canvas

**File**: `~/dashboards/jcaldwell-labs-health.txt`

**Auto-Updated**:
```bash
# Update every 15 minutes
*/15 * * * * ~/bin/update-health-dashboard.sh

# ~/bin/update-health-dashboard.sh
#!/bin/bash
jcaldwell-labs2canvas --dynamic \
                      --github-token "$GITHUB_TOKEN" \
                      --health-metrics > ~/dashboards/jcaldwell-labs-health.txt

# Send SIGUSR1 to reload if boxes-live is running
pkill -SIGUSR1 boxes-live
```

**Box Example**:
```
┌─────────────────────────────────┐
│ my-context Health              │
│                                 │
│ Status: ✓ Excellent            │
│ Last Commit: 2 hours ago        │
│ Open PRs: 0                     │
│ Open Issues: 2                  │
│ Test Coverage: 85%              │
│ Active Contributors: 2          │
│                                 │
│ Trend: ↗ Improving             │
└─────────────────────────────────┘
```

---

### 8. Testing Strategy

**Status**: 🟢 HIGH PRIORITY

**Vision**: Consistent testing across all projects

#### 8.1 Test Coverage Dashboard

**Connector**: `coverage2canvas`

**Purpose**: Visualize test coverage across all projects

```bash
# Run for all projects
for project in my-context boxes-live fintrack terminal-stars; do
    cd ~/projects/$project
    make test-coverage 2>/dev/null || echo "No coverage for $project"
done | coverage2canvas --group-by project > test-coverage.txt
```

**Canvas Shows**:
- Per-project coverage bars
- Uncovered modules highlighted
- Coverage trend over time
- Critical path coverage

#### 8.2 Coordinated Testing

**GitHub Actions Matrix**:
```yaml
# .github/workflows/cross-project-test.yml
name: Cross-Project Integration Tests

on: [push, pull_request]

jobs:
  test:
    strategy:
      matrix:
        project: [my-context, boxes-live, fintrack]
    runs-on: ubuntu-latest
    steps:
      - name: Test ${{ matrix.project }}
        run: |
          cd ${{ matrix.project }}
          make test
```

---

### 9. MCP Server Opportunities

**Status**: 🔵 FUTURE (Post-1.0.0)

**Vision**: AI agents orchestrating jcaldwell-labs tools

#### 9.1 Unified MCP Server

**New Project**: `jcaldwell-labs-mcp-server`

**Exposes**:
- my-context: Journal operations
- boxes-live: Canvas operations
- fintrack: Financial operations

**Example Workflow**:
```
User: "Create a visual summary of my work this week"

AI Agent:
1. Call my-context MCP: export(from: "2025-11-11", to: "2025-11-18")
2. Parse journal entries
3. Call boxes-live MCP: create_canvas(title: "Week Summary")
4. Call boxes-live MCP: add_box() for each day
5. Call boxes-live MCP: layout_auto(algorithm: "timeline")
6. Return canvas file path

Result: Visual weekly review canvas
```

#### 9.2 Cross-Project Agents

**Agent Scenarios**:
1. **Project Status Agent**: Aggregates status from all 8 projects → canvas
2. **Work Planning Agent**: Reads my-context goals → generates project canvas
3. **Budget Agent**: Reads fintrack data → creates visual budget board
4. **Code Review Agent**: Analyzes PRs → generates review canvas

---

## Implementation Priority

### Phase 1: Foundation (Now)

**Priority**: 🔴 CRITICAL

1. ✅ Fix PR #3 issues (completed)
2. ⚠️ Fix PR #4 canvas loading bug
3. ✅ Merge PR #3 (ready)
4. 🔄 Fix and merge PR #4

**Effort**: 4-8 hours

### Phase 2: Core Integration (Next 2 weeks)

**Priority**: 🟠 HIGH

1. Create `context2canvas` connector
2. Enhance `jcaldwell-labs2canvas` with GitHub API
3. Create unified health dashboard
4. Add tmux integration examples

**Effort**: 16-24 hours

### Phase 3: Ecosystem Expansion (Next month)

**Priority**: 🟡 MEDIUM

1. Create `fintrack2canvas` connector (when Phase 2 ready)
2. Standardize all CLAUDE.md files
3. Create connector framework library
4. Build per-project dashboards

**Effort**: 24-32 hours

### Phase 4: Advanced Features (2-3 months)

**Priority**: 🟢 LOW

1. MCP server integration
2. CI/CD canvas generation
3. Bidirectional connectors
4. AI agent orchestration

**Effort**: 40-60 hours

---

## Success Metrics

### Short-Term (1 month)

- ✅ boxes-live 1.0.0 released
- ✅ 2+ connectors for jcaldwell-labs projects
- ✅ 1+ developer using dashboard daily
- ✅ All 8 projects have CLAUDE.md

### Medium-Term (3 months)

- 5+ connectors for jcaldwell-labs ecosystem
- 3+ developers using boxes-live daily
- Dashboards integrated into workflow
- Health metrics tracked automatically

### Long-Term (6 months)

- MCP server deployed
- AI agents using boxes-live
- Cross-project visualizations standard
- External organizations adopting pattern

---

## Recommended Next Actions

### Immediate (This Week)

1. **Fix PR #4 Bug**: Fix boxes-cli canvas loading (4 hours)
2. **Merge PRs**: Get #3 and #4 merged (1 hour)
3. **Release 1.0.0-rc1**: Create pre-release tag (1 hour)

### Next Week

4. **context2canvas**: Build my-context connector (8 hours)
5. **Health Dashboard**: Dynamic jcaldwell-labs canvas (4 hours)
6. **tmux Integration**: Document and share patterns (2 hours)

### Next Month

7. **CLAUDE.md Standardization**: Create for all 8 projects (8 hours)
8. **Connector Framework**: Build reusable library (16 hours)
9. **CI/CD Integration**: Auto-generate canvases (8 hours)

---

## Conclusion

boxes-live has enormous potential as the visual coordination hub for the jcaldwell-labs ecosystem. By integrating with my-context, fintrack, and other projects, it can become the go-to tool for:

- Work visualization
- Project coordination
- Health monitoring
- Progress tracking
- Planning and organization

The Unix philosophy approach—composable tools that do one thing well—positions boxes-live perfectly for ecosystem integration. With the recommended connectors and integrations, the organization can achieve:

**Unified Visibility**: One place to see all project status
**Visual Coordination**: Clear view of interconnections
**Automated Monitoring**: Real-time health metrics
**Developer Productivity**: Tools that enhance workflow
**AI Integration**: Future-ready for agent orchestration

**Start with Phase 1 & 2** (next 2-3 weeks) to establish foundation, then expand based on usage patterns and feedback.

---

**Document Prepared By**: Claude (Sonnet 4.5)
**Date**: 2025-11-18
**Next Review**: After 1.0.0 release

**Status**: ✅ Ready for implementation
**Impact**: 🌟 High - Transforms organization coordination
