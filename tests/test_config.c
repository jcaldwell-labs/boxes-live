#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <unistd.h>
#include "test.h"
#include "../include/config.h"

int main(void) {
    TEST_START();

    TEST("Config initialization defaults") {
        AppConfig config;
        config_init_defaults(&config);

        ASSERT_EQ(config.template_square_width, 20, "Square width default is 20");
        ASSERT_EQ(config.template_square_height, 10, "Square height default is 10");
        ASSERT_EQ(config.template_horizontal_width, 40, "Horizontal width default is 40");
        ASSERT_EQ(config.template_horizontal_height, 10, "Horizontal height default is 10");
        ASSERT_EQ(config.template_vertical_width, 20, "Vertical width default is 20");
        ASSERT_EQ(config.template_vertical_height, 20, "Vertical height default is 20");
    }

    TEST("config_get_template_dimensions - Square template") {
        AppConfig config;
        config_init_defaults(&config);
        int width = 0, height = 0;

        config_get_template_dimensions(&config, BOX_TEMPLATE_SQUARE, &width, &height);

        ASSERT_EQ(width, 20, "Square template width is 20");
        ASSERT_EQ(height, 10, "Square template height is 10");
    }

    TEST("config_get_template_dimensions - Horizontal template") {
        AppConfig config;
        config_init_defaults(&config);
        int width = 0, height = 0;

        config_get_template_dimensions(&config, BOX_TEMPLATE_HORIZONTAL, &width, &height);

        ASSERT_EQ(width, 40, "Horizontal template width is 40");
        ASSERT_EQ(height, 10, "Horizontal template height is 10");
    }

    TEST("config_get_template_dimensions - Vertical template") {
        AppConfig config;
        config_init_defaults(&config);
        int width = 0, height = 0;

        config_get_template_dimensions(&config, BOX_TEMPLATE_VERTICAL, &width, &height);

        ASSERT_EQ(width, 20, "Vertical template width is 20");
        ASSERT_EQ(height, 20, "Vertical template height is 20");
    }

    TEST("config_get_template_dimensions - Unknown template defaults to Square") {
        AppConfig config;
        config_init_defaults(&config);
        int width = 0, height = 0;

        /* Pass an invalid template value (e.g., 99) */
        config_get_template_dimensions(&config, (BoxTemplate)99, &width, &height);

        ASSERT_EQ(width, 20, "Unknown template falls back to square width");
        ASSERT_EQ(height, 10, "Unknown template falls back to square height");
    }

    TEST("config_get_template_dimensions - Custom template values") {
        AppConfig config;
        config_init_defaults(&config);

        /* Override defaults with custom values */
        config.template_square_width = 25;
        config.template_square_height = 12;
        config.template_horizontal_width = 50;
        config.template_horizontal_height = 8;
        config.template_vertical_width = 15;
        config.template_vertical_height = 25;

        int width = 0, height = 0;

        config_get_template_dimensions(&config, BOX_TEMPLATE_SQUARE, &width, &height);
        ASSERT_EQ(width, 25, "Custom square width is 25");
        ASSERT_EQ(height, 12, "Custom square height is 12");

        config_get_template_dimensions(&config, BOX_TEMPLATE_HORIZONTAL, &width, &height);
        ASSERT_EQ(width, 50, "Custom horizontal width is 50");
        ASSERT_EQ(height, 8, "Custom horizontal height is 8");

        config_get_template_dimensions(&config, BOX_TEMPLATE_VERTICAL, &width, &height);
        ASSERT_EQ(width, 15, "Custom vertical width is 15");
        ASSERT_EQ(height, 25, "Custom vertical height is 25");
    }

    TEST("config_get_template_dimensions - NULL safety") {
        AppConfig config;
        config_init_defaults(&config);
        int width = 999, height = 999;

        /* NULL config should not crash */
        config_get_template_dimensions(NULL, BOX_TEMPLATE_SQUARE, &width, &height);
        ASSERT_EQ(width, 999, "Width unchanged when config is NULL");
        ASSERT_EQ(height, 999, "Height unchanged when config is NULL");

        /* NULL width pointer */
        config_get_template_dimensions(&config, BOX_TEMPLATE_SQUARE, NULL, &height);
        ASSERT_EQ(height, 999, "Height unchanged when width is NULL");

        /* NULL height pointer */
        config_get_template_dimensions(&config, BOX_TEMPLATE_SQUARE, &width, NULL);
        ASSERT_EQ(width, 999, "Width unchanged when height is NULL");
    }

    TEST("config_get_template_name - Square") {
        const char *name = config_get_template_name(BOX_TEMPLATE_SQUARE);
        ASSERT_STR_EQ(name, "Square", "Square template name is 'Square'");
    }

    TEST("config_get_template_name - Horizontal") {
        const char *name = config_get_template_name(BOX_TEMPLATE_HORIZONTAL);
        ASSERT_STR_EQ(name, "Horizontal", "Horizontal template name is 'Horizontal'");
    }

    TEST("config_get_template_name - Vertical") {
        const char *name = config_get_template_name(BOX_TEMPLATE_VERTICAL);
        ASSERT_STR_EQ(name, "Vertical", "Vertical template name is 'Vertical'");
    }

    TEST("config_get_template_name - Unknown template") {
        const char *name = config_get_template_name((BoxTemplate)99);
        ASSERT_STR_EQ(name, "Unknown", "Unknown template returns 'Unknown'");
    }

    TEST("Config save and load - Template values persisted") {
        AppConfig cfg_to_save;
        config_init_defaults(&cfg_to_save);

        /* Set custom template values */
        cfg_to_save.template_square_width = 25;
        cfg_to_save.template_square_height = 15;
        cfg_to_save.template_horizontal_width = 50;
        cfg_to_save.template_horizontal_height = 8;
        cfg_to_save.template_vertical_width = 18;
        cfg_to_save.template_vertical_height = 28;

        /* Save to temp file */
        const char *test_file = "/tmp/test_config_templates.ini";
        int result = config_save(&cfg_to_save, test_file);
        ASSERT_EQ(result, 0, "Config save succeeded");

        /* Load into fresh config */
        AppConfig config_loaded;
        config_init_defaults(&config_loaded);
        result = config_load(&config_loaded, test_file);
        ASSERT_EQ(result, 0, "Config load succeeded");

        /* Verify template values */
        ASSERT_EQ(config_loaded.template_square_width, 25, "Loaded square width is 25");
        ASSERT_EQ(config_loaded.template_square_height, 15, "Loaded square height is 15");
        ASSERT_EQ(config_loaded.template_horizontal_width, 50, "Loaded horizontal width is 50");
        ASSERT_EQ(config_loaded.template_horizontal_height, 8, "Loaded horizontal height is 8");
        ASSERT_EQ(config_loaded.template_vertical_width, 18, "Loaded vertical width is 18");
        ASSERT_EQ(config_loaded.template_vertical_height, 28, "Loaded vertical height is 28");

        /* Cleanup */
        unlink(test_file);
    }

    TEST("Config load - Template min/max validation") {
        /* Create a config file with out-of-range values */
        const char *test_file = "/tmp/test_config_validation.ini";
        FILE *f = fopen(test_file, "w");
        ASSERT_NOT_NULL(f, "Created test config file");

        fprintf(f, "[templates]\n");
        fprintf(f, "square_width = 5\n");       /* Below min (10) */
        fprintf(f, "square_height = 50\n");     /* Above max (30) */
        fprintf(f, "horizontal_width = 100\n"); /* Above max (80) */
        fprintf(f, "horizontal_height = 1\n");  /* Below min (3) */
        fprintf(f, "vertical_width = 0\n");     /* Below min (10) */
        fprintf(f, "vertical_height = 100\n");  /* Above max (30) */
        fclose(f);

        AppConfig config;
        config_init_defaults(&config);
        int result = config_load(&config, test_file);
        ASSERT_EQ(result, 0, "Config load succeeded despite invalid values");

        /* Verify values are clamped to valid range */
        ASSERT_EQ(config.template_square_width, 10, "Square width clamped to min 10");
        ASSERT_EQ(config.template_square_height, 30, "Square height clamped to max 30");
        ASSERT_EQ(config.template_horizontal_width, 80, "Horizontal width clamped to max 80");
        ASSERT_EQ(config.template_horizontal_height, 3, "Horizontal height clamped to min 3");
        ASSERT_EQ(config.template_vertical_width, 10, "Vertical width clamped to min 10");
        ASSERT_EQ(config.template_vertical_height, 30, "Vertical height clamped to max 30");

        /* Cleanup */
        unlink(test_file);
    }

    /* Proportional sizing config tests (Issue #18) */

    TEST("Proportional sizing - Default values") {
        AppConfig config;
        config_init_defaults(&config);

        ASSERT(config.proportional_sizing == true, "Proportional sizing enabled by default");
        ASSERT_EQ(config.proximity_radius, 30, "Default proximity radius is 30");
        ASSERT(config.use_nearest_neighbor == false, "Default uses average, not nearest");
        ASSERT_EQ(config.min_neighbors_required, 1, "Default min neighbors is 1");
    }

    TEST("Proportional sizing - Save and load persistence") {
        AppConfig cfg_to_save;
        config_init_defaults(&cfg_to_save);

        /* Set custom proportional sizing values */
        cfg_to_save.proportional_sizing = false;
        cfg_to_save.proximity_radius = 50;
        cfg_to_save.use_nearest_neighbor = true;
        cfg_to_save.min_neighbors_required = 3;

        /* Save to temp file */
        const char *test_file = "/tmp/test_config_proportional.ini";
        int result = config_save(&cfg_to_save, test_file);
        ASSERT_EQ(result, 0, "Config save succeeded");

        /* Load into fresh config */
        AppConfig config_loaded;
        config_init_defaults(&config_loaded);
        result = config_load(&config_loaded, test_file);
        ASSERT_EQ(result, 0, "Config load succeeded");

        /* Verify proportional sizing values */
        ASSERT(config_loaded.proportional_sizing == false, "Loaded proportional_sizing is false");
        ASSERT_EQ(config_loaded.proximity_radius, 50, "Loaded proximity_radius is 50");
        ASSERT(config_loaded.use_nearest_neighbor == true, "Loaded use_nearest_neighbor is true");
        ASSERT_EQ(config_loaded.min_neighbors_required, 3, "Loaded min_neighbors_required is 3");

        /* Cleanup */
        unlink(test_file);
    }

    TEST("Proportional sizing - proximity_radius min/max validation") {
        const char *test_file = "/tmp/test_config_prox_validation.ini";
        FILE *f = fopen(test_file, "w");
        ASSERT_NOT_NULL(f, "Created test config file");

        fprintf(f, "[proportional]\n");
        fprintf(f, "proximity_radius = 2\n");  /* Below min (5) */
        fclose(f);

        AppConfig config;
        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT_EQ(config.proximity_radius, 5, "Proximity radius clamped to min 5");

        /* Test max clamping */
        f = fopen(test_file, "w");
        fprintf(f, "[proportional]\n");
        fprintf(f, "proximity_radius = 500\n");  /* Above max (200) */
        fclose(f);

        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT_EQ(config.proximity_radius, 200, "Proximity radius clamped to max 200");

        /* Cleanup */
        unlink(test_file);
    }

    TEST("Proportional sizing - min_neighbors_required min/max validation") {
        const char *test_file = "/tmp/test_config_minneigh_validation.ini";
        FILE *f = fopen(test_file, "w");
        ASSERT_NOT_NULL(f, "Created test config file");

        fprintf(f, "[proportional]\n");
        fprintf(f, "min_neighbors_required = 0\n");  /* Below min (1) */
        fclose(f);

        AppConfig config;
        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT_EQ(config.min_neighbors_required, 1, "min_neighbors clamped to min 1");

        /* Test max clamping */
        f = fopen(test_file, "w");
        fprintf(f, "[proportional]\n");
        fprintf(f, "min_neighbors_required = 20\n");  /* Above max (10) */
        fclose(f);

        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT_EQ(config.min_neighbors_required, 10, "min_neighbors clamped to max 10");

        /* Cleanup */
        unlink(test_file);
    }

    TEST("Proportional sizing - Boolean parsing") {
        const char *test_file = "/tmp/test_config_bool_validation.ini";
        FILE *f = fopen(test_file, "w");
        ASSERT_NOT_NULL(f, "Created test config file");

        fprintf(f, "[proportional]\n");
        fprintf(f, "enabled = false\n");
        fprintf(f, "use_nearest_neighbor = true\n");
        fclose(f);

        AppConfig config;
        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT(config.proportional_sizing == false, "Boolean 'false' parsed correctly");
        ASSERT(config.use_nearest_neighbor == true, "Boolean 'true' parsed correctly");

        /* Test with 'true' for enabled */
        f = fopen(test_file, "w");
        fprintf(f, "[proportional]\n");
        fprintf(f, "enabled = true\n");
        fprintf(f, "use_nearest_neighbor = false\n");
        fclose(f);

        config_init_defaults(&config);
        config_load(&config, test_file);

        ASSERT(config.proportional_sizing == true, "Boolean 'true' parsed correctly for enabled");
        ASSERT(config.use_nearest_neighbor == false, "Boolean 'false' parsed correctly for use_nearest");

        /* Cleanup */
        unlink(test_file);
    }

    TEST_END();
}
