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
    char grid_info[64] = "";
    char conn_info[64] = "";

    /* Add selected box info if any */
    Box *selected = canvas_get_selected((Canvas *)canvas);
    if (selected && selected->title) {
        snprintf(selected_info, sizeof(selected_info), " | Selected: %s", selected->title);
    }

    /* Add grid/snap info (Phase 4) */
    if (canvas->grid.visible && canvas->grid.snap_enabled) {
        snprintf(grid_info, sizeof(grid_info), " [GRID:%d] [SNAP]", canvas->grid.spacing);
    } else if (canvas->grid.visible) {
        snprintf(grid_info, sizeof(grid_info), " [GRID:%d]", canvas->grid.spacing);
    } else if (canvas->grid.snap_enabled) {
        snprintf(grid_info, sizeof(grid_info), " [SNAP]");
    }

    /* Add connection info (Issue #20) */
    if (canvas->conn_count > 0) {
        snprintf(conn_info, sizeof(conn_info), " Conn: %d", canvas->conn_count);
    }

    snprintf(status, sizeof(status),
             " Pos: (%.1f, %.1f) | Zoom: %.2fx | Boxes: %d%s%s%s | [n]Sq [D]el [c]onn [G]rid",
             vp->cam_x, vp->cam_y, vp->zoom, canvas->box_count, conn_info, selected_info, grid_info);

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

    /* Only show cursor in NAV mode */
    if (js->mode != MODE_NAV) {
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
        case MODE_NAV:
            mode_text = "NAV";
            hint_text = "LB=Mode | A/B=Zoom | X=Sq | LB+X=Hor | RB+X=Vt | Y=Grid";
            break;
        case MODE_SELECTION:
            mode_text = "SELECTION";
            hint_text = "LB=Mode | A=Cycle | B=NAV | X=EDIT | Y=Delete";
            break;
        case MODE_EDIT:
            mode_text = "EDIT";
            hint_text = "LB=Mode | A=Text | B=Apply | X=Color | Y=Params";
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

/* Render parameter edit panel (Phase 2) */
void render_parameter_panel(const JoystickState *js, const Box *box) {
    if (!js || !box || !js->param_editor_active) {
        return;
    }

    /* Panel position: center of screen */
    int panel_width = 50;
    int panel_height = 12;
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

    /* Content */
    int content_y = panel_y + 2;

    /* Box name */
    attron(A_BOLD);
    mvprintw(content_y++, panel_x + 3, "Editing: %s", box->title);
    attroff(A_BOLD);

    content_y++;  /* Blank line */

    /* Color names for display */
    const char *color_names[] = {"Default", "Red", "Green", "Blue",
                                  "Yellow", "Magenta", "Cyan", "White"};

    /* Field 0: Width */
    int y = content_y++;
    if (js->param_selected_field == 0) {
        attron(A_REVERSE | COLOR_PAIR(2));  /* Highlighted */
        mvprintw(y, panel_x + 3, "[>] Width:  %2d  ", js->param_edit_width);
        attroff(A_REVERSE | COLOR_PAIR(2));
    } else {
        mvprintw(y, panel_x + 3, "[ ] Width:  %2d  ", js->param_edit_width);
    }
    /* Visual slider */
    int slider_x = panel_x + 22;
    mvprintw(y, slider_x, "< ");
    int bar_len = 15;
    int bar_pos = ((js->param_edit_width - 10) * bar_len) / (80 - 10);
    for (int i = 0; i < bar_len; i++) {
        mvaddch(y, slider_x + 2 + i, (i == bar_pos) ? 'O' : '-');
    }
    mvprintw(y, slider_x + 2 + bar_len, " >");

    /* Field 1: Height */
    y = content_y++;
    if (js->param_selected_field == 1) {
        attron(A_REVERSE | COLOR_PAIR(2));
        mvprintw(y, panel_x + 3, "[>] Height: %2d  ", js->param_edit_height);
        attroff(A_REVERSE | COLOR_PAIR(2));
    } else {
        mvprintw(y, panel_x + 3, "[ ] Height: %2d  ", js->param_edit_height);
    }
    /* Visual slider */
    mvprintw(y, slider_x, "< ");
    bar_pos = ((js->param_edit_height - 3) * bar_len) / (30 - 3);
    for (int i = 0; i < bar_len; i++) {
        mvaddch(y, slider_x + 2 + i, (i == bar_pos) ? 'O' : '-');
    }
    mvprintw(y, slider_x + 2 + bar_len, " >");

    /* Field 2: Color */
    y = content_y++;
    if (js->param_selected_field == 2) {
        attron(A_REVERSE | COLOR_PAIR(2));
        mvprintw(y, panel_x + 3, "[>] Color:  %-8s", color_names[js->param_edit_color]);
        attroff(A_REVERSE | COLOR_PAIR(2));
    } else {
        mvprintw(y, panel_x + 3, "[ ] Color:  %-8s", color_names[js->param_edit_color]);
    }
    mvprintw(y, slider_x, "< %s >", color_names[js->param_edit_color]);

    content_y += 2;  /* Space before controls */

    /* Control hints */
    attron(COLOR_PAIR(6));  /* Cyan */
    mvprintw(content_y++, panel_x + 3, "Up/Down: Select field");
    mvprintw(content_y++, panel_x + 3, "Left/Right or LB/RB: Adjust value");
    attroff(COLOR_PAIR(6));

    /* Action buttons */
    attron(A_BOLD);
    mvprintw(panel_y + panel_height - 2, panel_x + 3, "[A] Apply & Close    [B] Cancel & Close");
    attroff(A_BOLD);
}

/* Render joystick visualizer panel showing button states and stick position */
void render_joystick_visualizer(const JoystickState *js, const Viewport *vp) {
    if (!js || !js->available || !js->show_visualizer) {
        return;
    }

    /* Panel position: right side of screen */
    int panel_width = 35;
    int panel_height = 20;
    int panel_x = COLS - panel_width - 2;
    int panel_y = 3;

    /* Clamp position if terminal too small */
    if (panel_x < 0) panel_x = 0;
    if (panel_y < 0) panel_y = 0;

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
    mvprintw(panel_y, panel_x + 2, " JOYSTICK ");
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

    /* Current mode (prominent) */
    int content_y = panel_y + 2;
    attron(A_BOLD | COLOR_PAIR(2));  /* Bold green */
    const char *mode_text = "UNKNOWN";
    switch (js->mode) {
        case MODE_NAV:       mode_text = "NAV"; break;
        case MODE_SELECTION: mode_text = "SELECTION"; break;
        case MODE_EDIT:      mode_text = "EDIT"; break;
    }
    mvprintw(content_y++, panel_x + 3, "Mode: %s", mode_text);
    attroff(A_BOLD | COLOR_PAIR(2));

    content_y++;  /* Blank line */

    /* Global LB mode toggle (per Issue #15) */
    bool lb_pressed = joystick_button_held(js, BUTTON_LB);
    if (lb_pressed) {
        attron(A_REVERSE | COLOR_PAIR(2));
        mvprintw(content_y, panel_x + 3, "[LB]");
        attroff(A_REVERSE | COLOR_PAIR(2));
    } else {
        mvprintw(content_y, panel_x + 3, " LB ");
    }
    attron(A_BOLD | COLOR_PAIR(5));  /* Magenta, bold for emphasis */
    mvprintw(content_y, panel_x + 9, "= Mode Toggle (Global)");
    attroff(A_BOLD | COLOR_PAIR(5));
    content_y++;

    content_y++;  /* Blank line */

    /* Button states and actions (mode-specific) */
    /* Note: X button in NAV mode shows template combos (Issue #17) */
    struct {
        int button_id;
        const char *label;
        const char *action_nav;
        const char *action_selection;
        const char *action_edit;
    } buttons[] = {
        {BUTTON_A,  "A ", "Zoom In",        "Cycle Box",     "Edit Text"},
        {BUTTON_B,  "B ", "Zoom Out",       "-> NAV",        "Apply"},
        {BUTTON_X,  "X ", "Create Sq",      "-> EDIT",       "Cycle Color"},
        {BUTTON_Y,  "Y ", "Toggle Grid",    "Delete",        "Parameters"},
        {BUTTON_RB, "RB", "Snap / +X=Vt",   "Duplicate",     "Increase"}
    };

    for (int i = 0; i < 5; i++) {
        bool pressed = joystick_button_held(js, buttons[i].button_id);
        const char *action = NULL;

        /* Select action based on current mode */
        switch (js->mode) {
            case MODE_NAV:       action = buttons[i].action_nav; break;
            case MODE_SELECTION: action = buttons[i].action_selection; break;
            case MODE_EDIT:      action = buttons[i].action_edit; break;
        }

        /* Button label with visual indicator */
        if (pressed) {
            attron(A_REVERSE | COLOR_PAIR(2));  /* Highlighted when pressed */
            mvprintw(content_y, panel_x + 3, "[%s]", buttons[i].label);
            attroff(A_REVERSE | COLOR_PAIR(2));
        } else {
            mvprintw(content_y, panel_x + 3, " %s ", buttons[i].label);
        }

        /* Action description */
        attron(COLOR_PAIR(6));  /* Cyan */
        mvprintw(content_y, panel_x + 9, "%s", action ? action : "(unused)");
        attroff(COLOR_PAIR(6));

        content_y++;
    }

    content_y++;  /* Blank line */

    /* Global buttons */
    attron(COLOR_PAIR(7));
    mvprintw(content_y++, panel_x + 3, "START: Save Canvas");
    mvprintw(content_y++, panel_x + 3, "SELECT: Quit");
    mvprintw(content_y++, panel_x + 3, "BACK: Hide Panel");
    attroff(COLOR_PAIR(7));

    content_y++;  /* Blank line */

    /* Left stick position indicator */
    double axis_x = joystick_get_axis_normalized(js, AXIS_X);
    double axis_y = joystick_get_axis_normalized(js, AXIS_Y);

    attron(A_BOLD);
    mvprintw(content_y++, panel_x + 3, "Left Stick:");
    attroff(A_BOLD);

    /* Simple ASCII radial indicator (5x5 grid) */
    const int grid_size = 5;
    const int grid_start_x = panel_x + 8;
    const int grid_start_y = content_y;

    /* Draw grid */
    for (int gy = 0; gy < grid_size; gy++) {
        for (int gx = 0; gx < grid_size; gx++) {
            char ch = '.';
            int screen_x = grid_start_x + gx * 2;
            int screen_y = grid_start_y + gy;

            /* Center point */
            if (gx == 2 && gy == 2) {
                ch = '+';
            }

            /* Stick position (map -1..1 to 0..4) */
            int stick_gx = (int)((axis_x + 1.0) * 2.0);
            int stick_gy = (int)((axis_y + 1.0) * 2.0);
            if (stick_gx < 0) stick_gx = 0;
            if (stick_gx > 4) stick_gx = 4;
            if (stick_gy < 0) stick_gy = 0;
            if (stick_gy > 4) stick_gy = 4;

            if (gx == stick_gx && gy == stick_gy) {
                attron(A_REVERSE | COLOR_PAIR(2));
                ch = 'O';
            }

            mvaddch(screen_y, screen_x, ch);

            if (gx == stick_gx && gy == stick_gy) {
                attroff(A_REVERSE | COLOR_PAIR(2));
            }
        }
    }

    content_y += grid_size + 1;

    /* Numeric coordinates */
    attron(COLOR_PAIR(6));
    mvprintw(content_y++, panel_x + 3, "X: %+.2f  Y: %+.2f", axis_x, axis_y);
    attroff(COLOR_PAIR(6));

    content_y++;

    /* Footer: Toggle hint */
    attron(COLOR_PAIR(7));
    mvprintw(panel_y + panel_height - 2, panel_x + 3, "BACK button = hide");
    attroff(COLOR_PAIR(7));

    (void)vp;  /* Suppress unused warning */
}

/* Render text editor panel (Phase 3) */
void render_text_editor(const JoystickState *js, const Box *box) {
    if (!js || !box || !js->text_editor_active || !js->text_edit_buffer) {
        return;
    }

    /* Panel position: center of screen */
    int panel_width = 60;
    int panel_height = 10;
    int panel_x = (COLS - panel_width) / 2;
    int panel_y = (LINES - panel_height) / 2;

    /* Clamp position if terminal too small */
    if (panel_x < 0) panel_x = 0;
    if (panel_y < 0) panel_y = 0;

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
    mvprintw(panel_y, panel_x + 2, " EDIT TEXT ");
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

    /* Content */
    int content_y = panel_y + 2;

    /* Field label */
    attron(A_BOLD);
    mvprintw(content_y++, panel_x + 3, "Box Title:");
    attroff(A_BOLD);

    content_y++;  /* Blank line */

    /* Text input field */
    int field_x = panel_x + 3;
    int field_width = panel_width - 6;

    /* Draw field border */
    attron(COLOR_PAIR(6));
    mvaddch(content_y, field_x, ACS_ULCORNER);
    for (int x = 1; x < field_width - 1; x++) {
        mvaddch(content_y, field_x + x, ACS_HLINE);
    }
    mvaddch(content_y, field_x + field_width - 1, ACS_URCORNER);

    /* Text content with cursor */
    content_y++;
    mvaddch(content_y, field_x, ACS_VLINE);
    mvprintw(content_y, field_x + 2, " ");

    /* Display text with cursor */
    int text_len = strlen(js->text_edit_buffer);
    int display_start = 0;
    int display_len = field_width - 6;  /* Leave room for borders and padding */

    /* Scroll text if cursor goes off screen */
    if (js->text_cursor_pos >= display_len) {
        display_start = js->text_cursor_pos - display_len + 1;
    }

    /* Draw text before cursor */
    for (int i = 0; i < js->text_cursor_pos - display_start && i < display_len; i++) {
        int ch = js->text_edit_buffer[display_start + i];
        mvaddch(content_y, field_x + 2 + i, ch);
    }

    /* Draw cursor (reverse video or underscore) */
    int cursor_screen_x = field_x + 2 + (js->text_cursor_pos - display_start);
    if (cursor_screen_x < field_x + field_width - 2) {
        attron(A_REVERSE | COLOR_PAIR(2));  /* Green highlight */
        if (js->text_cursor_pos < text_len) {
            /* Cursor on character */
            mvaddch(content_y, cursor_screen_x, js->text_edit_buffer[js->text_cursor_pos]);
        } else {
            /* Cursor at end */
            mvaddch(content_y, cursor_screen_x, ' ');
        }
        attroff(A_REVERSE | COLOR_PAIR(2));
    }

    /* Draw text after cursor */
    for (int i = js->text_cursor_pos + 1 - display_start; i < text_len - display_start && i < display_len; i++) {
        int ch = js->text_edit_buffer[display_start + i];
        mvaddch(content_y, field_x + 2 + i, ch);
    }

    /* Clear rest of field */
    int drawn_chars = text_len - display_start;
    if (drawn_chars < 0) drawn_chars = 0;
    for (int i = drawn_chars; i < display_len; i++) {
        if (field_x + 2 + i < field_x + field_width - 2) {
            mvaddch(content_y, field_x + 2 + i, ' ');
        }
    }

    mvaddch(content_y, field_x + field_width - 1, ACS_VLINE);

    /* Bottom field border */
    content_y++;
    mvaddch(content_y, field_x, ACS_LLCORNER);
    for (int x = 1; x < field_width - 1; x++) {
        mvaddch(content_y, field_x + x, ACS_HLINE);
    }
    mvaddch(content_y, field_x + field_width - 1, ACS_LRCORNER);

    content_y += 2;  /* Blank line */

    /* Instructions */
    attron(COLOR_PAIR(6));  /* Cyan */
    mvprintw(content_y++, panel_x + 3, "Type to edit | Arrows=Move | Backspace=Delete");
    attroff(COLOR_PAIR(6));

    /* Action button */
    attron(A_BOLD);
    mvprintw(panel_y + panel_height - 2, panel_x + 3, "ESC or Button B: Save & Close");
    attroff(A_BOLD);
}

/* Render grid (Phase 4) */
void render_grid(const Canvas *canvas, const Viewport *vp) {
    if (!canvas || !vp || !canvas->grid.visible) {
        return;
    }

    /* Calculate visible world bounds */
    double world_left = vp->cam_x;
    double world_top = vp->cam_y;
    double world_right = vp->cam_x + (vp->term_width / vp->zoom);
    double world_bottom = vp->cam_y + (vp->term_height / vp->zoom);

    int spacing = canvas->grid.spacing;

    /* Find first grid point in visible area */
    int grid_start_x = ((int)(world_left / spacing)) * spacing;
    int grid_start_y = ((int)(world_top / spacing)) * spacing;

    /* Adjust if we're before the first grid line */
    if (grid_start_x < world_left) grid_start_x += spacing;
    if (grid_start_y < world_top) grid_start_y += spacing;

    /* Use dim color for grid (gray) */
    attron(COLOR_PAIR(8));  /* Dim gray - COLOR_PAIR(8) */

    /* Draw grid points (dot grid) */
    for (double world_x = grid_start_x; world_x <= world_right; world_x += spacing) {
        for (double world_y = grid_start_y; world_y <= world_bottom; world_y += spacing) {
            /* Convert world to screen coordinates */
            int screen_x = world_to_screen_x(vp, world_x);
            int screen_y = world_to_screen_y(vp, world_y);

            /* Check bounds */
            if (screen_x >= 0 && screen_x < vp->term_width &&
                screen_y >= 0 && screen_y < vp->term_height) {

                /* Highlight origin (0,0) with different character */
                if (world_x == 0 && world_y == 0) {
                    attron(A_BOLD);
                    mvaddch(screen_y, screen_x, '+');
                    attroff(A_BOLD);
                } else {
                    mvaddch(screen_y, screen_x, '.');
                }
            }
        }
    }

    attroff(COLOR_PAIR(8));
}

/* Render focused box in full-screen mode (Phase 5b) */
void render_focused_box(const Canvas *canvas) {
    if (!canvas || !canvas->focus.active) {
        return;
    }

    Box *box = canvas_get_box((Canvas *)canvas, canvas->focus.focused_box_id);
    if (!box) {
        return;
    }

    /* Title bar */
    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 1, " %s ", box->title ? box->title : "Untitled");

    /* Focus mode indicator */
    const char *hint = " [FOCUS MODE - ESC to exit] ";
    int hint_x = COLS - strlen(hint) - 1;
    if (hint_x > (int)strlen(box->title) + 4) {
        mvprintw(0, hint_x, "%s", hint);
    }

    /* Fill rest of title bar */
    int title_end = strlen(box->title ? box->title : "Untitled") + 3;
    for (int x = title_end; x < hint_x; x++) {
        mvaddch(0, x, ' ');
    }
    attroff(A_REVERSE | A_BOLD);

    /* Separator line */
    for (int x = 0; x < COLS; x++) {
        mvaddch(1, x, ACS_HLINE);
    }

    /* Content area */
    int content_start_y = 2;
    int content_height = LINES - 4;  /* Title(1) + Sep(1) + Status(2) */

    /* Calculate scroll max */
    int max_scroll = 0;
    if (box->content_lines > content_height) {
        max_scroll = box->content_lines - content_height;
    }

    /* Update scroll max in canvas */
    ((Canvas *)canvas)->focus.scroll_max = max_scroll;

    /* Clamp scroll offset */
    if (canvas->focus.scroll_offset > max_scroll) {
        ((Canvas *)canvas)->focus.scroll_offset = max_scroll;
    }
    if (canvas->focus.scroll_offset < 0) {
        ((Canvas *)canvas)->focus.scroll_offset = 0;
    }

    /* Render content lines with scrolling */
    if (box->content) {
        for (int i = 0; i < content_height; i++) {
            int line_idx = canvas->focus.scroll_offset + i;

            if (line_idx >= 0 && line_idx < box->content_lines) {
                /* Line numbers (dim) */
                attron(COLOR_PAIR(8));
                mvprintw(content_start_y + i, 1, "%4d ", line_idx + 1);
                attroff(COLOR_PAIR(8));

                /* Content (handle long lines by truncating) */
                int content_start_x = 7;
                int max_width = COLS - content_start_x - 1;
                char display_line[1024];

                if (box->content[line_idx]) {
                    strncpy(display_line, box->content[line_idx], sizeof(display_line) - 1);
                    display_line[sizeof(display_line) - 1] = '\0';

                    /* Truncate if too long */
                    if ((int)strlen(display_line) > max_width) {
                        display_line[max_width] = '\0';
                    }

                    mvprintw(content_start_y + i, content_start_x, "%s", display_line);
                }
            }
        }
    }

    /* Status separator */
    int status_y = LINES - 2;
    for (int x = 0; x < COLS; x++) {
        mvaddch(status_y, x, ACS_HLINE);
    }

    /* Status bar */
    attron(A_REVERSE);
    char status[256];
    int current_line = canvas->focus.scroll_offset + 1;
    int total_lines = box->content_lines;

    snprintf(status, sizeof(status),
             " j/k: Scroll | g: Top | G: Bottom | ESC: Exit | Line %d/%d ",
             current_line, total_lines > 0 ? total_lines : 1);

    mvprintw(LINES - 1, 0, "%s", status);
    for (int x = strlen(status); x < COLS; x++) {
        mvaddch(LINES - 1, x, ' ');
    }
    attroff(A_REVERSE);
}

