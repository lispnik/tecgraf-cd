/*
 * CD Library Test Suite - Comprehensive Integration Tests
 * Tests complex scenarios combining multiple CD features
 */

#include "test_utils.h"

int test_drawing_application_simulation(void) {
    printf("  Testing drawing application simulation...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 800, 600, "test_drawing_app_simulation.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Simulate a drawing application with multiple tools and layers */

    /* Background layer - gradient */
    for (int y = 0; y < 600; y += 2) {
        unsigned char intensity = (unsigned char)(255 - y * 255 / 600);
        cdCanvasForeground(canvas, cdEncodeColor(intensity, intensity, 255));
        cdCanvasLine(canvas, 0, y, 800, y);
    }

    /* Grid layer */
    cdCanvasForeground(canvas, cdEncodeColor(200, 200, 200));
    cdCanvasLineStyle(canvas, CD_DOTTED);
    for (int x = 50; x < 800; x += 50) {
        cdCanvasLine(canvas, x, 0, x, 600);
    }
    for (int y = 50; y < 600; y += 50) {
        cdCanvasLine(canvas, 0, y, 800, y);
    }

    /* Drawing tools simulation */

    /* 1. Pen tool - freehand drawing */
    cdCanvasLineStyle(canvas, CD_CONTINUOUS);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasForeground(canvas, CD_BLACK);

    /* Simulate pen strokes */
    double pen_path[][2] = {
        {100, 200}, {120, 180}, {150, 190}, {180, 170}, {200, 200},
        {220, 180}, {250, 190}, {280, 170}, {300, 200}
    };

    for (size_t i = 1; i < sizeof(pen_path) / sizeof(pen_path[0]); i++) {
        cdCanvasLine(canvas, (int)pen_path[i-1][0], (int)pen_path[i-1][1],
                           (int)pen_path[i][0], (int)pen_path[i][1]);
    }

    /* 2. Shape tool - geometric shapes */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 2);
    cdCanvasRect(canvas, 400, 500, 100, 200);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasArc(canvas, 600, 150, 120, 120, 0, 360);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 500, 300);
    cdCanvasVertex(canvas, 550, 250);
    cdCanvasVertex(canvas, 600, 300);
    cdCanvasVertex(canvas, 575, 350);
    cdCanvasVertex(canvas, 525, 350);
    cdCanvasEnd(canvas);

    /* 3. Text tool */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 18);
    cdCanvasText(canvas, 100, 400, "Drawing Application Test");

    cdCanvasFont(canvas, "Times", CD_ITALIC, 14);
    cdCanvasText(canvas, 450, 400, "Multiple tools and layers");

    /* 4. Brush tool - thick strokes with transparency */
    cdCanvasLineWidth(canvas, 15);
    cdCanvasOpacity(canvas, 128);
    cdCanvasForeground(canvas, CD_YELLOW);

    /* Brush stroke */
    for (int i = 0; i < 50; i++) {
        double angle = i * 0.2;
        int x1 = 350 + (int)(i * 3);
        int y1 = 450 + (int)(20 * sin(angle));
        int x2 = x1 + 10;
        int y2 = y1;
        cdCanvasLine(canvas, x1, y1, x2, y2);
    }

    cdCanvasOpacity(canvas, 255);  /* Reset opacity */

    test_destroy_canvas(canvas);
    return 1;
}

