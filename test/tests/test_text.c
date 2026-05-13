/*
 * CD Library Test Suite - Text Rendering
 * Tests fonts, text alignment, orientation, and text measurement functions
 */

#include "test_utils.h"

int test_font_selection(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_fonts.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test different font families */
    const char* fonts[] = {"Helvetica", "Times", "Courier", "Arial", "Verdana"};
    int num_fonts = sizeof(fonts) / sizeof(fonts[0]);
    int i;

    cdCanvasForeground(canvas, CD_BLACK);
    int y = 50;

    for (i = 0; i < num_fonts; i++) {
        cdCanvasFont(canvas, fonts[i], CD_PLAIN, 16);
        char text[128];
        snprintf(text, sizeof(text), "Font: %s - The quick brown fox jumps over lazy dog", fonts[i]);
        cdCanvasText(canvas, 50, y, text);
        y += 25;
    }

    /* Test font sizes */
    y += 20;
    int sizes[] = {8, 10, 12, 14, 18, 24, 32};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (i = 0; i < num_sizes; i++) {
        cdCanvasFont(canvas, "Helvetica", CD_PLAIN, sizes[i]);
        char text[64];
        snprintf(text, sizeof(text), "Size %d: Sample Text", sizes[i]);
        cdCanvasText(canvas, 50, y, text);
        y += sizes[i] + 5;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_font_styles(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 300, "test_font_styles.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);

    /* Test font styles */
    struct {
        int style;
        const char* name;
    } styles[] = {
        {CD_PLAIN, "Plain Text"},
        {CD_BOLD, "Bold Text"},
        {CD_ITALIC, "Italic Text"},
        {CD_BOLD | CD_ITALIC, "Bold Italic Text"},
        {CD_UNDERLINE, "Underlined Text"},
        {CD_STRIKEOUT, "Strikeout Text"},
        {CD_UNDERLINE | CD_STRIKEOUT, "Underline + Strikeout"}
    };
    int num_styles = sizeof(styles) / sizeof(styles[0]);
    int i;

    int y = 50;
    for (i = 0; i < num_styles; i++) {
        cdCanvasFont(canvas, "Helvetica", styles[i].style, 16);
        cdCanvasText(canvas, 50, y, styles[i].name);
        y += 30;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_text_alignment(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_text_alignment.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test text alignments */
    struct {
        int alignment;
        const char* name;
    } alignments[] = {
        {CD_NORTH, "NORTH"},
        {CD_SOUTH, "SOUTH"},
        {CD_EAST, "EAST"},
        {CD_WEST, "WEST"},
        {CD_NORTH_EAST, "NORTH_EAST"},
        {CD_NORTH_WEST, "NORTH_WEST"},
        {CD_SOUTH_EAST, "SOUTH_EAST"},
        {CD_SOUTH_WEST, "SOUTH_WEST"},
        {CD_CENTER, "CENTER"},
        {CD_BASE_LEFT, "BASE_LEFT"},
        {CD_BASE_CENTER, "BASE_CENTER"},
        {CD_BASE_RIGHT, "BASE_RIGHT"}
    };
    int num_alignments = sizeof(alignments) / sizeof(alignments[0]);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 12);

    /* Draw reference point grid */
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            int x = 100 + i * 150;
            int y = 80 + j * 100;

            if (i * 4 + j < num_alignments) {
                /* Draw reference point */
                cdCanvasForeground(canvas, CD_RED);
                cdCanvasMarkType(canvas, CD_CIRCLE);
                cdCanvasMarkSize(canvas, 5);
                cdCanvasMark(canvas, x, y);

                /* Draw text with alignment */
                cdCanvasForeground(canvas, CD_BLACK);
                cdCanvasTextAlignment(canvas, alignments[i * 4 + j].alignment);
                cdCanvasText(canvas, x, y, alignments[i * 4 + j].name);
            }
        }
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_text_orientation(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 500, "test_text_orientation.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 14);
    cdCanvasTextAlignment(canvas, CD_CENTER);

    int center_x = 250;
    int center_y = 250;

    /* Draw reference point */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasMarkType(canvas, CD_CIRCLE);
    cdCanvasMarkSize(canvas, 8);
    cdCanvasMark(canvas, center_x, center_y);

    /* Test different orientations */
    double angles[] = {0, 30, 45, 60, 90, 120, 135, 150, 180, 210, 270, 315};
    int num_angles = sizeof(angles) / sizeof(angles[0]);
    int i;

    for (i = 0; i < num_angles; i++) {
        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasTextOrientation(canvas, angles[i]);

        char text[32];
        snprintf(text, sizeof(text), "%.0f°", angles[i]);

        /* Position text away from center */
        double rad = angles[i] * TEST_DEG2RAD;
        int text_x = center_x + (int)(80 * cos(rad));
        int text_y = center_y + (int)(80 * sin(rad));

        cdCanvasText(canvas, text_x, text_y, text);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_text_measurement(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_text_measurement.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 16);

    const char* test_strings[] = {
        "Short",
        "Medium length text",
        "This is a very long string that should have different measurements",
        "Mixed CASE and numbers 123456"
    };
    int num_strings = sizeof(test_strings) / sizeof(test_strings[0]);
    int i;

    int y = 80;
    for (i = 0; i < num_strings; i++) {
        /* Measure text */
        int width, height;
        cdCanvasGetTextSize(canvas, test_strings[i], &width, &height);

        /* Get font dimensions */
        int max_width, font_height, ascent, descent;
        cdCanvasGetFontDim(canvas, &max_width, &font_height, &ascent, &descent);

        /* Get text box */
        int xmin, xmax, ymin, ymax;
        cdCanvasGetTextBox(canvas, 50, y, test_strings[i], &xmin, &xmax, &ymin, &ymax);

        /* Draw text */
        cdCanvasText(canvas, 50, y, test_strings[i]);

        /* Draw bounding box */
        cdCanvasForeground(canvas, CD_RED);
        cdCanvasLineStyle(canvas, CD_DASHED);
        cdCanvasRect(canvas, xmin, xmax, ymin, ymax);

        /* Display measurements */
        cdCanvasForeground(canvas, CD_BLUE);
        cdCanvasLineStyle(canvas, CD_CONTINUOUS);
        char info[256];
        snprintf(info, sizeof(info), "W:%d H:%d Box:(%d,%d)-(%d,%d)",
                width, height, xmin, ymin, xmax, ymax);
        cdCanvasText(canvas, 300, y, info);

        cdCanvasForeground(canvas, CD_BLACK);
        y += 60;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_vector_text(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_vector_text.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test vector text if available */
    cdCanvasForeground(canvas, CD_BLACK);

    /* Try to use vector text */
    cdCanvasVectorCharSize(canvas, 20);

    /* Test vector text direction */
    cdCanvasVectorTextDirection(canvas, 100, 100, 200, 150);
    cdCanvasVectorText(canvas, 100, 100, "Vector Text Direction");

    /* Test vector text size */
    cdCanvasVectorTextSize(canvas, 15, 20, "Size Test");
    cdCanvasVectorText(canvas, 50, 200, "Sized Vector Text");

    /* Test multi-line vector text */
    cdCanvasVectorText(canvas, 50, 250, "Line 1\nLine 2\nLine 3");

    test_destroy_canvas(canvas);
    return 1;
}

int test_unicode_text(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 300, "test_unicode_text.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 16);

    /* Test various text encodings and special characters */
    const char* test_texts[] = {
        "ASCII Text: Hello World!",
        "Symbols: ©®™§¶†‡•…‰′″‹›""''",
        "Math: α β γ δ ε Σ π ∞ ≠ ≤ ≥ ±",
        "Currency: $ € £ ¥ ¢ ₹ ₽",
        "Arrows: ← → ↑ ↓ ↔ ↕ ⇒ ⇔"
    };
    int num_texts = sizeof(test_texts) / sizeof(test_texts[0]);
    int i;

    int y = 50;
    for (i = 0; i < num_texts; i++) {
        cdCanvasText(canvas, 50, y, test_texts[i]);
        y += 30;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Text Tests...\n");

    RUN_TEST(test_font_selection);
    RUN_TEST(test_font_styles);
    RUN_TEST(test_text_alignment);
    RUN_TEST(test_text_orientation);
    RUN_TEST(test_text_measurement);
    RUN_TEST(test_vector_text);
    RUN_TEST(test_unicode_text);

    printf("\nText Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}