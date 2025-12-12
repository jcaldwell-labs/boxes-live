#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <curses.h>
#include "test_mode.h"
#include "types.h"  /* For GRID_COLOR_PAIR */

/* Global test mode pointer for key handling */
static TestMode *g_test_mode = NULL;

/* Get current time with milliseconds */
static void get_time_ms(time_t *sec, int *ms) {
#ifdef _WIN32
    /* Windows: use time() for seconds, clock() for ms approximation */
    *sec = time(NULL);
    *ms = (int)((clock() % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    *sec = ts.tv_sec;
    *ms = (int)(ts.tv_nsec / 1000000);
#endif
}

/* Get monotonic time for FPS calculation */
static double get_monotonic_time(void) {
#ifdef _WIN32
    return (double)clock() / CLOCKS_PER_SEC;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
#endif
}

void test_mode_init(TestMode *tm) {
    if (!tm) return;

    memset(tm, 0, sizeof(TestMode));
    tm->enabled = false;
    tm->debug_overlay = false;
    tm->event_logging = false;
    tm->event_overlay = true;  /* Show events in overlay by default */
    tm->grid_style = GRID_STYLE_AXES;  /* Axes with subtle dots is recommended */
    tm->mode_variant = 'A';
    tm->event_head = 0;
    tm->event_count = 0;
    tm->marker_count = 0;
    tm->next_marker_number = 1;
    tm->frame_count = 0;
    tm->fps = 0.0;
    tm->fps_update_time = get_monotonic_time();
    tm->fps_frame_count = 0;
    tm->start_time = time(NULL);
    tm->log_file = NULL;
}

void test_mode_cleanup(TestMode *tm) {
    if (!tm) return;

    if (tm->log_file) {
        fclose(tm->log_file);
        tm->log_file = NULL;
    }
}

void test_mode_enable(TestMode *tm, char variant) {
    if (!tm) return;

    tm->enabled = true;
    tm->debug_overlay = true;  /* Enable overlay by default in test mode */
    if (variant >= 'A' && variant <= 'Z') {
        tm->mode_variant = variant;
    }
    tm->start_time = time(NULL);

    test_mode_log_event(tm, "TEST MODE ENABLED (variant %c)", tm->mode_variant);
}

void test_mode_toggle_overlay(TestMode *tm) {
    if (!tm) return;

    tm->debug_overlay = !tm->debug_overlay;
    test_mode_log_event(tm, "Debug overlay: %s",
                        tm->debug_overlay ? "ON" : "OFF");
}

int test_mode_toggle_event_logging(TestMode *tm, const char *filename) {
    if (!tm) return -1;

    if (tm->log_file) {
        /* Close existing log */
        fclose(tm->log_file);
        tm->log_file = NULL;
        tm->event_logging = false;
        test_mode_log_event(tm, "Event logging: OFF");
        return 0;
    }

    /* Open new log file */
    const char *path = filename ? filename : "events.log";
    tm->log_file = fopen(path, "a");
    if (!tm->log_file) {
        return -1;
    }

    tm->event_logging = true;
    test_mode_log_event(tm, "Event logging: ON (%s)", path);

    /* Write header */
    time_t now = time(NULL);
    fprintf(tm->log_file, "\n=== Test Session Started: %s", ctime(&now));
    fprintf(tm->log_file, "=== Mode Variant: %c\n", tm->mode_variant);
    fflush(tm->log_file);

    return 0;
}

void test_mode_cycle_grid_style(TestMode *tm) {
    if (!tm) return;

    tm->grid_style = (tm->grid_style + 1) % GRID_STYLE_COUNT;
    test_mode_log_event(tm, "Grid style: %s",
                        test_mode_grid_style_name(tm->grid_style));
}

void test_mode_log_event(TestMode *tm, const char *format, ...) {
    if (!tm) return;

    time_t sec;
    int ms;
    get_time_ms(&sec, &ms);

    /* Add to circular buffer */
    TestEvent *event = &tm->events[tm->event_head];
    event->timestamp = sec;
    event->ms = ms;

    /* Format message directly into event buffer */
    va_list args;
    va_start(args, format);
    vsnprintf(event->message, TEST_MODE_EVENT_LEN, format, args);
    va_end(args);

    tm->event_head = (tm->event_head + 1) % TEST_MODE_MAX_EVENTS;
    if (tm->event_count < TEST_MODE_MAX_EVENTS) {
        tm->event_count++;
    }

    /* Write to log file if enabled */
    if (tm->event_logging && tm->log_file) {
        struct tm *lt = localtime(&sec);
        fprintf(tm->log_file, "[%02d:%02d:%02d.%03d] %s\n",
                lt->tm_hour, lt->tm_min, lt->tm_sec, ms, event->message);
        fflush(tm->log_file);
    }
}

void test_mode_log_key(TestMode *tm, int key_code, const char *key_name,
                       float world_x, float world_y, const char *mode) {
    if (!tm || !tm->enabled) return;

    if (world_x >= 0 && world_y >= 0) {
        test_mode_log_event(tm, "KEY: %s (%d) at (%.1f, %.1f) mode=%s",
                            key_name, key_code, world_x, world_y, mode);
    } else {
        test_mode_log_event(tm, "KEY: %s (%d) mode=%s",
                            key_name, key_code, mode);
    }
}

void test_mode_log_joystick(TestMode *tm, const char *event_type,
                            int value, const char *context) {
    if (!tm || !tm->enabled) return;

    test_mode_log_event(tm, "JOY: %s value=%d %s",
                        event_type, value, context ? context : "");
}

int test_mode_add_marker(TestMode *tm, float x, float y) {
    if (!tm || tm->marker_count >= TEST_MODE_MAX_MARKERS) return -1;

    TestMarker *marker = &tm->markers[tm->marker_count];
    marker->x = x;
    marker->y = y;
    marker->number = tm->next_marker_number++;
    marker->timestamp = time(NULL);

    tm->marker_count++;

    test_mode_log_event(tm, "MARKER #%d placed at (%.1f, %.1f)",
                        marker->number, x, y);

    return marker->number;
}

void test_mode_clear_markers(TestMode *tm) {
    if (!tm) return;

    int count = tm->marker_count;
    tm->marker_count = 0;
    tm->next_marker_number = 1;

    test_mode_log_event(tm, "Cleared %d markers", count);
}

void test_mode_update_fps(TestMode *tm) {
    if (!tm) return;

    tm->frame_count++;
    tm->fps_frame_count++;

    double now = get_monotonic_time();
    double elapsed = now - tm->fps_update_time;

    /* Update FPS every 0.5 seconds */
    if (elapsed >= 0.5) {
        tm->fps = tm->fps_frame_count / elapsed;
        tm->fps_frame_count = 0;
        tm->fps_update_time = now;
    }
}

const char *test_mode_grid_style_name(GridStyle style) {
    switch (style) {
        case GRID_STYLE_NONE:       return "None";
        case GRID_STYLE_AXES:       return "Axes";
        case GRID_STYLE_DOTS:       return "Dots";
        case GRID_STYLE_LINES:      return "Lines";
        case GRID_STYLE_DASHED:     return "Dashed";
        case GRID_STYLE_CROSSHAIRS: return "Crosshairs";
        default:                    return "Unknown";
    }
}

void test_mode_render_overlay(TestMode *tm, float cam_x, float cam_y,
                              float zoom, float cursor_x, float cursor_y,
                              const char *mode_name, int box_count,
                              int conn_count) {
    if (!tm || !tm->debug_overlay) return;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    /* Calculate screen coords for cursor */
    int screen_cx = (int)((cursor_x - cam_x) * zoom + max_x / 2);
    int screen_cy = (int)((cursor_y - cam_y) * zoom + max_y / 2);

    /* Draw overlay box in top-right corner */
    int overlay_width = 40;
    int overlay_height = 12;
    int overlay_x = max_x - overlay_width - 2;
    int overlay_y = 1;

    /* Semi-transparent background effect using reverse video */
    attron(A_REVERSE);
    for (int y = overlay_y; y < overlay_y + overlay_height && y < max_y; y++) {
        mvhline(y, overlay_x, ' ', overlay_width);
    }
    attroff(A_REVERSE);

    /* Border */
    attron(COLOR_PAIR(6));  /* Cyan */
    mvaddch(overlay_y, overlay_x, ACS_ULCORNER);
    mvaddch(overlay_y, overlay_x + overlay_width - 1, ACS_URCORNER);
    mvaddch(overlay_y + overlay_height - 1, overlay_x, ACS_LLCORNER);
    mvaddch(overlay_y + overlay_height - 1, overlay_x + overlay_width - 1, ACS_LRCORNER);
    mvhline(overlay_y, overlay_x + 1, ACS_HLINE, overlay_width - 2);
    mvhline(overlay_y + overlay_height - 1, overlay_x + 1, ACS_HLINE, overlay_width - 2);
    for (int y = overlay_y + 1; y < overlay_y + overlay_height - 1; y++) {
        mvaddch(y, overlay_x, ACS_VLINE);
        mvaddch(y, overlay_x + overlay_width - 1, ACS_VLINE);
    }

    /* Title */
    attron(A_BOLD);
    mvprintw(overlay_y, overlay_x + 2, " DEBUG [%c] ", tm->mode_variant);
    attroff(A_BOLD);
    attroff(COLOR_PAIR(6));

    /* Content */
    int y = overlay_y + 1;
    int x = overlay_x + 2;

    attron(A_REVERSE);

    /* FPS */
    mvprintw(y++, x, "FPS: %.1f", tm->fps);

    /* Camera position */
    mvprintw(y++, x, "Cam: (%.1f, %.1f) Z:%.2fx", cam_x, cam_y, zoom);

    /* Cursor position */
    mvprintw(y++, x, "Cursor: (%.1f, %.1f)", cursor_x, cursor_y);
    mvprintw(y++, x, "Screen: (%d, %d)", screen_cx, screen_cy);

    /* Mode */
    mvprintw(y++, x, "Mode: %s", mode_name);

    /* Grid style */
    mvprintw(y++, x, "Grid: %s", test_mode_grid_style_name(tm->grid_style));

    /* Stats */
    mvprintw(y++, x, "Boxes: %d  Conns: %d", box_count, conn_count);

    /* Markers */
    mvprintw(y++, x, "Markers: %d", tm->marker_count);

    /* Runtime */
    time_t runtime = time(NULL) - tm->start_time;
    mvprintw(y++, x, "Runtime: %ldm %lds", runtime / 60, runtime % 60);

    attroff(A_REVERSE);

    /* Event log (bottom portion) */
    if (tm->event_overlay && tm->event_count > 0) {
        int log_y = max_y - 6;
        int log_height = 5;

        /* Draw event log area */
        attron(COLOR_PAIR(3));  /* Blue */
        mvprintw(log_y - 1, 1, " Events (%d) ", tm->event_count);
        attroff(COLOR_PAIR(3));

        /* Show last N events */
        int show_count = (tm->event_count < log_height) ? tm->event_count : log_height;
        for (int i = 0; i < show_count; i++) {
            int idx = (tm->event_head - show_count + i + TEST_MODE_MAX_EVENTS)
                      % TEST_MODE_MAX_EVENTS;
            TestEvent *event = &tm->events[idx];

            struct tm *lt = localtime(&event->timestamp);
            attron(A_DIM);
            mvprintw(log_y + i, 1, "[%02d:%02d:%02d] ",
                     lt->tm_hour, lt->tm_min, lt->tm_sec);
            attroff(A_DIM);

            /* Truncate message to fit */
            int msg_max = max_x - 14;
            if ((int)strlen(event->message) > msg_max) {
                mvprintw(log_y + i, 12, "%.*s...", msg_max - 3, event->message);
            } else {
                mvprintw(log_y + i, 12, "%s", event->message);
            }
        }
    }
}

void test_mode_render_markers(TestMode *tm, float cam_x, float cam_y,
                              float zoom) {
    if (!tm || tm->marker_count == 0) return;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    attron(COLOR_PAIR(1) | A_BOLD);  /* Red, bold */

    for (int i = 0; i < tm->marker_count; i++) {
        TestMarker *marker = &tm->markers[i];

        /* Convert world to screen coords */
        int sx = (int)((marker->x - cam_x) * zoom + max_x / 2);
        int sy = (int)((marker->y - cam_y) * zoom + max_y / 2);

        /* Skip if off-screen */
        if (sx < 0 || sx >= max_x || sy < 0 || sy >= max_y) continue;

        /* Draw marker */
        if (marker->number < 10) {
            mvprintw(sy, sx, "[%d]", marker->number);
        } else {
            mvprintw(sy, sx, "[%d]", marker->number);
        }
    }

    attroff(COLOR_PAIR(1) | A_BOLD);
}

void test_mode_render_grid(TestMode *tm, float cam_x, float cam_y,
                           float zoom, int spacing, int screen_width,
                           int screen_height) {
    if (!tm || tm->grid_style == GRID_STYLE_NONE) return;

    /* Calculate visible world bounds (cam_x/cam_y is top-left corner) */
    float world_left = cam_x;
    float world_top = cam_y;
    float world_right = cam_x + screen_width / zoom;
    float world_bottom = cam_y + screen_height / zoom;

    /* Snap to grid - find first grid line in visible area */
    int start_x = ((int)(world_left / spacing)) * spacing;
    int start_y = ((int)(world_top / spacing)) * spacing;
    int end_x = ((int)(world_right / spacing) + 1) * spacing;
    int end_y = ((int)(world_bottom / spacing) + 1) * spacing;

    switch (tm->grid_style) {
        case GRID_STYLE_AXES: {
            /* Draw prominent X and Y axes at origin, with subtle dots elsewhere */
            int origin_sx = (int)((0 - cam_x) * zoom);
            int origin_sy = (int)((0 - cam_y) * zoom);

            /* Draw Y axis (vertical line at x=0) - bright */
            if (origin_sx >= 0 && origin_sx < screen_width) {
                attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_BOLD);
                for (int sy = 0; sy < screen_height - 1; sy++) {
                    mvaddch(sy, origin_sx, ACS_VLINE);
                }
                attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_BOLD);
            }

            /* Draw X axis (horizontal line at y=0) - bright */
            if (origin_sy >= 0 && origin_sy < screen_height - 1) {
                attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_BOLD);
                for (int sx = 0; sx < screen_width; sx++) {
                    if (sx == origin_sx) {
                        mvaddch(origin_sy, sx, ACS_PLUS);  /* Origin */
                    } else {
                        mvaddch(origin_sy, sx, ACS_HLINE);
                    }
                }
                attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_BOLD);
            }

            /* Draw origin marker more prominently */
            if (origin_sx >= 0 && origin_sx < screen_width &&
                origin_sy >= 0 && origin_sy < screen_height - 1) {
                attron(COLOR_PAIR(BOX_COLOR_WHITE) | A_BOLD);
                mvaddch(origin_sy, origin_sx, 'O');
                attroff(COLOR_PAIR(BOX_COLOR_WHITE) | A_BOLD);
            }

            /* Draw subtle dots at major grid intersections (not on axes) */
            int major_spacing = spacing * 5;  /* Every 5th grid line gets a dot */
            attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            for (int wy = start_y; wy <= end_y; wy += major_spacing) {
                if (wy == 0) continue;  /* Skip axis */
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                for (int wx = start_x; wx <= end_x; wx += major_spacing) {
                    if (wx == 0) continue;  /* Skip axis */
                    int sx = (int)((wx - cam_x) * zoom);
                    if (sx < 0 || sx >= screen_width) continue;

                    mvaddch(sy, sx, '+');
                }
            }

            /* Draw very subtle tick marks on axes */
            for (int wx = start_x; wx <= end_x; wx += spacing) {
                if (wx == 0) continue;
                int sx = (int)((wx - cam_x) * zoom);
                if (sx < 0 || sx >= screen_width) continue;

                /* Small tick above and below X axis */
                if (origin_sy >= 1 && origin_sy < screen_height - 2) {
                    if (wx % major_spacing == 0) {
                        /* Major tick */
                        mvaddch(origin_sy - 1, sx, '|');
                        mvaddch(origin_sy + 1, sx, '|');
                    } else {
                        /* Minor tick - just a dot */
                        mvaddch(origin_sy, sx, '.');
                    }
                }
            }

            for (int wy = start_y; wy <= end_y; wy += spacing) {
                if (wy == 0) continue;
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                /* Small tick left and right of Y axis */
                if (origin_sx >= 1 && origin_sx < screen_width - 1) {
                    if (wy % major_spacing == 0) {
                        /* Major tick */
                        mvaddch(sy, origin_sx - 1, '-');
                        mvaddch(sy, origin_sx + 1, '-');
                    } else {
                        /* Minor tick - just a dot */
                        mvaddch(sy, origin_sx, '.');
                    }
                }
            }
            attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            break;
        }

        case GRID_STYLE_DOTS:
            /* Dots at intersections */
            attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            for (int wy = start_y; wy <= end_y; wy += spacing) {
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                for (int wx = start_x; wx <= end_x; wx += spacing) {
                    int sx = (int)((wx - cam_x) * zoom);
                    if (sx < 0 || sx >= screen_width) continue;

                    mvaddch(sy, sx, '.');
                }
            }
            attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            break;

        case GRID_STYLE_LINES:
            /* Full lines */
            attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            for (int wy = start_y; wy <= end_y; wy += spacing) {
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                mvhline(sy, 0, ACS_HLINE, screen_width);
            }
            for (int wx = start_x; wx <= end_x; wx += spacing) {
                int sx = (int)((wx - cam_x) * zoom);
                if (sx < 0 || sx >= screen_width) continue;

                mvvline(0, sx, ACS_VLINE, screen_height - 1);
            }
            /* Intersections */
            for (int wy = start_y; wy <= end_y; wy += spacing) {
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                for (int wx = start_x; wx <= end_x; wx += spacing) {
                    int sx = (int)((wx - cam_x) * zoom);
                    if (sx < 0 || sx >= screen_width) continue;

                    mvaddch(sy, sx, ACS_PLUS);
                }
            }
            attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            break;

        case GRID_STYLE_DASHED:
            /* Dashed lines (every other character) */
            attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            for (int wy = start_y; wy <= end_y; wy += spacing) {
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 0 || sy >= screen_height - 1) continue;

                for (int sx = 0; sx < screen_width; sx += 2) {
                    mvaddch(sy, sx, '-');
                }
            }
            for (int wx = start_x; wx <= end_x; wx += spacing) {
                int sx = (int)((wx - cam_x) * zoom);
                if (sx < 0 || sx >= screen_width) continue;

                for (int sy = 0; sy < screen_height - 1; sy += 2) {
                    mvaddch(sy, sx, '|');
                }
            }
            attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            break;

        case GRID_STYLE_CROSSHAIRS:
            /* Small crosshairs at intersections */
            attron(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            for (int wy = start_y; wy <= end_y; wy += spacing) {
                int sy = (int)((wy - cam_y) * zoom);
                if (sy < 1 || sy >= screen_height - 2) continue;

                for (int wx = start_x; wx <= end_x; wx += spacing) {
                    int sx = (int)((wx - cam_x) * zoom);
                    if (sx < 1 || sx >= screen_width - 1) continue;

                    mvaddch(sy, sx, '+');
                    mvaddch(sy - 1, sx, '|');
                    mvaddch(sy + 1, sx, '|');
                    mvaddch(sy, sx - 1, '-');
                    mvaddch(sy, sx + 1, '-');
                }
            }
            attroff(COLOR_PAIR(GRID_COLOR_PAIR) | A_DIM);
            break;

        default:
            break;
    }
}

