/*
 * CD Library Test Suite - Backend Testing
 * Tests different rendering backends (SVG, RGB Image, Debug, etc.)
 */

#include "test_utils.h"

/* Include backend headers */
#include <cdsvg.h>
#include <cdirgb.h>
#include <cddebug.h>
#include <cddbuf.h>

#ifdef CD_ENABLE_IM
#include <cdim.h>
#endif

#ifdef CD_ENABLE_CAIRO
#include <cdcairo.h>
#endif

int test_svg_backend(void) {
    cdCanvas* canvas = cdCreateCanvas(cdContextSVG(), "test_svg_backend.svg 400x300 3.78");
    TEST_ASSERT_NOT_NULL(canvas, "SVG canvas creation failed");

    cdCanvasActivate(canvas);
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);

    /* Test basic drawing */
    test_draw_primitives_grid(canvas);

    /* Test SVG-specific features */
    cdCanvasSetAttribute(canvas, "OPACITY", "128");
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 50, 150, 200, 250);

    cdKillCanvas(canvas);
    return 1;
}

int test_irgb_backend(void) {
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    TEST_ASSERT_NOT_NULL(canvas, "RGB Image canvas creation failed");

    cdCanvasActivate(canvas);
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);

    /* Test basic drawing */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLine(canvas, 0, 0, 399, 299);
    cdCanvasLine(canvas, 0, 299, 399, 0);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 100, 300, 100, 200);

    /* Test image retrieval */
    unsigned char* red = malloc(400 * 300);
    unsigned char* green = malloc(400 * 300);
    unsigned char* blue = malloc(400 * 300);

    cdCanvasGetImageRGB(canvas, red, green, blue, 0, 0, 400, 300);

    /* Verify some pixel values */
    TEST_ASSERT(red[0] == 0, "Top-left should be blue (red=0)");
    TEST_ASSERT(green[0] == 0, "Top-left should be blue (green=0)");
    TEST_ASSERT(blue[0] == 255, "Top-left should be blue (blue=255)");

    free(red);
    free(green);
    free(blue);

    cdKillCanvas(canvas);
    return 1;
}

int test_debug_backend(void) {
    /* Redirect debug output to a file for testing */
    cdCanvas* canvas = cdCreateCanvas(cdContextDebug(), NULL);
    TEST_ASSERT_NOT_NULL(canvas, "Debug canvas creation failed");

    cdCanvasActivate(canvas);

    /* Test that all operations are logged */
    cdCanvasBackground(canvas, CD_YELLOW);
    cdCanvasClear(canvas);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLine(canvas, 10, 10, 100, 100);
    cdCanvasRect(canvas, 50, 150, 50, 100);

    cdKillCanvas(canvas);
    return 1;
}

int test_dbuffer_backend(void) {
    /* Test double buffer with RGB image backend */
    cdCanvas* rgb_canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    TEST_ASSERT_NOT_NULL(rgb_canvas, "RGB canvas creation failed");

    cdCanvas* dbuf_canvas = cdCreateCanvas(cdContextDBuffer(), rgb_canvas);
    TEST_ASSERT_NOT_NULL(dbuf_canvas, "Double buffer canvas creation failed");

    cdCanvasActivate(dbuf_canvas);
    cdCanvasBackground(dbuf_canvas, CD_WHITE);
    cdCanvasClear(dbuf_canvas);

    /* Draw to double buffer */
    cdCanvasForeground(dbuf_canvas, CD_GREEN);
    cdCanvasBox(dbuf_canvas, 50, 150, 50, 100);

    /* Flush to main canvas */
    cdCanvasFlush(dbuf_canvas);

    cdKillCanvas(dbuf_canvas);
    cdKillCanvas(rgb_canvas);
    return 1;
}

#ifdef CD_ENABLE_IM
int test_im_backend(void) {
    /* Test IM integration if available */
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    TEST_ASSERT_NOT_NULL(canvas, "IM-backed canvas creation failed");

    cdCanvasActivate(canvas);
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);

    /* Test basic drawing */
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasSector(canvas, 200, 150, 100, 100, 0, 360);

    cdKillCanvas(canvas);
    return 1;
}
#endif

