#define _POSIX_C_SOURCE 200809L
#include "config.h"
#include "input_unified.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

/* Initialize config with sensible defaults */
void config_init_defaults(AppConfig *config) {
    if (!config) return;

    /* General */
    config->show_visualizer = true;
    config->auto_save = false;

    /* Grid */
    config->grid_visible_default = false;
    config->grid_snap_default = false;
    config->grid_spacing = 10;

    /* Joystick */
    config->joystick_deadzone = 0.15;
    config->joystick_settling_frames = 30;

    /* VIEW mode button mappings (defaults match current implementation) */
    strncpy(config->view_button_a, "zoom_in", sizeof(config->view_button_a) - 1);
    config->view_button_a[sizeof(config->view_button_a) - 1] = '\0';
    strncpy(config->view_button_b, "zoom_out", sizeof(config->view_button_b) - 1);
    config->view_button_b[sizeof(config->view_button_b) - 1] = '\0';
    strncpy(config->view_button_x, "create_box", sizeof(config->view_button_x) - 1);
    config->view_button_x[sizeof(config->view_button_x) - 1] = '\0';
    strncpy(config->view_button_y, "enter_select", sizeof(config->view_button_y) - 1);
    config->view_button_y[sizeof(config->view_button_y) - 1] = '\0';
    strncpy(config->view_button_lb, "toggle_grid", sizeof(config->view_button_lb) - 1);
    config->view_button_lb[sizeof(config->view_button_lb) - 1] = '\0';
    strncpy(config->view_button_rb, "toggle_snap", sizeof(config->view_button_rb) - 1);
    config->view_button_rb[sizeof(config->view_button_rb) - 1] = '\0';

    /* SELECT mode */
    strncpy(config->select_button_a, "cycle_box", sizeof(config->select_button_a) - 1);
    config->select_button_a[sizeof(config->select_button_a) - 1] = '\0';
    strncpy(config->select_button_b, "back_to_view", sizeof(config->select_button_b) - 1);
    config->select_button_b[sizeof(config->select_button_b) - 1] = '\0';
    strncpy(config->select_button_x, "enter_edit", sizeof(config->select_button_x) - 1);
    config->select_button_x[sizeof(config->select_button_x) - 1] = '\0';
    strncpy(config->select_button_y, "delete_box", sizeof(config->select_button_y) - 1);
    config->select_button_y[sizeof(config->select_button_y) - 1] = '\0';

    /* EDIT mode */
    strncpy(config->edit_button_a, "edit_text", sizeof(config->edit_button_a) - 1);
    config->edit_button_a[sizeof(config->edit_button_a) - 1] = '\0';
    strncpy(config->edit_button_b, "back_to_select", sizeof(config->edit_button_b) - 1);
    config->edit_button_b[sizeof(config->edit_button_b) - 1] = '\0';
    strncpy(config->edit_button_x, "cycle_color", sizeof(config->edit_button_x) - 1);
    config->edit_button_x[sizeof(config->edit_button_x) - 1] = '\0';
    strncpy(config->edit_button_y, "parameters", sizeof(config->edit_button_y) - 1);
    config->edit_button_y[sizeof(config->edit_button_y) - 1] = '\0';
    strncpy(config->edit_button_lb, "decrease", sizeof(config->edit_button_lb) - 1);
    config->edit_button_lb[sizeof(config->edit_button_lb) - 1] = '\0';
    strncpy(config->edit_button_rb, "increase", sizeof(config->edit_button_rb) - 1);
    config->edit_button_rb[sizeof(config->edit_button_rb) - 1] = '\0';

    /* Global buttons */
    strncpy(config->global_button_menu, "cycle_mode", sizeof(config->global_button_menu) - 1);
    config->global_button_menu[sizeof(config->global_button_menu) - 1] = '\0';
    strncpy(config->global_button_start, "save_canvas", sizeof(config->global_button_start) - 1);
    config->global_button_start[sizeof(config->global_button_start) - 1] = '\0';
    strncpy(config->global_button_select, "quit", sizeof(config->global_button_select) - 1);
    config->global_button_select[sizeof(config->global_button_select) - 1] = '\0';
    strncpy(config->global_button_back, "toggle_visualizer", sizeof(config->global_button_back) - 1);
    config->global_button_back[sizeof(config->global_button_back) - 1] = '\0';
}

