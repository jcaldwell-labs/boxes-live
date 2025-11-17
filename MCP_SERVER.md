# boxes-live MCP Server

## Overview

This document outlines the Model Context Protocol (MCP) server implementation for boxes-live, enabling AI assistants like Claude to interact with the canvas programmatically. While Miro offers an official MCP server, boxes-live's CLI-first design provides superior context efficiency and Unix philosophy integration.

## Why boxes-live MCP > Miro MCP

| Feature | boxes-live | Miro |
|---------|------------|------|
| **Context Size** | Plain text, ~100 bytes/box | JSON API, ~500+ bytes/box |
| **Latency** | Local file I/O, <1ms | HTTP API, 100-500ms |
| **Offline** | Fully functional | Requires internet |
| **Composability** | Pipes, filters, streams | HTTP only |
| **Version Control** | Git-friendly text | Binary/JSON blobs |
| **Privacy** | Local-first | Cloud-hosted |
| **Cost** | Free, unlimited | Rate limits, paid tiers |
| **Integration** | stdin/stdout, files | REST API, webhooks |

## MCP Server Architecture

```
┌─────────────────┐
│  Claude Code    │
│   (AI Client)   │
└────────┬────────┘
         │ MCP Protocol (stdio)
         ▼
┌─────────────────┐
│ boxes-live MCP  │
│     Server      │
└────────┬────────┘
         │
    ┌────┴────┬──────────┬──────────┐
    ▼         ▼          ▼          ▼
┌────────┐ ┌────────┐ ┌────────┐ ┌─────────┐
│ Canvas │ │ Boxes  │ │ Export │ │ Search  │
│  CRUD  │ │ Query  │ │ Tools  │ │ Filter  │
└────────┘ └────────┘ └────────┘ └─────────┘
```

## MCP Tools Specification

### 1. Canvas Management

#### `canvas_create`
Create a new canvas

```json
{
  "name": "canvas_create",
  "description": "Create a new blank canvas",
  "inputSchema": {
    "type": "object",
    "properties": {
      "filename": {"type": "string", "description": "Canvas file path"},
      "width": {"type": "number", "default": 2000},
      "height": {"type": "number", "default": 1500}
    },
    "required": ["filename"]
  }
}
```

#### `canvas_load`
Load existing canvas

```json
{
  "name": "canvas_load",
  "description": "Load canvas from file",
  "inputSchema": {
    "type": "object",
    "properties": {
      "filename": {"type": "string"}
    },
    "required": ["filename"]
  }
}
```

#### `canvas_save`
Save current canvas

```json
{
  "name": "canvas_save",
  "description": "Save canvas to file",
  "inputSchema": {
    "type": "object",
    "properties": {
      "filename": {"type": "string"}
    },
    "required": ["filename"]
  }
}
```

### 2. Box Operations

#### `box_create`
Add a box to the canvas

```json
{
  "name": "box_create",
  "description": "Create a new box on the canvas",
  "inputSchema": {
    "type": "object",
    "properties": {
      "x": {"type": "number"},
      "y": {"type": "number"},
      "width": {"type": "number", "default": 30},
      "height": {"type": "number", "default": 8},
      "title": {"type": "string"},
      "content": {"type": "array", "items": {"type": "string"}},
      "color": {"type": "number", "minimum": 0, "maximum": 7, "default": 0}
    },
    "required": ["x", "y", "title"]
  }
}
```

#### `box_update`
Modify existing box

```json
{
  "name": "box_update",
  "description": "Update box properties",
  "inputSchema": {
    "type": "object",
    "properties": {
      "box_id": {"type": "number"},
      "x": {"type": "number"},
      "y": {"type": "number"},
      "title": {"type": "string"},
      "content": {"type": "array", "items": {"type": "string"}},
      "color": {"type": "number"}
    },
    "required": ["box_id"]
  }
}
```

#### `box_delete`
Remove a box

```json
{
  "name": "box_delete",
  "description": "Delete box from canvas",
  "inputSchema": {
    "type": "object",
    "properties": {
      "box_id": {"type": "number"}
    },
    "required": ["box_id"]
  }
}
```

