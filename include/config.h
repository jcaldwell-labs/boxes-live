#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* Box template types */
typedef enum {
    BOX_TEMPLATE_SQUARE = 0,      /* Default square (20x10) */
    BOX_TEMPLATE_HORIZONTAL,      /* Wide rectangle (40x10) */
    BOX_TEMPLATE_VERTICAL         /* Tall rectangle (20x20) */
} BoxTemplate;

/* Application configuration (Phase 5a) */
typedef struct {
    /* General settings */
    bool show_visualizer;
    bool auto_save;

    /* Box template settings (Issue #17) */
    int template_square_width;
    int template_square_height;
    int template_horizontal_width;
    int template_horizontal_height;
    int template_vertical_width;
    int template_vertical_height;

    /* Proportional sizing settings (Issue #18) */
    bool proportional_sizing;       /* Enable/disable proportional sizing */
    int proximity_radius;           /* Search distance in world units */
    bool use_nearest_neighbor;      /* true = use nearest box, false = use average */
    int min_neighbors_required;     /* Minimum neighbors to trigger proportional sizing */

    /* Grid settings */
    bool grid_visible_default;
    bool grid_snap_default;
    int grid_spacing;

    /* Box type icons (Issue #33) */
    char icon_note[8];          /* Icon for NOTE boxes */
    char icon_task[8];          /* Icon for TASK boxes */
    char icon_code[8];          /* Icon for CODE boxes */
    char icon_sticky[8];        /* Icon for STICKY boxes */

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

/* Get template dimensions (Issue #17) */
void config_get_template_dimensions(const AppConfig *config, BoxTemplate template,
                                    int *width, int *height);

/* Get template name for display */
const char* config_get_template_name(BoxTemplate template);

/* Get icon for box type (Issue #33) */
const char* config_get_box_icon(const AppConfig *config, int box_type);

#endif /* CONFIG_H */
