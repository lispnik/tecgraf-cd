/** \file
 * \brief Quartz Clipboard Driver
 *
 * Draws into an offscreen context and hands the result to the general
 * NSPasteboard when the canvas is killed.
 *
 * The creation data is "widthxheight [resolution]" in pixels, optionally
 * followed by a format flag, mirroring the Windows clipboard driver:
 *
 *   (none)  PDF, vector and natively pasteable into other macOS applications
 *   -b      bitmap, placed on the pasteboard as TIFF
 *   -m      CD metafile, the format the X11 and GDK drivers use
 *
 * A private pasteboard can be selected with -p<name>, which keeps tests from
 * disturbing what the user has copied.
 *
 * Play accepts any of them: a CD metafile is replayed through the metafile
 * driver, anything else is rendered and blitted as an image.
 *
 * See Copyright Notice in cd.h
 */

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdquartzctx.h"
#include "cdclipbd.h"
#include "cdmf.h"
#include "cdmf_private.h"

/* Private pasteboard type carrying a CD metafile, so that a copy made by CD
   can be pasted back into CD without losing anything. */
#define CD_PASTEBOARD_TYPE_MF @"br.puc-rio.tecgraf.cd.metafile"

enum { CD_CLIP_PDF, CD_CLIP_BITMAP, CD_CLIP_MF };

/* cxCreateCanvas runs immediately before cxInitTable on the same thread, and
   the metafile mode installs a completely different function table, so the
   mode chosen while creating is handed over through this. CD is not thread
   safe to begin with. */
static int sCreatingMode = CD_CLIP_PDF;

/*****************************************************************************\
* Pasteboard                                                                  *
\*****************************************************************************/

/* The general pasteboard is used unless the data string names a private one
   with -p<name>, which lets the tests run without clobbering what the user
   has copied. */
static NSString* sPasteboardName(const char* str_data)
{
  const char* p;

  if (!str_data)
    return nil;

  p = strstr(str_data, "-p");
  if (!p)
    return nil;

  p += 2;
  while (*p == ' ')
    p++;

  {
    int len = 0;
    while (p[len] && p[len] != ' ')
      len++;

    if (len == 0)
      return nil;

    return [[[NSString alloc] initWithBytes:p length:len
                                   encoding:NSUTF8StringEncoding] autorelease];
  }
}

static NSPasteboard* sGetPasteboard(NSString* name)
{
  if (name)
    return [NSPasteboard pasteboardWithName:name];

  return [NSPasteboard generalPasteboard];
}

static void sPutOnPasteboard(NSPasteboard* pb, NSData* data, NSString* type)
{
  if (!data || [data length] == 0)
    return;

  [pb clearContents];
  [pb setData:data forType:type];
}

/*****************************************************************************\
* PDF and bitmap canvases                                                     *
\*****************************************************************************/

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  NSMutableData* pdf = (NSMutableData*)ctxcanvas->clipboard_data;
  NSString* name = (NSString*)ctxcanvas->clipboard_name;
  NSPasteboard* pb = sGetPasteboard(name);
  int mode = ctxcanvas->clipboard_mode;
  CGContextRef cgc = ctxcanvas->cgc;

  if (mode == CD_CLIP_PDF)
  {
    CGPDFContextEndPage(cgc);
    CGPDFContextClose(cgc);

    sPutOnPasteboard(pb, pdf, NSPasteboardTypePDF);
  }
  else
  {
    CGImageRef image;

    CGContextFlush(cgc);

    image = CGBitmapContextCreateImage(cgc);
    if (image)
    {
      NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithCGImage:image];
      sPutOnPasteboard(pb, [rep representationUsingType:NSBitmapImageFileTypeTIFF properties:@{}],
                       NSPasteboardTypeTIFF);
      [rep release];
      CGImageRelease(image);
    }
  }

  ctxcanvas->clipboard_data = NULL;
  ctxcanvas->clipboard_name = NULL;
  cdquartzKillCanvas(ctxcanvas);

  [pdf release];
  [name release];
}

