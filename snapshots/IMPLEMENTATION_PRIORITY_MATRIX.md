╔═══════════════════════════════════════════════════════════════════════════════════════════════════╗
║                        🎯 Implementation Priority Matrix                                          ║
║                       Based on Snapshot Analysis & Use Case Frequency                             ║
╚═══════════════════════════════════════════════════════════════════════════════════════════════════╝

Analyzed snapshots:
  • developer_dashboard.txt       - Live monitoring (DevOps, SRE)
  • project_planning.txt           - Agile boards (PM, teams)
  • git_timeline_zoom_levels.txt   - Code exploration (developers)
  • meeting_notes_patat.txt        - Presentations (everyone)
  • research_canvas.txt            - Academia (researchers)

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 🔥 TIER 1: CRITICAL - Appears in 4-5 snapshots

These features are essential across almost all use cases:

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 1. COLOR-CODED STATUS INDICATORS                                                          │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 5/5 (developer_dashboard, project_planning, git_timeline, patat, research)  │
│    Use cases: Status tracking, categorization, visual grouping                            │
│    Complexity: LOW (already supported, needs UI exposure)                                 │
│    Impact: HIGH (fundamental to all visual organization)                                  │
│    Estimate: 3-5 days                                                                     │
│                                                                                            │
│    Implementation:                                                                         │
│    ✓ Already have 7 colors in boxes (red, green, blue, yellow, magenta, cyan, white)     │
│    ✓ CLI supports --color flag                                                            │
│    → Add keyboard shortcuts to change color of selected box                               │
│    → Add color picker UI (numeric keys 1-7 already work!)                                 │
│    → Add color legend/key to status bar                                                   │
└────────────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 2. CONNECTION LINES / ARROWS BETWEEN BOXES                                                │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 5/5 (developer_dashboard, project_planning, git_timeline, patat, research)  │
│    Use cases: Dependencies, citations, flow diagrams, relationships                       │
│    Complexity: MEDIUM (line drawing, routing, persistence)                                │
│    Impact: VERY HIGH (unlocks diagram use cases)                                          │
│    Estimate: 2-3 weeks                                                                    │
│                                                                                            │
│    Implementation:                                                                         │
│    → Add Connection data structure (src_box_id, dst_box_id, style, label)                 │
│    → Bresenham line drawing in render.c                                                   │
│    → Arrow rendering (Unicode arrows: →, ↓, ↗, etc.)                                      │
│    → UI: Select box 1, press 'C', select box 2 to create connection                       │
│    → Persistence: Extend canvas format with connections section                           │
│    → Smart routing: Avoid overlapping boxes when possible                                 │
└────────────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 3. EXPORT TO MULTIPLE FORMATS (Markdown, JSON, SVG, PNG)                                  │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 5/5 (all snapshots mention export needs)                                    │
│    Use cases: Sharing, documentation, reports, presentations                              │
│    Complexity: MEDIUM (Markdown/JSON done, SVG/PNG need work)                             │
│    Impact: HIGH (sharing and integration)                                                 │
│    Estimate: 1-2 weeks                                                                    │
│                                                                                            │
│    Implementation:                                                                         │
│    ✓ Markdown export exists (boxes-cli)                                                   │
│    ✓ JSON export exists (boxes-cli)                                                       │
│    ✓ CSV export exists (boxes-cli)                                                        │
│    → SVG export: Render boxes as <rect> and text as <text>                                │
│    → PNG export: Use ImageMagick or similar to convert SVG → PNG                          │
│    → Include connections in visual exports                                                │
│    → Add export menu in boxes-live (press 'E')                                            │
└────────────────────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 🚀 TIER 2: HIGH PRIORITY - Appears in 3-4 snapshots

