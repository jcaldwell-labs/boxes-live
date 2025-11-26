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

    /* Box templates (Issue #17) */
    config->template_square_width = 20;
    config->template_square_height = 10;
    config->template_horizontal_width = 40;
    config->template_horizontal_height = 10;
    config->template_vertical_width = 20;
    config->template_vertical_height = 20;

    /* Proportional sizing (Issue #18) */
    config->proportional_sizing = true;         /* Enabled by default */
    config->proximity_radius = 30;              /* 30 world units */
    config->use_nearest_neighbor = false;       /* Use average by default */
    config->min_neighbors_required = 1;         /* At least 1 neighbor */

    /* Grid */
    config->grid_visible_default = false;
    config->grid_snap_default = false;
    config->grid_spacing = 10;

    /* Box type icons (Issue #33) - using Unicode characters */
    strncpy(config->icon_note, "📝", sizeof(config->icon_note) - 1);
    config->icon_note[sizeof(config->icon_note) - 1] = '\0';
    strncpy(config->icon_task, "☑", sizeof(config->icon_task) - 1);
    config->icon_task[sizeof(config->icon_task) - 1] = '\0';
    strncpy(config->icon_code, "💻", sizeof(config->icon_code) - 1);
    config->icon_code[sizeof(config->icon_code) - 1] = '\0';
    strncpy(config->icon_sticky, "📌", sizeof(config->icon_sticky) - 1);
    config->icon_sticky[sizeof(config->icon_sticky) - 1] = '\0';

    /* Joystick */
    config->joystick_deadzone = 0.15;
    config->joystick_settling_frames = 30;

    /* NAV mode button mappings (defaults match current implementation) */
    strncpy(config->view_button_a, "zoom_in", sizeof(config->view_button_a) - 1);
    config->view_button_a[sizeof(config->view_button_a) - 1] = '\0';
    strncpy(config->view_button_b, "zoom_out", sizeof(config->view_button_b) - 1);
    config->view_button_b[sizeof(config->view_button_b) - 1] = '\0';
    strncpy(config->view_button_x, "create_box", sizeof(config->view_button_x) - 1);
    config->view_button_x[sizeof(config->view_button_x) - 1] = '\0';
    strncpy(config->view_button_y, "toggle_grid", sizeof(config->view_button_y) - 1);
    config->view_button_y[sizeof(config->view_button_y) - 1] = '\0';
    config->view_button_lb[0] = '\0';  /* LB is reserved as global mode toggle */
    strncpy(config->view_button_rb, "toggle_snap", sizeof(config->view_button_rb) - 1);
    config->view_button_rb[sizeof(config->view_button_rb) - 1] = '\0';

    /* SELECTION mode */
    strncpy(config->select_button_a, "cycle_box", sizeof(config->select_button_a) - 1);
    config->select_button_a[sizeof(config->select_button_a) - 1] = '\0';
    strncpy(config->select_button_b, "back_to_nav", sizeof(config->select_button_b) - 1);
    config->select_button_b[sizeof(config->select_button_b) - 1] = '\0';
    strncpy(config->select_button_x, "enter_edit", sizeof(config->select_button_x) - 1);
    config->select_button_x[sizeof(config->select_button_x) - 1] = '\0';
    strncpy(config->select_button_y, "delete_box", sizeof(config->select_button_y) - 1);
    config->select_button_y[sizeof(config->select_button_y) - 1] = '\0';

    /* EDIT mode */
    strncpy(config->edit_button_a, "edit_text", sizeof(config->edit_button_a) - 1);
    config->edit_button_a[sizeof(config->edit_button_a) - 1] = '\0';
    strncpy(config->edit_button_b, "back_to_selection", sizeof(config->edit_button_b) - 1);
    config->edit_button_b[sizeof(config->edit_button_b) - 1] = '\0';
    strncpy(config->edit_button_x, "cycle_color", sizeof(config->edit_button_x) - 1);
    config->edit_button_x[sizeof(config->edit_button_x) - 1] = '\0';
    strncpy(config->edit_button_y, "parameters", sizeof(config->edit_button_y) - 1);
    config->edit_button_y[sizeof(config->edit_button_y) - 1] = '\0';
    config->edit_button_lb[0] = '\0';  /* LB is reserved as global mode toggle */
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
    } else if (strcmp(section, "templates") == 0) {
        /* Box template settings (Issue #17) */
        if (strcmp(key, "square_width") == 0) {
            config->template_square_width = atoi(value);
            if (config->template_square_width < 10) config->template_square_width = 10;
            if (config->template_square_width > 80) config->template_square_width = 80;
        } else if (strcmp(key, "square_height") == 0) {
            config->template_square_height = atoi(value);
            if (config->template_square_height < 3) config->template_square_height = 3;
            if (config->template_square_height > 30) config->template_square_height = 30;
        } else if (strcmp(key, "horizontal_width") == 0) {
            config->template_horizontal_width = atoi(value);
            if (config->template_horizontal_width < 10) config->template_horizontal_width = 10;
            if (config->template_horizontal_width > 80) config->template_horizontal_width = 80;
        } else if (strcmp(key, "horizontal_height") == 0) {
            config->template_horizontal_height = atoi(value);
            if (config->template_horizontal_height < 3) config->template_horizontal_height = 3;
            if (config->template_horizontal_height > 30) config->template_horizontal_height = 30;
        } else if (strcmp(key, "vertical_width") == 0) {
            config->template_vertical_width = atoi(value);
            if (config->template_vertical_width < 10) config->template_vertical_width = 10;
            if (config->template_vertical_width > 80) config->template_vertical_width = 80;
        } else if (strcmp(key, "vertical_height") == 0) {
            config->template_vertical_height = atoi(value);
            if (config->template_vertical_height < 3) config->template_vertical_height = 3;
            if (config->template_vertical_height > 30) config->template_vertical_height = 30;
        }
    } else if (strcmp(section, "proportional") == 0) {
        /* Proportional sizing settings (Issue #18) */
        if (strcmp(key, "enabled") == 0) {
            config->proportional_sizing = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "proximity_radius") == 0) {
            config->proximity_radius = atoi(value);
            if (config->proximity_radius < 5) config->proximity_radius = 5;
            if (config->proximity_radius > 200) config->proximity_radius = 200;
        } else if (strcmp(key, "use_nearest_neighbor") == 0) {
            config->use_nearest_neighbor = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "min_neighbors_required") == 0) {
            config->min_neighbors_required = atoi(value);
            if (config->min_neighbors_required < 1) config->min_neighbors_required = 1;
            if (config->min_neighbors_required > 10) config->min_neighbors_required = 10;
        }
    } else if (strcmp(section, "icons") == 0) {
        /* Box type icons (Issue #33) */
        if (strcmp(key, "note") == 0) {
            strncpy(config->icon_note, value, sizeof(config->icon_note) - 1);
            config->icon_note[sizeof(config->icon_note) - 1] = '\0';
        } else if (strcmp(key, "task") == 0) {
            strncpy(config->icon_task, value, sizeof(config->icon_task) - 1);
            config->icon_task[sizeof(config->icon_task) - 1] = '\0';
        } else if (strcmp(key, "code") == 0) {
            strncpy(config->icon_code, value, sizeof(config->icon_code) - 1);
            config->icon_code[sizeof(config->icon_code) - 1] = '\0';
        } else if (strcmp(key, "sticky") == 0) {
            strncpy(config->icon_sticky, value, sizeof(config->icon_sticky) - 1);
            config->icon_sticky[sizeof(config->icon_sticky) - 1] = '\0';
        }
    } else if (strcmp(section, "joystick") == 0) {
        if (strcmp(key, "deadzone") == 0) {
            config->joystick_deadzone = atof(value);
        } else if (strcmp(key, "settling_frames") == 0) {
            config->joystick_settling_frames = atoi(value);
        }
    } else if (strcmp(section, "joystick.nav") == 0) {
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
            /* LB is global mode toggle - ignore config, but don't error */
        } else if (strcmp(key, "button_rb") == 0) {
            strncpy(config->view_button_rb, value, sizeof(config->view_button_rb) - 1);
            config->view_button_rb[sizeof(config->view_button_rb) - 1] = '\0';
        }
    } else if (strcmp(section, "joystick.selection") == 0) {
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

    fprintf(f, "[icons]\n");
    fprintf(f, "# Icons for different box types (Issue #33)\n");
    fprintf(f, "note = %s\n", config->icon_note);
    fprintf(f, "task = %s\n", config->icon_task);
    fprintf(f, "code = %s\n", config->icon_code);
    fprintf(f, "sticky = %s\n\n", config->icon_sticky);

    fprintf(f, "[templates]\n");
    fprintf(f, "# Square template (n key, joystick X button)\n");
    fprintf(f, "square_width = %d\n", config->template_square_width);
    fprintf(f, "square_height = %d\n", config->template_square_height);
    fprintf(f, "# Horizontal rectangle (Shift+N, joystick LB+X)\n");
    fprintf(f, "horizontal_width = %d\n", config->template_horizontal_width);
    fprintf(f, "horizontal_height = %d\n", config->template_horizontal_height);
    fprintf(f, "# Vertical rectangle (Ctrl+N, joystick RB+X)\n");
    fprintf(f, "vertical_width = %d\n", config->template_vertical_width);
    fprintf(f, "vertical_height = %d\n\n", config->template_vertical_height);

    fprintf(f, "[proportional]\n");
    fprintf(f, "# Enable proportional sizing based on nearby boxes (Issue #18)\n");
    fprintf(f, "enabled = %s\n", config->proportional_sizing ? "true" : "false");
    fprintf(f, "# Search radius in world units to find neighbor boxes\n");
    fprintf(f, "proximity_radius = %d\n", config->proximity_radius);
    fprintf(f, "# true = use nearest box dimensions, false = average of all neighbors\n");
    fprintf(f, "use_nearest_neighbor = %s\n", config->use_nearest_neighbor ? "true" : "false");
    fprintf(f, "# Minimum neighbors required to trigger proportional sizing\n");
    fprintf(f, "min_neighbors_required = %d\n\n", config->min_neighbors_required);

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
    if (strcmp(name, "enter_select") == 0) return ACTION_ENTER_EDIT_MODE;  /* Legacy name */
    if (strcmp(name, "enter_selection") == 0) return ACTION_ENTER_EDIT_MODE;  /* New name */
    if (strcmp(name, "enter_edit") == 0) return ACTION_ENTER_EDIT_MODE;
    if (strcmp(name, "back_to_view") == 0) return ACTION_ENTER_NAV_MODE;  /* Legacy name */
    if (strcmp(name, "back_to_nav") == 0) return ACTION_ENTER_NAV_MODE;  /* New name */
    if (strcmp(name, "back_to_select") == 0) return ACTION_ENTER_NAV_MODE;  /* Legacy name */
    if (strcmp(name, "back_to_selection") == 0) return ACTION_ENTER_NAV_MODE;  /* New name */

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

/* Get template dimensions (Issue #17) */
void config_get_template_dimensions(const AppConfig *config, BoxTemplate template,
                                    int *width, int *height) {
    if (!config || !width || !height) return;

    switch (template) {
        case BOX_TEMPLATE_SQUARE:
            *width = config->template_square_width;
            *height = config->template_square_height;
            break;
        case BOX_TEMPLATE_HORIZONTAL:
            *width = config->template_horizontal_width;
            *height = config->template_horizontal_height;
            break;
        case BOX_TEMPLATE_VERTICAL:
            *width = config->template_vertical_width;
            *height = config->template_vertical_height;
            break;
        default:
            /* Fallback to square */
            *width = config->template_square_width;
            *height = config->template_square_height;
            break;
    }
}

/* Get template name for display */
const char* config_get_template_name(BoxTemplate template) {
    switch (template) {
        case BOX_TEMPLATE_SQUARE:
            return "Square";
        case BOX_TEMPLATE_HORIZONTAL:
            return "Horizontal";
        case BOX_TEMPLATE_VERTICAL:
            return "Vertical";
        default:
            return "Unknown";
    }
}

/* Get icon for box type (Issue #33) */
const char* config_get_box_icon(const AppConfig *config, int box_type) {
    if (!config) return "";
    
    switch (box_type) {
        case 0: /* BOX_TYPE_NOTE */
            return config->icon_note;
        case 1: /* BOX_TYPE_TASK */
            return config->icon_task;
        case 2: /* BOX_TYPE_CODE */
            return config->icon_code;
        case 3: /* BOX_TYPE_STICKY */
            return config->icon_sticky;
        default:
            return config->icon_note;
    }
}
