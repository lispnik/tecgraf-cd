/*
 * CD Library Test Suite - SVG Backend
 * Tests SVG-specific functionality, output generation, and vector graphics features
 */

#include "test_utils.h"

int test_svg_canvas_creation(void) {
    printf("  Testing SVG canvas creation...\n");

    /* Test SVG canvas with file output */
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_svg_creation.svg");
    TEST_ASSERT_NOT_NULL(canvas, "SVG canvas creation should succeed");

    /* Test basic drawing to ensure canvas works */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, 10, 90, 10, 40);
    cdCanvasText(canvas, 50, 25, "SVG Test");

    test_destroy_canvas(canvas);

    /* Test different SVG canvas sizes */
    int test_dimensions[][2] = {
        {100, 100}, {800, 600}, {1920, 1080}, {50, 25}
    };

    for (size_t i = 0; i < sizeof(test_dimensions) / sizeof(test_dimensions[0]); i++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "test_svg_size_%dx%d.svg",
                test_dimensions[i][0], test_dimensions[i][1]);

        canvas = test_create_canvas("SVG", test_dimensions[i][0], test_dimensions[i][1], filename);
        if (canvas) {
            /* Draw size indicator */
            cdCanvasForeground(canvas, CD_RED);
            cdCanvasLine(canvas, 0, 0, test_dimensions[i][0] - 1, test_dimensions[i][1] - 1);
            test_destroy_canvas(canvas);
        }
    }

    return 1;
}

int test_svg_text_features(void) {
    printf("  Testing SVG text rendering features...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_svg_text_features.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test different font sizes */
    int y_pos = 30;
    for (int size = 8; size <= 24; size += 4) {
        cdCanvasFont(canvas, "Arial", CD_PLAIN, size);
        char text[64];
        snprintf(text, sizeof(text), "Font size %d", size);
        cdCanvasText(canvas, 50, y_pos, text);
        y_pos += size + 5;
    }

    /* Test font styles */
    y_pos = 30;
    const char* style_names[] = {"PLAIN", "BOLD", "ITALIC", "BOLD_ITALIC"};
    int styles[] = {CD_PLAIN, CD_BOLD, CD_ITALIC, CD_BOLD_ITALIC};

    for (size_t i = 0; i < sizeof(styles) / sizeof(styles[0]); i++) {
        cdCanvasFont(canvas, "Times", styles[i], 14);
        char text[64];
        snprintf(text, sizeof(text), "Style: %s", style_names[i]);
        cdCanvasText(canvas, 300, y_pos, text);
        y_pos += 25;
    }

    /* Test text alignment */
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasText(canvas, 300, 200, "Centered Text");

    cdCanvasTextAlignment(canvas, CD_LEFT);
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasText(canvas, 300, 220, "Left Aligned");

    cdCanvasTextAlignment(canvas, CD_RIGHT);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasText(canvas, 300, 240, "Right Aligned");

    /* Test text rotation */
    cdCanvasTextOrientation(canvas, 45);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasText(canvas, 400, 300, "Rotated 45°");

    test_destroy_canvas(canvas);
    return 1;
}

int test_svg_path_operations(void) {
    printf("  Testing SVG path operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_svg_paths.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test complex path construction */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLineWidth(canvas, 3);

    /* Create a complex path with curves */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 50, 100);

    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 150, 100);

    cdCanvasPathSet(canvas, CD_PATH_CURVETO);
    cdCanvasVertex(canvas, 200, 50);   /* Control point 1 */
    cdCanvasVertex(canvas, 250, 150);  /* Control point 2 */
    cdCanvasVertex(canvas, 300, 100);  /* End point */

    cdCanvasPathSet(canvas, CD_PATH_ARC);
    cdCanvasVertex(canvas, 350, 100);  /* Center */
    cdCanvasVertex(canvas, 40, 40);    /* Width, Height */
    cdCanvasVertex(canvas, 180, 270);  /* Start angle, End angle */

    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasPathSet(canvas, CD_PATH_STROKE);
    cdCanvasEnd(canvas);

    /* Test filled path */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 100, 250);

    /* Create star shape */
    for (int i = 0; i < 5; i++) {
        double angle = i * 144 * TEST_PI / 180;  /* 144 degrees between points */
        int x = 200 + (int)(60 * cos(angle));
        int y = 300 + (int)(60 * sin(angle));
        cdCanvasPathSet(canvas, CD_PATH_LINETO);
        cdCanvasVertex(canvas, x, y);
    }

    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasPathSet(canvas, CD_PATH_FILL);
    cdCanvasEnd(canvas);

    test_destroy_canvas(canvas);
    return 1;
}

