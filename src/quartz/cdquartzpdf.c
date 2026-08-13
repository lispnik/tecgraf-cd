/** \file
 * \brief Quartz PDF Driver
 *
 * Writes a PDF file through CoreGraphics' CGPDFContext, so it needs no
 * third-party library. It replaces the PDFlib-based driver in src/drv/cdpdf.c
 * on macOS: that one is 220 calls into PDFlib, which is commercial and is no
 * longer distributed at all, so CD_ENABLE_PDF is off by default and CD_PDF
 * simply does not exist here.
 *
 * All the drawing comes from the shared Quartz base in cdquartz.c, which works
 * against any CGContextRef. Only the document lifecycle is specific to this
 * driver: create the context, start a page, turn cdCanvasFlush into a page
 * break, and close the document. cdquartzclipboard.m already drives a
 * CGPDFContext through the same base for clipboard PDF.
 *
 * The data string is the one documented for CD_PDF, so this is drop-in:
 *
 *   "filename -p[paper] -w[width_mm] -h[height_mm] -s[resolution_dpi] [-o]"
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdquartzctx.h"
#include "cdpdf.h"


/* CD coordinates are pixels at the canvas resolution; PDF user space is points.
   Everything the base driver emits therefore goes through this scale. */
static double sPointsPerPixel(cdCanvas* canvas)
{
  double dpi = canvas->xres * 25.4;

  if (dpi <= 0)
    dpi = 300;

  return 72.0 / dpi;
}

/* Puts a freshly begun page into the state the base driver expects: the CTM
   scale has to be applied BEFORE the base graphics state is saved, because
   clipping is implemented by restoring to that state and re-saving, and a
   scale applied afterwards would be discarded by the first clip change. */
static void sBeginPage(cdCtxCanvas* ctxcanvas)
{
  CGContextRef cgc = ctxcanvas->cgc;
  double scale = sPointsPerPixel(ctxcanvas->canvas);

  CGPDFContextBeginPage(cgc, NULL);
  CGContextScaleCTM(cgc, (CGFloat)scale, (CGFloat)scale);
}

static void sEndPage(cdCtxCanvas* ctxcanvas)
{
  /* unwind our own base state before closing the page: CGPDFContextEndPage
     resets the graphics state stack, and leaving it unbalanced would make the
     next restore pop something that no longer belongs to us */
  if (ctxcanvas->gstate_saved)
  {
    CGContextRestoreGState(ctxcanvas->cgc);
    ctxcanvas->gstate_saved = 0;
  }

  CGPDFContextEndPage(ctxcanvas->cgc);
}

/* cdCanvasFlush means "start a new page" for a paged file format, the same as
   it does in the PDFlib and PostScript drivers. */
static void cdflush(cdCtxCanvas* ctxcanvas)
{
  CGContextRef cgc = ctxcanvas->cgc;
  int owns_cgc = ctxcanvas->owns_cgc;

  sEndPage(ctxcanvas);
  sBeginPage(ctxcanvas);

  /* Re-establish the base state and replay clipping and the transform onto the
     new page. cdquartzUpdateCanvas does exactly that, but it also releases the
     old context, so hide ownership from it -- the context is unchanged here. */
  ctxcanvas->owns_cgc = 0;
  cdquartzUpdateCanvas(ctxcanvas, cgc);
  ctxcanvas->owns_cgc = owns_cgc;
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->cgc)
  {
    sEndPage(ctxcanvas);
    CGPDFContextClose(ctxcanvas->cgc);
  }

  /* the base releases the context, since owns_cgc is set */
  cdquartzKillCanvas(ctxcanvas);
}