/* Trim whitespace from string */
static char* trim_whitespace(char *str) {
    if (!str) return str;

    /* Trim leading */
    while (isspace(*str)) str++;

    /* Trim trailing */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

/* Parse a single config line (returns 0 on success) */
static int parse_config_line(AppConfig *config, const char *section, const char *line) {
    char key[64], value[256];
    char line_copy[512];

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    /* Skip comments and empty lines */
    char *trimmed = trim_whitespace(line_copy);
    if (trimmed[0] == '#' || trimmed[0] == ';' || trimmed[0] == '\0') {
        return 0;
    }

    /* Parse key = value */
    char *equals = strchr(trimmed, '=');
    if (!equals) return -1;

    *equals = '\0';
    char *key_str = trim_whitespace(trimmed);
    char *val_str = trim_whitespace(equals + 1);

    strncpy(key, key_str, sizeof(key) - 1);
    key[sizeof(key) - 1] = '\0';
    strncpy(value, val_str, sizeof(value) - 1);
    value[sizeof(value) - 1] = '\0';

    /* Apply setting based on section */
    if (strcmp(section, "general") == 0) {
        if (strcmp(key, "show_visualizer") == 0) {
            config->show_visualizer = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "auto_save") == 0) {
            config->auto_save = (strcmp(value, "true") == 0);
        }
    } else if (strcmp(section, "grid") == 0) {
        if (strcmp(key, "visible") == 0) {
            config->grid_visible_default = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "snap_enabled") == 0) {
            config->grid_snap_default = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "spacing") == 0) {
            config->grid_spacing = atoi(value);
        }
    } else if (strcmp(section, "joystick") == 0) {
        if (strcmp(key, "deadzone") == 0) {
            config->joystick_deadzone = atof(value);
        } else if (strcmp(key, "settling_frames") == 0) {
            config->joystick_settling_frames = atoi(value);
        }
    } else if (strcmp(section, "joystick.view") == 0) {
        if (strcmp(key, "button_a") == 0) {
            strncpy(config->view_button_a, value, sizeof(config->view_button_a) - 1);
            config->view_button_a[sizeof(config->view_button_a) - 1] = '\0';
        } else if (strcmp(key, "button_b") == 0) {
            strncpy(config->view_button_b, value, sizeof(config->view_button_b) - 1);
            config->view_button_b[sizeof(config->view_button_b) - 1] = '\0';
        } else if (strcmp(key, "button_x") == 0) {
            strncpy(config->view_button_x, value, sizeof(config->view_button_x) - 1);
            config->view_button_x[sizeof(config->view_button_x) - 1] = '\0';
        } else if (strcmp(key, "button_y") == 0) {
            strncpy(config->view_button_y, value, sizeof(config->view_button_y) - 1);
            config->view_button_y[sizeof(config->view_button_y) - 1] = '\0';
        } else if (strcmp(key, "button_lb") == 0) {
            strncpy(config->view_button_lb, value, sizeof(config->view_button_lb) - 1);
            config->view_button_lb[sizeof(config->view_button_lb) - 1] = '\0';
        } else if (strcmp(key, "button_rb") == 0) {
            strncpy(config->view_button_rb, value, sizeof(config->view_button_rb) - 1);
            config->view_button_rb[sizeof(config->view_button_rb) - 1] = '\0';
        }
    } else if (strcmp(section, "joystick.select") == 0) {
        if (strcmp(key, "button_a") == 0) {
            strncpy(config->select_button_a, value, sizeof(config->select_button_a) - 1);
            config->select_button_a[sizeof(config->select_button_a) - 1] = '\0';
        } else if (strcmp(key, "button_b") == 0) {
            strncpy(config->select_button_b, value, sizeof(config->select_button_b) - 1);
            config->select_button_b[sizeof(config->select_button_b) - 1] = '\0';
        } else if (strcmp(key, "button_x") == 0) {
            strncpy(config->select_button_x, value, sizeof(config->select_button_x) - 1);
            config->select_button_x[sizeof(config->select_button_x) - 1] = '\0';
        } else if (strcmp(key, "button_y") == 0) {
            strncpy(config->select_button_y, value, sizeof(config->select_button_y) - 1);
            config->select_button_y[sizeof(config->select_button_y) - 1] = '\0';
        }
    } else if (strcmp(section, "joystick.edit") == 0) {
        if (strcmp(key, "button_a") == 0) {
            strncpy(config->edit_button_a, value, sizeof(config->edit_button_a) - 1);
            config->edit_button_a[sizeof(config->edit_button_a) - 1] = '\0';
        } else if (strcmp(key, "button_b") == 0) {
            strncpy(config->edit_button_b, value, sizeof(config->edit_button_b) - 1);
            config->edit_button_b[sizeof(config->edit_button_b) - 1] = '\0';
        } else if (strcmp(key, "button_x") == 0) {
            strncpy(config->edit_button_x, value, sizeof(config->edit_button_x) - 1);
            config->edit_button_x[sizeof(config->edit_button_x) - 1] = '\0';
        } else if (strcmp(key, "button_y") == 0) {
            strncpy(config->edit_button_y, value, sizeof(config->edit_button_y) - 1);
            config->edit_button_y[sizeof(config->edit_button_y) - 1] = '\0';
        } else if (strcmp(key, "button_lb") == 0) {
            strncpy(config->edit_button_lb, value, sizeof(config->edit_button_lb) - 1);
            config->edit_button_lb[sizeof(config->edit_button_lb) - 1] = '\0';
        } else if (strcmp(key, "button_rb") == 0) {
            strncpy(config->edit_button_rb, value, sizeof(config->edit_button_rb) - 1);
            config->edit_button_rb[sizeof(config->edit_button_rb) - 1] = '\0';
        }
    }

    return 0;
}

