# Multi-User Collaboration Protocol Specification

**Project:** boxes-live
**Version:** 1.0 (Proposed)
**Status:** Design Document
**Date:** 2024-06-10

## Overview

This document specifies the network protocol and architecture for multi-user collaboration in boxes-live, enabling real-time shared canvas editing across multiple terminal sessions.

## Table of Contents

- [Design Goals](#design-goals)
- [Architecture](#architecture)
- [Protocol Specification](#protocol-specification)
- [Message Format](#message-format)
- [Client-Server Relationship](#client-server-relationship)
- [Synchronization Strategy](#synchronization-strategy)
- [Security Considerations](#security-considerations)
- [Implementation Phases](#implementation-phases)

## Design Goals

### Primary Goals

1. **Real-time Collaboration**: Multiple users can view and edit the same canvas simultaneously
2. **Minimal Latency**: Changes propagate to all clients within 100ms
3. **Terminal Compatibility**: Works across different terminal emulators and sizes
4. **Conflict Resolution**: Handle concurrent edits gracefully
5. **Scalability**: Support 2-10 concurrent users per canvas
6. **Simplicity**: Text-based protocol compatible with existing file format

### Non-Goals (Out of Scope)

- Voice/video communication
- User authentication/authorization (first iteration)
- End-to-end encryption (first iteration)
- Persistent chat history
- Version control/branching

## Architecture

### Client-Server Model

```
┌──────────────┐         ┌──────────────┐         ┌──────────────┐
│   Client A   │         │   Client B   │         │   Client C   │
│  (Terminal)  │         │  (Terminal)  │         │  (Terminal)  │
└──────┬───────┘         └──────┬───────┘         └──────┬───────┘
       │                        │                        │
       │ TCP/WebSocket          │                        │
       │                        │                        │
       └────────────────────────┴────────────────────────┘
                                │
                                ▼
                      ┌─────────────────┐
                      │  boxes-live     │
                      │  Server         │
                      │                 │
                      │  - Canvas state │
                      │  - Client mgmt  │
                      │  - Broadcast    │
                      └─────────────────┘
```

### Component Roles

**Server Responsibilities:**
- Maintain authoritative canvas state
- Broadcast changes to all connected clients
- Handle client connections/disconnections
- Assign unique client IDs
- Manage concurrent edit conflicts
- Persist canvas to disk periodically

**Client Responsibilities:**
- Send local operations to server
- Apply remote operations from server
- Maintain local viewport (camera position, zoom)
- Handle user input and rendering
- Reconnect on connection loss

## Protocol Specification

### Transport Layer

**Primary:** TCP sockets with text-based protocol
**Alternative:** WebSocket for web-based clients (future)

**Port:** `7878` (configurable)
**Encoding:** UTF-8 text
**Line Terminator:** `\n` (LF)

### Connection Handshake

```
Client → Server:
HELLO boxes-live/1.0
CLIENT_NAME: <username>
CANVAS_ID: <canvas_name>

Server → Client:
WELCOME boxes-live/1.0
YOUR_ID: <unique_client_id>
CANVAS_STATE: <state_line_count>
<canvas_state_lines...>
CLIENTS: <client_count>
<client_id> <client_name>
...
END_WELCOME
```

### Disconnect

```
Client → Server:
BYE

Server → Client:
BYE

Server → All Clients:
CLIENT_LEFT: <client_id>
```

## Message Format

### General Message Structure

All messages follow this format:
```
<MESSAGE_TYPE> [<param1>] [<param2>] ...
[<payload_lines>...]
```

### Message Types

#### 1. BOX_ADD - Create new box

```
Client → Server:
BOX_ADD <temp_id>
<x> <y> <width> <height> <color>
<title>
<content_line_count>
<content_line_1>
<content_line_2>
...

Server → All Clients:
BOX_ADDED <permanent_id> <client_id>
<x> <y> <width> <height> <color>
<title>
<content_line_count>
<content_line_1>
...
```

**Parameters:**
- `temp_id`: Client-generated temporary ID (for correlation)
- `permanent_id`: Server-assigned permanent box ID
- `client_id`: ID of client who created the box

#### 2. BOX_MOVE - Move existing box

```
Client → Server:
BOX_MOVE <box_id> <new_x> <new_y>

Server → All Clients:
BOX_MOVED <box_id> <new_x> <new_y> <client_id>
```

#### 3. BOX_RESIZE - Resize box

```
Client → Server:
BOX_RESIZE <box_id> <new_width> <new_height>

Server → All Clients:
BOX_RESIZED <box_id> <new_width> <new_height> <client_id>
```

#### 4. BOX_DELETE - Remove box

```
Client → Server:
BOX_DELETE <box_id>

Server → All Clients:
BOX_DELETED <box_id> <client_id>
```

#### 5. BOX_EDIT - Edit box content

```
Client → Server:
BOX_EDIT <box_id>
<new_title>
<new_content_line_count>
<content_line_1>
...

Server → All Clients:
BOX_EDITED <box_id> <client_id>
<new_title>
<new_content_line_count>
<content_line_1>
...
```

#### 6. BOX_COLOR - Change box color

```
Client → Server:
BOX_COLOR <box_id> <color>

Server → All Clients:
BOX_COLORED <box_id> <color> <client_id>
```

#### 7. CURSOR - Broadcast cursor position (awareness)

```
Client → Server:
CURSOR <world_x> <world_y>

Server → All Clients (except sender):
CURSOR <client_id> <world_x> <world_y>
```

#### 8. SELECTION - Box selection awareness

```
Client → Server:
SELECT <box_id>

Server → All Clients (except sender):
SELECTED <client_id> <box_id>

Client → Server:
DESELECT

Server → All Clients (except sender):
DESELECTED <client_id>
```

#### 9. ERROR - Server error response

```
Server → Client:
ERROR <error_code> <error_message>
```

**Error Codes:**
- `404`: Box not found
- `409`: Conflict (concurrent edit)
- `413`: Content too large
- `429`: Rate limit exceeded
- `500`: Internal server error

#### 10. PING/PONG - Keepalive

```
Client → Server:
PING

Server → Client:
PONG

Interval: Every 30 seconds
Timeout: 90 seconds (3 missed pongs = disconnect)
```

## Client-Server Relationship

### State Synchronization

```
┌─────────────────────────────────────────────────────┐
│  Client State       Server State                    │
│  ─────────────      ─────────────                   │
│                                                      │
│  Local canvas  ──→  Authoritative canvas            │
│  (optimistic)       (source of truth)               │
│                                                      │
│      ↑                      │                        │
│      │                      │                        │
│      └──────────────────────┘                        │
│         Reconciliation                               │
└─────────────────────────────────────────────────────┘
```

### Operation Flow

**Optimistic Update (Fast Path):**

```
1. User action (e.g., move box)
   │
2. Client applies change locally (optimistic)
   │
3. Client sends BOX_MOVE to server
   │
4. Server validates and broadcasts BOX_MOVED
   │
5. Other clients receive and apply change
   │
6. Originating client receives confirmation
   (or correction if conflict)
```

**Conflict Scenario:**

```
Timeline:
T0: Box at (10, 20)

T1: Client A moves box to (15, 20) → BOX_MOVE sent
T1: Client B moves box to (10, 25) → BOX_MOVE sent

T2: Server receives Client A's move first
    Server state: (15, 20)
    Broadcasts: BOX_MOVED (15, 20) client_a

T3: Server receives Client B's move
    Server state: (15, 20) (Client A wins - timestamp)
    Sends to Client B: ERROR 409 "Concurrent modification"
    Client B rolls back to (15, 20)

Resolution: Last-write-wins (LWW) based on server timestamp
```

## Synchronization Strategy

### Operational Transformation (OT) - Simplified

For this first iteration, we use a simplified OT approach:

1. **Last-Write-Wins (LWW)**: Server timestamp determines winner
2. **Atomic Operations**: Each message is atomic and independent
3. **Operation IDs**: Each operation has a unique ID for tracking
4. **Conflict Detection**: Server detects when operations conflict

### Future Enhancement: True OT

For more sophisticated conflict resolution:

```
Operation types:
- INSERT(box_id, position, content)
- DELETE(box_id, position, length)
- MOVE(box_id, delta_x, delta_y)

Transform rules:
- MOVE vs MOVE: Vector addition
- INSERT vs INSERT: Position adjustment
- DELETE vs DELETE: Range adjustment
```

## Security Considerations

### Authentication (Phase 2)

```
Client → Server:
AUTH <token>

Server → Client:
AUTH_OK <session_id>

Or:
AUTH_FAILED <reason>
```

### Authorization (Phase 2)

Canvas-level permissions:
- `owner`: Full control
- `editor`: Can modify boxes
- `viewer`: Read-only access

### Rate Limiting

- **Message rate**: Max 100 messages/second per client
- **Connection rate**: Max 5 connections/minute per IP
- **Canvas size**: Max 1000 boxes per canvas
- **Content size**: Max 10KB per box

### Thread Safety

Server must be thread-safe:
- Mutex/lock for canvas state modifications
- Concurrent read access allowed
- Write operations are serialized

### Validation

All incoming messages must be validated:
- **Format validation**: Correct message structure
- **Range validation**: Coordinates within bounds
- **Permission validation**: Client can perform action
- **Sanitization**: Remove control characters from content

## Implementation Phases

### Phase 1: Basic Collaboration (MVP)

**Scope:**
- Single server process
- TCP sockets
- Basic operations (add, move, delete, color)
- Last-write-wins conflict resolution
- No authentication

**Deliverables:**
- `boxes-live-server` executable
- `boxes-live --connect <server>` client mode
- Protocol v1.0 implementation

**Estimated Effort:** 40-60 hours

### Phase 2: Enhanced Features

**Scope:**
- User authentication (token-based)
- Canvas permissions (owner/editor/viewer)
- Persistent user names/colors
- Cursor awareness (see other users' positions)
- Selection awareness (see what others are editing)

**Deliverables:**
- Authentication system
- User management
- Enhanced awareness features

**Estimated Effort:** 30-40 hours

### Phase 3: Production Readiness

**Scope:**
- WebSocket support (web client)
- SSL/TLS encryption
- Horizontal scaling (multiple servers)
- Redis for state synchronization
- Canvas persistence to database

**Deliverables:**
- Production-grade server
- Web client interface
- Deployment guides

**Estimated Effort:** 60-80 hours

## Example Session

### Two-Client Collaboration

```
=== Server Log ===

[SERVER] Starting on port 7878...
[SERVER] Listening for connections

[CLIENT_A] 192.168.1.100 connected
[CLIENT_A] → HELLO boxes-live/1.0
[CLIENT_A] → CLIENT_NAME: Alice
[CLIENT_A] → CANVAS_ID: project_planning
[SERVER] → WELCOME boxes-live/1.0
[SERVER] → YOUR_ID: client_1
[SERVER] → CANVAS_STATE: 50
[SERVER] → <canvas data...>
[SERVER] → CLIENTS: 1
[SERVER] → client_1 Alice
[SERVER] → END_WELCOME

[CLIENT_B] 192.168.1.101 connected
[CLIENT_B] → HELLO boxes-live/1.0
[CLIENT_B] → CLIENT_NAME: Bob
[CLIENT_B] → CANVAS_ID: project_planning
[SERVER] → WELCOME boxes-live/1.0
[SERVER] → YOUR_ID: client_2
[SERVER] → CANVAS_STATE: 50
[SERVER] → <canvas data...>
[SERVER] → CLIENTS: 2
[SERVER] → client_1 Alice
[SERVER] → client_2 Bob
[SERVER] → END_WELCOME

[BROADCAST] → CLIENT_JOINED: client_2 Bob

[CLIENT_A] → BOX_ADD temp_123
[CLIENT_A] → 50.0 50.0 30 8 1
[CLIENT_A] → New Feature
[CLIENT_A] → 1
[CLIENT_A] → Implement login system
[SERVER] → BOX_ADDED 17 client_1
[BROADCAST] → BOX_ADDED 17 client_1 ...

[CLIENT_B] → BOX_MOVE 17 55.0 50.0
[SERVER] → BOX_MOVED 17 55.0 50.0 client_2
[BROADCAST] → BOX_MOVED 17 55.0 50.0 client_2

[CLIENT_A] → SELECT 17
[SERVER] → SELECTED client_1 17
[TO CLIENT_B] → SELECTED client_1 17

[CLIENT_A] → BYE
[SERVER] → BYE
[BROADCAST] → CLIENT_LEFT: client_1
```

## References

- **Operational Transformation**: Ellis & Gibbs (1989)
- **CRDT**: Shapiro et al. (2011)
- **WebSocket Protocol**: RFC 6455
- **Real-time Collaboration**: Google Docs Engineering Blog

---

**Document Status:** Design Specification
**Implementation Status:** Not yet implemented
**Next Steps:** Create proof-of-concept server in Phase 1

**Maintainer:** boxes-live project team
