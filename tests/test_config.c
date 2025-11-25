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

    TEST_END();
}
