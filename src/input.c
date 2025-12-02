#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "input_unified.h"
#include "viewport.h"
#include "canvas.h"
#include "persistence.h"
#include "joystick.h"
#include "config.h"
#include "export.h"
#include "file_viewer.h"

/* Zoom factor per key press */
#define ZOOM_FACTOR 1.2

/* Default save file */
#define DEFAULT_SAVE_FILE "canvas.txt"

/* Default export file */
#define DEFAULT_EXPORT_FILE "canvas-export.txt"

/* Pan speed for joystick input (scaled by zoom) */
#define PAN_SPEED 2.0

/* Forward declarations for joystick mode handlers (legacy, kept for parameter mode) */
/* Removed in Phase 1 - will be reimplemented in Phase 2 */
/* static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y); */

/* Helper function to execute canvas actions */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event, const AppConfig *config);

/* Helper function to execute command line commands (Issue #55) */
static void execute_command(Canvas *canvas);

int handle_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config) {
    int ch = getch();

    if (ch == ERR) {
        /* No input available */
        return 0;
    }

    /* Help overlay visible - any key dismisses it (including F1) (Issue #34) */
    if (canvas->help.visible) {
        canvas->help.visible = false;
        return 0;
    }

    /* Command line active - handle command input (Issue #55) */
    if (canvas->command_line.active) {
        /* Clear any previous error on new input */
        canvas->command_line.has_error = false;

        if (ch == 27) {  /* ESC - cancel command */
            canvas->command_line.active = false;
            canvas->command_line.buffer[0] = '\0';
            canvas->command_line.length = 0;
            canvas->command_line.cursor_pos = 0;
            return 0;
        } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {  /* Enter - execute command */
            execute_command(canvas);
            canvas->command_line.active = false;
            canvas->command_line.buffer[0] = '\0';
            canvas->command_line.length = 0;
            canvas->command_line.cursor_pos = 0;
            return 0;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {  /* Backspace */
            if (canvas->command_line.cursor_pos > 0) {
                /* Shift characters left */
                for (int i = canvas->command_line.cursor_pos - 1; i < canvas->command_line.length; i++) {
                    canvas->command_line.buffer[i] = canvas->command_line.buffer[i + 1];
                }
                canvas->command_line.cursor_pos--;
                canvas->command_line.length--;
            }
            return 0;
        } else if (ch == KEY_LEFT) {
            if (canvas->command_line.cursor_pos > 0) {
                canvas->command_line.cursor_pos--;
            }
            return 0;
        } else if (ch == KEY_RIGHT) {
            if (canvas->command_line.cursor_pos < canvas->command_line.length) {
                canvas->command_line.cursor_pos++;
            }
            return 0;
        } else if (ch == KEY_HOME) {
            canvas->command_line.cursor_pos = 0;
            return 0;
        } else if (ch == KEY_END) {
            canvas->command_line.cursor_pos = canvas->command_line.length;
            return 0;
        } else if (ch >= 32 && ch < 127) {  /* Printable character */
            if (canvas->command_line.length < COMMAND_BUFFER_SIZE - 1) {
                /* Shift characters right to make room */
                for (int i = canvas->command_line.length; i > canvas->command_line.cursor_pos; i--) {
                    canvas->command_line.buffer[i] = canvas->command_line.buffer[i - 1];
                }
                canvas->command_line.buffer[canvas->command_line.cursor_pos] = (char)ch;
                canvas->command_line.cursor_pos++;
                canvas->command_line.length++;
                canvas->command_line.buffer[canvas->command_line.length] = '\0';
            }
            return 0;
        }
        /* Ignore other keys in command mode */
        return 0;
    }

    /* Check for ':' to enter command mode (Issue #55) */
    if (ch == ':') {
        canvas->command_line.active = true;
        canvas->command_line.buffer[0] = '\0';
        canvas->command_line.length = 0;
        canvas->command_line.cursor_pos = 0;
        canvas->command_line.has_error = false;
        return 0;
    }

    /* Focus mode active - handle focus mode input (Phase 5b) */
    if (canvas->focus.active) {
        Box *box = canvas_get_box(canvas, canvas->focus.focused_box_id);

        if (ch == 27 || ch == 'q') {  /* ESC or q */
            /* Exit focus mode */
            canvas->focus.active = false;
            canvas->focus.focused_box_id = -1;
            return 0;
        } else if (ch == 'j' || ch == KEY_DOWN) {
            /* Scroll down */
            if (canvas->focus.scroll_offset < canvas->focus.scroll_max) {
                canvas->focus.scroll_offset++;
            }
            return 0;
        } else if (ch == 'k' || ch == KEY_UP) {
            /* Scroll up */
            if (canvas->focus.scroll_offset > 0) {
                canvas->focus.scroll_offset--;
            }
            return 0;
        } else if (ch == 'g') {
            /* Jump to top */
            canvas->focus.scroll_offset = 0;
            return 0;
        } else if (ch == 'G') {
            /* Jump to bottom */
            canvas->focus.scroll_offset = canvas->focus.scroll_max;
            return 0;
        } else if (ch == KEY_NPAGE) {  /* Page Down */
            canvas->focus.scroll_offset += (LINES - 4) / 2;
            if (canvas->focus.scroll_offset > canvas->focus.scroll_max) {
                canvas->focus.scroll_offset = canvas->focus.scroll_max;
            }
            return 0;
        } else if (ch == KEY_PPAGE) {  /* Page Up */
            canvas->focus.scroll_offset -= (LINES - 4) / 2;
            if (canvas->focus.scroll_offset < 0) {
                canvas->focus.scroll_offset = 0;
            }
            return 0;
        }

        /* Ignore other keys in focus mode */
        (void)box;  /* Suppress unused warning */
        return 0;
    }

    /* Text editor active - handle keyboard input for text editing */
    if (js && js->text_editor_active) {
        Box *box = canvas_get_box(canvas, js->selected_box_id);

        if (ch == 27 || ch == KEY_F(10)) {  /* ESC or F10 */
            /* Save and close editor */
            joystick_close_text_editor(js, true, box);
            return 0;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            /* Backspace */
            joystick_text_editor_backspace(js);
            return 0;
        } else if (ch == KEY_LEFT) {
            /* Move cursor left */
            joystick_text_editor_move_cursor(js, -1);
            return 0;
        } else if (ch == KEY_RIGHT) {
            /* Move cursor right */
            joystick_text_editor_move_cursor(js, 1);
            return 0;
        } else if (ch == KEY_HOME) {
            /* Move to start */
            if (js->text_edit_buffer) {
                js->text_cursor_pos = 0;
            }
            return 0;
        } else if (ch == KEY_END) {
            /* Move to end */
            if (js->text_edit_buffer) {
                js->text_cursor_pos = strlen(js->text_edit_buffer);
            }
            return 0;
        } else if (ch >= 32 && ch < 127) {
            /* Printable character */
            joystick_text_editor_insert_char(js, (char)ch);
            return 0;
        }

        /* Ignore other keys while in text editor */
        return 0;
    }

    InputEvent event;
    int source = -1;

    /* Handle mouse events */
    if (ch == KEY_MOUSE) {
        MEVENT mouse_event;
        #ifdef _WIN32
        /* PDCurses mouse handling - API differs from ncurses */
        /* Clear the mouse event queue; ignore errors as we just want to flush */
        (void)getmouse();
        #ifdef PDC_WIDE
        /* PDC_WIDE builds have nc_getmouse() for MEVENT support */
        if (nc_getmouse(&mouse_event) == OK) {
        #else
        /* Non-PDC_WIDE builds: mouse support disabled due to MEVENT API differences.
         * This is intentional - older PDCurses versions lack full mouse support.
         * Mouse will work on Unix/Linux and PDC_WIDE Windows builds. */
        memset(&mouse_event, 0, sizeof(mouse_event));  /* Suppress uninitialized warning */
        if (false) {  /* Mouse handling disabled on non-PDC_WIDE Windows */
        #endif
        #else
        /* ncurses API: getmouse() takes pointer to MEVENT */
        if (getmouse(&mouse_event) == OK) {
        #endif
            source = input_unified_process_mouse(&mouse_event, canvas, vp, &event);
        }
    } else {
        /* Handle keyboard input */
        source = input_unified_process_keyboard(ch, vp, &event);
    }

    /* Execute the action if one was generated */
    if (source >= 0 && event.action != ACTION_NONE) {
        return execute_canvas_action(canvas, vp, js, &event, config);
    }

    return 0;
}

/* Handle joystick input based on current mode */
int handle_joystick_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config) {
    if (!canvas || !vp || !js || !js->available) {
        return 0;
    }

    /* Parameter mode handling removed - now part of unified input layer */

    /* Use unified input layer for navigation and edit modes */
    InputEvent event;
    int source = input_unified_process_joystick(js, canvas, vp, &event);

    if (source >= 0 && event.action != ACTION_NONE) {
        return execute_canvas_action(canvas, vp, js, &event, config);
    }

    return 0;
}

/* Execute canvas action from unified input event */
static int execute_canvas_action(Canvas *canvas, Viewport *vp, JoystickState *js,
                                  const InputEvent *event, const AppConfig *config) {
    if (!canvas || !vp || !event) return 0;

    switch (event->action) {
        case ACTION_QUIT:
            /* If in connection mode, cancel instead of quit (Issue #20) */
            if (canvas_in_connection_mode(canvas)) {
                canvas_cancel_connection(canvas);
                return 0;
            }
            return 1;

        case ACTION_PAN_UP:
        case ACTION_PAN_DOWN:
        case ACTION_PAN_LEFT:
        case ACTION_PAN_RIGHT:
            /* Handle panning */
            if (event->data.pan.continuous) {
                /* Continuous analog input (joystick) - scale by zoom */
                double scaled_speed = PAN_SPEED / vp->zoom;
                viewport_pan(vp, event->data.pan.dx * scaled_speed,
                            event->data.pan.dy * scaled_speed);
                
                /* Update joystick cursor if in NAV mode */
                if (js && js->mode == MODE_NAV) {
                    js->cursor_x = vp->cam_x + (vp->term_width / 2.0) / vp->zoom;
                    js->cursor_y = vp->cam_y + (vp->term_height / 2.0) / vp->zoom;
                }
            } else {
                /* Discrete input (keyboard) */
                viewport_pan(vp, event->data.pan.dx, event->data.pan.dy);
            }
            break;

        case ACTION_ZOOM_IN:
            viewport_zoom(vp, ZOOM_FACTOR);
            break;

        case ACTION_ZOOM_OUT:
            viewport_zoom(vp, 1.0 / ZOOM_FACTOR);
            break;

        case ACTION_SELECT_BOX:
            canvas_select_box(canvas, event->data.box.box_id);
            break;

        case ACTION_DESELECT_BOX:
            canvas_deselect(canvas);
            break;

        case ACTION_CYCLE_BOX:
            if (canvas->box_count > 0) {
                int next_index = (canvas->selected_index + 1) % canvas->box_count;
                Box *next_box = canvas_get_box_at(canvas, next_index);
                if (next_box) {
                    canvas_select_box(canvas, next_box->id);
                    
                    /* If joystick, enter edit mode */
                    if (js) {
                        joystick_enter_edit_mode(js, next_box->id);
                    }
                }
            }
            break;

        case ACTION_CREATE_BOX: {
            /* Get template dimensions (Issue #17) */
            int box_width = 25;
            int box_height = 6;
            const char *template_name = "New Box";

            if (config) {
                config_get_template_dimensions(config, event->data.box.template,
                                               &box_width, &box_height);
                template_name = config_get_template_name(event->data.box.template);

                /* Apply proportional sizing if enabled (Issue #18) */
                if (config->proportional_sizing) {
                    int prop_width, prop_height;
                    int neighbors = canvas_calc_proportional_size(
                        canvas,
                        event->data.box.world_x,
                        event->data.box.world_y,
                        config->proximity_radius,
                        config->use_nearest_neighbor,
                        config->min_neighbors_required,
                        box_width, box_height,
                        &prop_width, &prop_height);

                    if (neighbors > 0) {
                        box_width = prop_width;
                        box_height = prop_height;
                    }
                }
            }

            /* Create box at specified position with calculated dimensions */
            int box_id = canvas_add_box(canvas, event->data.box.world_x,
                                       event->data.box.world_y, box_width, box_height,
                                       template_name);
            if (box_id >= 0) {
                const char *content[] = {
                    "Click to select",
                    "Delete (Ctrl+D)"
                };
                canvas_add_box_content(canvas, box_id, content, 2);
                canvas_select_box(canvas, box_id);

                /* If joystick, enter edit mode */
                if (js) {
                    joystick_enter_edit_mode(js, box_id);
                }
            }
            break;
        }

        case ACTION_DELETE_BOX: {
            Box *selected = canvas_get_selected(canvas);
            if (selected) {
                int selected_id = selected->id;
                canvas_remove_box(canvas, selected_id);
                canvas_deselect(canvas);
                
                /* If joystick, return to NAV mode */
                if (js) {
                    joystick_enter_nav_mode(js);
                }
            }
            break;
        }

        case ACTION_MOVE_BOX: {
            /* Mouse drag or joystick movement */
            if (event->data.move.box_id >= 0) {
                Box *box = canvas_get_box(canvas, event->data.move.box_id);
                if (box) {
                    if (js && js->mode == MODE_EDIT) {
                        /* Joystick - relative movement (analog) */
                        double scaled_speed = PAN_SPEED / vp->zoom;
                        box->x += event->data.move.world_x * scaled_speed;
                        box->y += event->data.move.world_y * scaled_speed;
                        
                        /* Update cursor to box position */
                        js->cursor_x = box->x;
                        js->cursor_y = box->y;
                    } else {
                        /* Mouse - absolute position with offset */
                        box->x = event->data.move.world_x - event->data.move.offset_x;
                        box->y = event->data.move.world_y - event->data.move.offset_y;
                    }
                }
            }
            break;
        }

        case ACTION_COLOR_BOX:
            if (canvas->selected_index >= 0) {
                if (event->data.color.color_index == -1) {
                    /* Cycle color (joystick) */
                    canvas->boxes[canvas->selected_index].color =
                        (canvas->boxes[canvas->selected_index].color + 1) % 8;
                } else {
                    /* Set specific color */
                    canvas->boxes[canvas->selected_index].color = event->data.color.color_index;
                }
            } else if (event->data.color.color_index == 0) {
                /* No box selected, reset view */
                vp->cam_x = 0.0;
                vp->cam_y = 0.0;
                vp->zoom = 1.0;
            }
            break;

        case ACTION_CYCLE_BOX_TYPE:
            /* Cycle box type (Issue #33) */
            if (canvas->selected_index >= 0) {
                Box *box = &canvas->boxes[canvas->selected_index];
                box->box_type = (box->box_type + 1) % BOX_TYPE_COUNT;
            }
            break;

        case ACTION_RESET_VIEW:
            vp->cam_x = 0.0;
            vp->cam_y = 0.0;
            vp->zoom = 1.0;
            break;

        case ACTION_TOGGLE_GRID:
            canvas->grid.visible = !canvas->grid.visible;
            break;

        case ACTION_TOGGLE_SNAP:
            canvas->grid.snap_enabled = !canvas->grid.snap_enabled;
            break;

        /* Sidebar actions (Issue #35) */
        case ACTION_TOGGLE_SIDEBAR:
            /* Cycle through sidebar states: hidden -> collapsed -> expanded -> hidden */
            canvas->sidebar_state = (canvas->sidebar_state + 1) % (SIDEBAR_EXPANDED + 1);
            break;

        case ACTION_WIDEN_SIDEBAR:
            /* Increase sidebar width */
            if (canvas->sidebar_state != SIDEBAR_HIDDEN) {
                canvas->sidebar_width += 5;
                if (canvas->sidebar_width > 40) {
                    canvas->sidebar_width = 40;
                }
            }
            break;

        case ACTION_NARROW_SIDEBAR:
            /* Decrease sidebar width */
            if (canvas->sidebar_state != SIDEBAR_HIDDEN) {
                canvas->sidebar_width -= 5;
                if (canvas->sidebar_width < 20) {
                    canvas->sidebar_width = 20;
                }
            }
            break;

        case ACTION_EDIT_SIDEBAR:
            /* TODO: Implement sidebar editing - for now just expand sidebar */
            if (canvas->sidebar_state != SIDEBAR_HIDDEN) {
                canvas->sidebar_state = SIDEBAR_EXPANDED;
            }
            break;

        case ACTION_CYCLE_DISPLAY_MODE:
            /* Cycle through display modes: Full -> Compact -> Preview -> Full (Issue #33) */
            if (canvas->display_mode == DISPLAY_MODE_FULL) {
                canvas->display_mode = DISPLAY_MODE_COMPACT;
            } else if (canvas->display_mode == DISPLAY_MODE_COMPACT) {
                canvas->display_mode = DISPLAY_MODE_PREVIEW;
            } else {
                canvas->display_mode = DISPLAY_MODE_FULL;
            }
            break;

        case ACTION_FOCUS_BOX:
            /* Enter focus mode on selected box (Phase 5b) */
            if (canvas->selected_index >= 0) {
                Box *box = canvas_get_box_at(canvas, canvas->selected_index);
                if (box) {
                    canvas->focus.active = true;
                    canvas->focus.focused_box_id = box->id;
                    canvas->focus.scroll_offset = 0;
                    canvas->focus.scroll_max = 0;  /* Will be calculated in render */
                }
            }
            break;

        case ACTION_EXIT_FOCUS:
            /* Exit focus mode (Phase 5b) */
            canvas->focus.active = false;
            canvas->focus.focused_box_id = -1;
            break;

        case ACTION_SAVE_CANVAS:
            canvas_save(canvas, DEFAULT_SAVE_FILE);
            break;

        case ACTION_LOAD_CANVAS: {
            const char *file_to_load = persistence_get_current_file();
            if (file_to_load == NULL) {
                file_to_load = DEFAULT_SAVE_FILE;
            }

            Canvas old_canvas = *canvas;
            if (canvas_load(canvas, file_to_load) != 0) {
                *canvas = old_canvas;
            } else {
                canvas_cleanup(&old_canvas);
            }
            break;
        }

        case ACTION_EXPORT_CANVAS:
            export_viewport_to_file(canvas, vp, DEFAULT_EXPORT_FILE);
            break;

        case ACTION_TOGGLE_HELP:
            /* Toggle help overlay visibility (Issue #34) */
            canvas->help.visible = !canvas->help.visible;
            break;

        case ACTION_ENTER_EDIT_MODE:
            if (js && js->selected_box_id >= 0) {
                joystick_enter_edit_mode(js, js->selected_box_id);
            }
            break;

        case ACTION_ENTER_PARAM_MODE:
            /* TODO: Implement parameter panel in Phase 2 */
            /* For now, parameter editing is done directly in EDIT mode */
            break;

        case ACTION_ENTER_NAV_MODE:
            if (js) {
                joystick_enter_nav_mode(js);
                canvas_deselect(canvas);
            }
            break;

        /* Connection actions (Issue #20) */
        case ACTION_START_CONNECTION:
            /* 'c' key: Start connection from selected box, or finish if already in connection mode */
            if (canvas_in_connection_mode(canvas)) {
                /* Already in connection mode - finish connection to currently selected box */
                Box *selected = canvas_get_selected(canvas);
                if (selected) {
                    canvas_finish_connection(canvas, selected->id);
                } else {
                    /* No destination selected - cancel */
                    canvas_cancel_connection(canvas);
                }
            } else {
                /* Not in connection mode - start from selected box */
                Box *selected = canvas_get_selected(canvas);
                if (selected) {
                    canvas_start_connection(canvas, selected->id);
                }
            }
            break;

        case ACTION_FINISH_CONNECTION: {
            /* Finish connection to the specified box */
            Box *selected = canvas_get_selected(canvas);
            if (selected && canvas_in_connection_mode(canvas)) {
                canvas_finish_connection(canvas, selected->id);
            }
            break;
        }

        case ACTION_CANCEL_CONNECTION:
            /* Cancel connection mode */
            canvas_cancel_connection(canvas);
            break;

        case ACTION_DELETE_CONNECTION:
            /* Delete connection - handled by D key logic below */
            break;

        default:
            break;
    }

    return 0;
}

/* Parameter mode: Removed in Phase 1 - will be reimplemented in Phase 2 */
/*
static int handle_joystick_parameter(Canvas *canvas, Viewport *vp, JoystickState *js,
                                      double axis_x, double axis_y) {
    (void)canvas; (void)vp; (void)js; (void)axis_x; (void)axis_y;
    return 0;
}
*/

/* Execute command line command (Issue #55) */
static void execute_command(Canvas *canvas) {
    if (!canvas || canvas->command_line.length == 0) {
        return;
    }

    const char *cmd = canvas->command_line.buffer;

    /* Parse command - skip leading whitespace */
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    /* :file <path> - Load file into selected box */
    if (strncmp(cmd, "file ", 5) == 0) {
        const char *path = cmd + 5;
        /* Skip whitespace after command */
        while (*path == ' ' || *path == '\t') path++;

        if (*path == '\0') {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "Usage: :file <path>");
            canvas->command_line.has_error = true;
            return;
        }

        /* Need a selected box */
        Box *box = canvas_get_selected(canvas);
        if (!box) {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "No box selected");
            canvas->command_line.has_error = true;
            return;
        }

        /* Load file into box */
        if (file_viewer_load(box, path) != 0) {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "Cannot read file: %s", path);
            canvas->command_line.has_error = true;
            return;
        }

        /* Update box title to show filename */
        const char *basename = file_viewer_basename(path);
        if (basename && box->title) {
            free(box->title);
            box->title = strdup(basename);
        }

        return;
    }

    /* :reload - Reload file content for selected box */
    if (strcmp(cmd, "reload") == 0) {
        Box *box = canvas_get_selected(canvas);
        if (!box) {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "No box selected");
            canvas->command_line.has_error = true;
            return;
        }

        if (box->content_type != BOX_CONTENT_FILE) {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "Box is not a file viewer");
            canvas->command_line.has_error = true;
            return;
        }

        if (file_viewer_reload(box) != 0) {
            snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
                     "Cannot reload file");
            canvas->command_line.has_error = true;
            return;
        }

        return;
    }

    /* :q or :quit - quit application */
    if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
        /* Set a flag or directly exit - for now just close command mode */
        /* TODO: Implement proper quit */
        return;
    }

    /* Unknown command */
    snprintf(canvas->command_line.error_msg, COMMAND_BUFFER_SIZE,
             "Unknown command: %s", cmd);
    canvas->command_line.has_error = true;
}
