/*
 * CD Library Test Suite - Drawing Primitives
 * Tests all basic drawing functions: lines, rectangles, arcs, polygons, etc.
 */

#include "test_utils.h"

/* Test individual primitive functions */
int test_line_drawing(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_lines.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 1);

    /* Horizontal line */
    cdCanvasLine(canvas, 10, 10, 100, 10);

    /* Vertical line */
    cdCanvasLine(canvas, 10, 20, 10, 100);

    /* Diagonal lines */
    cdCanvasLine(canvas, 20, 20, 80, 80);
    cdCanvasLine(canvas, 80, 20, 20, 80);

    /* Test floating point version */
    cdfCanvasLine(canvas, 150.5, 50.5, 200.5, 100.5);

    test_destroy_canvas(canvas);
    return 1;
}

int test_rectangle_drawing(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_rectangles.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLUE);

    /* Outline rectangle */
    cdCanvasRect(canvas, 10, 100, 10, 50);

    /* Filled rectangle */
    cdCanvasBox(canvas, 120, 200, 10, 50);

    /* Test floating point version */
    cdfCanvasRect(canvas, 220.5, 300.5, 10.5, 50.5);
    cdfCanvasBox(canvas, 220.5, 300.5, 70.5, 110.5);

    test_destroy_canvas(canvas);
    return 1;
}

int test_arc_drawing(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_arcs.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_RED);

    /* Full circle */
    cdCanvasArc(canvas, 100, 100, 80, 80, 0, 360);

    /* Semicircle */
    cdCanvasArc(canvas, 200, 100, 60, 60, 0, 180);

    /* Quarter arc */
    cdCanvasArc(canvas, 300, 100, 40, 40, 45, 135);

    /* Elliptical arc */
    cdCanvasArc(canvas, 150, 200, 120, 60, 30, 150);

    /* Test floating point version */
    cdfCanvasArc(canvas, 300.5, 200.5, 50.5, 30.5, 45.0, 270.0);

    test_destroy_canvas(canvas);
    return 1;
}

int test_sector_drawing(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_sectors.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_GREEN);

    /* Full circle sector */
    cdCanvasSector(canvas, 100, 100, 80, 80, 0, 360);

    /* Pie slice */
    cdCanvasSector(canvas, 200, 100, 60, 60, 0, 90);

    /* Multiple pie slices */
    cdCanvasForeground(canvas, CD_YELLOW);
    cdCanvasSector(canvas, 300, 100, 60, 60, 90, 180);

    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasSector(canvas, 300, 100, 60, 60, 180, 270);

    /* Test floating point version */
    cdfCanvasSector(canvas, 200.5, 200.5, 80.5, 80.5, 45.0, 135.0);

    test_destroy_canvas(canvas);
    return 1;
}

int test_chord_drawing(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_chords.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_CYAN);

    /* Simple chord */
    cdCanvasChord(canvas, 100, 100, 80, 80, 30, 150);

    /* Multiple chords */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasChord(canvas, 200, 100, 60, 60, 0, 60);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasChord(canvas, 200, 100, 60, 60, 120, 180);

    /* Elliptical chord */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasChord(canvas, 300, 150, 100, 50, 45, 135);

    test_destroy_canvas(canvas);
    return 1;
}

