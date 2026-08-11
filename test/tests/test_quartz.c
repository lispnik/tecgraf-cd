/* Tests for the macOS Quartz driver.
 *
 * Everything here runs headless against the offscreen Quartz bitmap canvas
 * and verifies the rendered pixels, so it is safe in CI. Where the expected
 * result is a plain axis aligned fill, the output is also compared against
 * the ImageRGB driver, which is an independent implementation of the same
 * CD semantics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cd.h"
#include "cdquartz.h"
#include "cdirgb.h"
#include "cdimage.h"
#include "cddbuf.h"
#include "cdclipbd.h"
#include "test_utils.h"

#define CANVAS_W 200
#define CANVAS_H 150

static cdCanvas* create_quartz(void)
{
    char data[64];
    cdCanvas* canvas;

    sprintf(data, "%dx%d", CANVAS_W, CANVAS_H);
    canvas = cdCreateCanvas(cdContextQuartzBitmap(), data);

    if (canvas) {
        /* exact pixel comparisons need antialiasing out of the way */
        cdCanvasSetAttribute(canvas, "ANTIALIAS", "0");
        cdCanvasBackground(canvas, CD_WHITE);
        cdCanvasClear(canvas);
    }

    return canvas;
}

static long get_pixel(cdCanvas* canvas, int x, int y)
{
    unsigned char r = 0, g = 0, b = 0;
    cdCanvasGetImageRGB(canvas, &r, &g, &b, x, y, 1, 1);
    return cdEncodeColor(r, g, b);
}

static int same_color(long a, long b, int tolerance)
{
    int dr = (int)cdRed(a)   - (int)cdRed(b);
    int dg = (int)cdGreen(a) - (int)cdGreen(b);
    int db = (int)cdBlue(a)  - (int)cdBlue(b);

    if (dr < 0) dr = -dr;
    if (dg < 0) dg = -dg;
    if (db < 0) db = -db;

    return dr <= tolerance && dg <= tolerance && db <= tolerance;
}