#### `box_get`
Retrieve box details

```json
{
  "name": "box_get",
  "description": "Get box details by ID",
  "inputSchema": {
    "type": "object",
    "properties": {
      "box_id": {"type": "number"}
    },
    "required": ["box_id"]
  }
}
```

### 3. Query and Search

#### `boxes_list`
List all boxes on canvas

```json
{
  "name": "boxes_list",
  "description": "List all boxes with optional filtering",
  "inputSchema": {
    "type": "object",
    "properties": {
      "color": {"type": "number", "description": "Filter by color"},
      "search": {"type": "string", "description": "Search in title/content"},
      "region": {
        "type": "object",
        "description": "Filter by region",
        "properties": {
          "x1": {"type": "number"},
          "y1": {"type": "number"},
          "x2": {"type": "number"},
          "y2": {"type": "number"}
        }
      }
    }
  }
}
```

#### `boxes_search`
Full-text search across canvas

```json
{
  "name": "boxes_search",
  "description": "Search box titles and content",
  "inputSchema": {
    "type": "object",
    "properties": {
      "query": {"type": "string"},
      "case_sensitive": {"type": "boolean", "default": false}
    },
    "required": ["query"]
  }
}
```

### 4. Layout and Organization

#### `boxes_arrange`
Auto-arrange boxes

```json
{
  "name": "boxes_arrange",
  "description": "Auto-arrange boxes in layout pattern",
  "inputSchema": {
    "type": "object",
    "properties": {
      "layout": {
        "type": "string",
        "enum": ["grid", "tree", "circle", "flow"],
        "default": "grid"
      },
      "spacing": {"type": "number", "default": 50}
    }
  }
}
```

#### `boxes_group`
Group related boxes

```json
{
  "name": "boxes_group",
  "description": "Apply same color to related boxes",
  "inputSchema": {
    "type": "object",
    "properties": {
      "box_ids": {"type": "array", "items": {"type": "number"}},
      "color": {"type": "number"}
    },
    "required": ["box_ids", "color"]
  }
}
```

### 5. Export and Transform

#### `canvas_export`
Export canvas to various formats

```json
{
  "name": "canvas_export",
  "description": "Export canvas to different formats",
  "inputSchema": {
    "type": "object",
    "properties": {
      "format": {
        "type": "string",
        "enum": ["markdown", "json", "svg", "ascii"],
        "default": "markdown"
      },
      "output": {"type": "string", "description": "Output file path"}
    },
    "required": ["format"]
  }
}
```

### 6. Connectors

#### `connector_import`
Import data via connector

```json
{
  "name": "connector_import",
  "description": "Import data using connector script",
  "inputSchema": {
    "type": "object",
    "properties": {
      "connector": {"type": "string", "description": "Connector name (e.g., 'periodic2canvas')"},
      "input_file": {"type": "string"},
      "output_canvas": {"type": "string"}
    },
    "required": ["connector"]
  }
}
```

## Resource Providers

### `canvas://`
Access canvas files as resources

```json
{
  "uri": "canvas://path/to/file.txt",
  "name": "Project Canvas",
  "description": "Interactive project planning canvas",
  "mimeType": "text/plain"
}
```

### `box://`
Access individual boxes as resources

```json
{
  "uri": "box://canvas.txt#42",
  "name": "Box: Feature Ideas",
  "description": "Box #42 from canvas.txt",
  "mimeType": "text/plain"
}
```

## Implementation

### Python MCP Server