/* ============================================================
 * Connection Rendering Functions (Issue #20)
 * ============================================================ */

/* Helper: Draw a line using Bresenham's algorithm (screen coordinates) */
static void draw_bresenham_line(int x0, int y0, int x1, int y1, chtype ch,
                                int term_width, int term_height) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    /* Calculate absolute values */
    int abs_dx = dx < 0 ? -dx : dx;
    int abs_dy = dy < 0 ? -dy : dy;

    /* Determine direction signs */
    int sx = dx < 0 ? -1 : 1;
    int sy = dy < 0 ? -1 : 1;

    int err = abs_dx - abs_dy;
    int x = x0;
    int y = y0;

    while (1) {
        /* Draw point if within screen bounds */
        if (x >= 0 && x < term_width && y >= 0 && y < term_height - 1) {
            mvaddch(y, x, ch);
        }

        /* Check if we've reached the end */
        if (x == x1 && y == y1) break;

        int e2 = 2 * err;

        if (e2 > -abs_dy) {
            err -= abs_dy;
            x += sx;
        }
        if (e2 < abs_dx) {
            err += abs_dx;
            y += sy;
        }
    }
}

/* Render all connections between boxes (Issue #20) */
void render_connections(const Canvas *canvas, const Viewport *vp) {
    if (!canvas || !vp || canvas->conn_count == 0) {
        return;
    }

    for (int i = 0; i < canvas->conn_count; i++) {
        const Connection *conn = &canvas->connections[i];

        /* Get source and destination boxes */
        Box *source = canvas_get_box((Canvas *)canvas, conn->source_id);
        Box *dest = canvas_get_box((Canvas *)canvas, conn->dest_id);

        if (!source || !dest) continue;

        /* Calculate world centers of each box */
        double src_center_x = source->x + source->width / 2.0;
        double src_center_y = source->y + source->height / 2.0;
        double dest_center_x = dest->x + dest->width / 2.0;
        double dest_center_y = dest->y + dest->height / 2.0;

        /* Convert to screen coordinates */
        int sx0 = world_to_screen_x(vp, src_center_x);
        int sy0 = world_to_screen_y(vp, src_center_y);
        int sx1 = world_to_screen_x(vp, dest_center_x);
        int sy1 = world_to_screen_y(vp, dest_center_y);

        /* Skip if both endpoints are off-screen */
        if ((sx0 < 0 && sx1 < 0) || (sx0 >= vp->term_width && sx1 >= vp->term_width) ||
            (sy0 < 0 && sy1 < 0) || (sy0 >= vp->term_height && sy1 >= vp->term_height)) {
            continue;
        }

        /* Set connection color */
        if (conn->color > 0 && has_colors()) {
            attron(COLOR_PAIR(conn->color));
        }

        /* Choose appropriate line character based on angle */
        int ldx = sx1 - sx0;
        int ldy = sy1 - sy0;
        chtype line_ch = '*';  /* Default */

        if (ldx != 0 || ldy != 0) {
            /* Calculate approximate angle and choose character */
            double angle = 0.0;
            if (ldx != 0) {
                angle = (double)ldy / (double)ldx;
            }

            if (ldx == 0) {
                line_ch = '|';  /* Vertical */
            } else if (ldy == 0) {
                line_ch = '-';  /* Horizontal */
            } else if ((angle > 0.5 && angle < 2.0) || (angle < -0.5 && angle > -2.0)) {
                line_ch = (ldx * ldy > 0) ? '\\' : '/';  /* Diagonal */
            } else if (angle >= 2.0 || angle <= -2.0) {
                line_ch = '|';  /* Nearly vertical */
            } else {
                line_ch = '-';  /* Nearly horizontal */
            }
        }

        /* Draw the line */
        draw_bresenham_line(sx0, sy0, sx1, sy1, line_ch, vp->term_width, vp->term_height);

        /* Disable color */
        if (conn->color > 0 && has_colors()) {
            attroff(COLOR_PAIR(conn->color));
        }
    }
}