static int check_pixel(cdCanvas* canvas, int x, int y, long expected, const char* what)
{
    long got = get_pixel(canvas, x, y);

    if (!same_color(got, expected, 2)) {
        printf("FAIL: %s at (%d,%d): expected %02X%02X%02X, got %02X%02X%02X\n",
               what, x, y,
               cdRed(expected), cdGreen(expected), cdBlue(expected),
               cdRed(got), cdGreen(got), cdBlue(got));
        return 0;
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static int test_canvas_creation(void)
{
    cdCanvas* canvas = create_quartz();
    int w = 0, h = 0;
    double w_mm = 0, h_mm = 0;

    TEST_ASSERT_NOT_NULL(canvas, "bitmap canvas creation failed");

    cdCanvasGetSize(canvas, &w, &h, &w_mm, &h_mm);
    TEST_ASSERT_EQ(CANVAS_W, w, "canvas width");
    TEST_ASSERT_EQ(CANVAS_H, h, "canvas height");
    TEST_ASSERT(w_mm > 0 && h_mm > 0, "canvas size in mm should be set");

    cdKillCanvas(canvas);
    return 1;
}

static int test_clear_and_readback(void)
{
    cdCanvas* canvas = create_quartz();

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    TEST_ASSERT(check_pixel(canvas, 0, 0, CD_WHITE, "clear"), "clear bottom left");
    TEST_ASSERT(check_pixel(canvas, CANVAS_W-1, CANVAS_H-1, CD_WHITE, "clear"), "clear top right");

    cdCanvasBackground(canvas, CD_BLUE);
    cdCanvasClear(canvas);
    TEST_ASSERT(check_pixel(canvas, 100, 75, CD_BLUE, "clear"), "clear to blue");

    cdKillCanvas(canvas);
    return 1;
}

/* Draws the same fills into Quartz and into the ImageRGB driver and requires
   the results to agree pixel for pixel. This pins down the Y axis direction,
   the inclusive box limits and the color handling all at once. */
static int test_matches_imagergb(void)
{
    cdCanvas* quartz = create_quartz();
    cdCanvas* irgb;
    char data[64];
    int x, y, mismatches = 0;

    TEST_ASSERT_NOT_NULL(quartz, "quartz canvas creation failed");

    sprintf(data, "%dx%d", CANVAS_W, CANVAS_H);
    irgb = cdCreateCanvas(cdContextImageRGB(), data);
    TEST_ASSERT_NOT_NULL(irgb, "imagergb canvas creation failed");

    cdCanvasBackground(irgb, CD_WHITE);
    cdCanvasClear(irgb);

    {
        cdCanvas* canvases[2];
        int i;

        canvases[0] = quartz;
        canvases[1] = irgb;

        for (i = 0; i < 2; i++) {
            cdCanvas* c = canvases[i];

            /* a band along the bottom edge: catches an inverted Y axis */
            cdCanvasForeground(c, CD_RED);
            cdCanvasBox(c, 0, CANVAS_W-1, 0, 9);

            /* a band along the left edge */
            cdCanvasForeground(c, CD_GREEN);
            cdCanvasBox(c, 0, 9, 0, CANVAS_H-1);

            /* an interior square */
            cdCanvasForeground(c, CD_BLUE);
            cdCanvasBox(c, 50, 99, 40, 89);

            /* single pixels */
            cdCanvasPixel(c, 150, 120, CD_YELLOW);
            cdCanvasPixel(c, 151, 120, CD_MAGENTA);
        }
    }

    for (y = 0; y < CANVAS_H; y++) {
        for (x = 0; x < CANVAS_W; x++) {
            long a = get_pixel(quartz, x, y);
            long b = get_pixel(irgb, x, y);

            if (!same_color(a, b, 2)) {
                if (mismatches < 5)
                    printf("       mismatch at (%d,%d): quartz %02X%02X%02X vs irgb %02X%02X%02X\n",
                           x, y, cdRed(a), cdGreen(a), cdBlue(a), cdRed(b), cdGreen(b), cdBlue(b));
                mismatches++;
            }
        }
    }

    cdKillCanvas(quartz);
    cdKillCanvas(irgb);

    if (mismatches) {
        printf("FAIL: %s - %d pixels differ from the ImageRGB driver\n", __func__, mismatches);
        return 0;
    }

    return 1;
}

/* Pins down the Y axis direction and the angle convention without relying on
   another driver: CD text grows upwards from a SOUTH_WEST anchor, and a
   0..90 degree sector covers the quadrant up and to the right of its center. */
static int test_orientation(void)
{
    cdCanvas* canvas = create_quartz();
    int x, y, above = 0, below = 0;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 24);
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasTextAlignment(canvas, CD_SOUTH_WEST);
    cdCanvasText(canvas, 20, 75, "Ay");

    for (y = 40; y < 75; y++)
        for (x = 20; x < 90; x++)
            if (get_pixel(canvas, x, y) != CD_WHITE)
                below++;

    for (y = 75; y < 110; y++)
        for (x = 20; x < 90; x++)
            if (get_pixel(canvas, x, y) != CD_WHITE)
                above++;

    TEST_ASSERT(above > 20, "text should extend above a SOUTH_WEST anchor");
    TEST_ASSERT(below == 0, "text should not extend below a SOUTH_WEST anchor");

    /* first quadrant sector */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasSector(canvas, 140, 60, 80, 80, 0, 90);

    TEST_ASSERT(check_pixel(canvas, 155, 75, CD_RED, "up and right of the center"),
                "0..90 degrees covers the first quadrant");
    TEST_ASSERT(check_pixel(canvas, 125, 45, CD_WHITE, "down and left of the center"),
                "0..90 degrees leaves the third quadrant alone");
    TEST_ASSERT(check_pixel(canvas, 155, 45, CD_WHITE, "down and right of the center"),
                "0..90 degrees leaves the fourth quadrant alone");

    cdKillCanvas(canvas);
    return 1;
}

static int test_line_and_arc(void)
{
    cdCanvas* canvas = create_quartz();

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLine(canvas, 10, 50, 190, 50);

    TEST_ASSERT(check_pixel(canvas, 100, 50, CD_BLACK, "horizontal line"), "line drawn");
    TEST_ASSERT(check_pixel(canvas, 100, 60, CD_WHITE, "off the line"), "line does not bleed");

    cdCanvasLine(canvas, 20, 10, 20, 140);
    TEST_ASSERT(check_pixel(canvas, 20, 100, CD_BLACK, "vertical line"), "vertical line drawn");

    /* a filled sector must cover its own center */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasSector(canvas, 150, 100, 60, 60, 0, 360);
    TEST_ASSERT(check_pixel(canvas, 150, 100, CD_RED, "sector center"), "sector filled");
    TEST_ASSERT(check_pixel(canvas, 150, 140, CD_WHITE, "outside the sector"), "sector bounded");

    cdKillCanvas(canvas);
    return 1;
}

static int test_polygon_fill(void)
{
    cdCanvas* canvas = create_quartz();

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 20, 20);
    cdCanvasVertex(canvas, 120, 20);
    cdCanvasVertex(canvas, 120, 120);
    cdCanvasVertex(canvas, 20, 120);
    cdCanvasEnd(canvas);

    TEST_ASSERT(check_pixel(canvas, 70, 70, CD_GREEN, "polygon interior"), "polygon filled");
    TEST_ASSERT(check_pixel(canvas, 150, 70, CD_WHITE, "polygon exterior"), "polygon bounded");

    cdKillCanvas(canvas);
    return 1;
}