Core features that enable major use cases:

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 4. LIVE-UPDATING BOXES (Named Pipes / File Watchers)                                      │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 3/5 (developer_dashboard, project_planning, patat)                          │
│    Use cases: Monitoring, CI/CD status, real-time collaboration                           │
│    Complexity: MEDIUM (inotify, polling, threading)                                       │
│    Impact: VERY HIGH (enables live dashboards)                                            │
│    Estimate: 2-3 weeks                                                                    │
│                                                                                            │
│    Implementation (from PLUGIN_ARCHITECTURE.md):                                           │
│    → Add pipe_path and refresh_interval_ms to Box structure                               │
│    → Implement inotify file watching for auto-reload                                      │
│    → Named pipe reading in main loop (non-blocking)                                       │
│    → Adaptive polling with exponential backoff                                            │
│    → UI indicator showing live boxes (🔄 icon)                                            │
│    → boxes-cli --pipe flag for creating live boxes                                        │
└────────────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 5. ZOOM LEVELS WITH PROGRESSIVE DETAIL                                                    │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 3/5 (git_timeline shows 10%/50%/100%, others imply need)                    │
│    Use cases: Large canvases, overview + detail pattern                                   │
│    Complexity: HIGH (rendering optimization, detail hiding)                                │
│    Impact: HIGH (scalability for large projects)                                          │
│    Estimate: 3-4 weeks                                                                    │
│                                                                                            │
│    Implementation:                                                                         │
│    → Multi-level zoom (10%, 25%, 50%, 75%, 100%, 150%, 200%)                              │
│    → Box templates: minimal/compact/detailed based on zoom                                │
│    → Viewport culling: Only render visible boxes                                          │
│    → LOD (Level of Detail): Hide content at low zoom                                      │
│    → Smooth zoom transitions                                                              │
│    → Zoom-to-selection (Z key)                                                            │
└────────────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 6. TWO-WAY SYNC (GitHub Issues, Jira, etc.)                                               │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 3/5 (project_planning, patat action items, research citations)              │
│    Use cases: Project management, task tracking, bibliography                             │
│    Complexity: HIGH (API integration, conflict resolution)                                │
│    Impact: VERY HIGH (professional tool integration)                                      │
│    Estimate: 4-6 weeks per integration                                                    │
│                                                                                            │
│    Implementation (GitHub first):                                                          │
│    → Use gh CLI for API access                                                            │
│    → Metadata field: github:issue:NUMBER                                                  │
│    → Sync daemon (plugins/github-sync.py)                                                 │
│    → Webhook receiver for real-time updates                                               │
│    → Conflict resolution UI                                                               │
│    → Status mapping: Open→TODO, In Progress→IN_PROGRESS, Closed→DONE                      │
└────────────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 7. BOX GROUPING / SWIMLANES / CONTAINERS                                                  │
├────────────────────────────────────────────────────────────────────────────────────────────┤
│    Snapshots: 3/5 (project_planning kanban, patat sections, research topics)              │
│    Use cases: Kanban columns, topic areas, hierarchical organization                      │
│    Complexity: MEDIUM (parent-child relationships, visual nesting)                        │
│    Impact: HIGH (structured layouts)                                                      │
│    Estimate: 2-3 weeks                                                                    │
│                                                                                            │
│    Implementation:                                                                         │
│    → Add parent_id field to Box structure                                                 │
│    → Container boxes (larger, semi-transparent background)                                │
│    → Drag box into container to set parent                                                │
│    → Auto-arrange children within container                                               │
│    → Collapse/expand containers                                                           │
│    → Render container box behind children                                                 │
└────────────────────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 📊 TIER 3: MEDIUM PRIORITY - Appears in 2-3 snapshots

