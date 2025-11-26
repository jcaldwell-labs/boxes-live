#include "export.h"
#include "viewport.h"
#include "canvas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Simple approach: write directly to file line by line */
/* Build each line as a string with proper UTF-8 handling */

/* Box-drawing characters */
#define BOX_UL "┌"
#define BOX_UR "┐"
#define BOX_LL "└"
#define BOX_LR "┘"
#define BOX_H  "─"
#define BOX_V  "│"
#define ARROW_R "▶"
#define ARROW_D "▼"
#define ARROW_L "◀"
#define ARROW_U "▲"

/* Cell structure for grid */
typedef struct {
    char content[8];  /* UTF-8 character or space */
    int occupied;     /* Whether this cell is used */
} Cell;

/* Helper to set cell content */
static void set_cell(Cell *grid, int width, int x, int y, const char *ch) {
    if (x < 0 || y < 0 || x >= width) return;
    Cell *cell = &grid[y * width + x];
    snprintf(cell->content, sizeof(cell->content), "%s", ch);
    cell->occupied = 1;
}

/* Helper to render a box to grid */
static void render_box_to_grid(Cell *grid, int width, int height,
                                const Box *box, const Viewport *vp) {
    int sx = world_to_screen_x(vp, box->x);
    int sy = world_to_screen_y(vp, box->y);
    int sw = (int)(box->width * vp->zoom);
    int sh = (int)(box->height * vp->zoom);
    
    /* Skip if completely off-screen */
    if (sx + sw < 0 || sx >= width || sy + sh < 0 || sy >= height) {
        return;
    }
    
    /* Draw corners */
    set_cell(grid, width, sx, sy, BOX_UL);
    set_cell(grid, width, sx + sw, sy, BOX_UR);
    set_cell(grid, width, sx, sy + sh, BOX_LL);
    set_cell(grid, width, sx + sw, sy + sh, BOX_LR);
    
    /* Draw horizontal lines */
    for (int x = sx + 1; x < sx + sw; x++) {
        set_cell(grid, width, x, sy, BOX_H);
        set_cell(grid, width, x, sy + sh, BOX_H);
    }
    
    /* Draw vertical lines */
    for (int y = sy + 1; y < sy + sh; y++) {
        set_cell(grid, width, sx, y, BOX_V);
        set_cell(grid, width, sx + sw, y, BOX_V);
    }
    
    /* Draw title */
    if (box->title && sh > 1) {
        int tx = sx + 2;
        int ty = sy + 1;
        if (ty >= 0 && ty < height) {
            for (size_t i = 0; box->title[i] && tx + (int)i < sx + sw - 1; i++) {
                char buf[2] = {box->title[i], '\0'};
                set_cell(grid, width, tx + i, ty, buf);
            }
        }
    }
    
    /* Draw content */
    if (box->content && box->content_lines > 0 && sh > 2) {
        int cy = sy + 2;
        for (int i = 0; i < box->content_lines && cy + i < sy + sh; i++) {
            int cx = sx + 2;
            int line_y = cy + i;
            if (line_y >= 0 && line_y < height && box->content[i]) {
                for (size_t j = 0; box->content[i][j] && cx + (int)j < sx + sw - 1; j++) {
                    char buf[2] = {box->content[i][j], '\0'};
                    set_cell(grid, width, cx + j, line_y, buf);
                }
            }
        }
    }
}

/* Helper to render connections */
static void render_connections_to_grid(Cell *grid, int width,
                                        const Canvas *canvas, const Viewport *vp) {
    if (!canvas->connections) return;
    
    for (int i = 0; i < canvas->conn_count; i++) {
        Connection *conn = &canvas->connections[i];
        Box *src = canvas_get_box((Canvas *)canvas, conn->source_id);
        Box *dest = canvas_get_box((Canvas *)canvas, conn->dest_id);
        
        if (!src || !dest) continue;
        
        /* Calculate destination center */
        int dest_sx = world_to_screen_x(vp, dest->x + dest->width / 2.0);
        int dest_sy = world_to_screen_y(vp, dest->y + dest->height / 2.0);
        int src_sx = world_to_screen_x(vp, src->x + src->width / 2.0);
        int src_sy = world_to_screen_y(vp, src->y + src->height / 2.0);
        
        /* Draw arrow at destination */
        const char *arrow = ARROW_R;
        if (dest_sx < src_sx) arrow = ARROW_L;
        else if (dest_sy < src_sy) arrow = ARROW_U;
        else if (dest_sy > src_sy) arrow = ARROW_D;
        
        set_cell(grid, width, dest_sx, dest_sy, arrow);
    }
}

/* Export viewport to file */
int export_viewport_to_file(const Canvas *canvas, const Viewport *vp, const char *filename) {
    if (!canvas || !vp || !filename) return -1;
    
    int width = vp->term_width;
    int height = vp->term_height - 1;
    
    /* Allocate grid */
    Cell *grid = calloc(width * height, sizeof(Cell));
    if (!grid) return -1;
    
    /* Initialize grid with spaces */
    for (int i = 0; i < width * height; i++) {
        strcpy(grid[i].content, " ");
        grid[i].occupied = 0;
    }
    
    /* Render boxes and connections */
    for (int i = 0; i < canvas->box_count; i++) {
        render_box_to_grid(grid, width, height, &canvas->boxes[i], vp);
    }
    render_connections_to_grid(grid, width, canvas, vp);
    
    /* Open file */
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        free(grid);
        return -1;
    }
    
    /* Write header */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", tm_info);
    
    fprintf(fp, "boxes-live canvas export - %s\n", timestamp);
    fprintf(fp, "═══════════════════════════════════════════\n\n");
    
    /* Write grid */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fprintf(fp, "%s", grid[y * width + x].content);
        }
        fprintf(fp, "\n");
    }
    
    /* Write footer */
    fprintf(fp, "\nGrid: %s", canvas->grid.visible ? "ON" : "OFF");
    if (canvas->grid.visible) fprintf(fp, " (%d)", canvas->grid.spacing);
    fprintf(fp, "  Zoom: %.1fx  View: (%.0f,%.0f)\n", vp->zoom, vp->cam_x, vp->cam_y);
    fprintf(fp, "Boxes: %d  Connections: %d\n", canvas->box_count, canvas->conn_count);
    
    fclose(fp);
    free(grid);
    return 0;
}