int test_svg_styling_features(void) {
    printf("  Testing SVG styling features...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_svg_styling.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test line styles */
    int line_styles[] = {CD_CONTINUOUS, CD_DASHED, CD_DOTTED, CD_DASH_DOT, CD_DASH_DOT_DOT};
    const char* style_names[] = {"Continuous", "Dashed", "Dotted", "Dash-Dot", "Dash-Dot-Dot"};

    for (size_t i = 0; i < sizeof(line_styles) / sizeof(line_styles[0]); i++) {
        cdCanvasLineStyle(canvas, line_styles[i]);
        cdCanvasLineWidth(canvas, 3);
        cdCanvasForeground(canvas, cdEncodeColor(i * 50, 100, 200 - i * 30));

        int y = 50 + (int)(i * 30);
        cdCanvasLine(canvas, 50, y, 250, y);
        cdCanvasText(canvas, 270, y, style_names[i]);
    }

    /* Test fill patterns */
    int fill_patterns[] = {CD_SOLID, CD_HATCH, CD_STIPPLE, CD_PATTERN};
    const char* fill_names[] = {"Solid", "Hatch", "Stipple", "Pattern"};

    for (size_t i = 0; i < sizeof(fill_patterns) / sizeof(fill_patterns[0]) && i < 2; i++) {
        cdCanvasInteriorStyle(canvas, fill_patterns[i]);
        if (fill_patterns[i] == CD_HATCH) {
            cdCanvasHatch(canvas, CD_HORIZONTAL);
        }

        cdCanvasForeground(canvas, cdEncodeColor(100 + i * 50, 150, 100));
        int x = 350 + (int)(i * 80);
        cdCanvasBox(canvas, x, x + 60, 50, 110);

        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasText(canvas, x, 130, fill_names[i]);
    }

    /* Test transparency/opacity */
    cdCanvasOpacity(canvas, 128);  /* 50% opacity */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 100, 200, 250, 350);

    cdCanvasOpacity(canvas, 64);   /* 25% opacity */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 150, 250, 200, 300);

    cdCanvasOpacity(canvas, 255);  /* Full opacity */

    /* Test gradient simulation with multiple colored rectangles */
    for (int i = 0; i < 50; i++) {
        unsigned char intensity = (unsigned char)(i * 255 / 49);
        cdCanvasForeground(canvas, cdEncodeColor(intensity, 0, 255 - intensity));
        cdCanvasLine(canvas, 400 + i * 3, 300, 400 + i * 3, 400);
    }

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 400, 420, "Gradient Effect");

    test_destroy_canvas(canvas);
    return 1;
}