```python
#!/usr/bin/env python3
"""
boxes-live MCP Server

Provides Model Context Protocol interface to boxes-live canvas
"""

import asyncio
import json
import sys
from typing import Optional, Dict, Any, List
from mcp.server import Server
from mcp.types import Tool, TextContent, Resource

# Canvas manipulation functions
class CanvasManager:
    def __init__(self, filename: Optional[str] = None):
        self.filename = filename
        self.boxes = []
        self.world_width = 2000.0
        self.world_height = 1500.0

        if filename:
            self.load(filename)

    def load(self, filename: str):
        """Load canvas from file"""
        with open(filename, 'r') as f:
            lines = f.readlines()

        if lines[0].strip() != "BOXES_CANVAS_V1":
            raise ValueError("Invalid canvas format")

        self.world_width, self.world_height = map(float, lines[1].split())
        box_count = int(lines[2])

        self.boxes = []
        i = 3
        while len(self.boxes) < box_count and i < len(lines):
            parts = lines[i].split()
            box_id, x, y, width, height, selected, color = (
                int(parts[0]), float(parts[1]), float(parts[2]),
                int(parts[3]), int(parts[4]), int(parts[5]), int(parts[6])
            )

            title = lines[i+1].strip()
            content_lines = int(lines[i+2])
            content = [lines[i+3+j].strip() for j in range(content_lines)]

            self.boxes.append({
                'id': box_id,
                'x': x, 'y': y,
                'width': width, 'height': height,
                'selected': selected == 1,
                'color': color,
                'title': title,
                'content': content
            })

            i += 3 + content_lines

    def save(self, filename: str):
        """Save canvas to file"""
        with open(filename, 'w') as f:
            f.write("BOXES_CANVAS_V1\n")
            f.write(f"{self.world_width} {self.world_height}\n")
            f.write(f"{len(self.boxes)}\n")

            for box in self.boxes:
                f.write(f"{box['id']} {box['x']} {box['y']} "
                       f"{box['width']} {box['height']} "
                       f"{1 if box['selected'] else 0} {box['color']}\n")
                f.write(f"{box['title']}\n")
                f.write(f"{len(box['content'])}\n")
                for line in box['content']:
                    f.write(f"{line}\n")

    def create_box(self, x: float, y: float, title: str,
                   content: List[str], width: int = 30,
                   height: int = 8, color: int = 0) -> int:
        """Create new box"""
        box_id = max([b['id'] for b in self.boxes], default=0) + 1

        self.boxes.append({
            'id': box_id,
            'x': x, 'y': y,
            'width': width, 'height': height,
            'selected': False,
            'color': color,
            'title': title,
            'content': content
        })

        return box_id

    def delete_box(self, box_id: int) -> bool:
        """Delete box by ID"""
        original_len = len(self.boxes)
        self.boxes = [b for b in self.boxes if b['id'] != box_id]
        return len(self.boxes) < original_len

    def search_boxes(self, query: str, case_sensitive: bool = False) -> List[Dict]:
        """Search boxes by content"""
        results = []

        if not case_sensitive:
            query = query.lower()

        for box in self.boxes:
            title = box['title'] if case_sensitive else box['title'].lower()
            content_str = ' '.join(box['content'])
            if not case_sensitive:
                content_str = content_str.lower()

            if query in title or query in content_str:
                results.append(box)

        return results


# MCP Server implementation
server = Server("boxes-live")
canvas_manager = CanvasManager()

@server.list_tools()
async def list_tools() -> List[Tool]:
    """List available tools"""
    return [
        Tool(
            name="box_create",
            description="Create a new box on the canvas",
            inputSchema={
                "type": "object",
                "properties": {
                    "x": {"type": "number"},
                    "y": {"type": "number"},
                    "title": {"type": "string"},
                    "content": {"type": "array", "items": {"type": "string"}},
                    "width": {"type": "number", "default": 30},
                    "height": {"type": "number", "default": 8},
                    "color": {"type": "number", "default": 0}
                },
                "required": ["x", "y", "title"]
            }
        ),
        Tool(
            name="boxes_search",
            description="Search boxes by title and content",
            inputSchema={
                "type": "object",
                "properties": {
                    "query": {"type": "string"},
                    "case_sensitive": {"type": "boolean", "default": False}
                },
                "required": ["query"]
            }
        ),
        Tool(
            name="canvas_save",
            description="Save canvas to file",
            inputSchema={
                "type": "object",
                "properties": {
                    "filename": {"type": "string"}
                },
                "required": ["filename"]
            }
        ),
    ]

@server.call_tool()
async def call_tool(name: str, arguments: Dict[str, Any]) -> List[TextContent]:
    """Execute tool"""

    if name == "box_create":
        box_id = canvas_manager.create_box(
            x=arguments['x'],
            y=arguments['y'],
            title=arguments['title'],
            content=arguments.get('content', []),
            width=arguments.get('width', 30),
            height=arguments.get('height', 8),
            color=arguments.get('color', 0)
        )
        return [TextContent(
            type="text",
            text=f"Created box #{box_id} at ({arguments['x']}, {arguments['y']})"
        )]

    elif name == "boxes_search":
        results = canvas_manager.search_boxes(
            query=arguments['query'],
            case_sensitive=arguments.get('case_sensitive', False)
        )
        return [TextContent(
            type="text",
            text=json.dumps(results, indent=2)
        )]

    elif name == "canvas_save":
        canvas_manager.save(arguments['filename'])
        return [TextContent(
            type="text",
            text=f"Canvas saved to {arguments['filename']}"
        )]

    raise ValueError(f"Unknown tool: {name}")


async def main():
    """Run MCP server"""
    from mcp.server.stdio import stdio_server

    async with stdio_server() as (read_stream, write_stream):
        await server.run(read_stream, write_stream, server.create_initialization_options())


if __name__ == "__main__":
    asyncio.run(main())
```