int test_pixel_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_pixels.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    int x, y;

    /* Draw individual pixels */
    for (x = 10; x < 100; x += 5) {
        for (y = 10; y < 100; y += 5) {
            long color = cdEncodeColor((x * 255) / 100, (y * 255) / 100, 128);
            cdCanvasPixel(canvas, x, y, color);
        }
    }

    /* Test floating point pixels */
    for (x = 110; x < 200; x += 5) {
        for (y = 10; y < 100; y += 5) {
            long color = cdEncodeColor(255 - (x * 255) / 200, (y * 255) / 100, 200);
            cdfCanvasPixel(canvas, x + 0.5, y + 0.5, color);
        }
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_mark_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_marks.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    int mark_types[] = {CD_PLUS, CD_STAR, CD_CIRCLE, CD_X, CD_BOX, CD_DIAMOND,
                       CD_HOLLOW_CIRCLE, CD_HOLLOW_BOX, CD_HOLLOW_DIAMOND};
    const char* mark_names[] = {"PLUS", "STAR", "CIRCLE", "X", "BOX", "DIAMOND",
                               "HOLLOW_CIRCLE", "HOLLOW_BOX", "HOLLOW_DIAMOND"};
    int num_marks = sizeof(mark_types) / sizeof(mark_types[0]);
    int i;

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 10);

    for (i = 0; i < num_marks; i++) {
        int x = 50 + (i % 3) * 120;
        int y = 50 + (i / 3) * 80;

        cdCanvasMarkType(canvas, mark_types[i]);
        cdCanvasMarkSize(canvas, 15);
        cdCanvasMark(canvas, x, y);

        /* Label the mark */
        cdCanvasTextAlignment(canvas, CD_CENTER);
        cdCanvasText(canvas, x, y - 30, mark_names[i]);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_polygon_operations(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_polygons.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test polygon using Begin/Vertex/End */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 50, 50);
    cdCanvasVertex(canvas, 100, 50);
    cdCanvasVertex(canvas, 125, 100);
    cdCanvasVertex(canvas, 100, 150);
    cdCanvasVertex(canvas, 50, 150);
    cdCanvasVertex(canvas, 25, 100);
    cdCanvasEnd(canvas);

    /* Test open polyline */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasBegin(canvas, CD_OPEN_LINES);
    cdCanvasVertex(canvas, 200, 50);
    cdCanvasVertex(canvas, 250, 75);
    cdCanvasVertex(canvas, 275, 125);
    cdCanvasVertex(canvas, 225, 150);
    cdCanvasVertex(canvas, 175, 125);
    cdCanvasEnd(canvas);

    /* Test closed polyline */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBegin(canvas, CD_CLOSED_LINES);
    cdCanvasVertex(canvas, 300, 50);
    cdCanvasVertex(canvas, 350, 50);
    cdCanvasVertex(canvas, 375, 100);
    cdCanvasVertex(canvas, 350, 150);
    cdCanvasVertex(canvas, 300, 150);
    cdCanvasVertex(canvas, 275, 100);
    cdCanvasEnd(canvas);

    test_destroy_canvas(canvas);
    return 1;
}

int test_comprehensive_primitives(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 800, 600, "test_primitives_comprehensive.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Draw comprehensive test with all primitives */
    test_draw_primitives_grid(canvas);

    /* Add additional complex shapes */
    cdCanvasForeground(canvas, CD_DARK_BLUE);
    cdCanvasLineWidth(canvas, 2);

    /* Star shape using lines */
    int star_x = 650, star_y = 400;
    int i;
    for (i = 0; i < 5; i++) {
        double angle1 = i * 72 * TEST_DEG2RAD;
        double angle2 = (i + 2) * 72 * TEST_DEG2RAD;
        int x1 = star_x + (int)(40 * cos(angle1));
        int y1 = star_y + (int)(40 * sin(angle1));
        int x2 = star_x + (int)(40 * cos(angle2));
        int y2 = star_y + (int)(40 * sin(angle2));
        cdCanvasLine(canvas, x1, y1, x2, y2);
    }

    test_destroy_canvas(canvas);
    return 1;
}

/* Main test runner for primitives */
int main(void) {
    printf("Running CD Library Primitives Tests...\n");

    RUN_TEST(test_line_drawing);
    RUN_TEST(test_rectangle_drawing);
    RUN_TEST(test_arc_drawing);
    RUN_TEST(test_sector_drawing);
    RUN_TEST(test_chord_drawing);
    RUN_TEST(test_pixel_operations);
    RUN_TEST(test_mark_operations);
    RUN_TEST(test_polygon_operations);
    RUN_TEST(test_comprehensive_primitives);

    printf("\nPrimitives Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}