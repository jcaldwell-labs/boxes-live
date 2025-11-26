#include "export.h"
#include "viewport.h"
#include "canvas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Unicode box-drawing characters for ASCII export */
#define CHAR_ULCORNER "┌"
#define CHAR_URCORNER "┐"
#define CHAR_LLCORNER "└"
#define CHAR_LRCORNER "┘"
#define CHAR_HLINE "─"
#define CHAR_VLINE "│"
#define CHAR_ARROW_RIGHT "▶"
#define CHAR_ARROW_DOWN "▼"
#define CHAR_ARROW_LEFT "◀"
#define CHAR_ARROW_UP "▲"

/* Helper function to allocate 2D character buffer */
static char** allocate_buffer(int width, int height) {
    char **buffer = malloc(height * sizeof(char*));
    if (!buffer) return NULL;
    
    for (int y = 0; y < height; y++) {
        buffer[y] = malloc(width + 1);  /* +1 for null terminator */
        if (!buffer[y]) {
            /* Cleanup on failure */
            for (int i = 0; i < y; i++) {
                free(buffer[i]);
            }
            free(buffer);
            return NULL;
        }
        /* Initialize with spaces */
        memset(buffer[y], ' ', width);
        buffer[y][width] = '\0';
    }
    
    return buffer;
}

/* Helper function to free 2D character buffer */
static void free_buffer(char **buffer, int height) {
    if (!buffer) return;
    
    for (int y = 0; y < height; y++) {
        free(buffer[y]);
    }
    free(buffer);
}

/* Helper function to write string at position in buffer (UTF-8 safe single char) */
static void buffer_write_char(char **buffer, int width, int height, int x, int y, const char *ch) {
    if (x < 0 || y < 0 || y >= height) return;
    
    /* For UTF-8 characters, we need to ensure we don't overflow */
    int ch_len = strlen(ch);
    if (x + ch_len > width) return;
    
    /* Copy the character */
    memcpy(&buffer[y][x], ch, ch_len);
}

/* Helper function to write text at position in buffer */
static void buffer_write_text(char **buffer, int width, int height, int x, int y, const char *text) {
    if (x < 0 || y < 0 || y >= height || !text) return;
    
    int pos = x;
    for (size_t i = 0; text[i] != '\0' && pos < width; i++) {
        buffer[y][pos++] = text[i];
    }
}

/* Helper function to draw horizontal line in buffer */
static void buffer_draw_hline(char **buffer, int width, int height, int y, int x1, int x2) {
    if (y < 0 || y >= height) return;
    
    for (int x = x1; x <= x2 && x < width; x++) {
        if (x >= 0) {
            buffer_write_char(buffer, width, height, x, y, CHAR_HLINE);
        }
    }
}

/* Helper function to draw vertical line in buffer */
static void buffer_draw_vline(char **buffer, int width, int height, int x, int y1, int y2) {
    if (x < 0 || x >= width) return;
    
    for (int y = y1; y <= y2 && y < height; y++) {
        if (y >= 0) {
            buffer_write_char(buffer, width, height, x, y, CHAR_VLINE);
        }
    }
}

/* Helper function to render a box into the buffer */
static void render_box_to_buffer(char **buffer, int buf_width, int buf_height,
                                  const Box *box, const Viewport *vp) {
    /* Convert world coordinates to viewport screen coordinates */
    int sx = world_to_screen_x(vp, box->x);
    int sy = world_to_screen_y(vp, box->y);
    int scaled_width = (int)(box->width * vp->zoom);
    int scaled_height = (int)(box->height * vp->zoom);
    
    /* Skip if box is completely off-screen */
    if (sx + scaled_width < 0 || sx >= buf_width ||
        sy + scaled_height < 0 || sy >= buf_height) {
        return;
    }
    
    /* Draw box border */
    /* Top border */
    if (sy >= 0 && sy < buf_height) {
        if (sx >= 0 && sx < buf_width) {
            buffer_write_char(buffer, buf_width, buf_height, sx, sy, CHAR_ULCORNER);
        }
        buffer_draw_hline(buffer, buf_width, buf_height, sy, sx + 3, sx + scaled_width - 3);
        if (sx + scaled_width >= 0 && sx + scaled_width < buf_width) {
            buffer_write_char(buffer, buf_width, buf_height, sx + scaled_width, sy, CHAR_URCORNER);
        }
    }
    
    /* Bottom border */
    if (sy + scaled_height >= 0 && sy + scaled_height < buf_height) {
        if (sx >= 0 && sx < buf_width) {
            buffer_write_char(buffer, buf_width, buf_height, sx, sy + scaled_height, CHAR_LLCORNER);
        }
        buffer_draw_hline(buffer, buf_width, buf_height, sy + scaled_height, sx + 3, sx + scaled_width - 3);
        if (sx + scaled_width >= 0 && sx + scaled_width < buf_width) {
            buffer_write_char(buffer, buf_width, buf_height, sx + scaled_width, sy + scaled_height, CHAR_LRCORNER);
        }
    }
    
    /* Left and right borders */
    buffer_draw_vline(buffer, buf_width, buf_height, sx, sy + 1, sy + scaled_height - 1);
    buffer_draw_vline(buffer, buf_width, buf_height, sx + scaled_width, sy + 1, sy + scaled_height - 1);
    
    /* Draw title if it exists and there's room */
    if (box->title != NULL && scaled_height > 1 && scaled_width > 4) {
        int title_y = sy + 1;
        int title_x = sx + 2;
        
        if (title_y >= 0 && title_y < buf_height && title_x >= 0) {
            /* Calculate max title length that fits */
            int max_title_len = scaled_width - 3;
            if (max_title_len > 0) {
                char title_buf[256];
                snprintf(title_buf, sizeof(title_buf), "%s", box->title);
                
                /* Truncate if needed */
                if ((int)strlen(title_buf) > max_title_len) {
                    title_buf[max_title_len] = '\0';
                }
                
                buffer_write_text(buffer, buf_width, buf_height, title_x, title_y, title_buf);
            }
        }
    }
    
    /* Draw content if it exists and there's room */
    if (box->content != NULL && box->content_lines > 0 && scaled_height > 2) {
        int content_start_y = sy + 2;
        int content_x = sx + 2;
        
        for (int i = 0; i < box->content_lines && content_start_y + i < sy + scaled_height; i++) {
            int line_y = content_start_y + i;
            if (line_y >= 0 && line_y < buf_height && content_x >= 0) {
                /* Calculate max content length that fits */
                int max_content_len = scaled_width - 3;
                if (max_content_len > 0 && box->content[i]) {
                    char content_buf[256];
                    snprintf(content_buf, sizeof(content_buf), "%s", box->content[i]);
                    
                    /* Truncate if needed */
                    if ((int)strlen(content_buf) > max_content_len) {
                        content_buf[max_content_len] = '\0';
                    }
                    
                    buffer_write_text(buffer, buf_width, buf_height, content_x, line_y, content_buf);
                }
            }
        }
    }
}