static int test_clipping(void)
{
    cdCanvas* canvas = create_quartz();

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasClipArea(canvas, 50, 99, 50, 99);
    cdCanvasClip(canvas, CD_CLIPAREA);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 0, CANVAS_W-1, 0, CANVAS_H-1);

    TEST_ASSERT(check_pixel(canvas, 75, 75, CD_RED, "inside the clip area"), "clip lets drawing through");
    TEST_ASSERT(check_pixel(canvas, 20, 20, CD_WHITE, "outside the clip area"), "clip blocks drawing");
    TEST_ASSERT(check_pixel(canvas, 120, 120, CD_WHITE, "outside the clip area"), "clip blocks drawing");

    /* turning clipping off must restore the full canvas */
    cdCanvasClip(canvas, CD_CLIPOFF);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 0, 19, 0, 19);
    TEST_ASSERT(check_pixel(canvas, 10, 10, CD_BLUE, "after clip off"), "clip can be removed");

    cdKillCanvas(canvas);
    return 1;
}

static int test_transform(void)
{
    cdCanvas* canvas = create_quartz();
    double matrix[6];

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    /* translate by (100, 50) */
    matrix[0] = 1; matrix[1] = 0;
    matrix[2] = 0; matrix[3] = 1;
    matrix[4] = 100; matrix[5] = 50;
    cdCanvasTransform(canvas, matrix);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 0, 19, 0, 19);

    cdCanvasTransform(canvas, NULL);

    TEST_ASSERT(check_pixel(canvas, 110, 60, CD_RED, "translated box"), "transform applied");
    TEST_ASSERT(check_pixel(canvas, 10, 10, CD_WHITE, "untranslated position"), "transform moved the box");

    cdKillCanvas(canvas);
    return 1;
}

static int test_text(void)
{
    cdCanvas* canvas = create_quartz();
    int width = 0, height = 0, ascent = 0, descent = 0, max_width = 0;
    int x, y, dark = 0;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 24);

    cdCanvasGetTextSize(canvas, "Quartz", &width, &height);
    TEST_ASSERT(width > 0, "text width should be positive");
    TEST_ASSERT(height > 0, "text height should be positive");

    cdCanvasGetFontDim(canvas, &max_width, &height, &ascent, &descent);
    TEST_ASSERT(ascent > 0, "font ascent should be positive");
    TEST_ASSERT(descent > 0, "font descent should be positive");
    TEST_ASSERT(max_width > 0, "font max width should be positive");
    TEST_ASSERT(height >= ascent, "font height should cover the ascent");

    /* a longer string must be wider */
    {
        int width2 = 0;
        cdCanvasGetTextSize(canvas, "Quartz Quartz", &width2, NULL);
        TEST_ASSERT(width2 > width, "a longer string should be wider");
    }

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasTextAlignment(canvas, CD_SOUTH_WEST);
    cdCanvasText(canvas, 20, 60, "Quartz");

    /* the glyphs must have marked pixels inside the reported box */
    for (y = 60; y < 60 + height && y < CANVAS_H; y++) {
        for (x = 20; x < 20 + width && x < CANVAS_W; x++) {
            if (get_pixel(canvas, x, y) != CD_WHITE)
                dark++;
        }
    }

    TEST_ASSERT(dark > 20, "text should mark pixels inside its reported extent");

    cdKillCanvas(canvas);
    return 1;
}

