/*
 * CD Library Test Suite - Image Operations
 * Tests image loading, saving, and manipulation functions
 */

#include "test_utils.h"

int test_rgb_image_operations(void) {
    printf("  Testing RGB image operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_rgb_images.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Generate test image data */
    int width = 100, height = 100;
    unsigned char* red = malloc(width * height);
    unsigned char* green = malloc(width * height);
    unsigned char* blue = malloc(width * height);

    /* Create gradient pattern */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            red[idx] = (unsigned char)(x * 255 / width);
            green[idx] = (unsigned char)(y * 255 / height);
            blue[idx] = (unsigned char)((x + y) * 128 / (width + height));
        }
    }

    /* Put RGB image */
    cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 50, 50, width, height, 0, width-1, 0, height-1);

    /* Test floating point version */
    cdfCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 200.5, 50.5, width, height, 0, width-1, 0, height-1);

    /* Test partial image */
    cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 300, 50, 50, 50, 25, 74, 25, 74);

    free(red);
    free(green);
    free(blue);

    test_destroy_canvas(canvas);
    return 1;
}

int test_rgba_image_operations(void) {
    printf("  Testing RGBA image operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_rgba_images.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Generate test image data with alpha */
    int width = 80, height = 80;
    unsigned char* red = malloc(width * height);
    unsigned char* green = malloc(width * height);
    unsigned char* blue = malloc(width * height);
    unsigned char* alpha = malloc(width * height);

    /* Create pattern with varying alpha */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            red[idx] = 255;
            green[idx] = (unsigned char)(x * 255 / width);
            blue[idx] = (unsigned char)(y * 255 / height);

            /* Create checkerboard alpha pattern */
            if ((x / 10 + y / 10) % 2) {
                alpha[idx] = 255;  /* Opaque */
            } else {
                alpha[idx] = 128;  /* Semi-transparent */
            }
        }
    }

    /* Put RGBA image */
    cdCanvasPutImageRectRGBA(canvas, width, height, red, green, blue, alpha, 50, 50, width, height, 0, width-1, 0, height-1);

    /* Test floating point version */
    cdfCanvasPutImageRectRGBA(canvas, width, height, red, green, blue, alpha, 200.5, 50.5, width, height, 0, width-1, 0, height-1);

    free(red);
    free(green);
    free(blue);
    free(alpha);

    test_destroy_canvas(canvas);
    return 1;
}

int test_indexed_image_operations(void) {
    printf("  Testing indexed image operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_indexed_images.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create indexed image data */
    int width = 64, height = 64;
    unsigned char* index = malloc(width * height);
    long colors[16];

    /* Create a rainbow palette */
    for (int i = 0; i < 16; i++) {
        unsigned char r, g, b;
        cdColorHSV2RGB(i * 360.0 / 16, 1.0, 1.0, &r, &g, &b);
        colors[i] = cdEncodeColor(r, g, b);
    }

    /* Create index data with pattern */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            /* Create spiral pattern */
            int dist = (int)(sqrt((x - width/2) * (x - width/2) + (y - height/2) * (y - height/2)));
            index[y * width + x] = (unsigned char)((dist + x + y) % 16);
        }
    }

    /* Put indexed image */
    cdCanvasPutImageRectMap(canvas, width, height, index, colors, 50, 50, width, height, 0, width-1, 0, height-1);

    free(index);
    test_destroy_canvas(canvas);
    return 1;
}

int test_image_scaling_operations(void) {
    printf("  Testing image scaling operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 500, 400, "test_image_scaling.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create small test image */
    int width = 20, height = 20;
    unsigned char* red = malloc(width * height);
    unsigned char* green = malloc(width * height);
    unsigned char* blue = malloc(width * height);

    /* Create checkerboard pattern */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            if ((x / 5 + y / 5) % 2) {
                red[idx] = 255; green[idx] = 0; blue[idx] = 0;
            } else {
                red[idx] = 0; green[idx] = 0; blue[idx] = 255;
            }
        }
    }

    /* Test different scaling */
    cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 50, 50, 40, 40, 0, width-1, 0, height-1);    /* 2x scale */
    cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 120, 50, 80, 80, 0, width-1, 0, height-1);   /* 4x scale */
    cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, 230, 50, 10, 10, 0, width-1, 0, height-1);   /* 0.5x scale */

    /* Add labels */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasText(canvas, 50, 30, "2x");
    cdCanvasText(canvas, 120, 30, "4x");
    cdCanvasText(canvas, 230, 30, "0.5x");

    free(red);
    free(green);
    free(blue);
    test_destroy_canvas(canvas);
    return 1;
}