/* Helper function to render connections into the buffer */
static void render_connections_to_buffer(char **buffer, int buf_width, int buf_height,
                                          const Canvas *canvas, const Viewport *vp) {
    if (!canvas->connections) return;
    
    for (int i = 0; i < canvas->conn_count; i++) {
        Connection *conn = &canvas->connections[i];
        
        /* Find source and destination boxes */
        /* Note: cast away const since we're only reading */
        Box *src = canvas_get_box((Canvas *)canvas, conn->source_id);
        Box *dest = canvas_get_box((Canvas *)canvas, conn->dest_id);
        
        if (!src || !dest) continue;
        
        /* Calculate center points of boxes in screen coordinates */
        int src_sx = world_to_screen_x(vp, src->x + src->width / 2.0);
        int src_sy = world_to_screen_y(vp, src->y + src->height / 2.0);
        int dest_sx = world_to_screen_x(vp, dest->x + dest->width / 2.0);
        int dest_sy = world_to_screen_y(vp, dest->y + dest->height / 2.0);
        
        /* Draw a simple arrow from source to destination */
        /* For now, just draw the arrow at the destination */
        if (dest_sx >= 0 && dest_sx < buf_width && dest_sy >= 0 && dest_sy < buf_height) {
            /* Determine arrow direction based on relative positions */
            const char *arrow = CHAR_ARROW_RIGHT;
            if (dest_sx < src_sx) {
                arrow = CHAR_ARROW_LEFT;
            } else if (dest_sy < src_sy) {
                arrow = CHAR_ARROW_UP;
            } else if (dest_sy > src_sy) {
                arrow = CHAR_ARROW_DOWN;
            }
            
            buffer_write_char(buffer, buf_width, buf_height, dest_sx, dest_sy, arrow);
        }
    }
}

/* Export current viewport to ASCII art file */
int export_viewport_to_file(const Canvas *canvas, const Viewport *vp, const char *filename) {
    if (!canvas || !vp || !filename) {
        return -1;
    }
    
    /* Use viewport dimensions for export */
    int buf_width = vp->term_width;
    int buf_height = vp->term_height - 1;  /* Reserve last line for status */
    
    /* Allocate buffer */
    char **buffer = allocate_buffer(buf_width, buf_height);
    if (!buffer) {
        return -1;
    }
    
    /* Render boxes to buffer */
    for (int i = 0; i < canvas->box_count; i++) {
        render_box_to_buffer(buffer, buf_width, buf_height, &canvas->boxes[i], vp);
    }
    
    /* Render connections to buffer */
    render_connections_to_buffer(buffer, buf_width, buf_height, canvas, vp);
    
    /* Open file for writing */
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        free_buffer(buffer, buf_height);
        return -1;
    }
    
    /* Get current timestamp */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", tm_info);
    
    /* Write header */
    fprintf(fp, "boxes-live canvas export - %s\n", timestamp);
    fprintf(fp, "═══════════════════════════════════════════\n\n");
    
    /* Write buffer to file */
    for (int y = 0; y < buf_height; y++) {
        fprintf(fp, "%s\n", buffer[y]);
    }
    
    /* Write metadata footer */
    fprintf(fp, "\n");
    fprintf(fp, "Grid: %s", canvas->grid.visible ? "ON" : "OFF");
    if (canvas->grid.visible) {
        fprintf(fp, " (%d)", canvas->grid.spacing);
    }
    fprintf(fp, "  Zoom: %.1fx  View: (%.0f,%.0f)\n", vp->zoom, vp->cam_x, vp->cam_y);
    fprintf(fp, "Boxes: %d  Connections: %d\n", canvas->box_count, canvas->conn_count);
    
    /* Close file */
    fclose(fp);
    
    /* Free buffer */
    free_buffer(buffer, buf_height);
    
    return 0;
}
