/*
 * CD Library Test Suite - Clipping Operations
 * Tests rectangular clipping, polygon clipping, and region operations
 */

#include "test_utils.h"

int test_rectangular_clipping(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_rectangular_clipping.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Draw background pattern */
    test_draw_reference_grid(canvas, 400, 300);

    /* Test clipping area */
    cdCanvasClipArea(canvas, 100, 300, 50, 200);
    cdCanvasClip(canvas, CD_CLIPAREA);

    /* Draw lines that should be clipped */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 3);

    /* These lines extend outside clip area and should be clipped */
    cdCanvasLine(canvas, 50, 100, 350, 100);   /* Horizontal line */
    cdCanvasLine(canvas, 200, 25, 200, 225);   /* Vertical line */

    /* Draw diagonal lines */
    cdCanvasLine(canvas, 75, 75, 325, 175);
    cdCanvasLine(canvas, 325, 75, 75, 175);

    /* Draw some shapes that should be clipped */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 150, 250, 100, 150);   /* Partially clipped box */
    cdCanvasArc(canvas, 350, 125, 100, 100, 0, 360); /* Partially clipped circle */

    /* Test getting clip area */
    int xmin, xmax, ymin, ymax;
    int result = cdCanvasGetClipArea(canvas, &xmin, &xmax, &ymin, &ymax);
    TEST_ASSERT(result != 0, "GetClipArea should return non-zero");
    TEST_ASSERT_EQ(100, xmin, "Clip xmin should match");
    TEST_ASSERT_EQ(300, xmax, "Clip xmax should match");
    TEST_ASSERT_EQ(50, ymin, "Clip ymin should match");
    TEST_ASSERT_EQ(200, ymax, "Clip ymax should match");

    /* Turn off clipping */
    cdCanvasClip(canvas, CD_CLIPOFF);

    /* Draw something outside the previous clip area */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasText(canvas, 50, 250, "Clipping OFF");
    cdCanvasRect(canvas, 320, 380, 220, 270);

    test_destroy_canvas(canvas);
    return 1;
}

int test_floating_point_clipping(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_fp_clipping.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test floating point clipping area */
    cdfCanvasClipArea(canvas, 75.5, 324.5, 25.5, 274.5);
    cdCanvasClip(canvas, CD_CLIPAREA);

    /* Draw with floating point coordinates */
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdfCanvasLine(canvas, 0.0, 150.0, 400.0, 150.0);
    cdfCanvasLine(canvas, 200.0, 0.0, 200.0, 300.0);

    /* Test floating point shapes */
    cdfCanvasBox(canvas, 50.5, 150.5, 50.5, 100.5);
    cdfCanvasArc(canvas, 300.5, 200.5, 80.5, 80.5, 0, 360);

    /* Test getting floating point clip area */
    double fx1, fx2, fy1, fy2;
    int result = cdfCanvasGetClipArea(canvas, &fx1, &fx2, &fy1, &fy2);
    TEST_ASSERT(result != 0, "GetClipArea should work for floating point");

    test_destroy_canvas(canvas);
    return 1;
}

int test_polygon_clipping(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_polygon_clipping.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    test_draw_reference_grid(canvas, 400, 300);

    /* Create a polygon clipping region */
    cdCanvasBegin(canvas, CD_CLIP);
    cdCanvasVertex(canvas, 100, 50);
    cdCanvasVertex(canvas, 300, 50);
    cdCanvasVertex(canvas, 350, 150);
    cdCanvasVertex(canvas, 300, 250);
    cdCanvasVertex(canvas, 100, 250);
    cdCanvasVertex(canvas, 50, 150);
    cdCanvasEnd(canvas);

    /* Activate polygon clipping */
    cdCanvasClip(canvas, CD_CLIPPOLYGON);

    /* Draw content that should be clipped to the polygon */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 2);

    /* Grid of lines */
    for (int i = 0; i < 400; i += 20) {
        cdCanvasLine(canvas, i, 0, i, 300);
    }
    for (int j = 0; j < 300; j += 20) {
        cdCanvasLine(canvas, 0, j, 400, j);
    }

    /* Some shapes */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 150, 250, 100, 200);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasSector(canvas, 200, 150, 120, 120, 0, 360);

    test_destroy_canvas(canvas);
    return 1;
}

