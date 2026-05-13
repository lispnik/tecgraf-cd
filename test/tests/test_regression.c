/*
 * CD Library Test Suite - Regression Tests
 * Tests for previously fixed bugs and known issues
 */

#include "test_utils.h"

int test_canvas_creation_edge_cases(void) {
    /* Test for canvas creation with minimal sizes */
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "1x1");
    TEST_ASSERT_NOT_NULL(canvas, "1x1 canvas creation should work");
    if (canvas) {
        cdKillCanvas(canvas);
    }

    /* Test very large canvas creation (should not crash) */
    canvas = cdCreateCanvas(cdContextImageRGB(), "10000x10000");
    if (canvas) {
        cdKillCanvas(canvas);
    }
    /* This may fail due to memory limits, which is acceptable */

    return 1;
}

int test_text_rendering_issues(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 200, "test_text_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test empty string rendering (should not crash) */
    cdCanvasText(canvas, 50, 50, "");

    /* Test very long string */
    char long_string[1000];
    int i;
    for (i = 0; i < 999; i++) {
        long_string[i] = 'A' + (i % 26);
    }
    long_string[999] = '\0';

    cdCanvasText(canvas, 50, 100, long_string);

    /* Test string with special characters */
    cdCanvasText(canvas, 50, 150, "Special: \n\t\r\\\"");

    test_destroy_canvas(canvas);
    return 1;
}

int test_color_handling_regressions(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, "test_color_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test color overflow/underflow */
    long color1 = cdEncodeColor(256, -1, 300);  /* Should clamp values */
    long color2 = cdEncodeColor(255, 0, 255);

    cdCanvasForeground(canvas, color1);
    cdCanvasBox(canvas, 10, 60, 10, 60);

    cdCanvasForeground(canvas, color2);
    cdCanvasBox(canvas, 70, 120, 10, 60);

    /* Test alpha channel edge cases */
    long alpha_color = cdEncodeColorAlpha(128, 128, 128, 0);
    cdCanvasForeground(canvas, alpha_color);
    cdCanvasBox(canvas, 130, 180, 10, 60);

    test_destroy_canvas(canvas);
    return 1;
}

int test_coordinate_precision_issues(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, "test_precision_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test floating point precision near boundaries */
    cdfCanvasLine(canvas, 0.0001, 0.0001, 299.9999, 199.9999);
    cdfCanvasLine(canvas, -0.5, 50.5, 300.5, 50.5);

    /* Test very small floating point increments */
    double x = 50.0;
    int i;
    for (i = 0; i < 100; i++) {
        cdfCanvasPixel(canvas, x, 100.0, CD_BLACK);
        x += 0.01;  /* Very small increment */
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_arc_angle_edge_cases(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_arc_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLUE);

    /* Test arc with zero angle difference */
    cdCanvasArc(canvas, 100, 100, 50, 50, 0, 0);

    /* Test arc with negative angles */
    cdCanvasArc(canvas, 200, 100, 50, 50, -90, -45);

    /* Test arc with angles > 360 */
    cdCanvasArc(canvas, 300, 100, 50, 50, 0, 450);

    /* Test arc with inverted angles */
    cdCanvasArc(canvas, 100, 200, 50, 50, 180, 90);

    test_destroy_canvas(canvas);
    return 1;
}

int test_memory_management_issues(void) {
    /* Test repeated canvas creation/destruction */
    int i;
    for (i = 0; i < 100; i++) {
        cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "100x100");
        if (canvas) {
            cdCanvasActivate(canvas);
            cdCanvasLine(canvas, 0, 0, 99, 99);
            cdKillCanvas(canvas);
        }
    }

    /* Test canvas creation with invalid parameters */
    cdCanvas* invalid1 = cdCreateCanvas(cdContextImageRGB(), "0x0");
    TEST_ASSERT_NULL(invalid1, "0x0 canvas should fail");

    cdCanvas* invalid2 = cdCreateCanvas(cdContextImageRGB(), "invalid");
    TEST_ASSERT_NULL(invalid2, "Invalid size string should fail");

    return 1;
}

int test_attribute_state_issues(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, "test_attribute_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test attribute setting without crashes */
    cdCanvasLineWidth(canvas, 0);    /* Should use minimum valid width */
    cdCanvasLineWidth(canvas, -5);   /* Should handle negative width */
    cdCanvasLineWidth(canvas, 10000); /* Should handle very large width */

    /* Test font with edge cases */
    cdCanvasFont(canvas, "", CD_PLAIN, 12);     /* Empty font name */
    cdCanvasFont(canvas, "NonExistentFont", CD_PLAIN, 12);
    cdCanvasFont(canvas, "Arial", -1, 12);      /* Invalid style */
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 0); /* Zero size */

    /* Draw something to verify state is still valid */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLine(canvas, 10, 10, 100, 100);
    cdCanvasText(canvas, 150, 100, "State Test");

    test_destroy_canvas(canvas);
    return 1;
}

int test_polygon_edge_cases(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, "test_polygon_regression.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test polygon with no vertices */
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasEnd(canvas);

    /* Test polygon with single vertex */
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 50, 50);
    cdCanvasEnd(canvas);

    /* Test polygon with two vertices (line) */
    cdCanvasBegin(canvas, CD_OPEN_LINES);
    cdCanvasVertex(canvas, 100, 50);
    cdCanvasVertex(canvas, 150, 100);
    cdCanvasEnd(canvas);

    /* Test self-intersecting polygon */
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 200, 50);
    cdCanvasVertex(canvas, 250, 150);
    cdCanvasVertex(canvas, 200, 150);
    cdCanvasVertex(canvas, 250, 50);
    cdCanvasEnd(canvas);

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Regression Tests...\n");

    RUN_TEST(test_canvas_creation_edge_cases);
    RUN_TEST(test_text_rendering_issues);
    RUN_TEST(test_color_handling_regressions);
    RUN_TEST(test_coordinate_precision_issues);
    RUN_TEST(test_arc_angle_edge_cases);
    RUN_TEST(test_memory_management_issues);
    RUN_TEST(test_attribute_state_issues);
    RUN_TEST(test_polygon_edge_cases);

    printf("\nRegression Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}