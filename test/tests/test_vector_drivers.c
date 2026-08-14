/*
 * CD Library Test Suite - PostScript, CGM, DXF and DGN file drivers
 *
 * These four drivers live in src/drv and are portable C with no dependencies, but were in no
 * CMake source list, so cdContextPS/CGM/DXF/DGN did not exist in the library and every caller
 * got a NULL canvas. These tests exist to keep them in the build: a driver that is dropped
 * again fails at cdCreateCanvas, and one that is built but broken fails on its output.
 *
 * Output is checked by structure rather than by size. PostScript and DXF are text formats with
 * required markers, so those can be read directly. CGM is checked by the strongest means CD
 * has: cdplayCGM replays the file the writer just produced back into a bitmap canvas, so the
 * assertion covers the whole round trip.
 */

#include "test_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cd.h>
#include <cdps.h>
#include <cdcgm.h>
#include <cddxf.h>
#include <cddgn.h>
#include <cdirgb.h>

/* Reads a whole file. Returns NULL and a zero size if it is missing or empty. */
static char* sReadFile(const char* filename, long* size)
{
    FILE* file = fopen(filename, "rb");
    char* buffer;
    long length;

    *size = 0;
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0)
    {
        fclose(file);
        return NULL;
    }

    buffer = (char*)malloc((size_t)length + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, (size_t)length, file) != (size_t)length)
    {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = 0;
    fclose(file);
    *size = length;
    return buffer;
}

/* Counts non-overlapping occurrences of a marker in a text file. */
static int sCountMarker(const char* buffer, long size, const char* marker)
{
    size_t marker_len = strlen(marker);
    int count = 0;
    long i;

    for (i = 0; i + (long)marker_len <= size; i++)
    {
        if (memcmp(buffer + i, marker, marker_len) == 0)
        {
            count++;
            i += (long)marker_len - 1;
        }
    }

    return count;
}

static void sDrawSomething(cdCanvas* canvas)
{
    int w, h;
    cdCanvasGetSize(canvas, &w, &h, NULL, NULL);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 3);
    cdCanvasLine(canvas, 0, 0, w, h);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, w / 4, w / 2, h / 4, h / 2);

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 12);
    cdCanvasText(canvas, w / 2, (3 * h) / 4, "CD");
}

int test_ps_driver(void)
{
    cdCanvas* canvas;
    char* buffer;
    long size;

    printf("  Testing PostScript driver...\n");

    canvas = cdCreateCanvas(CD_PS, "test_ps_driver.ps -w200 -h150");
    TEST_ASSERT_NOT_NULL(canvas, "CD_PS should exist (src/drv/cdps.c must be in the build)");

    sDrawSomething(canvas);
    cdCanvasFlush(canvas);          /* second page */
    sDrawSomething(canvas);
    cdKillCanvas(canvas);

    buffer = sReadFile("test_ps_driver.ps", &size);
    TEST_ASSERT_NOT_NULL(buffer, "the driver should have written a file");
    TEST_ASSERT(strncmp(buffer, "%!PS-Adobe", 10) == 0, "it should carry the PostScript signature");
    TEST_ASSERT(sCountMarker(buffer, size, "%%Page:") == 2, "a flush should have started a second page");
    TEST_ASSERT(sCountMarker(buffer, size, "%%Trailer") >= 1, "the document should be closed properly");
    free(buffer);

    return 1;
}

/* EPS is the mode IupPlot's own export uses, so it gets its own test: it must be a single page
   and must carry a bounding box, which is what makes it embeddable. */
int test_eps_driver(void)
{
    cdCanvas* canvas;
    char* buffer;
    long size;

    printf("  Testing Encapsulated PostScript...\n");

    canvas = cdCreateCanvas(CD_PS, "test_eps_driver.eps -e -w100 -h100");
    TEST_ASSERT_NOT_NULL(canvas, "EPS canvas creation should succeed");

    sDrawSomething(canvas);
    cdKillCanvas(canvas);

    buffer = sReadFile("test_eps_driver.eps", &size);
    TEST_ASSERT_NOT_NULL(buffer, "the driver should have written a file");
    TEST_ASSERT(strncmp(buffer, "%!PS-Adobe", 10) == 0, "it should carry the PostScript signature");
    TEST_ASSERT(sCountMarker(buffer, size, "%%BoundingBox") >= 1, "EPS must declare a bounding box");
    free(buffer);

    return 1;
}

/* The round trip: write CGM with the driver, then replay it with CD's own interpreter into a
   bitmap and check something was actually drawn. */
