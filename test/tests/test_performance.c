/*
 * CD Library Test Suite - Performance Benchmarking
 * Tests performance of various CD operations and provides benchmarks
 */

#include "test_utils.h"
#include <time.h>
#include <sys/time.h>

/* Performance test helper */
void bench_line_drawing(cdCanvas* canvas) {
    for (int i = 0; i < 1000; i++) {
        int x1 = (i * 37) % 400;
        int y1 = (i * 73) % 300;
        int x2 = ((i + 100) * 41) % 400;
        int y2 = ((i + 100) * 83) % 300;
        cdCanvasLine(canvas, x1, y1, x2, y2);
    }
}

void bench_rectangle_drawing(cdCanvas* canvas) {
    for (int i = 0; i < 500; i++) {
        int x = (i * 23) % 350;
        int y = (i * 29) % 250;
        int w = 20 + (i % 30);
        int h = 15 + (i % 25);
        cdCanvasRect(canvas, x, x + w, y, y + h);
    }
}

void bench_circle_drawing(cdCanvas* canvas) {
    for (int i = 0; i < 200; i++) {
        int x = (i * 31) % 400;
        int y = (i * 37) % 300;
        int r = 5 + (i % 20);
        cdCanvasArc(canvas, x, y, r * 2, r * 2, 0, 360);
    }
}

void bench_text_rendering(cdCanvas* canvas) {
    const char* texts[] = {"Performance", "Test", "Benchmark", "CD", "Graphics"};
    int num_texts = sizeof(texts) / sizeof(texts[0]);

    for (int i = 0; i < 100; i++) {
        int x = (i * 43) % 350;
        int y = (i * 47) % 280;
        cdCanvasText(canvas, x, y, texts[i % num_texts]);
    }
}

void bench_color_operations(cdCanvas* canvas) {
    for (int i = 0; i < 1000; i++) {
        unsigned char r = (i * 7) % 256;
        unsigned char g = (i * 11) % 256;
        unsigned char b = (i * 13) % 256;
        long color = cdEncodeColor(r, g, b);
        cdCanvasForeground(canvas, color);

        /* Quick draw operation */
        cdCanvasPixel(canvas, (i * 17) % 400, (i * 19) % 300, color);
    }
}

void bench_polygon_drawing(cdCanvas* canvas) {
    for (int i = 0; i < 50; i++) {
        int sides = 3 + (i % 7);  /* 3-9 sided polygons */
        int cx = (i * 67) % 350;
        int cy = (i * 71) % 250;
        int radius = 10 + (i % 20);

        cdCanvasBegin(canvas, CD_FILL);
        for (int j = 0; j < sides; j++) {
            double angle = j * 2 * TEST_PI / sides;
            int x = cx + (int)(radius * cos(angle));
            int y = cy + (int)(radius * sin(angle));
            cdCanvasVertex(canvas, x, y);
        }
        cdCanvasEnd(canvas);
    }
}

