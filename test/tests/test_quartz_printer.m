/*
 * CD Library Test Suite - Quartz Printer Backend (macOS)
 *
 * Exercises CD_PRINTER end to end without touching a printer. NSPrintInfo can be told to save
 * the job to a file instead of spooling it to hardware, and the driver copies the shared
 * NSPrintInfo, so setting that here redirects the whole path -- NSPrintOperation, pagination
 * and all -- into a PDF this test can then re-open and inspect.
 *
 * That matters because the interesting failure modes are in pagination, not in drawing: a
 * -rectForPage: that returns the same rectangle for every page prints page one repeatedly, and
 * nothing about the file's existence or size would reveal it.
 */

#import <Cocoa/Cocoa.h>

#include "test_utils.h"

#include <cd.h>
#include <cdprint.h>

static NSString* sSavePath = nil;

/* Points the shared print info at a file. Returns the path the job will be written to. */
static NSString* sRedirectPrintingToFile(const char* name)
{
    NSPrintInfo* info = [NSPrintInfo sharedPrintInfo];
    NSString* path = [NSTemporaryDirectory() stringByAppendingPathComponent:
                        [NSString stringWithFormat:@"%s.pdf", name]];

    [[NSFileManager defaultManager] removeItemAtPath:path error:NULL];

    [info setJobDisposition:NSPrintSaveJob];
    [[info dictionary] setObject:[NSURL fileURLWithPath:path] forKey:NSPrintJobSavingURL];

    return path;
}

static CGPDFDocumentRef sOpenPDF(NSString* path)
{
    if (![[NSFileManager defaultManager] fileExistsAtPath:path])
        return NULL;

    return CGPDFDocumentCreateWithURL((CFURLRef)[NSURL fileURLWithPath:path]);
}

/* Renders a page and returns both an ink count and a cheap signature of where that ink is, so
   two pages can be told apart. */
static int sPageInk(CGPDFDocumentRef doc, size_t page_number, long* signature)
{
    CGPDFPageRef page = CGPDFDocumentGetPage(doc, page_number);
    CGRect box;
    CGColorSpaceRef space;
    CGContextRef bmp;
    unsigned char* buffer;
    int w, h, x, y, ink = 0;

    if (signature)
        *signature = 0;

    if (!page)
        return -1;

    box = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    w = (int)box.size.width;
    h = (int)box.size.height;
    if (w <= 0 || h <= 0)
        return -1;

    buffer = (unsigned char*)calloc((size_t)w * h * 4, 1);
    space = CGColorSpaceCreateDeviceRGB();
    bmp = CGBitmapContextCreate(buffer, w, h, 8, (size_t)w * 4, space,
                                (CGBitmapInfo)kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);

    CGContextSetRGBFillColor(bmp, 1, 1, 1, 1);
    CGContextFillRect(bmp, CGRectMake(0, 0, w, h));
    CGContextDrawPDFPage(bmp, page);

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            unsigned char* px = buffer + ((size_t)y * w + x) * 4;
            if (px[0] < 250 || px[1] < 250 || px[2] < 250)
            {
                ink++;
                if (signature)
                    *signature += x + y * 7;
            }
        }
    }

    CGContextRelease(bmp);
    free(buffer);
    return ink;
}