/* Global accessor functions */
TestMode *test_mode_get_global(void) {
    return g_test_mode;
}

void test_mode_set_global(TestMode *tm) {
    g_test_mode = tm;
}

/* Handle test mode specific key input */
int test_mode_handle_key(int ch, float cursor_x, float cursor_y) {
    TestMode *tm = g_test_mode;
    if (!tm || !tm->enabled) return 0;

    switch (ch) {
        case KEY_F(12):
        case 'd':
        case 'D':
            /* Toggle debug overlay */
            /* Note: 'd' conflicts with delete in normal mode,
               so only handle F12 if we want to be safe */
            if (ch == KEY_F(12)) {
                test_mode_toggle_overlay(tm);
                return 1;
            }
            break;

        case 'g':
            /* Cycle grid style (lowercase only to not conflict) */
            test_mode_cycle_grid_style(tm);
            return 1;

        case 'l':
        case 'L':
            /* Toggle event logging */
            test_mode_toggle_event_logging(tm, NULL);
            return 1;

        case ' ':
            /* Add marker at cursor position */
            test_mode_add_marker(tm, cursor_x, cursor_y);
            return 1;

        case 'c':
            /* Clear all markers (lowercase only) */
            test_mode_clear_markers(tm);
            return 1;

        case 'e':
            /* Toggle event overlay */
            tm->event_overlay = !tm->event_overlay;
            test_mode_log_event(tm, "Event overlay: %s",
                               tm->event_overlay ? "ON" : "OFF");
            return 1;

        default:
            break;
    }

    return 0;  /* Key not handled, pass through */
}