/* Render connection mode indicator (Issue #20) */
void render_connection_mode(const Canvas *canvas, const Viewport *vp) {
    if (!canvas || !vp) {
        return;
    }

    /* Show connection mode indicator */
    if (canvas->conn_mode.active) {
        /* Draw "CONNECTION MODE" indicator */
        attron(A_REVERSE | A_BOLD | COLOR_PAIR(BOX_COLOR_CYAN));
        mvprintw(0, 2, " CONNECTION MODE ");
        attroff(A_REVERSE | A_BOLD | COLOR_PAIR(BOX_COLOR_CYAN));

        /* Show source box name */
        Box *source = canvas_get_box((Canvas *)canvas, canvas->conn_mode.source_box_id);
        if (source && source->title) {
            attron(COLOR_PAIR(BOX_COLOR_CYAN));
            mvprintw(0, 21, " From: %s -> Select destination (c) or ESC to cancel",
                     source->title);
            attroff(COLOR_PAIR(BOX_COLOR_CYAN));
        }

        /* Draw visual indicator from source box center to cursor/selected box */
        if (source) {
            double src_center_x = source->x + source->width / 2.0;
            double src_center_y = source->y + source->height / 2.0;

            int sx0 = world_to_screen_x(vp, src_center_x);
            int sy0 = world_to_screen_y(vp, src_center_y);

            /* Draw a temporary line to the currently selected box (if different) */
            Box *selected = canvas_get_selected((Canvas *)canvas);
            if (selected && selected->id != canvas->conn_mode.source_box_id) {
                double dest_center_x = selected->x + selected->width / 2.0;
                double dest_center_y = selected->y + selected->height / 2.0;

                int sx1 = world_to_screen_x(vp, dest_center_x);
                int sy1 = world_to_screen_y(vp, dest_center_y);

                /* Draw preview line in yellow (dashed effect by using dots) */
                attron(COLOR_PAIR(BOX_COLOR_YELLOW) | A_DIM);
                draw_bresenham_line(sx0, sy0, sx1, sy1, '.', vp->term_width, vp->term_height);
                attroff(COLOR_PAIR(BOX_COLOR_YELLOW) | A_DIM);
            }
        }
    }

    /* Show delete confirmation if pending */
    if (canvas->conn_mode.pending_delete) {
        attron(A_REVERSE | A_BOLD | COLOR_PAIR(BOX_COLOR_RED));
        mvprintw(0, 2, " Press D again to delete connection ");
        attroff(A_REVERSE | A_BOLD | COLOR_PAIR(BOX_COLOR_RED));
    }
}