/* The metafile mode reuses the metafile driver wholesale, exactly as the X11
   and GDK clipboard drivers do. */
static void cdkillcanvas_mf(cdCtxCanvas* ctxcanvas)
{
  cdCanvasMF* mfcanvas = (cdCanvasMF*)ctxcanvas;
  NSString* name = (NSString*)mfcanvas->data;
  char filename[10240];
  NSData* data;

  strcpy(filename, mfcanvas->filename);

  cdkillcanvasMF(mfcanvas);

  data = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:filename]];
  if (data)
    sPutOnPasteboard(sGetPasteboard(name), data, CD_PASTEBOARD_TYPE_MF);

  remove(filename);
  [name release];
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  char* str_data = (char*)data;
  cdCtxCanvas* ctxcanvas;
  CGContextRef cgc = NULL;
  NSMutableData* pdf = NULL;
  unsigned char* buffer = NULL;
  int w = 0, h = 0, mode = CD_CLIP_PDF;
  double res = 0;

  if (!str_data)
    return;

  if (strstr(str_data, "-b"))
    mode = CD_CLIP_BITMAP;
  else if (strstr(str_data, "-m"))
    mode = CD_CLIP_MF;

  if (sscanf(str_data, "%dx%d %lg", &w, &h, &res) < 2)
    return;

  if (w <= 0 || h <= 0)
    return;

  if (res <= 0)
    res = cdquartzGetResolution();

  sCreatingMode = mode;

  if (mode == CD_CLIP_MF)
  {
    /* the metafile driver wants a file name and millimetres */
    char params[10240];

    if (!cdStrTmpFileName(params))
      return;

    sprintf(params + strlen(params), " %gx%g %g", w/res, h/res, res);

    cdcreatecanvasMF(canvas, params);
    if (canvas->ctxcanvas)
      ((cdCanvasMF*)canvas->ctxcanvas)->data = [sPasteboardName(str_data) retain];
    return;
  }

  if (mode == CD_CLIP_PDF)
  {
    CGRect media = CGRectMake(0, 0, (CGFloat)w, (CGFloat)h);
    CGDataConsumerRef consumer;

    pdf = [[NSMutableData alloc] init];
    consumer = CGDataConsumerCreateWithCFData((CFMutableDataRef)pdf);
    if (!consumer)
    {
      [pdf release];
      return;
    }

    cgc = CGPDFContextCreate(consumer, &media, NULL);
    CGDataConsumerRelease(consumer);

    if (!cgc)
    {
      [pdf release];
      return;
    }

    CGPDFContextBeginPage(cgc, NULL);
  }
  else
  {
    cgc = cdquartzCreateBitmap(w, h, &buffer);
    if (!cgc)
      return;
  }

  ctxcanvas = cdquartzCreateCanvas(canvas, cgc);
  if (!ctxcanvas)
  {
    CGContextRelease(cgc);
    if (buffer) free(buffer);
    [pdf release];
    return;
  }

  ctxcanvas->owns_cgc = 1;
  ctxcanvas->clipboard_mode = mode;
  ctxcanvas->clipboard_data = pdf;
  ctxcanvas->clipboard_name = [sPasteboardName(str_data) retain];

  if (buffer)
  {
    ctxcanvas->data = buffer;
    ctxcanvas->owns_data = 1;
  }

  cdquartzSetCanvasSize(canvas, w, h);
  canvas->xres = res;
  canvas->yres = res;
  canvas->w_mm = w/res;
  canvas->h_mm = h/res;
}

static void cdinittable(cdCanvas* canvas)
{
  if (sCreatingMode == CD_CLIP_MF)
  {
    cdinittableMF(canvas);
    canvas->cxKillCanvas = cdkillcanvas_mf;
    return;
  }

  cdquartzInitTable(canvas);
  canvas->cxKillCanvas = cdkillcanvas;
}

/*****************************************************************************\
* Play                                                                        *
\*****************************************************************************/

/* Renders whatever image the pasteboard holds and blits it through the public
   API, so that it works no matter which driver the target canvas uses. */
