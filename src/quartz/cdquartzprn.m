/** \file
 * \brief Quartz Printer Driver
 *
 * CD_PRINTER on macOS. The documentation says this driver "works only with the GDI, GDI+ and
 * Cairo base drivers", which on this platform meant cdContextPrinter returned NULL and every
 * print path in every application silently did nothing.
 *
 * macOS prints PDF: the whole printing system is built on it, and a print job is spooled as a
 * PDF document whatever the application draws with. So this driver draws into a PDF exactly as
 * cdquartzpdf.c does -- sharing that code rather than repeating its page and graphics-state
 * handling -- and hands the finished document to NSPrintOperation when the canvas is killed.
 * cdCanvasFlush therefore starts a new page in both senses at once.
 *
 * Data string, as documented for CD_PRINTER:
 *
 *   "name [-d]"
 *
 * where name is the job name shown in the print queue and -d asks for the print panel first.
 * Cancelling that panel produces a NULL canvas, which is what callers are told to expect.
 *
 * See Copyright Notice in cd.h
 */

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdquartzctx.h"
#include "cdquartzpdf.h"
#include "cdprint.h"


/* Renders the spooled PDF for NSPrintOperation: one PDF page per printed page, drawn at the
   size the page was written at. A view is needed because NSPrintOperation paginates through
   one; it is never added to a window. */
@interface CDQuartzPrintView : NSView
{
  CGPDFDocumentRef _document;
}
- (instancetype)initWithPDFDocument:(CGPDFDocumentRef)document;
@end

@implementation CDQuartzPrintView

- (instancetype)initWithPDFDocument:(CGPDFDocumentRef)document
{
  CGRect box;

  if (!document || CGPDFDocumentGetNumberOfPages(document) == 0)
    return nil;

  box = CGPDFPageGetBoxRect(CGPDFDocumentGetPage(document, 1), kCGPDFMediaBox);

  /* The frame has to span every page, not just the first. AppKit paginates by carving the
     view's frame into the rectangles -rectForPage: returns, and anything outside that frame is
     clipped away -- a one-page-tall frame prints page one and then blank sheets. */
  self = [super initWithFrame:NSMakeRect(0, 0, box.size.width,
                                         box.size.height * CGPDFDocumentGetNumberOfPages(document))];
  if (self)
  {
    _document = CGPDFDocumentRetain(document);
  }
  return self;
}

- (void)dealloc
{
  CGPDFDocumentRelease(_document);
  [super dealloc];
}

- (BOOL)isFlipped
{
  return NO;
}

- (BOOL)knowsPageRange:(NSRangePointer)range
{
  range->location = 1;
  range->length = CGPDFDocumentGetNumberOfPages(_document);
  return YES;
}

- (NSRect)rectForPage:(NSInteger)page
{
  CGPDFPageRef pdf_page = CGPDFDocumentGetPage(_document, (size_t)page);
  CGRect box;

  if (!pdf_page)
    return NSZeroRect;

  box = CGPDFPageGetBoxRect(pdf_page, kCGPDFMediaBox);

  /* Pages are laid out one above the other in the view's coordinate space, so each page's rect
     is distinct -- returning the same rect for every page prints the first page repeatedly. */
  return NSMakeRect(0, (CGFloat)(page - 1) * box.size.height, box.size.width, box.size.height);
}

- (void)drawRect:(NSRect)rect
{
  NSPrintOperation* operation = [NSPrintOperation currentOperation];
  CGContextRef cgc = [[NSGraphicsContext currentContext] CGContext];
  NSInteger page = operation ? [operation currentPage] : 1;
  CGPDFPageRef pdf_page = CGPDFDocumentGetPage(_document, (size_t)page);
  NSRect page_rect;

  (void)rect;

  if (!pdf_page || !cgc)
    return;

  page_rect = [self rectForPage:page];

  CGContextSaveGState(cgc);
  CGContextTranslateCTM(cgc, 0, page_rect.origin.y);
  CGContextDrawPDFPage(cgc, pdf_page);
  CGContextRestoreGState(cgc);
}

@end


struct cdPrinterData
{
  char* spool_filename;   /* the PDF being written, deleted after printing */
  void* print_info;       /* NSPrintInfo*, retained */
  void* job_name;         /* NSString*, retained */
};

/* CD keeps no room for driver-specific fields on cdCtxCanvas, and the printer needs three, so
   they hang off the one pointer the PDF path does not use. */
static struct cdPrinterData* sPrinterData(cdCtxCanvas* ctxcanvas)
{
  return (struct cdPrinterData*)ctxcanvas->clipboard_data;
}

static void sFreePrinterData(cdCtxCanvas* ctxcanvas)
{
  struct cdPrinterData* pdata = sPrinterData(ctxcanvas);

  if (!pdata)
    return;

  if (pdata->spool_filename)
  {
    remove(pdata->spool_filename);
    free(pdata->spool_filename);
  }

  [(NSPrintInfo*)pdata->print_info release];
  [(NSString*)pdata->job_name release];

  free(pdata);
  ctxcanvas->clipboard_data = NULL;
}

/* Spools to a file rather than to memory: a print job can be arbitrarily large, and this is
   also what makes the failure mode inspectable when something goes wrong. */