int test_cgm_driver(void)
{
    cdCanvas* canvas;
    cdCanvas* bitmap;
    unsigned char* red;
    unsigned char* green;
    unsigned char* blue;
    char data[256];
    int i, ink = 0, played;
    const int size = 200;

    printf("  Testing CGM driver...\n");

    /* CGM's grammar is "filename [w_mmxh_mm] [resolution]" -- not the -w/-h/-s of the other
       drivers. Getting it wrong is not an error: the canvas silently stays at its INT_MAX
       default size, which makes anything that draws in proportion to the canvas run forever. */
    canvas = cdCreateCanvas(CD_CGM, "test_cgm_driver.cgm 100x100 3.78");
    TEST_ASSERT_NOT_NULL(canvas, "CD_CGM should exist (src/drv/cdcgm.c must be in the build)");

    { int w, h;
      cdCanvasGetSize(canvas, &w, &h, NULL, NULL);
      TEST_ASSERT(w > 0 && w < 10000 && h > 0 && h < 10000,
                  "the canvas should be the requested size, not the INT_MAX default");
    }

    sDrawSomething(canvas);
    cdKillCanvas(canvas);

    /* replay into an RGB bitmap: cdplayCGM comes from src/intcgm, the interpreter */
    red = (unsigned char*)malloc((size_t)size * size);
    green = (unsigned char*)malloc((size_t)size * size);
    blue = (unsigned char*)malloc((size_t)size * size);
    TEST_ASSERT(red && green && blue, "allocation for the playback bitmap should succeed");

    memset(red, 255, (size_t)size * size);
    memset(green, 255, (size_t)size * size);
    memset(blue, 255, (size_t)size * size);

    sprintf(data, "%dx%d %p %p %p", size, size, red, green, blue);
    bitmap = cdCreateCanvas(CD_IMAGERGB, data);
    TEST_ASSERT_NOT_NULL(bitmap, "the playback canvas should be created");

    played = cdCanvasPlay(bitmap, CD_CGM, 0, 0, 0, 0, (void*)"test_cgm_driver.cgm");
    TEST_ASSERT(played == CD_OK, "CD should be able to replay the CGM it just wrote");

    for (i = 0; i < size * size; i++)
    {
        if (red[i] != 255 || green[i] != 255 || blue[i] != 255)
            ink++;
    }
    TEST_ASSERT(ink > 100, "the replayed drawing should put ink on the bitmap");

    cdKillCanvas(bitmap);
    free(red); free(green); free(blue);

    return 1;
}

int test_dxf_driver(void)
{
    cdCanvas* canvas;
    char* buffer;
    long size;

    printf("  Testing DXF driver...\n");

    canvas = cdCreateCanvas(CD_DXF, "test_dxf_driver.dxf -w100 -h100");
    TEST_ASSERT_NOT_NULL(canvas, "CD_DXF should exist (src/drv/cddxf.c must be in the build)");

    sDrawSomething(canvas);
    cdKillCanvas(canvas);

    buffer = sReadFile("test_dxf_driver.dxf", &size);
    TEST_ASSERT_NOT_NULL(buffer, "the driver should have written a file");
    TEST_ASSERT(sCountMarker(buffer, size, "SECTION") >= 1, "a DXF file is made of sections");
    TEST_ASSERT(sCountMarker(buffer, size, "ENTITIES") >= 1, "the drawing should be in an ENTITIES section");
    TEST_ASSERT(sCountMarker(buffer, size, "EOF") >= 1, "the file should be terminated");
    free(buffer);

    return 1;
}

int test_dgn_driver(void)
{
    cdCanvas* canvas;
    char* buffer;
    long size;

    printf("  Testing DGN driver...\n");

    canvas = cdCreateCanvas(CD_DGN, "test_dgn_driver.dgn -w100 -h100");
    TEST_ASSERT_NOT_NULL(canvas, "CD_DGN should exist (src/drv/cddgn.c must be in the build)");

    sDrawSomething(canvas);
    cdKillCanvas(canvas);

    /* DGN is a binary format with no convenient signature, so this only asserts that a
       non-trivial file was produced -- enough to catch the driver being dropped or failing. */
    buffer = sReadFile("test_dgn_driver.dgn", &size);
    TEST_ASSERT_NOT_NULL(buffer, "the driver should have written a file");
    TEST_ASSERT(size > 512, "the file should hold more than an empty header");
    free(buffer);

    return 1;
}

/* Every one of these drivers must reject bad input by returning NULL rather than writing a
   broken file or crashing. */
int test_vector_drivers_invalid(void)
{
    printf("  Testing invalid input...\n");

    TEST_ASSERT(cdCreateCanvas(CD_PS, "") == NULL, "PS should reject an empty data string");
    TEST_ASSERT(cdCreateCanvas(CD_CGM, "") == NULL, "CGM should reject an empty data string");
    TEST_ASSERT(cdCreateCanvas(CD_DXF, "") == NULL, "DXF should reject an empty data string");
    TEST_ASSERT(cdCreateCanvas(CD_DGN, "") == NULL, "DGN should reject an empty data string");

    return 1;
}

int main(void)
{
    printf("Running CD Library Vector File Driver Tests...\n");

    RUN_TEST(test_ps_driver);
    RUN_TEST(test_eps_driver);
    RUN_TEST(test_cgm_driver);
    RUN_TEST(test_dxf_driver);
    RUN_TEST(test_dgn_driver);
    RUN_TEST(test_vector_drivers_invalid);

    printf("\nVector File Driver Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}
