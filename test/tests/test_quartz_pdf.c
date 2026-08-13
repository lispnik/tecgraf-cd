/*
 * CD Library Test Suite - Quartz PDF Backend (macOS)
 *
 * Exercises the CGPDFContext-based CD_PDF driver. Unlike every other CD file
 * driver, this one can be checked properly rather than just for "a file
 * appeared": CoreGraphics is itself a PDF parser, so each test re-opens its own
 * output with CGPDFDocumentCreateWithURL, inspects it, and renders pages back
 * into a bitmap to confirm the drawing actually landed.
 */

#include "test_utils.h"

#include <ApplicationServices/ApplicationServices.h>

#include <cd.h>
#include <cdpdf.h>

/* Opens a PDF written by the driver. Returns NULL if it is not a valid PDF,
   which is itself the first thing worth asserting. */
static CGPDFDocumentRef sOpenPDF(const char* filename)
{
    CGPDFDocumentRef doc;
    CFStringRef path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    CFURLRef url;

    if (!path)
        return NULL;

    url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
    CFRelease(path);
    if (!url)
        return NULL;

    doc = CGPDFDocumentCreateWithURL(url);
    CFRelease(url);
    return doc;
}

/* Renders one page and counts pixels that are not the white background.
   Anti-aliasing makes exact colour matching unreliable -- a 3 pixel wide line
   at 300 dpi is under a point on the page, so every pixel of it is blended --
   so "is there ink here" is the honest question to ask. */
static int sCountInk(CGPDFDocumentRef doc, size_t page_number)
{
    CGPDFPageRef page = CGPDFDocumentGetPage(doc, page_number);
    CGRect box;
    CGColorSpaceRef space;
    CGContextRef bmp;
    unsigned char* buffer;
    int w, h, i, ink = 0;

    if (!page)
        return -1;

    box = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    w = (int)box.size.width;
    h = (int)box.size.height;
    if (w <= 0 || h <= 0)
        return -1;

    buffer = (unsigned char*)calloc((size_t)w * h * 4, 1);
    if (!buffer)
        return -1;

    space = CGColorSpaceCreateDeviceRGB();
    bmp = CGBitmapContextCreate(buffer, w, h, 8, (size_t)w * 4, space,
                                kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);
    if (!bmp)
    {
        free(buffer);
        return -1;
    }

    CGContextSetRGBFillColor(bmp, 1, 1, 1, 1);
    CGContextFillRect(bmp, CGRectMake(0, 0, w, h));
    CGContextDrawPDFPage(bmp, page);

    for (i = 0; i < w * h; i++)
    {
        unsigned char* px = buffer + i * 4;
        if (px[0] < 250 || px[1] < 250 || px[2] < 250)
            ink++;
    }

    CGContextRelease(bmp);
    free(buffer);
    return ink;
}

int test_quartz_pdf_creation(void)
{
    cdCanvas* canvas;
    CGPDFDocumentRef doc;
    CGPDFPageRef page;
    CGRect box;
    double w_mm, h_mm;
    int w, h;

    printf("  Testing Quartz PDF canvas creation...\n");

    /* 200 x 150 mm at 300 dpi */
    canvas = cdCreateCanvas(CD_PDF, "test_quartz_pdf_creation.pdf -w200 -h150 -s300");
    TEST_ASSERT_NOT_NULL(canvas, "PDF canvas creation should succeed");

    cdCanvasGetSize(canvas, &w, &h, &w_mm, &h_mm);
    TEST_ASSERT(w == (int)(200.0 / 25.4 * 300), "canvas width should be the requested mm at the requested dpi");
    TEST_ASSERT(w_mm > 199.0 && w_mm < 201.0, "canvas width in mm should be what was asked for");
    TEST_ASSERT(h_mm > 149.0 && h_mm < 151.0, "canvas height in mm should be what was asked for");

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 20);
    cdCanvasLine(canvas, 100, 100, w - 100, h - 100);
    cdKillCanvas(canvas);

    doc = sOpenPDF("test_quartz_pdf_creation.pdf");
    TEST_ASSERT_NOT_NULL(doc, "the output should be a PDF CoreGraphics can open");
    TEST_ASSERT(CGPDFDocumentGetNumberOfPages(doc) == 1, "a canvas that was never flushed should hold one page");

    page = CGPDFDocumentGetPage(doc, 1);
    box = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    /* the media box is in points, so 200 mm is 566.9 pt */
    TEST_ASSERT(box.size.width > 560 && box.size.width < 573, "media box width should be the page size in points");
    TEST_ASSERT(box.size.height > 419 && box.size.height < 431, "media box height should be the page size in points");

    TEST_ASSERT(sCountInk(doc, 1) > 100, "the line should have been written to the page");

    CGPDFDocumentRelease(doc);
    return 1;
}