int test_scientific_plotting(void) {
    printf("  Testing scientific plotting...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 700, 500, "test_scientific_plotting.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create a scientific plot with axes, data points, and labels */

    int plot_left = 80, plot_right = 620;
    int plot_bottom = 80, plot_top = 420;
    int plot_width = plot_right - plot_left;
    int plot_height = plot_top - plot_bottom;

    /* Background */
    cdCanvasForeground(canvas, CD_WHITE);
    cdCanvasBox(canvas, 0, 700, 0, 500);

    /* Plot area background */
    cdCanvasForeground(canvas, cdEncodeColor(248, 248, 248));
    cdCanvasBox(canvas, plot_left, plot_right, plot_bottom, plot_top);

    /* Axes */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 2);
    cdCanvasLine(canvas, plot_left, plot_bottom, plot_right, plot_bottom);  /* X axis */
    cdCanvasLine(canvas, plot_left, plot_bottom, plot_left, plot_top);      /* Y axis */

    /* Grid */
    cdCanvasForeground(canvas, cdEncodeColor(220, 220, 220));
    cdCanvasLineWidth(canvas, 1);

    for (int i = 1; i < 10; i++) {
        int x = plot_left + i * plot_width / 10;
        int y = plot_bottom + i * plot_height / 10;
        cdCanvasLine(canvas, x, plot_bottom, x, plot_top);     /* Vertical grid */
        cdCanvasLine(canvas, plot_left, y, plot_right, y);     /* Horizontal grid */
    }

    /* Data series 1: Sine wave */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLineWidth(canvas, 3);

    for (int i = 0; i < plot_width - 1; i++) {
        double x1 = i * 4.0 * TEST_PI / plot_width;
        double x2 = (i + 1) * 4.0 * TEST_PI / plot_width;
        int y1 = plot_bottom + (int)((sin(x1) + 1) * plot_height / 2);
        int y2 = plot_bottom + (int)((sin(x2) + 1) * plot_height / 2);
        cdCanvasLine(canvas, plot_left + i, y1, plot_left + i + 1, y2);
    }

    /* Data series 2: Cosine wave */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 2);
    cdCanvasLineStyle(canvas, CD_DASHED);

    for (int i = 0; i < plot_width - 1; i++) {
        double x1 = i * 4.0 * TEST_PI / plot_width;
        double x2 = (i + 1) * 4.0 * TEST_PI / plot_width;
        int y1 = plot_bottom + (int)((cos(x1) + 1) * plot_height / 2);
        int y2 = plot_bottom + (int)((cos(x2) + 1) * plot_height / 2);
        cdCanvasLine(canvas, plot_left + i, y1, plot_left + i + 1, y2);
    }

    /* Data points */
    cdCanvasLineStyle(canvas, CD_CONTINUOUS);
    cdCanvasMarkType(canvas, CD_CIRCLE);
    cdCanvasMarkSize(canvas, 6);

    for (int i = 0; i < plot_width; i += 20) {
        double x = i * 4.0 * TEST_PI / plot_width;
        int y_sin = plot_bottom + (int)((sin(x) + 1) * plot_height / 2);
        int y_cos = plot_bottom + (int)((cos(x) + 1) * plot_height / 2);

        cdCanvasForeground(canvas, CD_BLUE);
        cdCanvasMark(canvas, plot_left + i, y_sin);

        cdCanvasForeground(canvas, CD_RED);
        cdCanvasMark(canvas, plot_left + i, y_cos);
    }

    /* Labels and title */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 16);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasText(canvas, 350, 460, "Scientific Plot: Sine and Cosine Functions");

    test_destroy_canvas(canvas);
    return 1;
}

