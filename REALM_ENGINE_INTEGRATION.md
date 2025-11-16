# Realm-Engine Integration Guide

## Overview

This guide describes how to integrate boxes-live as a rendering frontend for large-scale map systems like realm-engine, supporting massive multiplayer worlds, procedurally generated terrain, and thousands of entities.

## What is Realm-Engine?

Realm-engine (hypothetical) is a backend engine for managing:
- Large-scale persistent worlds (potentially infinite)
- Entity-component systems
- Player sessions and state
- Procedural generation
- Multi-server architecture
- Real-time synchronization

Boxes-live can serve as a lightweight terminal client for visualizing and interacting with these worlds.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    Realm-Engine                         │
│  (Backend: World State, Entities, Generation)           │
└────────────────┬────────────────────────────────────────┘
                 │
                 │ Network Protocol (TCP/WebSocket)
                 │
                 │
┌────────────────▼────────────────────────────────────────┐
│              Boxes-Live Client                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │  Network Layer (receive updates, send commands)  │   │
│  └────────────┬─────────────────────────────────────┘   │
│               │                                          │
│  ┌────────────▼─────────────────────────────────────┐   │
│  │  Client State (local cache of world chunks)      │   │
│  └────────────┬─────────────────────────────────────┘   │
│               │                                          │
│  ┌────────────▼─────────────────────────────────────┐   │
│  │  Rendering (viewport, tiles, entities)           │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Core Concepts

### 1. Chunking System

Large worlds must be divided into chunks for efficient loading/unloading.

**Chunk Structure**:
```c
#define CHUNK_SIZE 64  // 64x64 tiles per chunk

typedef struct {
    int chunk_x;        // Chunk coordinate (not world coordinate)
    int chunk_y;
    Tile tiles[CHUNK_SIZE][CHUNK_SIZE];
    Entity *entities;   // Entities in this chunk
    int entity_count;
    bool loaded;
    bool dirty;         // Needs re-render
} Chunk;

typedef struct {
    char glyph;         // ASCII character to render
    int color_pair;     // ncurses color
    int flags;          // SOLID, WALKABLE, etc.
} Tile;
```

**Chunk Management**:
```c
typedef struct {
    Chunk *chunks;      // Hash table or array of chunks
    int chunk_capacity;
    int chunk_count;
} ChunkManager;

// Convert world coords to chunk coords
int world_to_chunk_x(double world_x) {
    return (int)floor(world_x / CHUNK_SIZE);
}

int world_to_chunk_y(double world_y) {
    return (int)floor(world_y / CHUNK_SIZE);
}

// Get chunk at world position (load if needed)
Chunk* get_chunk_at_world(ChunkManager *cm, double wx, double wy) {
    int cx = world_to_chunk_x(wx);
    int cy = world_to_chunk_y(wy);
    return get_or_load_chunk(cm, cx, cy);
}
```

### 2. Entity System

Replace Box with a more flexible Entity system.

**Entity-Component Architecture**:
```c
typedef struct Entity Entity;
typedef struct Component Component;

typedef enum {
    COMP_POSITION,
    COMP_SPRITE,
    COMP_VELOCITY,
    COMP_COLLIDER,
    COMP_HEALTH,
    COMP_PLAYER,
    COMP_AI,
    COMP_INVENTORY
} ComponentType;

struct Component {
    ComponentType type;
    void *data;
    Component *next;  // Linked list
};

struct Entity {
    int id;             // Unique entity ID (from realm-engine)
    Component *components;
};

// Example component data
typedef struct {
    double x, y;
    int chunk_x, chunk_y;  // Which chunk this entity is in
} PositionComponent;

typedef struct {
    char glyph;
    int color_pair;
    int layer;  // Render order
} SpriteComponent;

typedef struct {
    double dx, dy;
    double max_speed;
} VelocityComponent;
```

**Component Access**:
```c
Component* entity_get_component(Entity *e, ComponentType type) {
    Component *c = e->components;
    while (c) {
        if (c->type == type) return c;
        c = c->next;
    }
    return NULL;
}

void entity_add_component(Entity *e, Component *comp) {
    comp->next = e->components;
    e->components = comp;
}
```

### 3. Network Protocol

