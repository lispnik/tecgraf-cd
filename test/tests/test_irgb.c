/*
 * CD Library Test Suite - RGB Image Backend
 * Tests RGB canvas functionality, pixel manipulation, and image operations
 */

#include "test_utils.h"

int test_rgb_canvas_creation(void) {
    printf("  Testing RGB canvas creation...\n");

    /* Test basic RGB canvas creation */
    cdCanvas* canvas = test_create_canvas("RGB", 320, 240, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "RGB canvas creation should succeed");

    /* Test canvas dimensions */
    int width, height;
    cdCanvasGetSize(canvas, &width, &height, NULL, NULL);
    TEST_ASSERT_EQ(320, width, "Canvas width should match");
    TEST_ASSERT_EQ(240, height, "Canvas height should match");

    test_destroy_canvas(canvas);

    /* Test different sizes */
    int test_sizes[][2] = {
        {100, 100}, {640, 480}, {1, 1}, {1920, 1080}
    };

    for (size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++) {
        canvas = test_create_canvas("RGB", test_sizes[i][0], test_sizes[i][1], NULL);
        if (canvas) {
            cdCanvasGetSize(canvas, &width, &height, NULL, NULL);
            TEST_ASSERT_EQ(test_sizes[i][0], width, "Width should match for test size");
            TEST_ASSERT_EQ(test_sizes[i][1], height, "Height should match for test size");
            test_destroy_canvas(canvas);
        }
    }

    return 1;
}