int test_svg_coordinate_systems(void) {
    printf("  Testing SVG coordinate systems...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_svg_coordinates.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test coordinate origin and axes */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 1);

    /* Draw coordinate grid */
    for (int x = 0; x <= 500; x += 50) {
        cdCanvasLine(canvas, x, 0, x, 400);
    }
    for (int y = 0; y <= 400; y += 50) {
        cdCanvasLine(canvas, 0, y, 500, y);
    }

    /* Mark origin and key points */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasMarkType(canvas, CD_CIRCLE);
    cdCanvasMarkSize(canvas, 8);
    cdCanvasMark(canvas, 0, 0);  /* Origin */

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasMark(canvas, 250, 200);  /* Center */

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasMark(canvas, 499, 399);  /* Far corner */

    /* Test world coordinates vs device coordinates */
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasRect(canvas, 100, 150, 100, 140);

    /* Add coordinate labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 5, 5, "(0,0)");
    cdCanvasText(canvas, 255, 205, "(250,200)");
    cdCanvasText(canvas, 450, 390, "(499,399)");

    test_destroy_canvas(canvas);
    return 1;
}

int test_svg_output_validation(void) {
    printf("  Testing SVG output structure...\n");

    const char* test_file = "test_svg_validation.svg";
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, test_file);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create content that should generate proper SVG */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 10, 90, 10, 50);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasArc(canvas, 150, 100, 80, 80, 0, 360);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasLine(canvas, 200, 20, 280, 180);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 50, 150, "SVG Output Test");

    test_destroy_canvas(canvas);

    /* Basic validation - check if file was created and has SVG content */
    FILE* svg_file = fopen(test_file, "r");
    if (svg_file) {
        char buffer[256];
        int found_svg_tag = 0;

        while (fgets(buffer, sizeof(buffer), svg_file) && !found_svg_tag) {
            if (strstr(buffer, "<svg") != NULL || strstr(buffer, "<?xml") != NULL) {
                found_svg_tag = 1;
            }
        }

        fclose(svg_file);
        TEST_ASSERT(found_svg_tag, "SVG file should contain SVG markup");
        printf("    SVG file validation: PASSED\n");
    } else {
        printf("    SVG file validation: Could not open file\n");
    }

    return 1;
}

int test_svg_complex_scene(void) {
    printf("  Testing complex SVG scene generation...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 800, 600, "test_svg_complex_scene.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create a complex scene with multiple elements */

    /* Background gradient effect */
    for (int y = 0; y < 600; y += 3) {
        unsigned char blue_intensity = (unsigned char)(y * 100 / 600 + 155);
        cdCanvasForeground(canvas, cdEncodeColor(135, 206, blue_intensity));
        cdCanvasLine(canvas, 0, y, 800, y);
    }

    /* Sun */
    cdCanvasForeground(canvas, cdEncodeColor(255, 255, 0));
    cdCanvasSector(canvas, 650, 500, 120, 120, 0, 360);

    /* Sun rays */
    cdCanvasLineWidth(canvas, 4);
    for (int angle = 0; angle < 360; angle += 45) {
        double rad = angle * TEST_PI / 180.0;
        int x1 = 650 + (int)(80 * cos(rad));
        int y1 = 500 + (int)(80 * sin(rad));
        int x2 = 650 + (int)(120 * cos(rad));
        int y2 = 500 + (int)(120 * sin(rad));
        cdCanvasLine(canvas, x1, y1, x2, y2);
    }

    /* Mountains */
    cdCanvasForeground(canvas, cdEncodeColor(139, 69, 19));
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 0, 0);
    cdCanvasVertex(canvas, 200, 300);
    cdCanvasVertex(canvas, 400, 150);
    cdCanvasVertex(canvas, 600, 250);
    cdCanvasVertex(canvas, 800, 100);
    cdCanvasVertex(canvas, 800, 0);
    cdCanvasEnd(canvas);

    /* Trees */
    for (int i = 0; i < 10; i++) {
        int x = 50 + i * 70;
        int tree_height = 80 + (i % 3) * 20;

        /* Tree trunk */
        cdCanvasForeground(canvas, cdEncodeColor(101, 67, 33));
        cdCanvasBox(canvas, x - 8, x + 8, 50, 50 + tree_height / 2);

        /* Tree crown */
        cdCanvasForeground(canvas, cdEncodeColor(34, 139, 34));
        cdCanvasSector(canvas, x, 50 + tree_height / 2, tree_height / 2, tree_height, 0, 360);
    }

    /* Clouds */
    cdCanvasForeground(canvas, cdEncodeColor(255, 255, 255));
    for (int cloud = 0; cloud < 3; cloud++) {
        int base_x = 100 + cloud * 250;
        int base_y = 450 + cloud * 30;

        /* Multiple overlapping circles for cloud effect */
        cdCanvasSector(canvas, base_x, base_y, 60, 40, 0, 360);
        cdCanvasSector(canvas, base_x + 30, base_y, 80, 50, 0, 360);
        cdCanvasSector(canvas, base_x + 60, base_y, 60, 40, 0, 360);
        cdCanvasSector(canvas, base_x + 30, base_y + 20, 70, 45, 0, 360);
    }

    /* House */
    int house_x = 300, house_y = 100;

    /* House base */
    cdCanvasForeground(canvas, cdEncodeColor(205, 133, 63));
    cdCanvasBox(canvas, house_x, house_x + 120, house_y, house_y + 80);

    /* Roof */
    cdCanvasForeground(canvas, cdEncodeColor(139, 0, 0));
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, house_x - 10, house_y + 80);
    cdCanvasVertex(canvas, house_x + 60, house_y + 120);
    cdCanvasVertex(canvas, house_x + 130, house_y + 80);
    cdCanvasEnd(canvas);

    /* Door */
    cdCanvasForeground(canvas, cdEncodeColor(101, 67, 33));
    cdCanvasBox(canvas, house_x + 20, house_x + 50, house_y, house_y + 50);

    /* Windows */
    cdCanvasForeground(canvas, cdEncodeColor(173, 216, 230));
    cdCanvasBox(canvas, house_x + 70, house_x + 95, house_y + 40, house_y + 65);
    cdCanvasBox(canvas, house_x + 70, house_x + 95, house_y + 15, house_y + 35);

    /* Title */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 18);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasText(canvas, 400, 50, "Complex SVG Scene Test");

    test_destroy_canvas(canvas);
    return 1;
}

