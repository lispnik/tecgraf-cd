/*
 * CD Library Test Suite - Attributes Testing
 * Tests colors, line styles, fill patterns, fonts, and other rendering attributes
 */

#include "test_utils.h"

int test_line_attributes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_line_attributes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test line widths */
    int y = 50;
    int widths[] = {1, 2, 3, 5, 8, 12, 16, 20};
    int i;

    for (i = 0; i < 8; i++) {
        cdCanvasLineWidth(canvas, widths[i]);
        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasLine(canvas, 50, y, 200, y);

        /* Label the line width */
        cdCanvasLineWidth(canvas, 1);
        char label[16];
        snprintf(label, sizeof(label), "Width: %d", widths[i]);
        cdCanvasText(canvas, 220, y, label);

        y += 30;
    }

    /* Test line caps */
    y += 20;
    int caps[] = {CD_CAPFLAT, CD_CAPSQUARE, CD_CAPROUND};
    const char* cap_names[] = {"FLAT", "SQUARE", "ROUND"};

    cdCanvasLineWidth(canvas, 10);
    for (i = 0; i < 3; i++) {
        cdCanvasLineCap(canvas, caps[i]);
        cdCanvasForeground(canvas, CD_BLUE);
        cdCanvasLine(canvas, 50, y, 200, y);

        cdCanvasLineWidth(canvas, 1);
        cdCanvasText(canvas, 220, y, cap_names[i]);
        cdCanvasLineWidth(canvas, 10);

        y += 30;
    }

    /* Test line joins */
    int joins[] = {CD_MITER, CD_BEVEL, CD_ROUND};
    const char* join_names[] = {"MITER", "BEVEL", "ROUND"};

    for (i = 0; i < 3; i++) {
        cdCanvasLineJoin(canvas, joins[i]);
        cdCanvasForeground(canvas, CD_RED);

        /* Draw angle to show join style */
        cdCanvasBegin(canvas, CD_OPEN_LINES);
        cdCanvasVertex(canvas, 350, y - 15);
        cdCanvasVertex(canvas, 400, y);
        cdCanvasVertex(canvas, 450, y - 15);
        cdCanvasEnd(canvas);

        cdCanvasLineWidth(canvas, 1);
        cdCanvasText(canvas, 470, y, join_names[i]);
        cdCanvasLineWidth(canvas, 10);

        y += 40;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_line_styles(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_line_styles.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    test_draw_line_styles(canvas, 50, 50);

    /* Test custom line style */
    int dashes[] = {10, 5, 3, 5, 10, 15};
    cdCanvasLineStyleDashes(canvas, dashes, 6);
    cdCanvasLineStyle(canvas, CD_CUSTOM);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasLine(canvas, 50, 200, 400, 200);
    cdCanvasText(canvas, 50, 220, "CUSTOM DASH PATTERN");

    test_destroy_canvas(canvas);
    return 1;
}

int test_color_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_colors.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test basic color palette */
    test_draw_color_palette(canvas, 50, 50);

    /* Test alpha transparency */
    int y = 120;
    unsigned char alpha_values[] = {255, 192, 128, 64, 32};
    int i;

    for (i = 0; i < 5; i++) {
        long color = cdEncodeColorAlpha(255, 0, 0, alpha_values[i]);
        cdCanvasForeground(canvas, color);
        cdCanvasBox(canvas, 50 + i * 60, 80 + i * 60, y, y + 30);

        char label[16];
        snprintf(label, sizeof(label), "A:%d", alpha_values[i]);
        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasText(canvas, 50 + i * 60, y + 40, label);
    }

    /* Test RGB color encoding/decoding */
    y = 200;
    for (i = 0; i < 256; i += 32) {
        long color = cdEncodeColor(i, 255 - i, 128);

        unsigned char r, g, b;
        cdDecodeColor(color, &r, &g, &b);

        /* Verify encoding/decoding */
        TEST_ASSERT_EQ(i, r, "Red component mismatch");
        TEST_ASSERT_EQ(255 - i, g, "Green component mismatch");
        TEST_ASSERT_EQ(128, b, "Blue component mismatch");

        cdCanvasForeground(canvas, color);
        cdCanvasBox(canvas, 50 + (i/32) * 40, 70 + (i/32) * 40, y, y + 20);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_fill_patterns(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 800, 600, "test_fill_patterns.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test hatch patterns */
    int hatch_styles[] = {CD_HORIZONTAL, CD_VERTICAL, CD_FDIAGONAL, CD_BDIAGONAL, CD_CROSS, CD_DIAGCROSS};
    const char* hatch_names[] = {"HORIZONTAL", "VERTICAL", "FDIAGONAL", "BDIAGONAL", "CROSS", "DIAGCROSS"};
    int i;

    cdCanvasInteriorStyle(canvas, CD_HATCH);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBackground(canvas, CD_YELLOW);

    for (i = 0; i < 6; i++) {
        int x = 50 + (i % 3) * 200;
        int y = 50 + (i / 3) * 150;

        cdCanvasHatch(canvas, hatch_styles[i]);
        cdCanvasBox(canvas, x, x + 120, y, y + 80);

        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasInteriorStyle(canvas, CD_SOLID);
        cdCanvasText(canvas, x, y + 100, hatch_names[i]);
        cdCanvasInteriorStyle(canvas, CD_HATCH);
        cdCanvasForeground(canvas, CD_BLUE);
    }

    /* Test stipple pattern */
    unsigned char stipple[8*8];
    int j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            stipple[i*8 + j] = (i + j) % 2;
        }
    }

    cdCanvasInteriorStyle(canvas, CD_STIPPLE);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBackground(canvas, CD_GREEN);
    cdCanvasStipple(canvas, 8, 8, stipple);
    cdCanvasBox(canvas, 50, 170, 350, 430);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasText(canvas, 50, 450, "STIPPLE PATTERN");

    /* Test color pattern */
    long pattern[16*16];
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            unsigned char red = (i * 255) / 15;
            unsigned char green = (j * 255) / 15;
            unsigned char blue = 128;
            pattern[i*16 + j] = cdEncodeColor(red, green, blue);
        }
    }

    cdCanvasInteriorStyle(canvas, CD_PATTERN);
    cdCanvasPattern(canvas, 16, 16, pattern);
    cdCanvasBox(canvas, 250, 370, 350, 470);

    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasText(canvas, 250, 490, "COLOR PATTERN");

    test_destroy_canvas(canvas);
    return 1;
}

