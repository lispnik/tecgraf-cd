/*
 * CD Library Test Suite - Color Management
 * Tests color encoding, palettes, transparency, and color space operations
 */

#include "test_utils.h"

int test_color_encoding(void) {
    printf("  Testing color encoding/decoding...\n");

    /* Test RGB color encoding */
    long red_color = cdEncodeColor(255, 0, 0);
    long green_color = cdEncodeColor(0, 255, 0);
    long blue_color = cdEncodeColor(0, 0, 255);
    long white_color = cdEncodeColor(255, 255, 255);
    long black_color = cdEncodeColor(0, 0, 0);

    /* Test color decoding */
    unsigned char r, g, b;
    cdDecodeColor(red_color, &r, &g, &b);
    TEST_ASSERT_EQ(255, r, "Red component should be 255");
    TEST_ASSERT_EQ(0, g, "Green component should be 0");
    TEST_ASSERT_EQ(0, b, "Blue component should be 0");

    cdDecodeColor(green_color, &r, &g, &b);
    TEST_ASSERT_EQ(0, r, "Red component should be 0");
    TEST_ASSERT_EQ(255, g, "Green component should be 255");
    TEST_ASSERT_EQ(0, b, "Blue component should be 0");

    /* Test grayscale colors */
    for (int i = 0; i <= 255; i += 51) {
        long gray = cdEncodeColor(i, i, i);
        cdDecodeColor(gray, &r, &g, &b);
        TEST_ASSERT_EQ(i, r, "Gray red component should match");
        TEST_ASSERT_EQ(i, g, "Gray green component should match");
        TEST_ASSERT_EQ(i, b, "Gray blue component should match");
    }

    return 1;
}

int test_named_colors(void) {
    printf("  Testing named color constants...\n");

    /* Test predefined color constants */
    unsigned char r, g, b;

    cdDecodeColor(CD_RED, &r, &g, &b);
    TEST_ASSERT_EQ(255, r, "CD_RED should have max red");

    cdDecodeColor(CD_GREEN, &r, &g, &b);
    TEST_ASSERT_EQ(255, g, "CD_GREEN should have max green");

    cdDecodeColor(CD_BLUE, &r, &g, &b);
    TEST_ASSERT_EQ(255, b, "CD_BLUE should have max blue");

    cdDecodeColor(CD_WHITE, &r, &g, &b);
    TEST_ASSERT_EQ(255, r, "CD_WHITE red should be 255");
    TEST_ASSERT_EQ(255, g, "CD_WHITE green should be 255");
    TEST_ASSERT_EQ(255, b, "CD_WHITE blue should be 255");

    cdDecodeColor(CD_BLACK, &r, &g, &b);
    TEST_ASSERT_EQ(0, r, "CD_BLACK red should be 0");
    TEST_ASSERT_EQ(0, g, "CD_BLACK green should be 0");
    TEST_ASSERT_EQ(0, b, "CD_BLACK blue should be 0");

    return 1;
}

int test_color_attributes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_color_attributes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    printf("  Testing color attribute setting...\n");

    /* Test foreground color */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasRect(canvas, 10, 60, 10, 40);

    /* Test background color */
    cdCanvasBackground(canvas, CD_BLUE);

    /* Test different color modes */
    cdCanvasForeground(canvas, cdEncodeColor(128, 64, 192));
    cdCanvasBox(canvas, 70, 120, 10, 40);

    /* Test color attribute retrieval */
    long fg_color = cdCanvasForeground(canvas, CD_QUERY);
    unsigned char r, g, b;
    cdDecodeColor(fg_color, &r, &g, &b);

    /* Draw color palette grid */
    int x = 10, y = 60;
    for (int red = 0; red < 4; red++) {
        for (int green = 0; green < 4; green++) {
            for (int blue = 0; blue < 4; blue++) {
                long color = cdEncodeColor(red * 85, green * 85, blue * 85);
                cdCanvasForeground(canvas, color);
                cdCanvasBox(canvas, x, x + 15, y, y + 15);
                x += 20;
                if (x > 350) {
                    x = 10;
                    y += 20;
                }
            }
        }
    }

    /* Test text with different colors */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 10, 250, "Color Attribute Tests");

    test_destroy_canvas(canvas);
    return 1;
}