int test_complex_document_rendering(void) {
    printf("  Testing complex document rendering...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 600, 800, "test_complex_document.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Simulate a complex document with multiple elements */

    /* Header */
    cdCanvasForeground(canvas, cdEncodeColor(50, 50, 100));
    cdCanvasBox(canvas, 0, 600, 750, 800);

    cdCanvasForeground(canvas, CD_WHITE);
    cdCanvasFont(canvas, "Arial", CD_BOLD, 24);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasText(canvas, 300, 775, "Technical Report");

    cdCanvasFont(canvas, "Arial", CD_PLAIN, 14);
    cdCanvasText(canvas, 300, 755, "CD Graphics Library Performance Analysis");

    /* Body content with multiple columns */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);
    cdCanvasTextAlignment(canvas, CD_LEFT);

    /* Left column */
    const char* text_lines[] = {
        "Executive Summary",
        "",
        "This document presents a comprehensive",
        "analysis of the CD Graphics Library",
        "performance across multiple backends",
        "and rendering scenarios.",
        "",
        "Key findings include:",
        "• SVG backend provides excellent",
        "  vector output quality",
        "• RGB backend offers fast pixel",
        "  manipulation capabilities",
        "• Cross-platform compatibility",
        "  verified on all target systems"
    };

    int y_pos = 720;
    for (size_t i = 0; i < sizeof(text_lines) / sizeof(text_lines[0]); i++) {
        if (strlen(text_lines[i]) == 0) {
            y_pos -= 6;
        } else {
            if (strstr(text_lines[i], "Executive") != NULL) {
                cdCanvasFont(canvas, "Arial", CD_BOLD, 16);
            } else {
                cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);
            }
            cdCanvasText(canvas, 50, y_pos, text_lines[i]);
            y_pos -= 16;
        }
    }

    /* Simple bar chart */
    int chart_left = 350, chart_right = 550;
    int chart_bottom = 500, chart_top = 700;

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasRect(canvas, chart_left, chart_right, chart_bottom, chart_top);

    /* Chart bars */
    struct {
        const char* label;
        int value;
        long color;
    } chart_data[] = {
        {"SVG", 85, CD_BLUE},
        {"RGB", 95, CD_GREEN},
        {"Debug", 60, CD_YELLOW},
        {"Cairo", 80, CD_RED}
    };

    int bar_width = (chart_right - chart_left - 50) / 4;
    for (int i = 0; i < 4; i++) {
        int x = chart_left + 20 + i * (bar_width + 5);
        int height = chart_data[i].value * (chart_top - chart_bottom - 40) / 100;
        int y = chart_bottom + 20;

        cdCanvasForeground(canvas, chart_data[i].color);
        cdCanvasBox(canvas, x, x + bar_width, y, y + height);

        cdCanvasForeground(canvas, CD_BLACK);
        cdCanvasRect(canvas, x, x + bar_width, y, y + height);

        /* Labels */
        cdCanvasFont(canvas, "Arial", CD_PLAIN, 10);
        cdCanvasTextAlignment(canvas, CD_CENTER);
        cdCanvasText(canvas, x + bar_width/2, chart_bottom + 5, chart_data[i].label);
    }

    /* Footer */
    cdCanvasForeground(canvas, cdEncodeColor(220, 220, 220));
    cdCanvasBox(canvas, 0, 600, 0, 50);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLine(canvas, 0, 50, 600, 50);

    cdCanvasFont(canvas, "Arial", CD_PLAIN, 10);
    cdCanvasTextAlignment(canvas, CD_CENTER);
    cdCanvasText(canvas, 300, 25, "CD Graphics Library Technical Documentation - Page 1 of 1");

    test_destroy_canvas(canvas);
    return 1;
}

int test_stress_scenario(void) {
    printf("  Testing stress scenario with many elements...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 1000, 800, "test_stress_scenario.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* Stress test: many drawing operations */
    int total_operations = 0;

    /* 1. Background pattern */
    for (int x = 0; x < 1000; x += 20) {
        for (int y = 0; y < 800; y += 20) {
            cdCanvasForeground(canvas, cdEncodeColor((x + y) % 255, x % 255, y % 255));
            cdCanvasPixel(canvas, x, y, cdCanvasForeground(canvas, CD_QUERY));
            total_operations++;
        }
    }

    /* 2. Many lines */
    for (int i = 0; i < 1000; i++) {
        int x1 = (i * 17) % 1000;
        int y1 = (i * 23) % 800;
        int x2 = ((i + 500) * 19) % 1000;
        int y2 = ((i + 500) * 29) % 800;

        cdCanvasForeground(canvas, cdEncodeColor(i % 256, (i * 2) % 256, (i * 3) % 256));
        cdCanvasLine(canvas, x1, y1, x2, y2);
        total_operations++;
    }

    /* 3. Many shapes */
    for (int i = 0; i < 500; i++) {
        int x = (i * 47) % 900;
        int y = (i * 53) % 700;
        int size = 10 + (i % 30);

        cdCanvasForeground(canvas, cdEncodeColor((i * 5) % 256, (i * 7) % 256, (i * 11) % 256));

        switch (i % 3) {
            case 0:
                cdCanvasRect(canvas, x, x + size, y, y + size);
                break;
            case 1:
                cdCanvasArc(canvas, x + size/2, y + size/2, size, size, 0, 360);
                break;
            case 2:
                cdCanvasBegin(canvas, CD_FILL);
                cdCanvasVertex(canvas, x, y);
                cdCanvasVertex(canvas, x + size, y);
                cdCanvasVertex(canvas, x + size/2, y + size);
                cdCanvasEnd(canvas);
                break;
        }
        total_operations++;
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("    Stress test: %d operations in %.3f seconds (%.0f ops/sec)\n",
           total_operations, elapsed, total_operations / elapsed);

    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Comprehensive Integration Tests...\n");

    RUN_TEST(test_drawing_application_simulation);
    RUN_TEST(test_scientific_plotting);
    RUN_TEST(test_complex_document_rendering);
    RUN_TEST(test_stress_scenario);

    printf("\nComprehensive Integration Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}