### Installation

```bash
# Install MCP SDK
pip install mcp

# Make server executable
chmod +x mcp_server.py

# Test server
echo '{"method":"tools/list"}' | ./mcp_server.py
```

### Claude Desktop Integration

Add to `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "boxes-live": {
      "command": "/path/to/boxes-live/mcp_server.py",
      "args": []
    }
  }
}
```

## Usage Examples

### AI-Assisted Canvas Creation

```
User: Create a periodic table canvas

Claude: I'll use the boxes-live MCP server to create an interactive periodic table:
1. Creating canvas...
2. Adding elements with color coding by category...
3. Positioning based on groups and periods...
4. Saved to periodic_table.txt

You can now open it with boxes-live and navigate interactively!
```

### Research Organization

```
User: Help me organize these 50 research papers

Claude: I'll create a canvas to organize your research:
- Loading paper metadata...
- Creating boxes for each paper...
- Color coding by topic (ML=blue, NLP=green, CV=red)...
- Auto-arranging in grid layout...
- Adding citation relationships...

Canvas saved to research.txt. Use 'boxes-live' to explore and rearrange!
```

### Project Planning

```
User: Convert my GitHub issues to a canvas

Claude: Using boxes-live MCP to create project canvas:
- Fetching issues from GitHub...
- Creating box for each issue...
- Color by priority: P0=red, P1=yellow, P2=green
- Arranging by milestone...
- Saved to project_roadmap.txt

Load in boxes-live to drag issues between milestones!
```

## Advantages Over Miro MCP

1. **Context Efficiency**: Plain text is 5-10x more compact than JSON API responses
2. **Speed**: Local file operations vs HTTP round-trips
3. **Offline**: Works without internet connection
4. **Version Control**: Text files in git, full history
5. **Composability**: Pipe to/from other Unix tools
6. **Privacy**: Data never leaves your machine
7. **Cost**: Free, no API rate limits
8. **Extensibility**: Easy to add custom connectors
9. **Simplicity**: Direct file manipulation vs API authentication
10. **Integration**: Works with existing terminal workflows

## Future Enhancements

1. **Real-time Collaboration**: Multi-user editing via operational transforms
2. **Connection Lines**: Visual relationships between boxes
3. **Templates**: Pre-built canvas templates for common use cases
4. **Plugins**: Custom tool extensions for specific domains
5. **Sync**: Optional cloud backup while maintaining local-first approach
6. **Export**: More output formats (PDF, PNG, HTML)
7. **Import**: More connectors (Notion, Obsidian, Roam)
8. **Query Language**: SQL-like queries for complex filtering
9. **Automation**: Scheduled canvas updates from data sources
10. **Analytics**: Usage patterns and productivity metrics

## Conclusion

The boxes-live MCP server transforms the terminal canvas into an AI-assisted workspace. By leveraging the Model Context Protocol with Unix philosophy principles, we achieve superior context efficiency compared to cloud-based alternatives like Miro, while maintaining local-first privacy and composability with existing CLI tools.