static char* sCreateSpoolFilename(void)
{
  /* pid plus a timestamp, so two canvases printing at once do not share a spool file */
  NSString* path = [NSTemporaryDirectory() stringByAppendingPathComponent:
                      [NSString stringWithFormat:@"cd_print_%d_%.6f.pdf",
                        (int)getpid(), [NSDate timeIntervalSinceReferenceDate]]];
  const char* utf8 = [path UTF8String];
  char* copy;

  if (!utf8)
    return NULL;

  copy = (char*)malloc(strlen(utf8) + 1);
  if (copy)
    strcpy(copy, utf8);

  return copy;
}

static void sPrintDocument(cdCtxCanvas* ctxcanvas)
{
  struct cdPrinterData* pdata = sPrinterData(ctxcanvas);
  NSPrintInfo* print_info = (NSPrintInfo*)pdata->print_info;
  CGPDFDocumentRef document;
  CDQuartzPrintView* view;
  NSPrintOperation* operation;
  NSURL* url;

  url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:pdata->spool_filename]];
  document = CGPDFDocumentCreateWithURL((CFURLRef)url);
  if (!document)
    return;

  view = [[CDQuartzPrintView alloc] initWithPDFDocument:document];
  CGPDFDocumentRelease(document);
  if (!view)
    return;

  operation = [NSPrintOperation printOperationWithView:view printInfo:print_info];
  [operation setShowsPrintPanel:NO];       /* already shown at creation, if it was asked for */
  [operation setShowsProgressPanel:NO];
  if (pdata->job_name)
    [operation setJobTitle:(NSString*)pdata->job_name];

  [operation runOperation];

  [view release];
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  /* close the document first: the file has to be complete before it can be printed */
  cdquartzPDFClose(ctxcanvas);

  if (sPrinterData(ctxcanvas))
  {
    sPrintDocument(ctxcanvas);
    sFreePrinterData(ctxcanvas);
  }

  cdquartzKillCanvas(ctxcanvas);
}

/* "name [-d]" */
static void sParseData(const char* data, char* jobname, int* show_dialog)
{
  const char* dash_d;

  jobname[0] = 0;
  *show_dialog = 0;

  if (!data)
    return;

  dash_d = strstr(data, "-d");
  if (dash_d)
    *show_dialog = 1;

  if (data[0] != 0 && data[0] != '-')
  {
    size_t length = dash_d ? (size_t)(dash_d - data) : strlen(data);

    while (length > 0 && (data[length - 1] == ' ' || data[length - 1] == '\t'))
      length--;

    if (length > 1023)
      length = 1023;

    memcpy(jobname, data, length);
    jobname[length] = 0;
  }
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  struct cdPrinterData* pdata;
  NSPrintInfo* print_info;
  NSSize paper;
  NSRect imageable;
  char jobname[1024];
  char* spool;
  double w_mm, h_mm, dpi = 300.0;
  int show_dialog = 0;

  sParseData((const char*)data, jobname, &show_dialog);

  print_info = [[NSPrintInfo sharedPrintInfo] copy];
  if (!print_info)
    return;

  if (show_dialog)
  {
    /* CD_PRINTER's contract is that a cancelled dialog produces a NULL canvas. */
    if ([[NSPrintPanel printPanel] runModalWithPrintInfo:print_info] != NSModalResponseOK)
    {
      [print_info release];
      return;
    }
  }

  /* Print inside the imageable area rather than the full sheet, so nothing lands in the
     printer's unprintable margin. Both are in points. */
  paper = [print_info paperSize];
  imageable = [print_info imageablePageBounds];
  if (imageable.size.width <= 0 || imageable.size.height <= 0)
    imageable = NSMakeRect(0, 0, paper.width, paper.height);

  [print_info setLeftMargin:imageable.origin.x];
  [print_info setBottomMargin:imageable.origin.y];
  [print_info setRightMargin:paper.width - NSMaxX(imageable)];
  [print_info setTopMargin:paper.height - NSMaxY(imageable)];
  [print_info setHorizontallyCentered:NO];
  [print_info setVerticallyCentered:NO];

  w_mm = imageable.size.width / 72.0 * 25.4;
  h_mm = imageable.size.height / 72.0 * 25.4;

  spool = sCreateSpoolFilename();
  if (!spool)
  {
    [print_info release];
    return;
  }

  ctxcanvas = cdquartzPDFCreateCanvas(canvas, spool, w_mm, h_mm, dpi);
  if (!ctxcanvas)
  {
    free(spool);
    [print_info release];
    return;
  }

  pdata = (struct cdPrinterData*)malloc(sizeof(struct cdPrinterData));
  if (!pdata)
  {
    free(spool);
    [print_info release];
    return;
  }

  pdata->spool_filename = spool;
  pdata->print_info = print_info;                       /* the copy is owned from here */
  pdata->job_name = jobname[0] ? [[NSString stringWithUTF8String:jobname] retain] : nil;

  ctxcanvas->clipboard_data = pdata;
}

static void cdinittable(cdCanvas* canvas)
{
  cdquartzPDFInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdQuartzPrinterContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE |
                 CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV),
  CD_CTX_DEVICE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextPrinter(void)
{
  return &cdQuartzPrinterContext;
}