int test_background_modes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_background.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test opaque background */
    cdCanvasBackground(canvas, CD_YELLOW);
    cdCanvasBackOpacity(canvas, CD_OPAQUE);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasInteriorStyle(canvas, CD_HATCH);
    cdCanvasHatch(canvas, CD_CROSS);
    cdCanvasBox(canvas, 50, 150, 50, 100);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasText(canvas, 50, 120, "OPAQUE BACKGROUND");

    /* Test transparent background */
    cdCanvasBackOpacity(canvas, CD_TRANSPARENT);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasInteriorStyle(canvas, CD_HATCH);
    cdCanvasHatch(canvas, CD_CROSS);
    cdCanvasBox(canvas, 200, 300, 50, 100);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasText(canvas, 200, 120, "TRANSPARENT BACKGROUND");

    test_destroy_canvas(canvas);
    return 1;
}

int test_write_modes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_write_modes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Note: Write modes may not be supported by all backends */
    int modes[] = {CD_REPLACE, CD_XOR, CD_NOT_XOR};
    const char* mode_names[] = {"REPLACE", "XOR", "NOT_XOR"};
    int i;

    for (i = 0; i < 3; i++) {
        cdCanvasWriteMode(canvas, modes[i]);
        cdCanvasForeground(canvas, CD_RED);
        cdCanvasBox(canvas, 50 + i * 100, 100 + i * 100, 50, 100);

        /* Draw overlapping rectangle */
        cdCanvasForeground(canvas, CD_BLUE);
        cdCanvasBox(canvas, 75 + i * 100, 125 + i * 100, 75, 125);

        cdCanvasWriteMode(canvas, CD_REPLACE);
        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasText(canvas, 50 + i * 100, 150, mode_names[i]);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Attributes Tests...\n");

    RUN_TEST(test_line_attributes);
    RUN_TEST(test_line_styles);
    RUN_TEST(test_color_operations);
    RUN_TEST(test_fill_patterns);
    RUN_TEST(test_background_modes);
    RUN_TEST(test_write_modes);

    printf("\nAttributes Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}