int test_pixel_operations(void) {
    printf("  Testing pixel operations...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 200, 150, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test individual pixel setting */
    cdCanvasPixel(canvas, 10, 10, CD_RED);
    cdCanvasPixel(canvas, 11, 10, CD_GREEN);
    cdCanvasPixel(canvas, 12, 10, CD_BLUE);

    /* Test pixel pattern */
    for (int x = 50; x < 150; x++) {
        for (int y = 50; y < 100; y++) {
            long color = cdEncodeColor((x - 50) * 255 / 100, (y - 50) * 255 / 50, 128);
            cdCanvasPixel(canvas, x, y, color);
        }
    }

    /* Test line drawing with pixels */
    cdCanvasForeground(canvas, CD_WHITE);
    cdCanvasLine(canvas, 0, 0, 199, 149);
    cdCanvasLine(canvas, 0, 149, 199, 0);

    /* Test boundary pixels */
    cdCanvasPixel(canvas, 0, 0, CD_YELLOW);
    cdCanvasPixel(canvas, 199, 0, CD_YELLOW);
    cdCanvasPixel(canvas, 0, 149, CD_YELLOW);
    cdCanvasPixel(canvas, 199, 149, CD_YELLOW);

    test_destroy_canvas(canvas);
    return 1;
}

int test_image_data_access(void) {
    printf("  Testing image data access...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 100, 100, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Fill canvas with known pattern */
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            unsigned char r = (unsigned char)(x * 255 / 99);
            unsigned char g = (unsigned char)(y * 255 / 99);
            unsigned char b = 128;
            long color = cdEncodeColor(r, g, b);
            cdCanvasPixel(canvas, x, y, color);
        }
    }

    /* Test image data retrieval */
    unsigned char* rgb_data = cdCanvasGetImageRGB(canvas);
    if (rgb_data) {
        /* Verify some pixel values */
        /* RGB data is typically stored as R,G,B,R,G,B... */
        int pixel_index = (50 * 100 + 50) * 3;  /* Middle pixel */
        unsigned char r = rgb_data[pixel_index];
        unsigned char g = rgb_data[pixel_index + 1];
        unsigned char b = rgb_data[pixel_index + 2];

        printf("    Middle pixel RGB: (%d,%d,%d)\n", r, g, b);
        TEST_ASSERT(b == 128, "Blue component should be 128");

        /* Clean up image data */
        free(rgb_data);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_rgb_formats(void) {
    printf("  Testing RGB format variations...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 150, 100, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test different RGB color ranges */
    struct {
        unsigned char r, g, b;
        const char* description;
    } test_colors[] = {
        {0, 0, 0, "Black"},
        {255, 255, 255, "White"},
        {255, 0, 0, "Pure Red"},
        {0, 255, 0, "Pure Green"},
        {0, 0, 255, "Pure Blue"},
        {128, 128, 128, "Mid Gray"},
        {255, 128, 0, "Orange"},
        {128, 0, 128, "Purple"},
    };

    int x_offset = 0;
    for (size_t i = 0; i < sizeof(test_colors) / sizeof(test_colors[0]); i++) {
        long color = cdEncodeColor(test_colors[i].r, test_colors[i].g, test_colors[i].b);
        cdCanvasForeground(canvas, color);
        cdCanvasBox(canvas, x_offset, x_offset + 15, 10, 40);
        x_offset += 18;
    }

    /* Test RGB gradients */
    for (int x = 0; x < 150; x++) {
        /* Red gradient */
        long color = cdEncodeColor(x * 255 / 149, 0, 0);
        cdCanvasPixel(canvas, x, 50, color);

        /* Green gradient */
        color = cdEncodeColor(0, x * 255 / 149, 0);
        cdCanvasPixel(canvas, x, 60, color);

        /* Blue gradient */
        color = cdEncodeColor(0, 0, x * 255 / 149);
        cdCanvasPixel(canvas, x, 70, color);
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_image_operations(void) {
    printf("  Testing image manipulation operations...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 256, 256, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create test pattern */
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            unsigned char intensity = (unsigned char)((x + y) / 2);
            long color = cdEncodeColor(intensity, intensity, intensity);
            cdCanvasPixel(canvas, x, y, color);
        }
    }

    /* Add colored overlay patterns */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 50, 100, 50, 100);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasArc(canvas, 180, 180, 100, 100, 0, 360);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 200, 50);
    cdCanvasVertex(canvas, 240, 100);
    cdCanvasVertex(canvas, 200, 150);
    cdCanvasVertex(canvas, 160, 100);
    cdCanvasEnd(canvas);

    /* Test text overlay */
    cdCanvasForeground(canvas, CD_WHITE);
    cdCanvasText(canvas, 10, 10, "RGB Image Test");

    test_destroy_canvas(canvas);
    return 1;
}

int test_memory_management(void) {
    printf("  Testing memory management...\n");

    /* Test multiple canvas creation/destruction */
    const int num_canvases = 20;
    cdCanvas* canvases[num_canvases];

    /* Create multiple canvases */
    for (int i = 0; i < num_canvases; i++) {
        canvases[i] = test_create_canvas("RGB", 64 + i * 4, 48 + i * 3, NULL);
        if (canvases[i]) {
            /* Draw something in each canvas */
            cdCanvasForeground(canvases[i], cdEncodeColor(i * 10, 128, 255 - i * 10));
            cdCanvasBox(canvases[i], 0, 32, 0, 24);
        }
    }

    /* Destroy all canvases */
    for (int i = 0; i < num_canvases; i++) {
        if (canvases[i]) {
            test_destroy_canvas(canvases[i]);
        }
    }

    /* Test large canvas */
    cdCanvas* large_canvas = test_create_canvas("RGB", 1024, 768, NULL);
    if (large_canvas) {
        printf("    Large canvas (1024x768) created successfully\n");

        /* Fill with pattern to test memory usage */
        for (int y = 0; y < 768; y += 10) {
            for (int x = 0; x < 1024; x += 10) {
                long color = cdEncodeColor((x / 4) % 256, (y / 3) % 256, 128);
                cdCanvasPixel(large_canvas, x, y, color);
            }
        }

        test_destroy_canvas(large_canvas);
    } else {
        printf("    Large canvas creation failed (memory limitations)\n");
    }

    return 1;
}

int test_rgb_performance(void) {
    printf("  Testing RGB backend performance...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 400, 300, NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* Performance test: pixel setting */
    for (int i = 0; i < 10000; i++) {
        int x = i % 400;
        int y = (i / 400) % 300;
        long color = cdEncodeColor((i % 256), ((i * 2) % 256), ((i * 3) % 256));
        cdCanvasPixel(canvas, x, y, color);
    }

    gettimeofday(&end, NULL);
    double pixel_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("    10,000 pixels in %.3f ms (%.0f pixels/sec)\n",
           pixel_time * 1000, 10000.0 / pixel_time);

    /* Performance test: line drawing */
    gettimeofday(&start, NULL);

    for (int i = 0; i < 1000; i++) {
        int x1 = (i * 17) % 400;
        int y1 = (i * 23) % 300;
        int x2 = ((i + 200) * 19) % 400;
        int y2 = ((i + 200) * 29) % 300;
        cdCanvasForeground(canvas, cdEncodeColor(i % 256, (i * 2) % 256, 128));
        cdCanvasLine(canvas, x1, y1, x2, y2);
    }

    gettimeofday(&end, NULL);
    double line_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("    1,000 lines in %.3f ms (%.0f lines/sec)\n",
           line_time * 1000, 1000.0 / line_time);

    test_destroy_canvas(canvas);
    return 1;
}

int test_rgb_edge_cases(void) {
    printf("  Testing RGB edge cases...\n");

    /* Test minimum size canvas */
    cdCanvas* tiny_canvas = test_create_canvas("RGB", 1, 1, NULL);
    if (tiny_canvas) {
        cdCanvasPixel(tiny_canvas, 0, 0, CD_RED);
        test_destroy_canvas(tiny_canvas);
    }

    /* Test zero-size canvas (should fail gracefully) */
    cdCanvas* zero_canvas = test_create_canvas("RGB", 0, 0, NULL);
    TEST_ASSERT_NULL(zero_canvas, "Zero-size canvas should fail");

    /* Test negative size (should fail gracefully) */
    cdCanvas* negative_canvas = test_create_canvas("RGB", -10, -10, NULL);
    TEST_ASSERT_NULL(negative_canvas, "Negative size canvas should fail");

    /* Test out-of-bounds pixel access */
    cdCanvas* canvas = test_create_canvas("RGB", 100, 100, NULL);
    if (canvas) {
        /* These should not crash, but may be ignored */
        cdCanvasPixel(canvas, -1, -1, CD_RED);
        cdCanvasPixel(canvas, 100, 100, CD_GREEN);
        cdCanvasPixel(canvas, 500, 500, CD_BLUE);

        test_destroy_canvas(canvas);
    }

    return 1;
}

int main(void) {
    printf("Running CD Library RGB Image Backend Tests...\n");

    RUN_TEST(test_rgb_canvas_creation);
    RUN_TEST(test_pixel_operations);
    RUN_TEST(test_image_data_access);
    RUN_TEST(test_rgb_formats);
    RUN_TEST(test_image_operations);
    RUN_TEST(test_memory_management);
    RUN_TEST(test_rgb_performance);
    RUN_TEST(test_rgb_edge_cases);

    printf("\nRGB Image Backend Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}