int test_region_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 400, "test_region_operations.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Note: Region operations may not be fully supported in SVG backend */
    /* This test demonstrates the API usage */

    /* Create base region */
    cdCanvasBegin(canvas, CD_REGION);
    cdCanvasBox(canvas, 100, 200, 100, 200);
    cdCanvasEnd(canvas);

    /* Test point in region (if supported) */
    int in_region = cdCanvasIsPointInRegion(canvas, 150, 150);
    printf("Point (150,150) in region: %s\n", in_region ? "YES" : "NO");

    /* Test region offset */
    cdCanvasOffsetRegion(canvas, 50, 50);

    /* Get region bounding box */
    int xmin, xmax, ymin, ymax;
    cdCanvasGetRegionBox(canvas, &xmin, &xmax, &ymin, &ymax);
    printf("Region box: (%d,%d) to (%d,%d)\n", xmin, ymin, xmax, ymax);

    /* Draw visual representation of region testing */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasRect(canvas, xmin, xmax, ymin, ymax);

    /* Test different region combine modes */
    cdCanvasRegionCombineMode(canvas, CD_UNION);
    cdCanvasBegin(canvas, CD_REGION);
    cdCanvasBox(canvas, 250, 350, 150, 250);
    cdCanvasEnd(canvas);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasText(canvas, 50, 50, "Region Operations Test");
    cdCanvasText(canvas, 50, 350, "Note: Full region support depends on backend");

    test_destroy_canvas(canvas);
    return 1;
}

int test_clipping_combinations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_clipping_combinations.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test multiple clipping scenarios */

    /* Scenario 1: Rectangular clip */
    cdCanvasClipArea(canvas, 50, 250, 50, 200);
    cdCanvasClip(canvas, CD_CLIPAREA);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 0, 300, 0, 250);
    cdCanvasText(canvas, 100, 100, "Rect Clip");

    /* Scenario 2: Change clip area */
    cdCanvasClipArea(canvas, 300, 550, 50, 200);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 250, 600, 0, 250);
    cdCanvasText(canvas, 400, 100, "New Rect Clip");

    /* Scenario 3: No clipping */
    cdCanvasClip(canvas, CD_CLIPOFF);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasLine(canvas, 0, 250, 600, 250);
    cdCanvasText(canvas, 300, 300, "No Clipping - Full Canvas");

    /* Scenario 4: Small clip area */
    cdCanvasClipArea(canvas, 200, 400, 350, 450);
    cdCanvasClip(canvas, CD_CLIPAREA);

    cdCanvasForeground(canvas, CD_MAGENTA);
    for (int i = 0; i < 50; i++) {
        cdCanvasLine(canvas, 150 + i * 6, 320, 150 + i * 6, 480);
    }

    cdCanvasText(canvas, 300, 400, "Small Clip Test");

    test_destroy_canvas(canvas);
    return 1;
}

int test_clipping_edge_cases(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_clipping_edge_cases.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test edge cases */

    /* Zero-size clip area */
    cdCanvasClipArea(canvas, 100, 100, 100, 100);
    cdCanvasClip(canvas, CD_CLIPAREA);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLine(canvas, 50, 100, 150, 100);  /* Should not be visible */

    /* Invalid clip area (min > max) */
    cdCanvasClipArea(canvas, 200, 100, 150, 50);  /* Invalid coordinates */

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 75, 225, 25, 175);

    /* Very large clip area */
    cdCanvasClipArea(canvas, -1000, 1000, -1000, 1000);
    cdCanvasClip(canvas, CD_CLIPAREA);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasText(canvas, 200, 200, "Large Clip Area");

    /* Clip area outside canvas */
    cdCanvasClipArea(canvas, 500, 600, 400, 500);

    cdCanvasForeground(canvas, CD_YELLOW);
    cdCanvasBox(canvas, 450, 650, 350, 550);  /* Should not be visible */

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Clipping Tests...\n");

    RUN_TEST(test_rectangular_clipping);
    RUN_TEST(test_floating_point_clipping);
    RUN_TEST(test_polygon_clipping);
    RUN_TEST(test_region_operations);
    RUN_TEST(test_clipping_combinations);
    RUN_TEST(test_clipping_edge_cases);

    printf("\nClipping Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}