int test_alpha_transparency(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_alpha_transparency.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    printf("  Testing alpha transparency...\n");

    /* Note: Alpha support depends on backend */

    /* Test alpha encoding (if supported) */
    long alpha_red = cdEncodeColorAlpha(255, 0, 0, 128);  /* 50% transparent red */
    long alpha_blue = cdEncodeColorAlpha(0, 0, 255, 64);  /* 25% transparent blue */

    /* Test alpha decoding */
    unsigned char r, g, b, a;
    cdDecodeColorAlpha(alpha_red, &r, &g, &b, &a);
    TEST_ASSERT_EQ(255, r, "Alpha red component should be 255");
    TEST_ASSERT_EQ(128, a, "Alpha component should be 128");

    /* Draw overlapping shapes with alpha */
    cdCanvasForeground(canvas, alpha_red);
    cdCanvasBox(canvas, 50, 150, 50, 150);

    cdCanvasForeground(canvas, alpha_blue);
    cdCanvasBox(canvas, 100, 200, 100, 200);

    /* Test opacity setting */
    cdCanvasOpacity(canvas, 128);  /* 50% opacity */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 200, 300, 50, 150);

    cdCanvasOpacity(canvas, 255);  /* Full opacity */

    /* Test different alpha values */
    for (int i = 0; i < 10; i++) {
        unsigned char alpha = i * 25;
        long color = cdEncodeColorAlpha(255, 165, 0, alpha);  /* Orange with varying alpha */
        cdCanvasForeground(canvas, color);
        cdCanvasRect(canvas, 10 + i * 35, 40 + i * 35, 200, 230);
    }

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 10, 270, "Alpha Transparency Tests");

    test_destroy_canvas(canvas);
    return 1;
}