**Message Types**:
```c
typedef enum {
    MSG_CHUNK_DATA,      // Server sends chunk data
    MSG_ENTITY_UPDATE,   // Server sends entity positions
    MSG_ENTITY_SPAWN,    // New entity appeared
    MSG_ENTITY_DESPAWN,  // Entity removed
    MSG_PLAYER_MOVE,     // Client sends movement
    MSG_PLAYER_ACTION,   // Client sends action (attack, use, etc.)
    MSG_CHAT,            // Chat message
} MessageType;

typedef struct {
    MessageType type;
    int length;
    unsigned char data[];  // Variable-length payload
} NetworkMessage;
```

**Example Chunk Data Message**:
```c
// Server sends:
// MSG_CHUNK_DATA | chunk_x | chunk_y | compressed_tile_data

void handle_chunk_data(Client *client, NetworkMessage *msg) {
    int chunk_x = read_int(msg->data);
    int chunk_y = read_int(msg->data + 4);

    Chunk *chunk = get_or_create_chunk(&client->chunk_mgr, chunk_x, chunk_y);

    // Decompress tile data (use zlib or simple RLE)
    decompress_tiles(chunk->tiles, msg->data + 8, msg->length - 8);

    chunk->loaded = true;
    chunk->dirty = true;  // Trigger re-render
}
```

**Example Entity Update**:
```c
// Server sends delta updates (only changed entities)
// MSG_ENTITY_UPDATE | count | [entity_id, x, y] * count

void handle_entity_update(Client *client, NetworkMessage *msg) {
    int count = read_int(msg->data);
    unsigned char *ptr = msg->data + 4;

    for (int i = 0; i < count; i++) {
        int entity_id = read_int(ptr);
        double x = read_double(ptr + 4);
        double y = read_double(ptr + 12);
        ptr += 20;

        Entity *entity = find_entity(&client->entity_mgr, entity_id);
        if (entity) {
            PositionComponent *pos = entity_get_component(entity, COMP_POSITION);
            if (pos) {
                pos->x = x;
                pos->y = y;
                // Update chunk membership if needed
                update_entity_chunk(&client->chunk_mgr, entity);
            }
        }
    }
}
```

### 4. Viewport and Streaming

Only load chunks visible in viewport + buffer zone.

**View Distance**:
```c
#define VIEW_DISTANCE 2  // Load 2 chunks beyond visible area

void update_chunk_streaming(Client *client, Viewport *vp) {
    // Calculate visible chunk range
    double view_left = vp->cam_x;
    double view_right = vp->cam_x + vp->term_width / vp->zoom;
    double view_top = vp->cam_y;
    double view_bottom = vp->cam_y + vp->term_height / vp->zoom;

    int min_chunk_x = world_to_chunk_x(view_left) - VIEW_DISTANCE;
    int max_chunk_x = world_to_chunk_x(view_right) + VIEW_DISTANCE;
    int min_chunk_y = world_to_chunk_y(view_top) - VIEW_DISTANCE;
    int max_chunk_y = world_to_chunk_y(view_bottom) + VIEW_DISTANCE;

    // Request missing chunks from server
    for (int cy = min_chunk_y; cy <= max_chunk_y; cy++) {
        for (int cx = min_chunk_x; cx <= max_chunk_x; cx++) {
            if (!is_chunk_loaded(&client->chunk_mgr, cx, cy)) {
                request_chunk_from_server(client, cx, cy);
            }
        }
    }

    // Unload distant chunks (optional, for memory management)
    unload_distant_chunks(&client->chunk_mgr, min_chunk_x, max_chunk_x,
                          min_chunk_y, max_chunk_y);
}
```

### 5. Rendering Pipeline