/* Load config from file */
int config_load(AppConfig *config, const char *path) {
    if (!config || !path) return -1;

    /* Initialize with defaults first */
    config_init_defaults(config);

    FILE *f = fopen(path, "r");
    if (!f) {
        /* File doesn't exist - not an error, just use defaults */
        return 0;
    }

    char line[512];
    char current_section[64] = "";

    while (fgets(line, sizeof(line), f)) {
        char *trimmed = trim_whitespace(line);

        /* Check for section header [section] */
        if (trimmed[0] == '[') {
            char *end = strchr(trimmed, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, trimmed + 1, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0';
            }
            continue;
        }

        /* Parse key=value pair */
        parse_config_line(config, current_section, trimmed);
    }

    fclose(f);
    return 0;
}

/* Save config to file */
int config_save(const AppConfig *config, const char *path) {
    if (!config || !path) return -1;

    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "# Boxes-Live Configuration\n");
    fprintf(f, "# Edit this file to customize keybindings and settings\n\n");

    fprintf(f, "[general]\n");
    fprintf(f, "show_visualizer = %s\n", config->show_visualizer ? "true" : "false");
    fprintf(f, "auto_save = %s\n\n", config->auto_save ? "true" : "false");

    fprintf(f, "[grid]\n");
    fprintf(f, "visible = %s\n", config->grid_visible_default ? "true" : "false");
    fprintf(f, "snap_enabled = %s\n", config->grid_snap_default ? "true" : "false");
    fprintf(f, "spacing = %d\n\n", config->grid_spacing);

    fprintf(f, "[joystick]\n");
    fprintf(f, "deadzone = %.2f\n", config->joystick_deadzone);
    fprintf(f, "settling_frames = %d\n\n", config->joystick_settling_frames);

    fprintf(f, "[joystick.view]\n");
    fprintf(f, "button_a = %s\n", config->view_button_a);
    fprintf(f, "button_b = %s\n", config->view_button_b);
    fprintf(f, "button_x = %s\n", config->view_button_x);
    fprintf(f, "button_y = %s\n", config->view_button_y);
    fprintf(f, "button_lb = %s\n", config->view_button_lb);
    fprintf(f, "button_rb = %s\n\n", config->view_button_rb);

    fprintf(f, "[joystick.select]\n");
    fprintf(f, "button_a = %s\n", config->select_button_a);
    fprintf(f, "button_b = %s\n", config->select_button_b);
    fprintf(f, "button_x = %s\n", config->select_button_x);
    fprintf(f, "button_y = %s\n\n", config->select_button_y);

    fprintf(f, "[joystick.edit]\n");
    fprintf(f, "button_a = %s\n", config->edit_button_a);
    fprintf(f, "button_b = %s\n", config->edit_button_b);
    fprintf(f, "button_x = %s\n", config->edit_button_x);
    fprintf(f, "button_y = %s\n", config->edit_button_y);
    fprintf(f, "button_lb = %s\n", config->edit_button_lb);
    fprintf(f, "button_rb = %s\n\n", config->edit_button_rb);

    fprintf(f, "[global]\n");
    fprintf(f, "button_menu = %s\n", config->global_button_menu);
    fprintf(f, "button_start = %s\n", config->global_button_start);
    fprintf(f, "button_select = %s\n", config->global_button_select);
    fprintf(f, "button_back = %s\n", config->global_button_back);

    fclose(f);
    return 0;
}

