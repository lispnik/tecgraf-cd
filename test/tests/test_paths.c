/*
 * CD Library Test Suite - Path Operations
 * Tests complex paths, Bezier curves, path construction and path operations
 */

#include "test_utils.h"

int test_basic_path_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_basic_paths.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    test_draw_reference_grid(canvas, 600, 500);

    /* Test basic path construction */
    cdCanvasBegin(canvas, CD_PATH);

    /* Path 1: Simple path with moveto and lineto */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 50, 50);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 150, 50);
    cdCanvasVertex(canvas, 150, 150);
    cdCanvasVertex(canvas, 50, 150);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasPathSet(canvas, CD_PATH_STROKE);

    /* Path 2: Filled path */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 200, 50);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 300, 75);
    cdCanvasVertex(canvas, 280, 150);
    cdCanvasVertex(canvas, 220, 130);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasPathSet(canvas, CD_PATH_FILL);

    /* Path 3: Fill and stroke */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 350, 50);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 450, 100);
    cdCanvasVertex(canvas, 400, 150);
    cdCanvasVertex(canvas, 370, 120);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasPathSet(canvas, CD_PATH_FILLSTROKE);

    cdCanvasEnd(canvas);

    /* Add labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);
    cdCanvasText(canvas, 50, 170, "Stroke Only");
    cdCanvasText(canvas, 200, 170, "Fill Only");
    cdCanvasText(canvas, 350, 170, "Fill & Stroke");

    test_destroy_canvas(canvas);
    return 1;
}

int test_bezier_curves(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_bezier_curves.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test Bezier curves using Begin/End */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLineWidth(canvas, 2);

    /* Simple Bezier curve */
    cdCanvasBegin(canvas, CD_BEZIER);
    cdCanvasVertex(canvas, 50, 200);   /* Start point */
    cdCanvasVertex(canvas, 100, 100);  /* Control point 1 */
    cdCanvasVertex(canvas, 200, 100);  /* Control point 2 */
    cdCanvasVertex(canvas, 250, 200);  /* End point */
    cdCanvasEnd(canvas);

    /* Multiple connected Bezier curves */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBegin(canvas, CD_BEZIER);
    cdCanvasVertex(canvas, 300, 200);  /* Start */
    cdCanvasVertex(canvas, 350, 100);  /* Control 1 */
    cdCanvasVertex(canvas, 400, 100);  /* Control 2 */
    cdCanvasVertex(canvas, 450, 200);  /* End = next start */
    cdCanvasVertex(canvas, 500, 300);  /* Control 1 */
    cdCanvasVertex(canvas, 400, 350);  /* Control 2 */
    cdCanvasVertex(canvas, 300, 300);  /* End */
    cdCanvasEnd(canvas);

    /* Complex Bezier shape */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBegin(canvas, CD_BEZIER);
    cdCanvasVertex(canvas, 100, 350);
    cdCanvasVertex(canvas, 150, 300);
    cdCanvasVertex(canvas, 200, 400);
    cdCanvasVertex(canvas, 150, 450);
    cdCanvasVertex(canvas, 100, 400);
    cdCanvasVertex(canvas, 50, 450);
    cdCanvasVertex(canvas, 100, 350);  /* Back to start */
    cdCanvasEnd(canvas);

    /* Add control point visualization */
    cdCanvasForeground(canvas, CD_DARK_GRAY);
    cdCanvasLineStyle(canvas, CD_DOTTED);
    cdCanvasLineWidth(canvas, 1);

    /* Show control lines for first curve */
    cdCanvasLine(canvas, 50, 200, 100, 100);   /* Start to control 1 */
    cdCanvasLine(canvas, 100, 100, 200, 100);  /* Control 1 to control 2 */
    cdCanvasLine(canvas, 200, 100, 250, 200);  /* Control 2 to end */

    /* Mark control points */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasMarkType(canvas, CD_CIRCLE);
    cdCanvasMarkSize(canvas, 4);
    cdCanvasMark(canvas, 100, 100);
    cdCanvasMark(canvas, 200, 100);

    /* Labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineStyle(canvas, CD_CONTINUOUS);
    cdCanvasText(canvas, 50, 50, "Bezier Curve Tests");

    test_destroy_canvas(canvas);
    return 1;
}

int test_complex_paths(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 700, 600, "test_complex_paths.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Complex path with mixed operations */
    cdCanvasBegin(canvas, CD_PATH);

    /* Path 1: Complex shape with curves and lines */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 100, 100);

    /* Add some straight lines */
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 200, 100);
    cdCanvasVertex(canvas, 220, 120);

    /* Add a curve */
    cdCanvasPathSet(canvas, CD_PATH_CURVETO);
    cdCanvasVertex(canvas, 240, 140);  /* Control 1 */
    cdCanvasVertex(canvas, 240, 180);  /* Control 2 */
    cdCanvasVertex(canvas, 200, 200);  /* End point */

    /* More lines */
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 150, 220);
    cdCanvasVertex(canvas, 80, 180);

    /* Another curve back to start */
    cdCanvasPathSet(canvas, CD_PATH_CURVETO);
    cdCanvasVertex(canvas, 60, 160);   /* Control 1 */
    cdCanvasVertex(canvas, 70, 120);   /* Control 2 */
    cdCanvasVertex(canvas, 100, 100);  /* Back to start */

    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasPathSet(canvas, CD_PATH_FILL);

    /* Path 2: Arc in path (if supported) */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 400, 150);

    /* Note: CD_PATH_ARC usage varies by backend */
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 500, 150);
    cdCanvasVertex(canvas, 520, 200);
    cdCanvasVertex(canvas, 480, 250);
    cdCanvasVertex(canvas, 420, 230);

    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasPathSet(canvas, CD_PATH_STROKE);

    cdCanvasEnd(canvas);

    /* Path 3: Multiple subpaths */
    cdCanvasBegin(canvas, CD_PATH);

    /* First subpath */
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 100, 350);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 150, 350);
    cdCanvasVertex(canvas, 150, 400);
    cdCanvasVertex(canvas, 100, 400);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);

    /* Second subpath (hole) */
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 115, 365);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 135, 365);
    cdCanvasVertex(canvas, 135, 385);
    cdCanvasVertex(canvas, 115, 385);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasPathSet(canvas, CD_PATH_FILL);

    cdCanvasEnd(canvas);

    /* Add title */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 16);
    cdCanvasText(canvas, 50, 50, "Complex Path Operations");

    test_destroy_canvas(canvas);
    return 1;
}