**Layered Rendering**:
```c
typedef enum {
    LAYER_GROUND,      // Terrain
    LAYER_OBJECTS,     // Trees, rocks, items
    LAYER_ENTITIES,    // Players, NPCs, enemies
    LAYER_EFFECTS,     // Particles, spells
    LAYER_UI,          // Health bars, names
    LAYER_COUNT
} RenderLayer;

void render_world(Client *client, Viewport *vp) {
    // Render each layer in order
    for (int layer = 0; layer < LAYER_COUNT; layer++) {
        render_tiles(client, vp, layer);
        render_entities(client, vp, layer);
    }
}

void render_tiles(Client *client, Viewport *vp, RenderLayer layer) {
    // Only render tiles in visible chunks
    int min_chunk_x = world_to_chunk_x(vp->cam_x);
    int max_chunk_x = world_to_chunk_x(vp->cam_x + vp->term_width / vp->zoom) + 1;
    int min_chunk_y = world_to_chunk_y(vp->cam_y);
    int max_chunk_y = world_to_chunk_y(vp->cam_y + vp->term_height / vp->zoom) + 1;

    for (int cy = min_chunk_y; cy <= max_chunk_y; cy++) {
        for (int cx = min_chunk_x; cx <= max_chunk_x; cx++) {
            Chunk *chunk = get_chunk(&client->chunk_mgr, cx, cy);
            if (!chunk || !chunk->loaded) {
                // Render loading indicator or blank space
                continue;
            }

            render_chunk_tiles(chunk, vp, layer, cx, cy);
        }
    }
}

void render_chunk_tiles(Chunk *chunk, Viewport *vp, RenderLayer layer,
                       int chunk_x, int chunk_y) {
    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            Tile *tile = &chunk->tiles[y][x];

            // Calculate world position
            double wx = chunk_x * CHUNK_SIZE + x;
            double wy = chunk_y * CHUNK_SIZE + y;

            // Convert to screen position
            int sx = world_to_screen_x(vp, wx);
            int sy = world_to_screen_y(vp, wy);

            // Skip if off-screen
            if (sx < 0 || sx >= vp->term_width ||
                sy < 0 || sy >= vp->term_height) {
                continue;
            }

            // Render tile
            if (tile->color_pair > 0) {
                attron(COLOR_PAIR(tile->color_pair));
            }
            mvaddch(sy, sx, tile->glyph);
            if (tile->color_pair > 0) {
                attroff(COLOR_PAIR(tile->color_pair));
            }
        }
    }
}

void render_entities(Client *client, Viewport *vp, RenderLayer layer) {
    // Iterate through all entities
    for (int i = 0; i < client->entity_mgr.entity_count; i++) {
        Entity *entity = &client->entity_mgr.entities[i];

        PositionComponent *pos = entity_get_component(entity, COMP_POSITION);
        SpriteComponent *sprite = entity_get_component(entity, COMP_SPRITE);

        if (!pos || !sprite || sprite->layer != layer) {
            continue;
        }

        // Convert to screen coords
        int sx = world_to_screen_x(vp, pos->x);
        int sy = world_to_screen_y(vp, pos->y);

        // Skip if off-screen
        if (sx < 0 || sx >= vp->term_width ||
            sy < 0 || sy >= vp->term_height) {
            continue;
        }

        // Render sprite
        if (sprite->color_pair > 0) {
            attron(COLOR_PAIR(sprite->color_pair));
        }
        mvaddch(sy, sx, sprite->glyph);
        if (sprite->color_pair > 0) {
            attroff(COLOR_PAIR(sprite->color_pair));
        }
    }
}
```

## Implementation Phases

### Phase A: Offline Map Viewer

**Goal**: Display large pre-generated maps without networking.

1. **Load map from file**:
   - Save chunks to disk (e.g., `world/chunk_0_0.dat`)
   - Load chunks on demand as viewport moves

2. **Implement chunking**:
   - Replace Canvas with ChunkManager
   - Implement chunk loading/unloading

3. **Tile rendering**:
   - Replace box rendering with tile rendering
   - Support different tile types (grass, water, wall, etc.)

**Estimated effort**: 10-15 hours

### Phase B: Client-Server Architecture

**Goal**: Connect to realm-engine backend.

1. **Network module**:
   - TCP socket connection to server
   - Non-blocking I/O or separate thread
   - Message serialization/deserialization

2. **Protocol implementation**:
   - Handle chunk data messages
   - Handle entity update messages
   - Send player input to server

3. **Client state management**:
   - Maintain local cache of chunks
   - Interpolate entity positions for smooth movement
   - Handle latency and packet loss

**Estimated effort**: 20-30 hours

### Phase C: Entity System

**Goal**: Support dynamic entities.

1. **Component system**:
   - Implement entity-component architecture
   - Basic components (position, sprite, velocity)