/* Create default config file with documentation */
int config_create_default_file(const char *path) {
    if (!path) return -1;

    /* Ensure directory exists */
    char *dir_end = strrchr(path, '/');
    if (dir_end) {
        char dir_path[512];
        size_t dir_len = dir_end - path;
        if (dir_len < sizeof(dir_path)) {
            strncpy(dir_path, path, dir_len);
            dir_path[dir_len] = '\0';
            mkdir(dir_path, 0755);  /* Create if doesn't exist */
        }
    }

    AppConfig defaults;
    config_init_defaults(&defaults);
    return config_save(&defaults, path);
}

/* Get default config file path */
char* config_get_default_path(void) {
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";

    const char *config_home = getenv("XDG_CONFIG_HOME");
    char *path = malloc(512);
    if (!path) return NULL;

    if (config_home) {
        snprintf(path, 512, "%s/boxes-live/config.ini", config_home);
    } else {
        snprintf(path, 512, "%s/.config/boxes-live/config.ini", home);
    }

    return path;
}

/* Map action name string to CanvasAction enum */
int config_action_from_name(const char *name) {
    if (!name) return ACTION_NONE;

    /* Navigation */
    if (strcmp(name, "zoom_in") == 0) return ACTION_ZOOM_IN;
    if (strcmp(name, "zoom_out") == 0) return ACTION_ZOOM_OUT;
    if (strcmp(name, "reset_view") == 0) return ACTION_RESET_VIEW;

    /* Box management */
    if (strcmp(name, "create_box") == 0) return ACTION_CREATE_BOX;
    if (strcmp(name, "delete_box") == 0) return ACTION_DELETE_BOX;
    if (strcmp(name, "cycle_box") == 0) return ACTION_CYCLE_BOX;

    /* Mode transitions */
    if (strcmp(name, "enter_select") == 0) return ACTION_ENTER_EDIT_MODE;  /* Will be mapped to mode change */
    if (strcmp(name, "enter_edit") == 0) return ACTION_ENTER_EDIT_MODE;
    if (strcmp(name, "back_to_view") == 0) return ACTION_ENTER_NAV_MODE;
    if (strcmp(name, "back_to_select") == 0) return ACTION_ENTER_NAV_MODE;

    /* Editing */
    if (strcmp(name, "edit_text") == 0) return ACTION_ENTER_EDIT_MODE;  /* Triggers text editor */
    if (strcmp(name, "cycle_color") == 0) return ACTION_COLOR_BOX;
    if (strcmp(name, "parameters") == 0) return ACTION_ENTER_PARAM_MODE;

    /* Grid */
    if (strcmp(name, "toggle_grid") == 0) return ACTION_TOGGLE_GRID;
    if (strcmp(name, "toggle_snap") == 0) return ACTION_TOGGLE_SNAP;

    /* File operations */
    if (strcmp(name, "save_canvas") == 0) return ACTION_SAVE_CANVAS;
    if (strcmp(name, "load_canvas") == 0) return ACTION_LOAD_CANVAS;

    /* Application */
    if (strcmp(name, "quit") == 0) return ACTION_QUIT;

    /* Special actions */
    if (strcmp(name, "cycle_mode") == 0) return -2;  /* Special: mode cycling */
    if (strcmp(name, "toggle_visualizer") == 0) return -3;  /* Special: visualizer toggle */
    if (strcmp(name, "decrease") == 0) return -4;  /* Special: context-dependent */
    if (strcmp(name, "increase") == 0) return -5;  /* Special: context-dependent */

    return ACTION_NONE;
}