static int test_put_get_image_rgb(void)
{
    cdCanvas* canvas = create_quartz();
    unsigned char r[32*16], g[32*16], b[32*16];
    unsigned char out_r[32*16], out_g[32*16], out_b[32*16];
    int i, x, y;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    /* a gradient that differs in every row and column */
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 32; x++) {
            i = y*32 + x;
            r[i] = (unsigned char)(x*8);
            g[i] = (unsigned char)(y*16);
            b[i] = 128;
        }
    }

    cdCanvasPutImageRectRGB(canvas, 32, 16, r, g, b, 10, 20, 32, 16, 0, 0, 0, 0);
    cdCanvasGetImageRGB(canvas, out_r, out_g, out_b, 10, 20, 32, 16);

    for (i = 0; i < 32*16; i++) {
        if (out_r[i] != r[i] || out_g[i] != g[i] || out_b[i] != b[i]) {
            printf("FAIL: %s - image roundtrip differs at %d: put (%d,%d,%d) got (%d,%d,%d)\n",
                   __func__, i, r[i], g[i], b[i], out_r[i], out_g[i], out_b[i]);
            cdKillCanvas(canvas);
            return 0;
        }
    }

    cdKillCanvas(canvas);
    return 1;
}

static int test_server_image(void)
{
    cdCanvas* canvas = create_quartz();
    cdImage* image;
    cdCanvas* image_canvas;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    image = cdCanvasCreateImage(canvas, 40, 30);
    TEST_ASSERT_NOT_NULL(image, "server image creation failed");

    /* draw into the image through an image canvas */
    image_canvas = cdCreateCanvas(cdContextImage(), image);
    TEST_ASSERT_NOT_NULL(image_canvas, "image canvas creation failed");

    cdCanvasSetAttribute(image_canvas, "ANTIALIAS", "0");
    cdCanvasBackground(image_canvas, CD_YELLOW);
    cdCanvasClear(image_canvas);
    cdCanvasForeground(image_canvas, CD_RED);
    cdCanvasBox(image_canvas, 0, 19, 0, 29);
    cdKillCanvas(image_canvas);

    /* and blit it back */
    cdCanvasPutImageRect(canvas, image, 100, 60, 0, 0, 0, 0);

    TEST_ASSERT(check_pixel(canvas, 105, 70, CD_RED, "server image left half"), "image content blitted");
    TEST_ASSERT(check_pixel(canvas, 130, 70, CD_YELLOW, "server image right half"), "image content blitted");
    TEST_ASSERT(check_pixel(canvas, 90, 70, CD_WHITE, "outside the blit"), "blit is bounded");

    /* a sub rectangle of the image: the right hand, yellow, half */
    cdCanvasPutImageRect(canvas, image, 10, 100, 20, 39, 0, 29);
    TEST_ASSERT(check_pixel(canvas, 15, 110, CD_YELLOW, "sub rectangle blit"), "sub rectangle blitted");

    cdKillImage(image);
    cdKillCanvas(canvas);
    return 1;
}

/* Captures a rectangle of the canvas into a server image and puts it back
   somewhere else. */
static int test_get_image(void)
{
    cdCanvas* canvas = create_quartz();
    cdImage* image;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, 10, 39, 10, 39);

    image = cdCanvasCreateImage(canvas, 30, 30);
    TEST_ASSERT_NOT_NULL(image, "server image creation failed");

    cdCanvasGetImage(canvas, image, 10, 10);
    cdCanvasPutImageRect(canvas, image, 120, 100, 0, 0, 0, 0);

    TEST_ASSERT(check_pixel(canvas, 130, 110, CD_BLUE, "captured region"), "GetImage captured the box");
    TEST_ASSERT(check_pixel(canvas, 160, 110, CD_WHITE, "outside the capture"), "capture is bounded");

    cdKillImage(image);
    cdKillCanvas(canvas);
    return 1;
}

static int test_bezier(void)
{
    cdCanvas* canvas = create_quartz();
    int x, y, painted = 0;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasBegin(canvas, CD_BEZIER);
    cdCanvasVertex(canvas, 20, 20);
    cdCanvasVertex(canvas, 60, 140);
    cdCanvasVertex(canvas, 140, 140);
    cdCanvasVertex(canvas, 180, 20);
    cdCanvasEnd(canvas);

    /* the curve must arch through the upper middle of the canvas */
    for (y = 90; y < 130; y++)
        for (x = 80; x < 120; x++)
            if (get_pixel(canvas, x, y) != CD_WHITE)
                painted++;

    TEST_ASSERT(painted > 0, "bezier should arch through the upper middle");
    TEST_ASSERT(check_pixel(canvas, 100, 40, CD_WHITE, "below the curve"), "bezier is not filled");

    cdKillCanvas(canvas);
    return 1;
}