int test_image_data_retrieval(void) {
    printf("  Testing image data retrieval...\n");

    /* Test with RGB canvas */
    cdCanvas* rgb_canvas = test_create_canvas("RGB", 100, 100, NULL);
    if (rgb_canvas) {
        /* Draw something */
        cdCanvasForeground(rgb_canvas, CD_RED);
        cdCanvasBox(rgb_canvas, 20, 80, 20, 80);

        /* Try to get image data back */
        unsigned char* image_data = cdCanvasGetImageRGB(rgb_canvas);
        if (image_data) {
            printf("    Retrieved RGB image data successfully\n");
            /* Check some pixels */
            int center_pixel = (50 * 100 + 50) * 3;  /* RGB pixel at center */
            printf("    Center pixel RGB: (%d,%d,%d)\n",
                   image_data[center_pixel], image_data[center_pixel+1], image_data[center_pixel+2]);
            free(image_data);
        }

        test_destroy_canvas(rgb_canvas);
    }

    return 1;
}

int test_server_images(void) {
    printf("  Testing server image operations...\n");

    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_server_images.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Create image data */
    int width = 50, height = 50;
    unsigned char* red = malloc(width * height);
    unsigned char* green = malloc(width * height);
    unsigned char* blue = malloc(width * height);

    /* Create gradient */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            red[idx] = (unsigned char)(x * 255 / width);
            green[idx] = (unsigned char)(y * 255 / height);
            blue[idx] = 128;
        }
    }

    /* Create server image */
    cdImage* server_image = cdCanvasCreateImageRGB(canvas, width, height, red, green, blue);
    if (server_image) {
        /* Use server image multiple times */
        cdCanvasPutImageRect(canvas, server_image, 50, 50, width, height, 0, width-1, 0, height-1);
        cdCanvasPutImageRect(canvas, server_image, 150, 50, width/2, height/2, 0, width-1, 0, height-1);
        cdCanvasPutImageRect(canvas, server_image, 250, 50, width*2, height*2, 0, width-1, 0, height-1);

        /* Clean up */
        cdKillImage(server_image);
        printf("    Server image operations completed\n");
    }

    free(red);
    free(green);
    free(blue);
    test_destroy_canvas(canvas);
    return 1;
}

int test_image_performance(void) {
    printf("  Testing image operation performance...\n");

    cdCanvas* canvas = test_create_canvas("RGB", 200, 200, NULL);
    if (!canvas) {
        printf("    RGB canvas not available, skipping image performance test\n");
        return 1;
    }

    /* Create test image */
    int width = 50, height = 50;
    unsigned char* red = malloc(width * height);
    unsigned char* green = malloc(width * height);
    unsigned char* blue = malloc(width * height);

    for (int i = 0; i < width * height; i++) {
        red[i] = i % 256;
        green[i] = (i * 2) % 256;
        blue[i] = (i * 3) % 256;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* Performance test: multiple image puts */
    for (int i = 0; i < 100; i++) {
        int x = (i * 7) % 150;
        int y = (i * 11) % 150;
        cdCanvasPutImageRectRGB(canvas, width, height, red, green, blue, x, y, width, height, 0, width-1, 0, height-1);
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("    100 image operations in %.3f ms (%.0f ops/sec)\n",
           elapsed * 1000, 100.0 / elapsed);

    free(red);
    free(green);
    free(blue);
    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running CD Library Image Tests...\n");

    RUN_TEST(test_rgb_image_operations);
    RUN_TEST(test_rgba_image_operations);
    RUN_TEST(test_indexed_image_operations);
    RUN_TEST(test_image_scaling_operations);
    RUN_TEST(test_image_data_retrieval);
    RUN_TEST(test_server_images);
    RUN_TEST(test_image_performance);

    printf("\nImage Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}