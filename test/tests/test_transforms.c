/*
 * CD Library Test Suite - Coordinate Transformations
 * Tests matrix operations, scaling, rotation, translation, and coordinate mapping
 */

#include "test_utils.h"

int test_basic_transformations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_basic_transforms.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Draw reference coordinate system */
    test_draw_reference_grid(canvas, 500, 400);

    /* Test translation */
    double translate_matrix[6] = {1, 0, 0, 1, 100, 50};
    cdCanvasTransform(canvas, translate_matrix);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasRect(canvas, 0, 50, 0, 30);
    cdCanvasText(canvas, 25, 15, "Translated");

    /* Reset transformation */
    cdCanvasTransform(canvas, NULL);

    /* Test scaling */
    double scale_matrix[6] = {2.0, 0, 0, 1.5, 0, 0};
    cdCanvasTransform(canvas, scale_matrix);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, 10, 60, 80, 120);
    cdCanvasText(canvas, 35, 100, "Scaled 2x1.5");

    /* Reset transformation */
    cdCanvasTransform(canvas, NULL);

    /* Test rotation (45 degrees) */
    double cos45 = 0.707107;
    double sin45 = 0.707107;
    double rotate_matrix[6] = {cos45, sin45, -sin45, cos45, 200, 200};
    cdCanvasTransform(canvas, rotate_matrix);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasRect(canvas, -25, 25, -15, 15);
    cdCanvasText(canvas, -20, 0, "Rotated 45°");

    /* Reset transformation */
    cdCanvasTransform(canvas, NULL);

    test_destroy_canvas(canvas);
    return 1;
}

int test_transformation_functions(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_transform_functions.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test individual transformation functions */

    /* 1. Translation */
    cdCanvasTransformTranslate(canvas, 100, 100);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 0, 40, 0, 30);
    cdCanvasText(canvas, 5, 15, "Translate");

    /* 2. Scale (multiply with existing transform) */
    cdCanvasTransformScale(canvas, 1.5, 2.0);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 50, 90, 0, 30);
    cdCanvasText(canvas, 55, 15, "Scale");

    /* 3. Rotation (multiply with existing transform) */
    cdCanvasTransformRotate(canvas, 30); /* 30 degrees */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 100, 140, 0, 30);
    cdCanvasText(canvas, 105, 15, "Rotate");

    /* Reset all transformations */
    cdCanvasTransform(canvas, NULL);

    /* Test composite transformation using multiply */
    double base_matrix[6] = {1, 0, 0, 1, 300, 200};
    cdCanvasTransform(canvas, base_matrix);

    double rotation[6] = {0.866, 0.5, -0.5, 0.866, 0, 0}; /* 30° rotation */
    cdCanvasTransformMultiply(canvas, rotation);

    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasBox(canvas, -30, 30, -20, 20);
    cdCanvasText(canvas, -25, 0, "Composite");

    test_destroy_canvas(canvas);
    return 1;
}

int test_coordinate_mapping(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_coordinate_mapping.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test coordinate transformation */
    double transform[6] = {2, 0, 0, 2, 50, 50}; /* Scale 2x + translate */
    cdCanvasTransform(canvas, transform);

    /* Draw some points and test coordinate mapping */
    int test_points[][2] = {{0, 0}, {50, 25}, {100, 50}, {25, 75}};
    int num_points = sizeof(test_points) / sizeof(test_points[0]);

    for (int i = 0; i < num_points; i++) {
        int original_x = test_points[i][0];
        int original_y = test_points[i][1];

        /* Transform the point */
        int transformed_x, transformed_y;
        cdCanvasTransformPoint(canvas, original_x, original_y, &transformed_x, &transformed_y);

        /* Draw original position (small red circle) */
        cdCanvasForeground(canvas, CD_RED);
        cdCanvasMarkType(canvas, CD_CIRCLE);
        cdCanvasMarkSize(canvas, 3);
        cdCanvasMark(canvas, original_x, original_y);

        /* Draw transformed position (blue square) */
        cdCanvasForeground(canvas, CD_BLUE);
        cdCanvasMarkType(canvas, CD_BOX);
        cdCanvasMarkSize(canvas, 5);

        /* Note: Mark is also affected by transformation, so we see the result */
        cdCanvasMark(canvas, transformed_x, transformed_y);

        /* Label the transformation */
        char label[64];
        snprintf(label, sizeof(label), "(%d,%d)->(%d,%d)",
                original_x, original_y, transformed_x, transformed_y);
        cdCanvasText(canvas, original_x, original_y + 10, label);
    }

    /* Test floating point coordinate mapping */
    double fx = 25.5, fy = 37.5;
    double ftx, fty;
    cdfCanvasTransformPoint(canvas, fx, fy, &ftx, &fty);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasText(canvas, 25, 150, "Float transform test");

    test_destroy_canvas(canvas);
    return 1;
}

