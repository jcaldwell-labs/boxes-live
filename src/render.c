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

/* Render joystick cursor indicator */
void render_joystick_cursor(const JoystickState *js, const Viewport *vp) {
    if (!js || !js->available) {
        return;
    }

    /* Only show cursor in navigation mode */
    if (js->mode != MODE_NAVIGATION) {
        return;
    }

    /* Convert cursor world position to screen position */
    int screen_x = world_to_screen_x(vp, js->cursor_x);
    int screen_y = world_to_screen_y(vp, js->cursor_y);

    /* Check if cursor is visible */
    if (screen_x >= 0 && screen_x < vp->term_width - 1 &&
        screen_y >= 0 && screen_y < vp->term_height - 2) {

        /* Draw cursor as a crosshair */
        attron(COLOR_PAIR(5) | A_BOLD);  /* Magenta, bold */
        mvaddch(screen_y, screen_x, '+');
        attroff(COLOR_PAIR(5) | A_BOLD);
    }
}

/* Render joystick mode indicator (enhanced status bar) */
void render_joystick_mode(const JoystickState *js, const Canvas *canvas) {
    if (!js || !js->available) {
        return;
    }

    /* Get terminal height for status bar position */
    int term_height = LINES;

    /* Mode indicator at far right of status bar */
    const char *mode_text = NULL;
    const char *hint_text = NULL;

    switch (js->mode) {
        case MODE_NAVIGATION:
            mode_text = "NAV";
            hint_text = "A=Zoom+ B=Zoom- X=Create Y=Del";
            break;
        case MODE_EDIT:
            mode_text = "EDIT";
            hint_text = "A=Params B=Nav X=Color Y=Del";
            break;
        case MODE_PARAMETER:
            mode_text = "PARAMS";
            hint_text = "Y=Select X=Adjust A=OK B=Cancel";
            break;
    }

    if (mode_text) {
        attron(A_REVERSE | A_BOLD);
        int x_pos = COLS - strlen(mode_text) - 2;
        mvprintw(term_height - 1, x_pos, " %s ", mode_text);
        attroff(A_REVERSE | A_BOLD);
    }

    /* Show button hints on second-to-last line if in joystick mode */
    if (hint_text) {
        attron(COLOR_PAIR(6));  /* Cyan for hints */
        mvprintw(term_height - 2, 2, "%s", hint_text);
        attroff(COLOR_PAIR(6));
    }

    (void)canvas;  /* Suppress unused warning */
}

/* Render parameter edit panel (when in parameter mode) */
void render_parameter_panel(const JoystickState *js, const Box *box) {
    if (!js || !box || js->mode != MODE_PARAMETER) {
        return;
    }

    /* Panel position: center of screen */
    int panel_width = 25;
    int panel_height = 10;
    int panel_x = (COLS - panel_width) / 2;
    int panel_y = (LINES - panel_height) / 2;

    /* Draw panel border */
    attron(COLOR_PAIR(7) | A_BOLD);  /* White, bold */

    /* Top border */
    mvaddch(panel_y, panel_x, ACS_ULCORNER);
    for (int x = 1; x < panel_width - 1; x++) {
        mvaddch(panel_y, panel_x + x, ACS_HLINE);
    }
    mvaddch(panel_y, panel_x + panel_width - 1, ACS_URCORNER);

    /* Title */
    attron(A_REVERSE);
    mvprintw(panel_y, panel_x + 2, " BOX PARAMETERS ");
    attroff(A_REVERSE);

    /* Side borders and content */
    for (int y = 1; y < panel_height - 1; y++) {
        mvaddch(panel_y + y, panel_x, ACS_VLINE);
        mvaddch(panel_y + y, panel_x + panel_width - 1, ACS_VLINE);

        /* Clear interior */
        for (int x = 1; x < panel_width - 1; x++) {
            mvaddch(panel_y + y, panel_x + x, ' ');
        }
    }

    /* Bottom border */
    mvaddch(panel_y + panel_height - 1, panel_x, ACS_LLCORNER);
    for (int x = 1; x < panel_width - 1; x++) {
        mvaddch(panel_y + panel_height - 1, panel_x + x, ACS_HLINE);
    }
    mvaddch(panel_y + panel_height - 1, panel_x + panel_width - 1, ACS_LRCORNER);

    attroff(COLOR_PAIR(7) | A_BOLD);

    /* Parameter list */
    const char *param_names[] = {"Width", "Height", "Color"};
    int param_values[] = {box->width, box->height, box->color};

    for (int i = 0; i < 3; i++) {
        int y = panel_y + 3 + (i * 2);
        int x = panel_x + 3;

        /* Arrow for selected parameter */
        if ((int)js->selected_param == i) {
            attron(COLOR_PAIR(2) | A_BOLD);  /* Green, bold */
            mvprintw(y, x, ">");
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else {
            mvprintw(y, x, " ");
        }

        /* Parameter name and value */
        if (i == PARAM_COLOR) {
            const char *color_names[] = {"Default", "Red", "Green", "Blue",
                                          "Yellow", "Magenta", "Cyan", "White"};
            mvprintw(y, x + 2, "%-8s: %s", param_names[i], color_names[param_values[i]]);
        } else {
            mvprintw(y, x + 2, "%-8s: %d", param_names[i], param_values[i]);
        }
    }

    /* Instructions */
    attron(COLOR_PAIR(6));  /* Cyan */
    mvprintw(panel_y + panel_height - 2, panel_x + 3, "Y=Select X=Adjust");
    attroff(COLOR_PAIR(6));
}