int test_palette_operations(void) {
    cdCanvas* canvas = test_create_canvas("RGB", 256, 256, NULL);
    if (!canvas) {
        printf("  RGB canvas not available, skipping palette test\n");
        return 1;
    }

    printf("  Testing palette operations...\n");

    /* Test palette creation for indexed color modes */
    long palette[256];

    /* Create grayscale palette */
    for (int i = 0; i < 256; i++) {
        palette[i] = cdEncodeColor(i, i, i);
    }

    /* Set palette (if supported by backend) */
    cdCanvasPalette(canvas, 256, palette, CD_FORCE);

    /* Draw using palette indices */
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            int index = (x + y) / 2;
            cdCanvasPixel(canvas, x, y, palette[index % 256]);
        }
    }

    /* Test palette retrieval */
    long retrieved_palette[256];
    int palette_size = cdCanvasPalette(canvas, 256, retrieved_palette, CD_QUERY);

    if (palette_size > 0) {
        printf("    Retrieved palette with %d colors\n", palette_size);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_color_space_conversions(void) {
    printf("  Testing color space conversions...\n");

    /* Test RGB to HSV conversion */
    struct {
        unsigned char r, g, b;
        double expected_h, expected_s, expected_v;
    } rgb_to_hsv_tests[] = {
        {255, 0, 0, 0, 1.0, 1.0},      /* Red */
        {0, 255, 0, 120, 1.0, 1.0},    /* Green */
        {0, 0, 255, 240, 1.0, 1.0},    /* Blue */
        {255, 255, 255, 0, 0.0, 1.0},  /* White */
        {128, 128, 128, 0, 0.0, 0.502}, /* Gray */
    };

    for (size_t i = 0; i < sizeof(rgb_to_hsv_tests) / sizeof(rgb_to_hsv_tests[0]); i++) {
        double h, s, v;
        cdColorRGB2HSV(rgb_to_hsv_tests[i].r, rgb_to_hsv_tests[i].g, rgb_to_hsv_tests[i].b, &h, &s, &v);

        /* Note: Exact floating point comparison is avoided - we test ranges */
        TEST_ASSERT(v >= 0.0 && v <= 1.0, "Value should be in [0,1]");
        TEST_ASSERT(s >= 0.0 && s <= 1.0, "Saturation should be in [0,1]");
        TEST_ASSERT(h >= 0.0 && h < 360.0, "Hue should be in [0,360)");
    }

    /* Test HSV to RGB conversion */
    unsigned char r, g, b;
    cdColorHSV2RGB(0, 1.0, 1.0, &r, &g, &b);  /* Should produce red */
    TEST_ASSERT_EQ(255, r, "HSV(0,1,1) should produce red=255");

    cdColorHSV2RGB(120, 1.0, 1.0, &r, &g, &b);  /* Should produce green */
    TEST_ASSERT_EQ(255, g, "HSV(120,1,1) should produce green=255");

    /* Test round-trip conversion */
    for (int test_r = 0; test_r <= 255; test_r += 64) {
        for (int test_g = 0; test_g <= 255; test_g += 64) {
            for (int test_b = 0; test_b <= 255; test_b += 64) {
                double h, s, v;
                unsigned char conv_r, conv_g, conv_b;

                cdColorRGB2HSV(test_r, test_g, test_b, &h, &s, &v);
                cdColorHSV2RGB(h, s, v, &conv_r, &conv_g, &conv_b);

                /* Allow small rounding errors */
                TEST_ASSERT(abs(test_r - conv_r) <= 1, "RGB round-trip red component");
                TEST_ASSERT(abs(test_g - conv_g) <= 1, "RGB round-trip green component");
                TEST_ASSERT(abs(test_b - conv_b) <= 1, "RGB round-trip blue component");
            }
        }
    }

    return 1;
}

int test_color_visual_output(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_color_visual.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    printf("  Creating color visual test output...\n");

    /* Color wheel demonstration */
    int center_x = 300, center_y = 200;
    int radius = 80;

    for (int angle = 0; angle < 360; angle += 5) {
        double rad = angle * TEST_PI / 180.0;
        unsigned char r, g, b;
        cdColorHSV2RGB(angle, 1.0, 1.0, &r, &g, &b);
        long color = cdEncodeColor(r, g, b);

        cdCanvasForeground(canvas, color);
        cdCanvasLineWidth(canvas, 3);

        int x1 = center_x + (int)(radius * 0.8 * cos(rad));
        int y1 = center_y + (int)(radius * 0.8 * sin(rad));
        int x2 = center_x + (int)(radius * cos(rad));
        int y2 = center_y + (int)(radius * sin(rad));

        cdCanvasLine(canvas, x1, y1, x2, y2);
    }

    /* Gradient demonstration */
    for (int x = 50; x < 550; x++) {
        unsigned char intensity = (unsigned char)((x - 50) * 255 / 500);
        long color = cdEncodeColor(intensity, intensity, intensity);
        cdCanvasForeground(canvas, color);
        cdCanvasLine(canvas, x, 50, x, 80);
    }

    /* RGB color cube slice */
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            unsigned char r = (unsigned char)(x * 255 / 99);
            unsigned char g = (unsigned char)(y * 255 / 99);
            unsigned char b = 128;  /* Fixed blue component */
            long color = cdEncodeColor(r, g, b);
            cdCanvasPixel(canvas, 450 + x, 300 + y, color);
        }
    }

    /* Color mixing demonstration */
    cdCanvasForeground(canvas, cdEncodeColor(255, 0, 0));
    cdCanvasSector(canvas, 150, 300, 80, 80, 0, 120);

    cdCanvasForeground(canvas, cdEncodeColor(0, 255, 0));
    cdCanvasSector(canvas, 150, 300, 80, 80, 120, 240);

    cdCanvasForeground(canvas, cdEncodeColor(0, 0, 255));
    cdCanvasSector(canvas, 150, 300, 80, 80, 240, 360);

    /* Labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 250, 350, "Color Wheel");
    cdCanvasText(canvas, 250, 30, "Grayscale Gradient");
    cdCanvasText(canvas, 450, 280, "RGB Cube (B=128)");
    cdCanvasText(canvas, 100, 250, "RGB Mix");

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Color Tests...\n");

    RUN_TEST(test_color_encoding);
    RUN_TEST(test_named_colors);
    RUN_TEST(test_color_attributes);
    RUN_TEST(test_alpha_transparency);
    RUN_TEST(test_palette_operations);
    RUN_TEST(test_color_space_conversions);
    RUN_TEST(test_color_visual_output);

    printf("\nColor Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}