static int sPlayImage(cdCanvas* canvas, NSData* data, int xmin, int xmax, int ymin, int ymax)
{
  NSImage* image = [[NSImage alloc] initWithData:data];
  NSBitmapImageRep* rep;
  int w, h, x, y, ret = CD_ERROR;
  unsigned char *r = NULL, *g = NULL, *b = NULL;

  if (!image)
    return CD_ERROR;

  w = (int)[image size].width;
  h = (int)[image size].height;

  if (w <= 0 || h <= 0)
  {
    [image release];
    return CD_ERROR;
  }

  /* 24 bit without alpha is not a format CoreGraphics can back a bitmap
     context with, so the rep has to carry alpha for the drawing to land */
  rep = [[NSBitmapImageRep alloc]
      initWithBitmapDataPlanes:NULL pixelsWide:w pixelsHigh:h
      bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO
      colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];

  if (rep)
  {
    NSGraphicsContext* gc = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];

    if (!gc)
    {
      [rep release];
      [image release];
      return CD_ERROR;
    }

    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:gc];
    [[NSColor whiteColor] setFill];
    NSRectFill(NSMakeRect(0, 0, w, h));
    [image drawInRect:NSMakeRect(0, 0, w, h)];
    [NSGraphicsContext restoreGraphicsState];

    r = (unsigned char*)malloc((size_t)w*h);
    g = (unsigned char*)malloc((size_t)w*h);
    b = (unsigned char*)malloc((size_t)w*h);

    if (r && g && b)
    {
      unsigned char* bits = [rep bitmapData];
      NSInteger stride = [rep bytesPerRow];
      NSInteger spp = [rep samplesPerPixel];

      for (y = 0; y < h; y++)
      {
        /* CD image data is bottom-up, the rep rows are top-down */
        unsigned char* line = bits + (size_t)(h-1-y)*stride;
        for (x = 0; x < w; x++)
        {
          size_t pos = (size_t)y*w + x;
          r[pos] = line[x*spp+0];
          g[pos] = line[x*spp+1];
          b[pos] = line[x*spp+2];
        }
      }

      if (xmin == 0 && xmax == 0 && ymin == 0 && ymax == 0)
        cdCanvasPutImageRectRGB(canvas, w, h, r, g, b, 0, 0, w, h, 0, 0, 0, 0);
      else
        cdCanvasPutImageRectRGB(canvas, w, h, r, g, b, xmin, ymin,
                                xmax-xmin+1, ymax-ymin+1, 0, 0, 0, 0);

      ret = CD_OK;
    }

    free(r); free(g); free(b);
    [rep release];
  }

  [image release];
  return ret;
}

static int cdplay(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void* data)
{
  NSPasteboard* pb = sGetPasteboard(sPasteboardName((const char*)data));
  NSData* mf;
  NSString* type;

  /* a CD metafile round-trips without loss, so prefer it */
  mf = [pb dataForType:CD_PASTEBOARD_TYPE_MF];
  if (mf)
  {
    char filename[10240];
    int ret;

    if (!cdStrTmpFileName(filename))
      return CD_ERROR;

    if (![mf writeToFile:[NSString stringWithUTF8String:filename] atomically:NO])
      return CD_ERROR;

    ret = cdCanvasPlay(canvas, CD_METAFILE, xmin, xmax, ymin, ymax, filename);
    remove(filename);
    return ret;
  }

  type = [pb availableTypeFromArray:@[NSPasteboardTypePDF,
                                      NSPasteboardTypeTIFF,
                                      NSPasteboardTypePNG]];
  if (!type)
    return CD_ERROR;

  return sPlayImage(canvas, [pb dataForType:type], xmin, xmax, ymin, ymax);
}

/*****************************************************************************\
* Context                                                                     *
\*****************************************************************************/

static cdContext cdClipboardContext =
{
  CD_CAP_ALL & ~(CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  CD_CTX_DEVICE,
  cdcreatecanvas,
  cdinittable,
  cdplay,
  NULL
};

cdContext* cdContextClipboard(void)
{
  return &cdClipboardContext;
}
