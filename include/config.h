#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* Application configuration (Phase 5a) */
typedef struct {
    /* General settings */
    bool show_visualizer;
    bool auto_save;

    /* Grid settings */
    bool grid_visible_default;
    bool grid_snap_default;
    int grid_spacing;

    /* Joystick settings */
    double joystick_deadzone;
    int joystick_settling_frames;

    /* Joystick button action mappings (action names as strings) */
    /* VIEW mode */
    char view_button_a[32];
    char view_button_b[32];
    char view_button_x[32];
    char view_button_y[32];
    char view_button_lb[32];
    char view_button_rb[32];

    /* SELECT mode */
    char select_button_a[32];
    char select_button_b[32];
    char select_button_x[32];
    char select_button_y[32];

    /* EDIT mode */
    char edit_button_a[32];
    char edit_button_b[32];
    char edit_button_x[32];
    char edit_button_y[32];
    char edit_button_lb[32];
    char edit_button_rb[32];

    /* Global buttons (same across all modes) */
    char global_button_menu[32];
    char global_button_start[32];
    char global_button_select[32];
    char global_button_back[32];
} AppConfig;

/* Initialize config with defaults */
void config_init_defaults(AppConfig *config);

/* Load config from file (returns 0 on success, -1 on error) */
/* Falls back to defaults for missing values */
int config_load(AppConfig *config, const char *path);

/* Save current config to file */
int config_save(const AppConfig *config, const char *path);

/* Create default config file */
int config_create_default_file(const char *path);

/* Get config file path (returns malloc'd string, caller must free) */
char* config_get_default_path(void);

/* Map action name to CanvasAction enum */
int config_action_from_name(const char *name);

#endif /* CONFIG_H */