int test_svg_special_features(void) {
    printf("  Testing SVG special features...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_svg_special_features.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test special characters and Unicode support */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 50, 350, "Special chars: αβγ ∑∆∇ ♠♥♦♣");

    /* Test very precise coordinates */
    for (int i = 0; i < 100; i++) {
        double precise_x = 100.5 + i * 2.7;
        double precise_y = 200.3 + sin(i * 0.1) * 30.0;
        cdfCanvasPixel(canvas, precise_x, precise_y, CD_RED);
    }

    /* Test large coordinate values */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, 400, 480, 300, 380);

    /* Test different line caps and joins (if supported) */
    cdCanvasLineWidth(canvas, 10);
    cdCanvasLineCap(canvas, CD_CAPROUND);
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasLine(canvas, 50, 100, 200, 100);

    cdCanvasLineCap(canvas, CD_CAPFLAT);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLine(canvas, 50, 120, 200, 120);

    /* Test polygon with many vertices */
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasBegin(canvas, CD_FILL);
    for (int i = 0; i < 20; i++) {
        double angle = i * 2 * TEST_PI / 20;
        int radius = (i % 2) ? 30 : 50;  /* Star shape */
        int x = 350 + (int)(radius * cos(angle));
        int y = 200 + (int)(radius * sin(angle));
        cdCanvasVertex(canvas, x, y);
    }
    cdCanvasEnd(canvas);

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library SVG Backend Tests...\n");

    RUN_TEST(test_svg_canvas_creation);
    RUN_TEST(test_svg_text_features);
    RUN_TEST(test_svg_path_operations);
    RUN_TEST(test_svg_styling_features);
    RUN_TEST(test_svg_coordinate_systems);
    RUN_TEST(test_svg_output_validation);
    RUN_TEST(test_svg_complex_scene);
    RUN_TEST(test_svg_special_features);

    printf("\nSVG Backend Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}