static int test_double_buffer(void)
{
    cdCanvas* canvas = create_quartz();
    cdCanvas* dbuffer;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    dbuffer = cdCreateCanvas(cdContextDBuffer(), canvas);
    TEST_ASSERT_NOT_NULL(dbuffer, "double buffer canvas creation failed");

    cdCanvasSetAttribute(dbuffer, "ANTIALIAS", "0");
    cdCanvasBackground(dbuffer, CD_WHITE);
    cdCanvasClear(dbuffer);
    cdCanvasForeground(dbuffer, CD_GREEN);
    cdCanvasBox(dbuffer, 30, 79, 30, 79);

    /* nothing should have reached the target canvas yet */
    TEST_ASSERT(check_pixel(canvas, 50, 50, CD_WHITE, "before flush"), "dbuffer holds the drawing");

    cdCanvasFlush(dbuffer);

    TEST_ASSERT(check_pixel(canvas, 50, 50, CD_GREEN, "after flush"), "flush blits to the target");
    TEST_ASSERT(check_pixel(canvas, 150, 120, CD_WHITE, "outside the box"), "flush preserves the rest");

    cdKillCanvas(dbuffer);
    cdKillCanvas(canvas);
    return 1;
}

static int test_interior_styles(void)
{
    cdCanvas* canvas = create_quartz();
    int x, y, fg = 0, bg = 0;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasInteriorStyle(canvas, CD_HATCH);
    cdCanvasHatch(canvas, CD_HORIZONTAL);
    cdCanvasBox(canvas, 20, 119, 20, 119);

    /* a hatch must leave both painted and unpainted pixels behind */
    for (y = 30; y < 110; y++) {
        for (x = 30; x < 110; x++) {
            long c = get_pixel(canvas, x, y);
            if (same_color(c, CD_RED, 2)) fg++;
            else if (same_color(c, CD_WHITE, 2)) bg++;
        }
    }

    TEST_ASSERT(fg > 0, "hatch should paint some pixels in the foreground");
    TEST_ASSERT(bg > 0, "hatch should leave some pixels unpainted");

    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasBox(canvas, 130, 179, 20, 69);
    TEST_ASSERT(check_pixel(canvas, 150, 40, CD_RED, "solid fill after hatch"),
                "interior style returns to solid");

    cdKillCanvas(canvas);
    return 1;
}

static int test_pattern_fill(void)
{
    cdCanvas* canvas = create_quartz();
    long pattern[4];

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    /* a 2x2 checker: CD pattern data is bottom-up */
    pattern[0] = CD_RED;   pattern[1] = CD_GREEN;   /* bottom row */
    pattern[2] = CD_BLUE;  pattern[3] = CD_YELLOW;  /* top row */

    cdCanvasPattern(canvas, 2, 2, pattern);
    cdCanvasInteriorStyle(canvas, CD_PATTERN);
    cdCanvasBox(canvas, 20, 119, 20, 119);

    {
        int x, y, red = 0, green = 0, blue = 0, yellow = 0;

        for (y = 40; y < 100; y++) {
            for (x = 40; x < 100; x++) {
                long c = get_pixel(canvas, x, y);
                if (same_color(c, CD_RED, 2)) red++;
                else if (same_color(c, CD_GREEN, 2)) green++;
                else if (same_color(c, CD_BLUE, 2)) blue++;
                else if (same_color(c, CD_YELLOW, 2)) yellow++;
            }
        }

        TEST_ASSERT(red > 0 && green > 0 && blue > 0 && yellow > 0,
                    "every pattern color should appear in the fill");
    }

    TEST_ASSERT(check_pixel(canvas, 150, 60, CD_WHITE, "outside the pattern box"),
                "pattern fill is bounded");

    cdKillCanvas(canvas);
    return 1;
}

static int test_stipple_fill(void)
{
    cdCanvas* canvas = create_quartz();
    unsigned char stipple[16];
    int i, x, y, fg = 0, bg = 0;

    TEST_ASSERT_NOT_NULL(canvas, "canvas creation failed");

    for (i = 0; i < 16; i++)
        stipple[i] = (unsigned char)((i % 2) ? 1 : 0);

    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasStipple(canvas, 4, 4, stipple);
    cdCanvasInteriorStyle(canvas, CD_STIPPLE);
    cdCanvasBox(canvas, 20, 119, 20, 119);

    for (y = 40; y < 100; y++) {
        for (x = 40; x < 100; x++) {
            long c = get_pixel(canvas, x, y);
            if (same_color(c, CD_BLUE, 2)) fg++;
            else if (same_color(c, CD_WHITE, 2)) bg++;
        }
    }

    TEST_ASSERT(fg > 0, "stipple should paint the set bits in the foreground");
    TEST_ASSERT(bg > 0, "stipple should paint the clear bits in the background");

    cdKillCanvas(canvas);
    return 1;
}

