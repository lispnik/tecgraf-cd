/** \file
 * \brief Quartz Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDQUARTZCTX_H
#define __CDQUARTZCTX_H

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

#include "cd.h"
#include "cd_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Server image. Always backed by a bitmap context so it can be both
   drawn into (cxGetImage) and drawn from (cxPutImageRect). */
struct _cdCtxImage
{
  int w, h;
  double w_mm, h_mm;
  double xres, yres;
  int bpp;

  CGContextRef bitmap;
  unsigned char* data;   /* premultiplied BGRA, top-down, owned */
};

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  CGContextRef cgc;
  int owns_cgc;            /* release cgc in KillCanvas */

  /* bitmap backing, when there is one (bitmap, image and dbuffer contexts).
     Required by GetImageRGB and by the double buffer blit. */
  unsigned char* data;     /* premultiplied BGRA, top-down, NOT owned unless owns_data */
  int owns_data;

  void* view;              /* NSView*, retained, used by the native window driver */
  int focus_locked;        /* the native window driver focused the view itself */

  /* text */
  CTFontRef font;
  double font_ascent, font_descent, font_leading, font_maxwidth;

  /* fill */
  CGPatternRef pattern;
  CGColorSpaceRef pattern_space;
  int hatchboxsize;

  /* line */
  CGFloat* dashes;
  int dashes_count;

  /* CD calls cxTransform before it updates canvas->matrix, so the driver
     keeps its own copy */
  double matrix[6];
  int use_matrix;

  /* the base graphics state is saved once at creation so that clipping,
     which is subtractive in Quartz, can be reset by restoring it */
  int gstate_saved;

  cdImage* image_dbuffer;    /* used by the double buffer driver */
  cdCanvas* canvas_dbuffer;

  /* used by the clipboard driver */
  int clipboard_mode;
  void* clipboard_data;      /* NSMutableData* holding the PDF being written */
  void* clipboard_name;      /* NSString*, retained, nil for the general pasteboard */
};

#define cdQuartzRed(_)   (((CGFloat)cdRed(_))/255.)
#define cdQuartzGreen(_) (((CGFloat)cdGreen(_))/255.)
#define cdQuartzBlue(_)  (((CGFloat)cdBlue(_))/255.)
#define cdQuartzAlpha(_) (((CGFloat)cdAlpha(_))/255.)

/* base driver */
cdCtxCanvas* cdquartzCreateCanvas(cdCanvas* canvas, CGContextRef cgc);
void cdquartzInitTable(cdCanvas* canvas);
void cdquartzKillCanvas(cdCtxCanvas* ctxcanvas);

/* re-targets an existing context canvas, used when a window is resized */
void cdquartzUpdateCanvas(cdCtxCanvas* ctxcanvas, CGContextRef cgc);

/* creates a premultiplied BGRA bitmap context of w x h, bottom-up user space.
   Returns NULL on failure, otherwise *data receives the owned pixel buffer. */
CGContextRef cdquartzCreateBitmap(int w, int h, unsigned char** data);

/* true when the canvas is driven by the Quartz base driver, so its cdCtxImage is ours */
int cdquartzIsCanvas(cdCanvas* canvas);

/* fills in canvas size/resolution fields from a pixel size */
void cdquartzSetCanvasSize(cdCanvas* canvas, int w, int h);

/* default screen resolution in pixels/mm */
double cdquartzGetResolution(void);

#ifdef __cplusplus
}
#endif

#endif