2. **Entity rendering**:
   - Render entities on top of tiles
   - Support multiple layers

3. **Local prediction**:
   - Predict player movement locally
   - Reconcile with server updates

**Estimated effort**: 15-20 hours

### Phase D: Interaction

**Goal**: Allow player to interact with the world.

1. **Player controls**:
   - Send movement commands to server
   - Handle server authoritative movement

2. **Actions**:
   - Attack, use items, interact with objects
   - Show action feedback

3. **Inventory and UI**:
   - Display player stats
   - Inventory management
   - Contextual UI

**Estimated effort**: 15-20 hours

### Phase E: Advanced Features

**Goal**: Polish and optimize.

1. **Minimap**:
   - Small overview of surrounding area
   - Show player and entities

2. **Fog of war**:
   - Hide unexplored areas
   - Reveal as player explores

3. **Particle effects**:
   - Simple text-based effects
   - Spell effects, damage numbers

4. **Performance optimization**:
   - Spatial indexing for entities
   - Dirty rectangle rendering
   - Multi-threaded chunk loading

**Estimated effort**: 20-30 hours

## Example Integration Code

### Main Loop with Networking

```c
int main(void) {
    // Initialize terminal
    terminal_init();

    // Initialize viewport
    Viewport viewport;
    viewport_init(&viewport);

    // Initialize client
    Client client;
    client_init(&client);

    // Connect to realm-engine server
    if (client_connect(&client, "localhost", 9999) != 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return 1;
    }

    // Main loop
    int running = 1;
    while (running) {
        // Process network messages (non-blocking)
        client_process_messages(&client);

        // Update chunk streaming based on viewport
        update_chunk_streaming(&client, &viewport);

        // Update local entity predictions
        update_entities(&client.entity_mgr, 0.016);  // ~60 FPS

        // Update terminal size
        terminal_update_size(&viewport);

        // Clear screen
        terminal_clear();

        // Render world (tiles + entities)
        render_world(&client, &viewport);

        // Render UI overlay
        render_ui(&client, &viewport);

        // Render status bar
        render_status(&viewport);

        // Refresh display
        terminal_refresh();

        // Handle input
        if (handle_input(&viewport, &client)) {
            running = 0;
        }

        // Frame limiting
        struct timespec ts = {0, 16667000};  // ~60 FPS
        nanosleep(&ts, NULL);
    }

    // Cleanup
    client_disconnect(&client);
    client_cleanup(&client);
    terminal_cleanup();

    return 0;
}
```

### Input Handler with Client Commands

```c
int handle_input(Viewport *vp, Client *client) {
    int ch = getch();

    if (ch == ERR) return 0;

    switch (ch) {
        case 'q':
        case 'Q':
            return 1;

        // Movement (send to server)
        case KEY_UP:
        case 'w':
            client_send_move(client, MOVE_UP);
            viewport_pan(vp, 0, -2.0);  // Optimistic local update
            break;

        case KEY_DOWN:
        case 's':
            client_send_move(client, MOVE_DOWN);
            viewport_pan(vp, 0, 2.0);
            break;

        case KEY_LEFT:
        case 'a':
            client_send_move(client, MOVE_LEFT);
            viewport_pan(vp, -2.0, 0);
            break;

        case KEY_RIGHT:
        case 'd':
            client_send_move(client, MOVE_RIGHT);
            viewport_pan(vp, 2.0, 0);
            break;

        // Actions
        case ' ':  // Attack/interact
            client_send_action(client, ACTION_PRIMARY);
            break;

        case 'e':  // Use/interact
            client_send_action(client, ACTION_INTERACT);
            break;

        case 'i':  // Inventory
            toggle_inventory_ui(client);
            break;

        // Zoom (local only)
        case '+':
        case 'z':
            viewport_zoom(vp, 1.2);
            break;

        case '-':
        case 'x':
            viewport_zoom(vp, 1.0 / 1.2);
            break;
    }

    return 0;
}
```

## Performance Considerations

### Memory Management

For massive worlds:
- **Chunk limit**: Keep only N chunks loaded (e.g., 100 chunks = ~400KB)
- **Entity culling**: Only track entities in loaded chunks
- **LRU cache**: Unload least-recently-used chunks first