/*---------------------------------------------------------------------------*/

/* All clipboard tests use a private pasteboard so that running the suite does
   not disturb whatever the user has copied. */
#define CLIP_BOARD "-pCDQuartzTestPasteboard"

/* Copies a red box through the clipboard in the given format and checks that
   playing it back reproduces the drawing. */
static int clipboard_roundtrip(const char* flags, const char* what)
{
    char data[128];
    cdCanvas* clip;
    cdCanvas* target;
    int ok = 1;

    sprintf(data, "%dx%d %s %s", CANVAS_W, CANVAS_H, flags, CLIP_BOARD);

    clip = cdCreateCanvas(cdContextClipboard(), data);
    if (!clip) {
        printf("FAIL: %s - could not create a clipboard canvas (%s)\n", what, data);
        return 0;
    }

    cdCanvasBackground(clip, CD_WHITE);
    cdCanvasClear(clip);
    cdCanvasForeground(clip, CD_RED);
    cdCanvasBox(clip, 20, 99, 20, 79);
    cdKillCanvas(clip);            /* this is what puts it on the pasteboard */

    target = create_quartz();
    if (!target) {
        printf("FAIL: %s - could not create the target canvas\n", what);
        return 0;
    }

    if (cdCanvasPlay(target, cdContextClipboard(), 0, 0, 0, 0, CLIP_BOARD) != CD_OK) {
        printf("FAIL: %s - Play returned an error\n", what);
        cdKillCanvas(target);
        return 0;
    }

    if (!check_pixel(target, 60, 50, CD_RED, what))
        ok = 0;
    if (!check_pixel(target, 160, 120, CD_WHITE, what))
        ok = 0;

    cdKillCanvas(target);
    return ok;
}

static int test_clipboard_pdf(void)
{
    return clipboard_roundtrip("", "clipboard PDF");
}

static int test_clipboard_bitmap(void)
{
    return clipboard_roundtrip("-b", "clipboard bitmap");
}

static int test_clipboard_metafile(void)
{
    return clipboard_roundtrip("-m", "clipboard metafile");
}

/* Play must report an error rather than draw something arbitrary when the
   pasteboard holds nothing it understands. */
static int test_clipboard_empty(void)
{
    cdCanvas* target = create_quartz();
    int ret;

    TEST_ASSERT_NOT_NULL(target, "canvas creation failed");

    ret = cdCanvasPlay(target, cdContextClipboard(), 0, 0, 0, 0,
                       "-pCDQuartzTestEmptyPasteboard");

    TEST_ASSERT(ret == CD_ERROR, "Play on an empty pasteboard should fail");
    TEST_ASSERT(check_pixel(target, 100, 75, CD_WHITE, "untouched canvas"),
                "a failed Play should not draw");

    cdKillCanvas(target);
    return 1;
}

/*---------------------------------------------------------------------------*/

int main(void)
{
    printf("Running CD Quartz Driver Tests...\n");

    RUN_TEST(test_canvas_creation);
    RUN_TEST(test_clear_and_readback);
    RUN_TEST(test_matches_imagergb);
    RUN_TEST(test_orientation);
    RUN_TEST(test_line_and_arc);
    RUN_TEST(test_polygon_fill);
    RUN_TEST(test_bezier);
    RUN_TEST(test_clipping);
    RUN_TEST(test_transform);
    RUN_TEST(test_text);
    RUN_TEST(test_put_get_image_rgb);
    RUN_TEST(test_server_image);
    RUN_TEST(test_get_image);
    RUN_TEST(test_double_buffer);
    RUN_TEST(test_interior_styles);
    RUN_TEST(test_pattern_fill);
    RUN_TEST(test_stipple_fill);
    RUN_TEST(test_clipboard_pdf);
    RUN_TEST(test_clipboard_bitmap);
    RUN_TEST(test_clipboard_metafile);
    RUN_TEST(test_clipboard_empty);

    printf("\nQuartz Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}
