/*
 * CD Library Test Suite - Edge Cases and Error Conditions
 * Tests boundary conditions, error handling, and unusual input
 */

#include "test_utils.h"

int test_boundary_conditions(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 100, 100, "test_boundaries.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test drawing at canvas boundaries */
    cdCanvasForeground(canvas, CD_RED);

    /* Corners */
    cdCanvasPixel(canvas, 0, 0, CD_BLUE);
    cdCanvasPixel(canvas, 99, 0, CD_GREEN);
    cdCanvasPixel(canvas, 0, 99, CD_YELLOW);
    cdCanvasPixel(canvas, 99, 99, CD_MAGENTA);

    /* Edge lines */
    cdCanvasLine(canvas, 0, 0, 99, 0);     /* Top edge */
    cdCanvasLine(canvas, 0, 99, 99, 99);   /* Bottom edge */
    cdCanvasLine(canvas, 0, 0, 0, 99);     /* Left edge */
    cdCanvasLine(canvas, 99, 0, 99, 99);   /* Right edge */

    /* Outside boundaries (should be clipped) */
    cdCanvasLine(canvas, -10, 50, 110, 50);
    cdCanvasLine(canvas, 50, -10, 50, 110);

    test_destroy_canvas(canvas);
    return 1;
}

int test_zero_dimensions(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 100, 100, "test_zero_dims.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);

    /* Zero-size rectangles */
    cdCanvasRect(canvas, 50, 50, 50, 50);  /* Point rectangle */
    cdCanvasBox(canvas, 60, 60, 60, 60);   /* Point box */

    /* Zero-size arcs */
    cdCanvasArc(canvas, 70, 70, 0, 0, 0, 360);

    /* Lines with same start/end points */
    cdCanvasLine(canvas, 80, 80, 80, 80);

    test_destroy_canvas(canvas);
    return 1;
}

int test_large_coordinates(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_large_coords.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_RED);

    /* Very large coordinates (should be handled gracefully) */
    cdCanvasLine(canvas, 1000000, 1000000, 200, 150);
    cdCanvasLine(canvas, -1000000, -1000000, 300, 200);

    /* Floating point precision tests */
    cdfCanvasLine(canvas, 0.000001, 0.000001, 100.999999, 100.999999);

    test_destroy_canvas(canvas);
    return 1;
}

int test_invalid_parameters(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 200, 200, "test_invalid_params.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Invalid colors (should use defaults) */
    cdCanvasForeground(canvas, -1);
    cdCanvasLine(canvas, 10, 10, 50, 50);

    /* Invalid line widths */
    cdCanvasLineWidth(canvas, -5);  /* Should use default */
    cdCanvasLineWidth(canvas, 0);   /* Should use minimum */

    /* Invalid font sizes */
    cdCanvasFont(canvas, "Arial", CD_PLAIN, -10);  /* Should handle gracefully */
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 0);

    /* Invalid angles */
    cdCanvasArc(canvas, 100, 100, 50, 50, -1000, 2000);

    test_destroy_canvas(canvas);
    return 1;
}

int test_null_parameters(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 200, 200, "test_null_params.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* NULL text */
    cdCanvasText(canvas, 50, 50, NULL);

    /* NULL font name */
    cdCanvasFont(canvas, NULL, CD_PLAIN, 12);

    /* NULL attribute values */
    cdCanvasSetAttribute(canvas, "TEST_ATTR", NULL);

    test_destroy_canvas(canvas);
    return 1;
}

int test_memory_limits(void) {
    /* Test very large image allocations */
    cdCanvas* canvas = test_create_canvas("RGB", 1, 1, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Small canvas creation failed");

    /* Try to create very large images (should fail gracefully) */
    unsigned char* large_data = NULL;

    /* This should not crash, just fail */
    large_data = malloc(100000000);  /* 100MB */
    if (large_data) {
        free(large_data);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_concurrent_access(void) {
    /* Test multiple canvases */
    cdCanvas* canvas1 = test_create_canvas("SVG", 200, 200, "test_concurrent1.svg");
    cdCanvas* canvas2 = test_create_canvas("SVG", 200, 200, "test_concurrent2.svg");

    TEST_ASSERT_NOT_NULL(canvas1, "Canvas 1 creation failed");
    TEST_ASSERT_NOT_NULL(canvas2, "Canvas 2 creation failed");

    /* Draw to both canvases */
    cdCanvasActivate(canvas1);
    cdCanvasForeground(canvas1, CD_RED);
    cdCanvasLine(canvas1, 0, 0, 199, 199);

    cdCanvasActivate(canvas2);
    cdCanvasForeground(canvas2, CD_BLUE);
    cdCanvasLine(canvas2, 199, 0, 0, 199);

    /* Switch back and forth */
    cdCanvasActivate(canvas1);
    cdCanvasRect(canvas1, 50, 150, 50, 150);

    cdCanvasActivate(canvas2);
    cdCanvasBox(canvas2, 50, 150, 50, 150);

    test_destroy_canvas(canvas1);
    test_destroy_canvas(canvas2);
    return 1;
}

int test_state_persistence(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 300, 200, "test_state_persistence.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Set various attributes */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 5);
    cdCanvasFont(canvas, "Helvetica", CD_BOLD, 16);

    /* Save state */
    cdState* state = cdCanvasSaveState(canvas);
    TEST_ASSERT_NOT_NULL(state, "State save failed");

    /* Change attributes */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLineWidth(canvas, 1);
    cdCanvasFont(canvas, "Times", CD_ITALIC, 10);

    /* Draw something */
    cdCanvasLine(canvas, 10, 10, 100, 100);
    cdCanvasText(canvas, 50, 150, "Changed State");

    /* Restore state */
    cdCanvasRestoreState(canvas, state);

    /* Draw something else (should use restored attributes) */
    cdCanvasLine(canvas, 200, 10, 290, 100);
    cdCanvasText(canvas, 200, 150, "Restored State");

    cdReleaseState(state);
    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Edge Cases Tests...\n");

    RUN_TEST(test_boundary_conditions);
    RUN_TEST(test_zero_dimensions);
    RUN_TEST(test_large_coordinates);
    RUN_TEST(test_invalid_parameters);
    RUN_TEST(test_null_parameters);
    RUN_TEST(test_memory_limits);
    RUN_TEST(test_concurrent_access);
    RUN_TEST(test_state_persistence);

    printf("\nEdge Cases Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}