/* cdCanvasFlush means "next page" for a paged format. This is the part most
   likely to break, because CGPDFContextEndPage resets the graphics state stack
   that the shared Quartz driver relies on for clipping. */
int test_quartz_pdf_pages(void)
{
    cdCanvas* canvas;
    CGPDFDocumentRef doc;
    int page_ink[3], i;

    printf("  Testing Quartz PDF page breaks...\n");

    canvas = cdCreateCanvas(CD_PDF, "test_quartz_pdf_pages.pdf -w100 -h100 -s150");
    TEST_ASSERT_NOT_NULL(canvas, "PDF canvas creation should succeed");

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 20);
    cdCanvasLine(canvas, 50, 50, 500, 500);

    cdCanvasFlush(canvas);   /* page 2 */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasBox(canvas, 50, 500, 50, 500);

    cdCanvasFlush(canvas);   /* page 3 */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasArc(canvas, 300, 300, 200, 200, 0, 360);

    cdKillCanvas(canvas);

    doc = sOpenPDF("test_quartz_pdf_pages.pdf");
    TEST_ASSERT_NOT_NULL(doc, "the output should be a valid PDF");
    TEST_ASSERT(CGPDFDocumentGetNumberOfPages(doc) == 3, "two flushes should produce three pages");

    /* every page must carry its own drawing: a page break that lost the
       graphics state would leave later pages blank or clipped away */
    for (i = 0; i < 3; i++)
    {
        page_ink[i] = sCountInk(doc, (size_t)(i + 1));
        TEST_ASSERT(page_ink[i] > 50, "each page should contain its own drawing");
    }

    CGPDFDocumentRelease(doc);
    return 1;
}

/* Text goes through CoreText, so it is embedded and selectable rather than
   rasterised. Metrics must come back in canvas pixels. */
int test_quartz_pdf_text(void)
{
    cdCanvas* canvas;
    CGPDFDocumentRef doc;
    int width = 0, height = 0, ascent = 0, descent = 0;

    printf("  Testing Quartz PDF text...\n");

    canvas = cdCreateCanvas(CD_PDF, "test_quartz_pdf_text.pdf -w150 -h100 -s150");
    TEST_ASSERT_NOT_NULL(canvas, "PDF canvas creation should succeed");

    cdCanvasFont(canvas, "Helvetica", CD_BOLD, 36);
    cdCanvasGetTextSize(canvas, "Hello PDF", &width, &height);
    TEST_ASSERT(width > 0 && height > 0, "text metrics should be available (CoreText, not the FreeType simulation)");

    cdCanvasGetFontDim(canvas, NULL, NULL, &ascent, &descent);
    TEST_ASSERT(ascent > 0, "font ascent should be available");

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasText(canvas, 100, 300, "Hello PDF");
    cdKillCanvas(canvas);

    doc = sOpenPDF("test_quartz_pdf_text.pdf");
    TEST_ASSERT_NOT_NULL(doc, "the output should be a valid PDF");
    TEST_ASSERT(sCountInk(doc, 1) > 100, "the text should have been written to the page");

    CGPDFDocumentRelease(doc);
    return 1;
}

/* The driver must reject what it cannot honour rather than producing a broken
   file, and cdCreateCanvas signals that by returning NULL. */
int test_quartz_pdf_invalid(void)
{
    printf("  Testing Quartz PDF invalid input...\n");

    TEST_ASSERT(cdCreateCanvas(CD_PDF, "") == NULL, "an empty data string should be rejected");
    TEST_ASSERT(cdCreateCanvas(CD_PDF, "bad.pdf -w0 -h0") == NULL, "a zero page size should be rejected");

    return 1;
}

int main(void)
{
    printf("Running CD Library Quartz PDF Backend Tests...\n");

    RUN_TEST(test_quartz_pdf_creation);
    RUN_TEST(test_quartz_pdf_pages);
    RUN_TEST(test_quartz_pdf_text);
    RUN_TEST(test_quartz_pdf_invalid);

    printf("\nQuartz PDF Backend Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}
