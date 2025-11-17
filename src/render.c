#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "render.h"
#include "viewport.h"
#include "canvas.h"

/* Helper function to draw a horizontal line */
static void draw_hline(int y, int x1, int x2, chtype ch) {
    if (y < 0 || y >= LINES) return;

    for (int x = x1; x <= x2; x++) {
        if (x >= 0 && x < COLS) {
            mvaddch(y, x, ch);
        }
    }
}

/* Helper function to draw a vertical line */
static void draw_vline(int x, int y1, int y2, chtype ch) {
    if (x < 0 || x >= COLS) return;

    for (int y = y1; y <= y2; y++) {
        if (y >= 0 && y < LINES) {
            mvaddch(y, x, ch);
        }
    }
}

/* Helper function to safely print text at position */
static void safe_mvprintw(int y, int x, const char *text) {
    if (y < 0 || y >= LINES || x >= COLS) return;

    int max_len = COLS - x;
    if (max_len <= 0) return;

    move(y, x);
    for (int i = 0; text[i] != '\0' && i < max_len; i++) {
        addch(text[i]);
    }
}

void render_box(const Box *box, const Viewport *vp) {
    /* Convert world coordinates to screen coordinates */
    int sx = world_to_screen_x(vp, box->x);
    int sy = world_to_screen_y(vp, box->y);
    int scaled_width = (int)(box->width * vp->zoom);
    int scaled_height = (int)(box->height * vp->zoom);

    /* Skip if box is completely off-screen */
    if (sx + scaled_width < 0 || sx >= vp->term_width ||
        sy + scaled_height < 0 || sy >= vp->term_height) {
        return;
    }

    /* Enable color for the box */
    if (box->color > 0 && has_colors()) {
        attron(COLOR_PAIR(box->color));
    }

    /* Enable standout mode for selected boxes */
    if (box->selected) {
        attron(A_STANDOUT);
    }

    /* Draw box border using Unicode box-drawing characters */
    /* Top border */
    if (sy >= 0 && sy < vp->term_height) {
        if (sx >= 0 && sx < vp->term_width) {
            mvaddch(sy, sx, ACS_ULCORNER);
        }
        draw_hline(sy, sx + 1, sx + scaled_width - 1, ACS_HLINE);
        if (sx + scaled_width >= 0 && sx + scaled_width < vp->term_width) {
            mvaddch(sy, sx + scaled_width, ACS_URCORNER);
        }
    }

    /* Bottom border */
    if (sy + scaled_height >= 0 && sy + scaled_height < vp->term_height) {
        if (sx >= 0 && sx < vp->term_width) {
            mvaddch(sy + scaled_height, sx, ACS_LLCORNER);
        }
        draw_hline(sy + scaled_height, sx + 1, sx + scaled_width - 1, ACS_HLINE);
        if (sx + scaled_width >= 0 && sx + scaled_width < vp->term_width) {
            mvaddch(sy + scaled_height, sx + scaled_width, ACS_LRCORNER);
        }
    }

    /* Left and right borders */
    draw_vline(sx, sy + 1, sy + scaled_height - 1, ACS_VLINE);
    draw_vline(sx + scaled_width, sy + 1, sy + scaled_height - 1, ACS_VLINE);

    /* Disable standout mode after border */
    if (box->selected) {
        attroff(A_STANDOUT);
    }

    /* Draw title if it exists and there's room */
    if (box->title != NULL && scaled_height > 1) {
        int title_y = sy + 1;
        int title_x = sx + 2;
        if (title_y >= 0 && title_y < vp->term_height && title_x < vp->term_width) {
            attron(A_BOLD);
            if (box->selected) {
                attron(A_STANDOUT);
            }
            safe_mvprintw(title_y, title_x, box->title);
            if (box->selected) {
                attroff(A_STANDOUT);
            }
            attroff(A_BOLD);
        }
    }

    /* Draw content if it exists and there's room */
    if (box->content != NULL && scaled_height > 2) {
        int content_start_y = sy + (box->title ? 2 : 1);
        for (int i = 0; i < box->content_lines && content_start_y + i < sy + scaled_height; i++) {
            int line_y = content_start_y + i;
            int line_x = sx + 2;
            if (line_y >= 0 && line_y < vp->term_height && line_x < vp->term_width) {
                safe_mvprintw(line_y, line_x, box->content[i]);
            }
        }
    }

    /* Disable color */
    if (box->color > 0 && has_colors()) {
        attroff(COLOR_PAIR(box->color));
    }
}

void render_canvas(const Canvas *canvas, const Viewport *vp) {
    for (int i = 0; i < canvas->box_count; i++) {
        render_box(&canvas->boxes[i], vp);
    }
}

void render_status(const Canvas *canvas, const Viewport *vp) {
    char status[512];
    char selected_info[128] = "";

    /* Add selected box info if any */
    Box *selected = canvas_get_selected((Canvas *)canvas);
    if (selected && selected->title) {
        snprintf(selected_info, sizeof(selected_info), " | Selected: %s", selected->title);
    }

    snprintf(status, sizeof(status),
             " Pos: (%.1f, %.1f) | Zoom: %.2fx | Boxes: %d%s | [N]ew [D]el [F2]Save [F3]Load",
             vp->cam_x, vp->cam_y, vp->zoom, canvas->box_count, selected_info);

    /* Draw status bar at bottom */
    attron(A_REVERSE);
    safe_mvprintw(vp->term_height - 1, 0, status);

    /* Fill rest of status line */
    int status_len = strlen(status);
    for (int x = status_len; x < vp->term_width; x++) {
        mvaddch(vp->term_height - 1, x, ' ');
    }
    attroff(A_REVERSE);
}