### Network Optimization

- **Delta encoding**: Only send changed tiles/entities
- **Compression**: Use zlib or LZ4 for chunk data
- **Batching**: Batch multiple entity updates into single message
- **Interpolation**: Smooth entity movement between updates

### Rendering Optimization

- **Dirty chunks**: Only re-render chunks that changed
- **Spatial indexing**: Use quadtree/grid for entity queries
- **Frustum culling**: Already implemented (viewport bounds check)
- **Level of detail**: Use simpler glyphs when zoomed out

## Comparison to Alternative Approaches

### Boxes-Live vs. Other Terminal Renderers

| Feature | Boxes-Live | Notcurses | Termbox | Raw ANSI |
|---------|-----------|-----------|---------|----------|
| Box drawing | ✅ ACS | ✅ Unicode | ✅ Limited | ✅ Manual |
| Colors | ✅ 256 | ✅ RGB | ✅ 256 | ✅ Varies |
| Mouse | ✅ Easy | ✅ Easy | ✅ Manual | ✅ Manual |
| Portability | ✅ High | ⚠️ Medium | ✅ High | ✅ High |
| Performance | ✅ Good | ✅ Excellent | ✅ Good | ✅ Best |
| Learning curve | ✅ Easy | ⚠️ Steep | ✅ Easy | ⚠️ Manual |

**Recommendation**: Stick with ncurses for boxes-live. It's portable, well-documented, and performant enough for most use cases.

## Example Realm-Engine Integration

### Hypothetical Realm-Engine API

```c
// Realm-engine provides:
typedef struct RealmEngine RealmEngine;

// Initialize connection
RealmEngine* realm_connect(const char *host, int port);

// Set callbacks
void realm_on_chunk_loaded(RealmEngine *re,
                           void (*callback)(int cx, int cy, Chunk *chunk));
void realm_on_entity_updated(RealmEngine *re,
                             void (*callback)(Entity *entity));

// Request data
void realm_request_chunk(RealmEngine *re, int chunk_x, int chunk_y);

// Send player input
void realm_send_player_move(RealmEngine *re, int dx, int dy);
void realm_send_player_action(RealmEngine *re, ActionType action);

// Process events (call every frame)
void realm_tick(RealmEngine *re);
```

### Boxes-Live Integration

```c
static RealmEngine *g_realm = NULL;
static Client g_client;

void on_chunk_loaded(int cx, int cy, Chunk *chunk) {
    // Copy chunk data to client state
    store_chunk(&g_client.chunk_mgr, cx, cy, chunk);
}

void on_entity_updated(Entity *entity) {
    // Update entity in client state
    update_entity(&g_client.entity_mgr, entity);
}

int main(void) {
    terminal_init();

    // Connect to realm-engine
    g_realm = realm_connect("localhost", 9999);
    realm_on_chunk_loaded(g_realm, on_chunk_loaded);
    realm_on_entity_updated(g_realm, on_entity_updated);

    client_init(&g_client);

    Viewport viewport;
    viewport_init(&viewport);

    int running = 1;
    while (running) {
        // Process realm-engine events
        realm_tick(g_realm);

        // Update viewport
        update_chunk_streaming(&g_client, &viewport);

        // Render
        terminal_clear();
        render_world(&g_client, &viewport);
        terminal_refresh();

        // Input
        int ch = getch();
        if (ch == 'q') running = 0;
        if (ch == KEY_UP) realm_send_player_move(g_realm, 0, -1);
        if (ch == KEY_DOWN) realm_send_player_move(g_realm, 0, 1);
        // etc.

        nanosleep(&ts, NULL);
    }

    realm_disconnect(g_realm);
    terminal_cleanup();
    return 0;
}
```

## Conclusion

Boxes-live's viewport system and coordinate transformations make it well-suited for large-scale map rendering. The key changes needed are:

1. **Replace static boxes with dynamic tile/entity system**
2. **Implement chunk-based streaming**
3. **Add network layer for server communication**
4. **Optimize rendering for thousands of entities**

The current architecture is flexible enough to support all of this without major refactoring. The viewport module can remain largely unchanged, serving as a stable foundation for the more complex map rendering system.

For more details on the implementation roadmap, see [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md).