/* "filename -p[paper] -w[mm] -h[mm] -s[dpi] [-o]", as documented for CD_PDF */
static void sParseData(const char* data, char* filename, double* w_mm, double* h_mm,
                       double* dpi, int* landscape)
{
  const char* line = data;
  double w_pt = 0, h_pt = 0;
  int paper = CD_A4;

  cdSetPaperSize(paper, &w_pt, &h_pt);
  *w_mm = w_pt / 72.0 * 25.4;
  *h_mm = h_pt / 72.0 * 25.4;
  *dpi = 300;
  *landscape = 0;

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;

  while (*line != 0)
  {
    while (*line != 0 && *line != '-')
      line++;

    if (*line == 0)
      break;

    line++;  /* skip the '-' */

    switch (*line++)
    {
      case 'p':
        if (sscanf(line, "%d", &paper) == 1)
        {
          cdSetPaperSize(paper, &w_pt, &h_pt);
          *w_mm = w_pt / 72.0 * 25.4;
          *h_mm = h_pt / 72.0 * 25.4;
        }
        break;
      case 'w':
        sscanf(line, "%lg", w_mm);
        break;
      case 'h':
        sscanf(line, "%lg", h_mm);
        break;
      case 's':
        sscanf(line, "%lg", dpi);
        break;
      case 'o':
        *landscape = 1;
        break;
    }
  }
}

static CFDictionaryRef sCreateAuxiliaryInfo(void)
{
  CFMutableDictionaryRef info = CFDictionaryCreateMutable(NULL, 0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);
  if (info)
    CFDictionarySetValue(info, kCGPDFContextCreator, CFSTR("CD (Canvas Draw)"));

  return info;
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  CGContextRef cgc;
  CFURLRef url;
  CFStringRef path;
  CFDictionaryRef info;
  CGRect media;
  char filename[10240] = "";
  double w_mm, h_mm, dpi, points_per_pixel;
  int landscape;

  if (!data)
    return;

  sParseData((const char*)data, filename, &w_mm, &h_mm, &dpi, &landscape);
  if (filename[0] == 0)
    return;

  if (landscape)
  {
    double tmp = w_mm;
    w_mm = h_mm;
    h_mm = tmp;
  }

  if (w_mm <= 0 || h_mm <= 0 || dpi <= 0)
    return;

  path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
  if (!path)
    return;

  url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
  CFRelease(path);
  if (!url)
    return;

  media = CGRectMake(0, 0, (CGFloat)(w_mm / 25.4 * 72.0), (CGFloat)(h_mm / 25.4 * 72.0));

  info = sCreateAuxiliaryInfo();
  cgc = CGPDFContextCreateWithURL(url, &media, info);
  CFRelease(url);
  if (info)
    CFRelease(info);

  if (!cgc)
    return;

  /* The size fields must be right before the first page is set up, because the
     CTM scale is derived from the resolution. */
  cdquartzSetCanvasSize(canvas, (int)(w_mm / 25.4 * dpi), (int)(h_mm / 25.4 * dpi));
  canvas->xres = dpi / 25.4;
  canvas->yres = dpi / 25.4;
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;

  points_per_pixel = 72.0 / dpi;

  CGPDFContextBeginPage(cgc, NULL);
  CGContextScaleCTM(cgc, (CGFloat)points_per_pixel, (CGFloat)points_per_pixel);

  /* after the scale, so the base state carries it (see sBeginPage) */
  ctxcanvas = cdquartzCreateCanvas(canvas, cgc);
  if (!ctxcanvas)
  {
    CGPDFContextClose(cgc);
    CGContextRelease(cgc);
    return;
  }

  ctxcanvas->owns_cgc = 1;
}

static void cdinittable(cdCanvas* canvas)
{
  cdquartzInitTable(canvas);

  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;

  /* a PDF page has no pixels to read back, as for the native window driver */
  canvas->cxGetImageRGB = NULL;
  canvas->cxScrollArea = NULL;
}

static cdContext cdQuartzPDFContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE |
                 CD_CAP_GETIMAGERGB),
  CD_CTX_FILE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextPDF(void)
{
  return &cdQuartzPDFContext;
}