Valuable features for specific use cases:

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 8. RICH METADATA (Tags, Assignments, Due Dates, Links)                                    │
│    Snapshots: 3/5 (project_planning, patat, research)                                     │
│    Estimate: 1-2 weeks                                                                    │
│                                                                                            │
│ 9. SEARCH AND FILTERING                                                                   │
│    Snapshots: 3/5 (all large canvases need search)                                        │
│    Estimate: 1 week (basic search exists, needs UI)                                       │
│                                                                                            │
│ 10. PRESENTATION MODE / SLIDE NAVIGATION                                                  │
│     Snapshots: 2/5 (patat, research - sequential viewing)                                 │
│     Estimate: 2 weeks                                                                     │
│                                                                                            │
│ 11. ACTIVITY FEED / HISTORY                                                               │
│     Snapshots: 2/5 (project_planning, patat - audit log)                                  │
│     Estimate: 1-2 weeks                                                                   │
└────────────────────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 🎨 TIER 4: NICE TO HAVE - Appears in 1-2 snapshots

Polish and specialized features:

┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ 12. BOX RESIZING                                                                           │
│     Snapshots: 2/5 (varied box sizes in most snapshots)                                   │
│     Estimate: 1 week                                                                      │
│                                                                                            │
│ 13. TEXT EDITING WITHIN BOXES                                                             │
│     Snapshots: 2/5 (patat notes, research annotations)                                    │
│     Estimate: 2-3 weeks                                                                   │
│                                                                                            │
│ 14. EMBEDDED CONTENT (Images, Equations, Code)                                            │
│     Snapshots: 2/5 (patat, research - rich content)                                       │
│     Estimate: 3-4 weeks                                                                   │
│                                                                                            │
│ 15. REAL-TIME COLLABORATION                                                               │
│     Snapshots: 2/5 (patat meetings, project_planning)                                     │
│     Estimate: 6-8 weeks (complex)                                                         │
└────────────────────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 📅 RECOMMENDED IMPLEMENTATION ROADMAP

### Phase 1: Visual Foundation (4-6 weeks)
Priority: Make boxes-live visually competitive with Miro for static content

1. Connection lines/arrows (2-3 weeks) ← HIGHEST IMPACT
2. Enhanced color coding UI (3-5 days)
3. Box grouping/containers (2-3 weeks)
4. SVG/PNG export (1-2 weeks)

**Outcome**: Can create professional diagrams, flowcharts, mindmaps

### Phase 2: Live Data Integration (6-8 weeks)
Priority: Enable real-time monitoring and updates

5. Live-updating boxes via pipes (2-3 weeks)
6. File watchers for auto-reload (1 week)
7. System monitoring plugins (1 week)
8. Webhook receiver (1-2 weeks)

**Outcome**: Can build live dashboards, monitoring tools

### Phase 3: Professional Integrations (8-12 weeks)
Priority: Connect with real workflows

9. GitHub Issues two-way sync (4-6 weeks)
10. Search and filtering UI (1 week)
11. Rich metadata support (1-2 weeks)
12. Presentation mode (2 weeks)

**Outcome**: Viable Miro/Jira/Linear alternative

### Phase 4: Scalability & Polish (4-6 weeks)
Priority: Handle large, complex canvases

13. Multi-level zoom with LOD (3-4 weeks)
14. Box resizing (1 week)
15. Activity history (1-2 weeks)

**Outcome**: Production-ready for large teams

═══════════════════════════════════════════════════════════════════════════════════════════════════

## 🎯 MVP RECOMMENDATION

**If you had to pick 3 features to implement first** (next 4-6 weeks):

1. **Connection lines/arrows** (3 weeks)
   - Unlocks: Flowcharts, architecture diagrams, mindmaps, dependencies
   - Appears in: ALL snapshots
   - Differentiator: Most visual impact

2. **Live-updating boxes** (2 weeks)
   - Unlocks: Developer dashboards, monitoring, CI/CD status
   - Appears in: 3/5 snapshots
   - Differentiator: Unique capability (Miro doesn't do this well)

3. **SVG/PNG export** (1 week)
   - Unlocks: Sharing, documentation, presentations
   - Appears in: ALL snapshots
   - Differentiator: Integration with existing tools

**Total: 6 weeks → Boxes-live becomes genuinely useful for real work**

═══════════════════════════════════════════════════════════════════════════════════════════════════