int test_matrix_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 600, 500, "test_matrix_operations.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test getting current transformation matrix */
    double* matrix = cdCanvasGetTransform(canvas);
    TEST_ASSERT_NOT_NULL(matrix, "Should get identity matrix initially");

    /* Verify identity matrix */
    if (matrix) {
        TEST_ASSERT(matrix[0] == 1.0, "Identity matrix m11 should be 1");
        TEST_ASSERT(matrix[1] == 0.0, "Identity matrix m12 should be 0");
        TEST_ASSERT(matrix[2] == 0.0, "Identity matrix m21 should be 0");
        TEST_ASSERT(matrix[3] == 1.0, "Identity matrix m22 should be 1");
        TEST_ASSERT(matrix[4] == 0.0, "Identity matrix dx should be 0");
        TEST_ASSERT(matrix[5] == 0.0, "Identity matrix dy should be 0");
    }

    /* Set a custom transformation */
    double custom_matrix[6] = {1.5, 0.2, -0.1, 1.2, 100, 50};
    cdCanvasTransform(canvas, custom_matrix);

    /* Get the transformation back */
    matrix = cdCanvasGetTransform(canvas);
    if (matrix) {
        /* Verify we get back what we set (within tolerance) */
        double tolerance = 0.001;
        TEST_ASSERT(fabs(matrix[0] - 1.5) < tolerance, "Matrix m11 should match");
        TEST_ASSERT(fabs(matrix[4] - 100.0) < tolerance, "Matrix dx should match");
    }

    /* Test multiple matrix operations */
    double rotation[6] = {0.866, 0.5, -0.5, 0.866, 0, 0}; /* 30° rotation */
    cdCanvasTransformMultiply(canvas, rotation);

    /* Draw test pattern with complex transformation */
    cdCanvasForeground(canvas, CD_BLUE);
    for (int i = 0; i < 5; i++) {
        cdCanvasLine(canvas, i * 20, 0, i * 20, 60);
        cdCanvasLine(canvas, 0, i * 15, 80, i * 15);
    }

    cdCanvasText(canvas, 10, 30, "Complex Matrix");

    test_destroy_canvas(canvas);
    return 1;
}

int test_transformation_combinations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 700, 600, "test_transform_combinations.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test various combinations of transformations */

    /* Combination 1: Scale then translate */
    cdCanvasTransformScale(canvas, 1.5, 1.5);
    cdCanvasTransformTranslate(canvas, 100, 100);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasRect(canvas, 0, 50, 0, 40);
    cdCanvasText(canvas, 5, 20, "Scale+Translate");

    /* Reset and try different order */
    cdCanvasTransform(canvas, NULL);

    /* Combination 2: Translate then scale */
    cdCanvasTransformTranslate(canvas, 100, 100);
    cdCanvasTransformScale(canvas, 1.5, 1.5);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, 200, 250, 0, 40);
    cdCanvasText(canvas, 205, 20, "Translate+Scale");

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Combination 3: Complex transformation chain */
    cdCanvasTransformTranslate(canvas, 350, 300);
    cdCanvasTransformRotate(canvas, 45);
    cdCanvasTransformScale(canvas, 0.8, 1.2);
    cdCanvasTransformTranslate(canvas, -25, -25);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 0, 50, 0, 50);
    cdCanvasText(canvas, 5, 25, "Multi-step");

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Test transformation with different primitives */
    double test_matrix[6] = {1.2, 0.3, -0.2, 1.1, 400, 150};
    cdCanvasTransform(canvas, test_matrix);

    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasLine(canvas, 0, 0, 80, 60);
    cdCanvasArc(canvas, 40, 30, 60, 40, 0, 360);
    cdCanvasSector(canvas, 100, 80, 50, 50, 45, 135);

    /* Test text with transformation */
    cdCanvasText(canvas, 20, 100, "Transformed Text");

    test_destroy_canvas(canvas);
    return 1;
}

int test_transformation_edge_cases(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_transform_edge_cases.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test edge cases and error conditions */

    /* Zero scale transformation */
    double zero_scale[6] = {0, 0, 0, 0, 100, 100};
    cdCanvasTransform(canvas, zero_scale);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasRect(canvas, 10, 60, 10, 40);  /* Should be invisible or error */

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Very large scale */
    cdCanvasTransformScale(canvas, 100.0, 100.0);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasPixel(canvas, 1, 1, CD_BLUE);  /* Should be very large */

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Very small scale */
    cdCanvasTransformScale(canvas, 0.01, 0.01);
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 100, 200, 100, 200);  /* Should be very small */

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Negative scale (flip) */
    double flip_matrix[6] = {-1, 0, 0, -1, 250, 200};
    cdCanvasTransform(canvas, flip_matrix);

    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasText(canvas, -100, -50, "Flipped Text");
    cdCanvasRect(canvas, -50, 0, -25, 0);

    /* Reset */
    cdCanvasTransform(canvas, NULL);

    /* Very large translation */
    cdCanvasTransformTranslate(canvas, 10000, -10000);
    cdCanvasForeground(canvas, CD_YELLOW);
    cdCanvasBox(canvas, 0, 50, 0, 50);  /* Should be off-canvas */

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Transformation Tests...\n");

    RUN_TEST(test_basic_transformations);
    RUN_TEST(test_transformation_functions);
    RUN_TEST(test_coordinate_mapping);
    RUN_TEST(test_matrix_operations);
    RUN_TEST(test_transformation_combinations);
    RUN_TEST(test_transformation_edge_cases);

    printf("\nTransformation Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}