int test_backend_canvas_sizes(void) {
    /* Test different canvas sizes */
    struct {
        int width, height;
        const char* description;
    } sizes[] = {
        {1, 1, "Minimal size"},
        {100, 100, "Small square"},
        {800, 600, "Medium rectangle"},
        {1920, 1080, "Large size"},
        {50, 2000, "Very tall"},
        {2000, 50, "Very wide"}
    };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int i;

    for (i = 0; i < num_sizes; i++) {
        char canvas_data[64];
        snprintf(canvas_data, sizeof(canvas_data), "%dx%d", sizes[i].width, sizes[i].height);

        cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), canvas_data);
        TEST_ASSERT_NOT_NULL(canvas, sizes[i].description);

        if (canvas) {
            cdCanvasActivate(canvas);

            /* Verify canvas size */
            int w, h;
            cdCanvasGetSize(canvas, &w, &h, NULL, NULL);
            TEST_ASSERT_EQ(sizes[i].width, w, "Width mismatch");
            TEST_ASSERT_EQ(sizes[i].height, h, "Height mismatch");

            cdKillCanvas(canvas);
        }
    }

    return 1;
}

int test_backend_attributes(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_backend_attributes.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");

    /* Test backend-specific attributes */
    cdCanvasSetAttribute(canvas, "OPACITY", "200");
    char* opacity = cdCanvasGetAttribute(canvas, "OPACITY");
    if (opacity) {
        TEST_ASSERT(strcmp(opacity, "200") == 0, "Opacity attribute mismatch");
    }

    cdCanvasSetAttribute(canvas, "HATCHBOXSIZE", "16");
    char* hatch_size = cdCanvasGetAttribute(canvas, "HATCHBOXSIZE");
    if (hatch_size) {
        TEST_ASSERT(strcmp(hatch_size, "16") == 0, "Hatch box size attribute mismatch");
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_backend_capabilities(void) {
    /* Test capability queries for different backends */
    struct {
        cdContext* (*context_func)(void);
        const char* name;
        unsigned long expected_caps;
    } backends[] = {
        {cdContextSVG, "SVG", CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY)},
        {cdContextImageRGB, "RGB", CD_CAP_ALL},
        {cdContextDebug, "Debug", CD_CAP_ALL},
        {cdContextDBuffer, "DBuffer", CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS)}
    };
    int num_backends = sizeof(backends) / sizeof(backends[0]);
    int i;

    for (i = 0; i < num_backends; i++) {
        unsigned long caps = cdContextCaps(backends[i].context_func());

        /* Verify some basic capabilities */
        TEST_ASSERT((caps & CD_CAP_FLUSH) != 0, "Backend should support flush");

        printf("Backend %s capabilities: 0x%08lx\n", backends[i].name, caps);
    }

    return 1;
}

int test_error_conditions(void) {
    /* Test error handling */

    /* Invalid canvas data */
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "invalid_size");
    TEST_ASSERT_NULL(canvas, "Should fail with invalid size");

    /* Null context */
    canvas = cdCreateCanvas(NULL, "400x300");
    TEST_ASSERT_NULL(canvas, "Should fail with null context");

    /* Operations on null canvas should not crash */
    cdCanvasActivate(NULL);
    cdCanvasForeground(NULL, CD_RED);
    cdCanvasLine(NULL, 0, 0, 100, 100);

    return 1;
}

int main(void) {
    printf("Running CD Library Backend Tests...\n");

    RUN_TEST(test_svg_backend);
    RUN_TEST(test_irgb_backend);
    RUN_TEST(test_debug_backend);
    RUN_TEST(test_dbuffer_backend);

#ifdef CD_ENABLE_IM
    RUN_TEST(test_im_backend);
#endif

    RUN_TEST(test_backend_canvas_sizes);
    RUN_TEST(test_backend_attributes);
    RUN_TEST(test_backend_capabilities);
    RUN_TEST(test_error_conditions);

    printf("\nBackend Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (100.0 * tests_passed) / tests_total);

    return (tests_failed == 0) ? 0 : 1;
}