int test_path_clipping(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_path_clipping.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create a clipping path */
    cdCanvasBegin(canvas, CD_PATH);

    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 150, 100);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 350, 100);
    cdCanvasVertex(canvas, 400, 200);
    cdCanvasVertex(canvas, 350, 300);
    cdCanvasVertex(canvas, 150, 300);
    cdCanvasVertex(canvas, 100, 200);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasPathSet(canvas, CD_PATH_CLIP);

    cdCanvasEnd(canvas);

    /* Now draw content that should be clipped */
    cdCanvasForeground(canvas, CD_RED);
    for (int i = 0; i < 500; i += 20) {
        cdCanvasLine(canvas, i, 0, i, 400);
    }

    cdCanvasForeground(canvas, CD_BLUE);
    for (int j = 0; j < 400; j += 20) {
        cdCanvasLine(canvas, 0, j, 500, j);
    }

    /* Add some shapes */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 50, 150, 150, 250);

    cdCanvasForeground(canvas, CD_YELLOW);
    cdCanvasSector(canvas, 300, 200, 100, 100, 0, 360);

    test_destroy_canvas(canvas);
    return 1;
}

int test_path_edge_cases(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_path_edge_cases.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test edge cases and error conditions */

    /* Empty path */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_STROKE);
    cdCanvasEnd(canvas);

    /* Path with only moveto */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 100, 100);
    cdCanvasPathSet(canvas, CD_PATH_STROKE);
    cdCanvasEnd(canvas);

    /* Path with single point */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 150, 150);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 150, 150);  /* Same point */
    cdCanvasPathSet(canvas, CD_PATH_STROKE);
    cdCanvasEnd(canvas);

    /* Path with very close points */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 200, 200);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 200, 200);  /* Same */
    cdCanvasVertex(canvas, 201, 200);  /* Very close */
    cdCanvasVertex(canvas, 200, 201);  /* Very close */
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);
    cdCanvasPathSet(canvas, CD_PATH_FILL);
    cdCanvasEnd(canvas);

    /* Bezier with coincident control points */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBegin(canvas, CD_BEZIER);
    cdCanvasVertex(canvas, 300, 300);
    cdCanvasVertex(canvas, 300, 300);  /* Same as start */
    cdCanvasVertex(canvas, 400, 400);  /* Different */
    cdCanvasVertex(canvas, 400, 300);
    cdCanvasEnd(canvas);

    /* Very large coordinates in path */
    cdCanvasBegin(canvas, CD_PATH);
    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 10000, 10000);  /* Off canvas */
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 400, 400);      /* On canvas */
    cdCanvasPathSet(canvas, CD_PATH_STROKE);
    cdCanvasEnd(canvas);

    /* Add informational text */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 50, 50, "Path Edge Cases Test");
    cdCanvasText(canvas, 50, 450, "Tests: empty paths, single points, coincident points");

    test_destroy_canvas(canvas);
    return 1;
}

int test_path_fill_modes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_path_fill_modes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test different fill modes with self-intersecting paths */

    /* Even-odd fill rule */
    cdCanvasFillMode(canvas, CD_EVENODD);
    cdCanvasBegin(canvas, CD_PATH);

    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 100, 100);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 250, 200);
    cdCanvasVertex(canvas, 100, 300);
    cdCanvasVertex(canvas, 250, 100);
    cdCanvasVertex(canvas, 100, 200);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasPathSet(canvas, CD_PATH_FILL);

    cdCanvasEnd(canvas);

    /* Winding fill rule */
    cdCanvasFillMode(canvas, CD_WINDING);
    cdCanvasBegin(canvas, CD_PATH);

    cdCanvasPathSet(canvas, CD_PATH_NEW);
    cdCanvasPathSet(canvas, CD_PATH_MOVETO);
    cdCanvasVertex(canvas, 350, 100);
    cdCanvasPathSet(canvas, CD_PATH_LINETO);
    cdCanvasVertex(canvas, 500, 200);
    cdCanvasVertex(canvas, 350, 300);
    cdCanvasVertex(canvas, 500, 100);
    cdCanvasVertex(canvas, 350, 200);
    cdCanvasPathSet(canvas, CD_PATH_CLOSE);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasPathSet(canvas, CD_PATH_FILL);

    cdCanvasEnd(canvas);

    /* Labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 14);
    cdCanvasText(canvas, 100, 50, "Even-Odd Fill");
    cdCanvasText(canvas, 350, 50, "Winding Fill");

    cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);
    cdCanvasText(canvas, 50, 400, "Self-intersecting star pattern with different fill rules");

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Path Tests...\n");

    RUN_TEST(test_basic_path_operations);
    RUN_TEST(test_bezier_curves);
    RUN_TEST(test_complex_paths);
    RUN_TEST(test_path_clipping);
    RUN_TEST(test_path_edge_cases);
    RUN_TEST(test_path_fill_modes);

    printf("\nPath Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}