int test_printer_creation(void)
{
    cdCanvas* canvas;
    int w = 0, h = 0;
    double w_mm = 0, h_mm = 0;

    printf("  Testing printer canvas creation...\n");

    sSavePath = sRedirectPrintingToFile("test_printer_creation");

    canvas = cdCreateCanvas(CD_PRINTER, "CD test job");
    TEST_ASSERT_NOT_NULL(canvas, "CD_PRINTER should exist on macOS");

    cdCanvasGetSize(canvas, &w, &h, &w_mm, &h_mm);
    /* the imageable area of any real paper size, in millimetres */
    TEST_ASSERT(w_mm > 50 && w_mm < 1000, "the canvas should be the width of a sheet of paper");
    TEST_ASSERT(h_mm > 50 && h_mm < 1000, "the canvas should be the height of a sheet of paper");
    TEST_ASSERT(w > 100 && h > 100, "the canvas should have a usable pixel size");

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLineWidth(canvas, 10);
    cdCanvasLine(canvas, 0, 0, w, h);
    cdKillCanvas(canvas);

    {
        CGPDFDocumentRef doc = sOpenPDF(sSavePath);
        TEST_ASSERT_NOT_NULL(doc, "killing the canvas should have sent the job");
        TEST_ASSERT(CGPDFDocumentGetNumberOfPages(doc) == 1, "one page should have been printed");
        TEST_ASSERT(sPageInk(doc, 1, NULL) > 100, "the drawing should be on the printed page");
        CGPDFDocumentRelease(doc);
    }

    return 1;
}

/* cdCanvasFlush means a new page here as it does for the file drivers, and each page must carry
   its own drawing -- the pagination bug this is aimed at reprints page one instead. */
int test_printer_pages(void)
{
    cdCanvas* canvas;
    CGPDFDocumentRef doc;
    long sig1 = 0, sig2 = 0;
    int ink1, ink2, w = 0, h = 0;

    printf("  Testing printer page breaks...\n");

    sSavePath = sRedirectPrintingToFile("test_printer_pages");

    canvas = cdCreateCanvas(CD_PRINTER, "CD test job");
    TEST_ASSERT_NOT_NULL(canvas, "printer canvas creation should succeed");

    cdCanvasGetSize(canvas, &w, &h, NULL, NULL);

    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, w / 10, w / 2, h / 10, h / 2);           /* lower left */

    cdCanvasFlush(canvas);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, w / 2, (9 * w) / 10, h / 2, (9 * h) / 10); /* upper right */

    cdKillCanvas(canvas);

    doc = sOpenPDF(sSavePath);
    TEST_ASSERT_NOT_NULL(doc, "the job should have been written");
    TEST_ASSERT(CGPDFDocumentGetNumberOfPages(doc) == 2, "a flush should have produced a second page");

    ink1 = sPageInk(doc, 1, &sig1);
    ink2 = sPageInk(doc, 2, &sig2);
    printf("    page 1: %d px (sig %ld), page 2: %d px (sig %ld)\n", ink1, sig1, ink2, sig2);
    TEST_ASSERT(ink1 > 100 && ink2 > 100, "both pages should carry drawing");
    TEST_ASSERT(sig1 != sig2, "the second page should not be a reprint of the first");

    CGPDFDocumentRelease(doc);
    return 1;
}

/* The job name is optional and the data string may be empty, in which case the canvas must
   still be created -- only a cancelled print dialog produces NULL, and that cannot be
   automated. */
int test_printer_data_string(void)
{
    cdCanvas* canvas;

    printf("  Testing printer data string handling...\n");

    sSavePath = sRedirectPrintingToFile("test_printer_data_string");

    canvas = cdCreateCanvas(CD_PRINTER, "");
    TEST_ASSERT_NOT_NULL(canvas, "an empty data string should still give a printer canvas");
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasBox(canvas, 10, 200, 10, 200);
    cdKillCanvas(canvas);

    {
        CGPDFDocumentRef doc = sOpenPDF(sSavePath);
        TEST_ASSERT_NOT_NULL(doc, "the job should still have been sent");
        CGPDFDocumentRelease(doc);
    }

    return 1;
}

int main(void)
{
    printf("Running CD Library Quartz Printer Backend Tests...\n");

    /* NSPrintOperation is AppKit, and AppKit wants an application object to exist even when
       nothing is shown. */
    [NSApplication sharedApplication];

    RUN_TEST(test_printer_creation);
    RUN_TEST(test_printer_pages);
    RUN_TEST(test_printer_data_string);

    printf("\nQuartz Printer Backend Tests Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}