int test_line_performance(void) {
    printf("  Benchmarking line drawing...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "perf_lines.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    performance_result result = test_measure_performance(
        "Line Drawing", bench_line_drawing, canvas, 5
    );

    printf("    Lines: %.2f ms, %.0f ops/sec\n",
           result.elapsed_time * 1000, result.ops_per_second);

    test_destroy_canvas(canvas);
    return 1;
}

int test_shape_performance(void) {
    printf("  Benchmarking shape drawing...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "perf_shapes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    performance_result rect_result = test_measure_performance(
        "Rectangle Drawing", bench_rectangle_drawing, canvas, 3
    );

    performance_result circle_result = test_measure_performance(
        "Circle Drawing", bench_circle_drawing, canvas, 3
    );

    printf("    Rectangles: %.2f ms, %.0f ops/sec\n",
           rect_result.elapsed_time * 1000, rect_result.ops_per_second);
    printf("    Circles: %.2f ms, %.0f ops/sec\n",
           circle_result.elapsed_time * 1000, circle_result.ops_per_second);

    test_destroy_canvas(canvas);
    return 1;
}

int test_text_performance(void) {
    printf("  Benchmarking text rendering...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "perf_text.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasFont(canvas, "Arial", CD_PLAIN, 12);

    performance_result result = test_measure_performance(
        "Text Rendering", bench_text_rendering, canvas, 3
    );

    printf("    Text: %.2f ms, %.0f ops/sec\n",
           result.elapsed_time * 1000, result.ops_per_second);

    test_destroy_canvas(canvas);
    return 1;
}

int test_color_performance(void) {
    printf("  Benchmarking color operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "perf_colors.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    performance_result result = test_measure_performance(
        "Color Operations", bench_color_operations, canvas, 3
    );

    printf("    Colors: %.2f ms, %.0f ops/sec\n",
           result.elapsed_time * 1000, result.ops_per_second);

    test_destroy_canvas(canvas);
    return 1;
}

int test_complex_shape_performance(void) {
    printf("  Benchmarking complex shapes...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "perf_polygons.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    cdCanvasForeground(canvas, CD_BLUE);

    performance_result result = test_measure_performance(
        "Polygon Drawing", bench_polygon_drawing, canvas, 3
    );

    printf("    Polygons: %.2f ms, %.0f ops/sec\n",
           result.elapsed_time * 1000, result.ops_per_second);

    test_destroy_canvas(canvas);
    return 1;
}

int test_large_dataset_performance(void) {
    printf("  Benchmarking large dataset rendering...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 800, 600, "perf_large_dataset.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* Render a large dataset */
    int total_elements = 0;

    /* Dense grid of points */
    cdCanvasForeground(canvas, CD_RED);
    for (int x = 0; x < 800; x += 4) {
        for (int y = 0; y < 600; y += 4) {
            cdCanvasPixel(canvas, x, y, CD_RED);
            total_elements++;
        }
    }

    /* Grid of small rectangles */
    cdCanvasForeground(canvas, CD_BLUE);
    for (int x = 10; x < 790; x += 20) {
        for (int y = 10; y < 590; y += 20) {
            cdCanvasRect(canvas, x, x + 8, y, y + 8);
            total_elements++;
        }
    }

    /* Scattered circles */
    cdCanvasForeground(canvas, CD_GREEN);
    for (int i = 0; i < 500; i++) {
        int x = (i * 123 + 456) % 800;
        int y = (i * 789 + 234) % 600;
        int r = 3 + (i % 8);
        cdCanvasArc(canvas, x, y, r * 2, r * 2, 0, 360);
        total_elements++;
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    double ops_per_sec = total_elements / elapsed;

    printf("    Large dataset: %d elements in %.2f ms (%.0f elements/sec)\n",
           total_elements, elapsed * 1000, ops_per_sec);

    test_destroy_canvas(canvas);
    return 1;
}

int test_canvas_creation_performance(void) {
    printf("  Benchmarking canvas operations...\n");

    struct timeval start, end;
    const int num_canvases = 100;

    /* Test canvas creation/destruction performance */
    gettimeofday(&start, NULL);

    for (int i = 0; i < num_canvases; i++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "temp_canvas_%d.svg", i);

        cdCanvas* canvas = test_create_canvas("SVG", 200, 150, filename);
        if (canvas) {
            /* Do a minimal operation */
            cdCanvasLine(canvas, 0, 0, 199, 149);
            test_destroy_canvas(canvas);
        }
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    double canvases_per_sec = num_canvases / elapsed;

    printf("    Canvas ops: %d create/destroy in %.2f ms (%.0f/sec)\n",
           num_canvases, elapsed * 1000, canvases_per_sec);

    return 1;
}

int test_backend_performance_comparison(void) {
    printf("  Comparing backend performance...\n");

    const char* backends[] = {"SVG", "RGB", "DEBUG"};
    const char* filenames[] = {"backend_svg.svg", NULL, NULL};
    int num_backends = 3;

    for (int b = 0; b < num_backends; b++) {
        printf("    Testing %s backend: ", backends[b]);

        cdCanvas* canvas = test_create_canvas(backends[b], 300, 200, filenames[b]);
        if (canvas) {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            /* Standard test pattern */
            for (int i = 0; i < 200; i++) {
                cdCanvasLine(canvas, i, 0, 299-i, 199);
                if (i % 20 == 0) {
                    cdCanvasRect(canvas, i, i+15, 10, 25);
                }
                if (i % 50 == 0) {
                    cdCanvasArc(canvas, i+10, 100, 20, 20, 0, 360);
                }
            }

            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

            printf("%.2f ms\n", elapsed * 1000);
            test_destroy_canvas(canvas);
        } else {
            printf("FAILED\n");
        }
    }

    return 1;
}

int test_memory_usage_patterns(void) {
    printf("  Testing memory usage patterns...\n");

    /* Note: This is a basic memory usage test */
    /* Real memory profiling would require external tools */

    test_memory_usage_start();

    /* Create and destroy many small canvases */
    for (int i = 0; i < 50; i++) {
        cdCanvas* canvas = test_create_canvas("RGB", 50, 50, NULL);
        if (canvas) {
            /* Do some drawing */
            for (int j = 0; j < 25; j++) {
                cdCanvasLine(canvas, j, 0, j, 49);
            }
            test_destroy_canvas(canvas);
        }
    }

    test_memory_usage_report("Small canvases");

    /* Create one large canvas */
    cdCanvas* large_canvas = test_create_canvas("RGB", 1000, 800, NULL);
    if (large_canvas) {
        /* Fill with content */
        for (int i = 0; i < 100; i++) {
            int x1 = (i * 123) % 1000;
            int y1 = (i * 456) % 800;
            int x2 = ((i + 50) * 789) % 1000;
            int y2 = ((i + 50) * 234) % 800;
            cdCanvasLine(large_canvas, x1, y1, x2, y2);
        }
        test_destroy_canvas(large_canvas);
    }

    test_memory_usage_report("Large canvas");

    return 1;
}

int main(void) {
    printf("Running CD Library Performance Tests...\n");
    printf("======================================\n");

    /* Run performance benchmarks */
    RUN_TEST(test_line_performance);
    RUN_TEST(test_shape_performance);
    RUN_TEST(test_text_performance);
    RUN_TEST(test_color_performance);
    RUN_TEST(test_complex_shape_performance);
    RUN_TEST(test_large_dataset_performance);
    RUN_TEST(test_canvas_creation_performance);
    RUN_TEST(test_backend_performance_comparison);
    RUN_TEST(test_memory_usage_patterns);

    printf("\nPerformance Tests Summary:\n");
    printf("==========================\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    printf("\nNote: Performance results depend on system capabilities\n");
    printf("and backend implementation. Use for relative comparisons.\n");

    return (tests_failed == 0) ? 0 : 1;
}
