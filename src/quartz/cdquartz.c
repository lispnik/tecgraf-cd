/** \file
 * \brief Quartz Base Driver
 *
 * Implements the CD driver interface on top of CoreGraphics and CoreText.
 * The driver is device independent: it draws into whatever CGContextRef it
 * is given, so the same code serves the bitmap, image, double buffer and
 * native window contexts.
 *
 * Quartz uses a bottom-up coordinate system just like CD, so this driver
 * declares CD_CAP_YAXIS and never asks CD to invert coordinates.
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "cdquartzctx.h"
#include "cdimage.h"

#ifndef CD_DEG2RAD
#define CD_DEG2RAD 0.01745329251994329576
#endif

/* Quartz strokes centered on the path. Integer CD coordinates address pixel
   centers, so half a pixel is added to keep thin lines crisp. */
#define CD_HALF 0.5

/*****************************************************************************\
* Graphics state                                                              *
\*****************************************************************************/

static void sSetFillColor(cdCtxCanvas* ctxcanvas, long color)
{
  CGContextSetRGBFillColor(ctxcanvas->cgc, cdQuartzRed(color), cdQuartzGreen(color),
                                           cdQuartzBlue(color), cdQuartzAlpha(color));
}

static void sSetStrokeColor(cdCtxCanvas* ctxcanvas, long color)
{
  CGContextSetRGBStrokeColor(ctxcanvas->cgc, cdQuartzRed(color), cdQuartzGreen(color),
                                             cdQuartzBlue(color), cdQuartzAlpha(color));
}

/* Applies every stroke attribute. Called before stroking instead of being
   tracked incrementally, so that the graphics state never drifts out of sync
   with the canvas after a clip reset. */
static void sUpdateStroke(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  sSetStrokeColor(ctxcanvas, canvas->foreground);

  CGContextSetLineWidth(ctxcanvas->cgc, canvas->line_width < 1? 1.0: (CGFloat)canvas->line_width);

  switch (canvas->line_cap)
  {
  case CD_CAPFLAT:   CGContextSetLineCap(ctxcanvas->cgc, kCGLineCapButt);   break;
  case CD_CAPROUND:  CGContextSetLineCap(ctxcanvas->cgc, kCGLineCapRound);  break;
  case CD_CAPSQUARE: CGContextSetLineCap(ctxcanvas->cgc, kCGLineCapSquare); break;
  }

  switch (canvas->line_join)
  {
  case CD_MITER: CGContextSetLineJoin(ctxcanvas->cgc, kCGLineJoinMiter); break;
  case CD_BEVEL: CGContextSetLineJoin(ctxcanvas->cgc, kCGLineJoinBevel); break;
  case CD_ROUND: CGContextSetLineJoin(ctxcanvas->cgc, kCGLineJoinRound); break;
  }

  if (ctxcanvas->dashes && ctxcanvas->dashes_count)
    CGContextSetLineDash(ctxcanvas->cgc, 0, ctxcanvas->dashes, ctxcanvas->dashes_count);
  else
    CGContextSetLineDash(ctxcanvas->cgc, 0, NULL, 0);
}

/* Selects the fill source: a solid color, or the pattern built by
   cxHatch/cxStipple/cxPattern. */
static void sUpdateFill(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  if (canvas->interior_style != CD_SOLID && ctxcanvas->pattern && ctxcanvas->pattern_space)
  {
    CGFloat alpha = 1.0;
    CGContextSetFillColorSpace(ctxcanvas->cgc, ctxcanvas->pattern_space);
    CGContextSetFillPattern(ctxcanvas->cgc, ctxcanvas->pattern, &alpha);
  }
  else
  {
    CGContextSetFillColorSpace(ctxcanvas->cgc, NULL);
    sSetFillColor(ctxcanvas, canvas->foreground);
  }
}

static void sApplyMatrix(cdCtxCanvas* ctxcanvas)
{
  const double* m = ctxcanvas->matrix;
  CGAffineTransform t = CGAffineTransformMake((CGFloat)m[0], (CGFloat)m[1],
                                              (CGFloat)m[2], (CGFloat)m[3],
                                              (CGFloat)m[4], (CGFloat)m[5]);
  CGContextConcatCTM(ctxcanvas->cgc, t);
}

static void sSetClipPath(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  CGMutablePathRef path;
  int i;

  if (canvas->clip_poly)
  {
    path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, (CGFloat)canvas->clip_poly[0].x, (CGFloat)canvas->clip_poly[0].y);
    for (i = 1; i < canvas->clip_poly_n; i++)
      CGPathAddLineToPoint(path, NULL, (CGFloat)canvas->clip_poly[i].x, (CGFloat)canvas->clip_poly[i].y);
  }
  else if (canvas->clip_fpoly)
  {
    path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, (CGFloat)canvas->clip_fpoly[0].x, (CGFloat)canvas->clip_fpoly[0].y);
    for (i = 1; i < canvas->clip_poly_n; i++)
      CGPathAddLineToPoint(path, NULL, (CGFloat)canvas->clip_fpoly[i].x, (CGFloat)canvas->clip_fpoly[i].y);
  }
  else
    return;

  CGPathCloseSubpath(path);

  CGContextBeginPath(ctxcanvas->cgc);
  CGContextAddPath(ctxcanvas->cgc, path);
  if (canvas->fill_mode == CD_EVENODD)
    CGContextEOClip(ctxcanvas->cgc);
  else
    CGContextClip(ctxcanvas->cgc);

  CGPathRelease(path);
}

/* Clipping in Quartz only ever shrinks the clip area, so removing a clip
   means going back to the state saved when the canvas was created and
   re-applying whatever should still be in effect. */
static void sUpdateClip(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  if (ctxcanvas->gstate_saved)
    CGContextRestoreGState(ctxcanvas->cgc);

  CGContextSaveGState(ctxcanvas->cgc);
  ctxcanvas->gstate_saved = 1;

  if (canvas->clip_mode == CD_CLIPAREA)
  {
    cdRect* r = &canvas->clip_rect;
    CGContextClipToRect(ctxcanvas->cgc, CGRectMake((CGFloat)r->xmin, (CGFloat)r->ymin,
                                                   (CGFloat)(r->xmax - r->xmin + 1),
                                                   (CGFloat)(r->ymax - r->ymin + 1)));
  }
  else if (canvas->clip_mode == CD_CLIPPOLYGON)
    sSetClipPath(ctxcanvas);

  /* the transformation shares the graphics state with the clip */
  if (ctxcanvas->use_matrix)
    sApplyMatrix(ctxcanvas);
}

static int cdclip(cdCtxCanvas* ctxcanvas, int mode)
{
  ctxcanvas->canvas->clip_mode = mode;
  sUpdateClip(ctxcanvas);
  return mode;
}

static void cdcliparea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  (void)xmin; (void)xmax; (void)ymin; (void)ymax;  /* already stored in canvas->clip_rect */
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    sUpdateClip(ctxcanvas);
}

static void cdtransform(cdCtxCanvas* ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    memcpy(ctxcanvas->matrix, matrix, sizeof(double)*6);
    ctxcanvas->use_matrix = 1;
  }
  else
    ctxcanvas->use_matrix = 0;

  sUpdateClip(ctxcanvas);
}

/*****************************************************************************\
* Primitives                                                                  *
\*****************************************************************************/

static void cdpixel(cdCtxCanvas* ctxcanvas, int x, int y, long color)
{
  CGContextSetFillColorSpace(ctxcanvas->cgc, NULL);
  sSetFillColor(ctxcanvas, color);
  CGContextFillRect(ctxcanvas->cgc, CGRectMake((CGFloat)x, (CGFloat)y, 1, 1));
}

static void cdfpixel(cdCtxCanvas* ctxcanvas, double x, double y, long color)
{
  CGContextSetFillColorSpace(ctxcanvas->cgc, NULL);
  sSetFillColor(ctxcanvas, color);
  CGContextFillRect(ctxcanvas->cgc, CGRectMake((CGFloat)x, (CGFloat)y, 1, 1));
}

static void cdfline(cdCtxCanvas* ctxcanvas, double x1, double y1, double x2, double y2)
{
  sUpdateStroke(ctxcanvas);
  CGContextBeginPath(ctxcanvas->cgc);
  CGContextMoveToPoint(ctxcanvas->cgc, (CGFloat)x1, (CGFloat)y1);
  CGContextAddLineToPoint(ctxcanvas->cgc, (CGFloat)x2, (CGFloat)y2);
  CGContextStrokePath(ctxcanvas->cgc);
}

static void cdline(cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdfline(ctxcanvas, x1 + CD_HALF, y1 + CD_HALF, x2 + CD_HALF, y2 + CD_HALF);
}

static void cdfrect(cdCtxCanvas* ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateStroke(ctxcanvas);
  CGContextStrokeRect(ctxcanvas->cgc, CGRectMake((CGFloat)xmin, (CGFloat)ymin,
                                                 (CGFloat)(xmax - xmin), (CGFloat)(ymax - ymin)));
}

static void cdrect(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfrect(ctxcanvas, xmin + CD_HALF, xmax + CD_HALF, ymin + CD_HALF, ymax + CD_HALF);
}

static void cdfbox(cdCtxCanvas* ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas);
  CGContextFillRect(ctxcanvas->cgc, CGRectMake((CGFloat)xmin, (CGFloat)ymin,
                                               (CGFloat)(xmax - xmin), (CGFloat)(ymax - ymin)));
}

static void cdbox(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  /* CD box limits are inclusive */
  cdfbox(ctxcanvas, (double)xmin, (double)(xmax + 1), (double)ymin, (double)(ymax + 1));
}

/* CD angles are counter-clockwise degrees, which is also the Quartz direction
   in a bottom-up context. The arc is built through a transform so that the
   same code handles circles and ellipses. */
static CGMutablePathRef sCreateArcPath(double xc, double yc, double w, double h,
                                       double a1, double a2, int close_center, int close_chord)
{
  CGMutablePathRef path = CGPathCreateMutable();
  CGAffineTransform t;

  if (a2 < a1)
    a2 += 360;

  t = CGAffineTransformMakeTranslation((CGFloat)xc, (CGFloat)yc);
  t = CGAffineTransformScale(t, (CGFloat)(w/2.0), (CGFloat)(h/2.0));

  if (close_center)
    CGPathMoveToPoint(path, NULL, (CGFloat)xc, (CGFloat)yc);

  CGPathAddArc(path, &t, 0, 0, 1,
               (CGFloat)(a1*CD_DEG2RAD), (CGFloat)(a2*CD_DEG2RAD), false);

  if (close_center || close_chord)
    CGPathCloseSubpath(path);

  return path;
}

static void cdfarc(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  CGMutablePathRef path = sCreateArcPath(xc, yc, w, h, a1, a2, 0, 0);
  sUpdateStroke(ctxcanvas);
  CGContextBeginPath(ctxcanvas->cgc);
  CGContextAddPath(ctxcanvas->cgc, path);
  CGContextStrokePath(ctxcanvas->cgc);
  CGPathRelease(path);
}

static void cdarc(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, xc + CD_HALF, yc + CD_HALF, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  CGMutablePathRef path = sCreateArcPath(xc, yc, w, h, a1, a2, 1, 0);
  sUpdateFill(ctxcanvas);
  CGContextBeginPath(ctxcanvas->cgc);
  CGContextAddPath(ctxcanvas->cgc, path);
  CGContextFillPath(ctxcanvas->cgc);
  CGPathRelease(path);
}

static void cdsector(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  CGMutablePathRef path = sCreateArcPath(xc, yc, w, h, a1, a2, 0, 1);
  sUpdateFill(ctxcanvas);
  CGContextBeginPath(ctxcanvas->cgc);
  CGContextAddPath(ctxcanvas->cgc, path);
  CGContextFillPath(ctxcanvas->cgc);
  CGPathRelease(path);
}

static void cdchord(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

/* Replays the path recorded by CD in canvas->path over the point list. */
static void sPlayPath(cdCtxCanvas* ctxcanvas, const cdfPoint* poly, int n)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  int p, i = 0;

  CGContextBeginPath(ctxcanvas->cgc);

  for (p = 0; p < canvas->path_n; p++)
  {
    switch (canvas->path[p])
    {
    case CD_PATH_NEW:
      CGContextBeginPath(ctxcanvas->cgc);
      break;
    case CD_PATH_MOVETO:
      if (i+1 > n) return;
      CGContextMoveToPoint(ctxcanvas->cgc, (CGFloat)poly[i].x, (CGFloat)poly[i].y);
      i++;
      break;
    case CD_PATH_LINETO:
      if (i+1 > n) return;
      if (CGContextIsPathEmpty(ctxcanvas->cgc))
        CGContextMoveToPoint(ctxcanvas->cgc, (CGFloat)poly[i].x, (CGFloat)poly[i].y);
      else
        CGContextAddLineToPoint(ctxcanvas->cgc, (CGFloat)poly[i].x, (CGFloat)poly[i].y);
      i++;
      break;
    case CD_PATH_ARC:
      {
        double xc, yc, w, h, a1, a2;
        CGMutablePathRef arc;

        if (i+3 > n) return;
        if (!cdfGetArcPath(poly+i, &xc, &yc, &w, &h, &a1, &a2))
          return;

        arc = sCreateArcPath(xc, yc, w, h, a1, a2, 0, 0);
        CGContextAddPath(ctxcanvas->cgc, arc);
        CGPathRelease(arc);
        i += 3;
      }
      break;
    case CD_PATH_CURVETO:
      if (i+3 > n) return;
      if (CGContextIsPathEmpty(ctxcanvas->cgc))
        CGContextMoveToPoint(ctxcanvas->cgc, (CGFloat)poly[i].x, (CGFloat)poly[i].y);
      CGContextAddCurveToPoint(ctxcanvas->cgc,
                               (CGFloat)poly[i].x,   (CGFloat)poly[i].y,
                               (CGFloat)poly[i+1].x, (CGFloat)poly[i+1].y,
                               (CGFloat)poly[i+2].x, (CGFloat)poly[i+2].y);
      i += 3;
      break;
    case CD_PATH_CLOSE:
      CGContextClosePath(ctxcanvas->cgc);
      break;
    case CD_PATH_FILL:
      sUpdateFill(ctxcanvas);
      CGContextDrawPath(ctxcanvas->cgc, canvas->fill_mode == CD_EVENODD? kCGPathEOFill: kCGPathFill);
      break;
    case CD_PATH_STROKE:
      sUpdateStroke(ctxcanvas);
      CGContextDrawPath(ctxcanvas->cgc, kCGPathStroke);
      break;
    case CD_PATH_FILLSTROKE:
      sUpdateFill(ctxcanvas);
      sUpdateStroke(ctxcanvas);
      CGContextDrawPath(ctxcanvas->cgc, canvas->fill_mode == CD_EVENODD? kCGPathEOFillStroke: kCGPathFillStroke);
      break;
    case CD_PATH_CLIP:
      if (canvas->fill_mode == CD_EVENODD)
        CGContextEOClip(ctxcanvas->cgc);
      else
        CGContextClip(ctxcanvas->cgc);
      canvas->clip_mode = CD_CLIPPATH;
      break;
    }
  }
}

static void cdfpoly(cdCtxCanvas* ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;  /* handled by cdclip through canvas->clip_fpoly */

  if (mode == CD_PATH)
  {
    sPlayPath(ctxcanvas, poly, n);
    return;
  }

  if (n < 2)
    return;

  CGContextBeginPath(ctxcanvas->cgc);
  CGContextMoveToPoint(ctxcanvas->cgc, (CGFloat)poly[0].x, (CGFloat)poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i = 1; i+2 < n; i += 3)
      CGContextAddCurveToPoint(ctxcanvas->cgc,
                               (CGFloat)poly[i].x,   (CGFloat)poly[i].y,
                               (CGFloat)poly[i+1].x, (CGFloat)poly[i+1].y,
                               (CGFloat)poly[i+2].x, (CGFloat)poly[i+2].y);
  }
  else
  {
    for (i = 1; i < n; i++)
      CGContextAddLineToPoint(ctxcanvas->cgc, (CGFloat)poly[i].x, (CGFloat)poly[i].y);
  }

  switch (mode)
  {
  case CD_FILL:
    sUpdateFill(ctxcanvas);
    CGContextClosePath(ctxcanvas->cgc);
    CGContextDrawPath(ctxcanvas->cgc, ctxcanvas->canvas->fill_mode == CD_EVENODD? kCGPathEOFill: kCGPathFill);
    break;
  case CD_CLOSED_LINES:
    CGContextClosePath(ctxcanvas->cgc);
    /* fall through */
  case CD_OPEN_LINES:
  case CD_BEZIER:
  default:
    sUpdateStroke(ctxcanvas);
    CGContextDrawPath(ctxcanvas->cgc, kCGPathStroke);
    break;
  }
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  cdfPoint* fpoly;
  double offset = (mode == CD_FILL)? 0.0: CD_HALF;
  int i;

  if (mode == CD_CLIP)
    return;

  fpoly = (cdfPoint*)malloc(sizeof(cdfPoint)*n);
  if (!fpoly)
    return;

  for (i = 0; i < n; i++)
  {
    fpoly[i].x = poly[i].x + offset;
    fpoly[i].y = poly[i].y + offset;
  }

  cdfpoly(ctxcanvas, mode, fpoly, n);
  free(fpoly);
}

/*****************************************************************************\
* Fill patterns                                                               *
\*****************************************************************************/

static void sPatternDrawCallback(void* info, CGContextRef cgc)
{
  CGImageRef image = (CGImageRef)info;
  CGContextDrawImage(cgc, CGRectMake(0, 0, (CGFloat)CGImageGetWidth(image),
                                           (CGFloat)CGImageGetHeight(image)), image);
}

static void sPatternReleaseCallback(void* info)
{
  CGImageRelease((CGImageRef)info);
}

static void sKillPattern(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->pattern)
  {
    CGPatternRelease(ctxcanvas->pattern);
    ctxcanvas->pattern = NULL;
  }
  if (ctxcanvas->pattern_space)
  {
    CGColorSpaceRelease(ctxcanvas->pattern_space);
    ctxcanvas->pattern_space = NULL;
  }
}

/* Wraps a premultiplied BGRA buffer, top-down, into a CGImage.
   The buffer is handed over to the image. */
static CGImageRef sCreateImageFromBGRA(unsigned char* data, int w, int h)
{
  CGColorSpaceRef space;
  CGDataProviderRef provider;
  CGImageRef image;

  provider = CGDataProviderCreateWithData(NULL, data, (size_t)w*h*4, NULL);
  if (!provider)
    return NULL;

  space = CGColorSpaceCreateDeviceRGB();
  image = CGImageCreate(w, h, 8, 32, (size_t)w*4, space,
                        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst,
                        provider, NULL, false, kCGRenderingIntentDefault);

  CGColorSpaceRelease(space);
  CGDataProviderRelease(provider);
  return image;
}

static void sSetPixelBGRA(unsigned char* data, int w, int x, int y, long color)
{
  unsigned char* p = data + ((size_t)y*w + x)*4;
  unsigned char a = cdAlpha(color);
  /* premultiplied, little endian: B G R A */
  p[0] = (unsigned char)((cdBlue(color)  * a)/255);
  p[1] = (unsigned char)((cdGreen(color) * a)/255);
  p[2] = (unsigned char)((cdRed(color)   * a)/255);
  p[3] = a;
}

static void sBuildPattern(cdCtxCanvas* ctxcanvas, unsigned char* data, int w, int h)
{
  CGPatternCallbacks callbacks;
  CGImageRef image;

  sKillPattern(ctxcanvas);

  image = sCreateImageFromBGRA(data, w, h);
  if (!image)
  {
    free(data);
    return;
  }

  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.version = 0;
  callbacks.drawPattern = sPatternDrawCallback;
  callbacks.releaseInfo = sPatternReleaseCallback;

  ctxcanvas->pattern = CGPatternCreate((void*)image,
                                       CGRectMake(0, 0, (CGFloat)w, (CGFloat)h),
                                       CGAffineTransformIdentity,
                                       (CGFloat)w, (CGFloat)h,
                                       kCGPatternTilingConstantSpacing,
                                       true,  /* colored, foreground is baked in */
                                       &callbacks);
  ctxcanvas->pattern_space = CGColorSpaceCreatePattern(NULL);
}

static int cdhatch(cdCtxCanvas* ctxcanvas, int style)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  int size = ctxcanvas->hatchboxsize;
  int i, transparent = (canvas->back_opacity == CD_TRANSPARENT);
  unsigned char* data = (unsigned char*)calloc((size_t)size*size*4, 1);

  if (!data)
    return style;

  if (!transparent)
  {
    int x, y;
    for (y = 0; y < size; y++)
      for (x = 0; x < size; x++)
        sSetPixelBGRA(data, size, x, y, canvas->background);
  }

  for (i = 0; i < size; i++)
  {
    switch (style)
    {
    case CD_HORIZONTAL:
      sSetPixelBGRA(data, size, i, size/2, canvas->foreground);
      break;
    case CD_VERTICAL:
      sSetPixelBGRA(data, size, size/2, i, canvas->foreground);
      break;
    case CD_FDIAGONAL:
      sSetPixelBGRA(data, size, i, size-1-i, canvas->foreground);
      break;
    case CD_BDIAGONAL:
      sSetPixelBGRA(data, size, i, i, canvas->foreground);
      break;
    case CD_CROSS:
      sSetPixelBGRA(data, size, i, size/2, canvas->foreground);
      sSetPixelBGRA(data, size, size/2, i, canvas->foreground);
      break;
    case CD_DIAGCROSS:
      sSetPixelBGRA(data, size, i, i, canvas->foreground);
      sSetPixelBGRA(data, size, i, size-1-i, canvas->foreground);
      break;
    }
  }

  sBuildPattern(ctxcanvas, data, size, size);
  return style;
}

static void cdstipple(cdCtxCanvas* ctxcanvas, int w, int h, const unsigned char* stipple)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  int x, y, transparent = (canvas->back_opacity == CD_TRANSPARENT);
  unsigned char* data = (unsigned char*)calloc((size_t)w*h*4, 1);

  if (!data)
    return;

  /* CD images are bottom-up, CGImage rows are top-down */
  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      if (stipple[(size_t)(h-1-y)*w + x])
        sSetPixelBGRA(data, w, x, y, canvas->foreground);
      else if (!transparent)
        sSetPixelBGRA(data, w, x, y, canvas->background);
    }
  }

  sBuildPattern(ctxcanvas, data, w, h);
}

static void cdpattern(cdCtxCanvas* ctxcanvas, int w, int h, const long* pattern)
{
  int x, y;
  unsigned char* data = (unsigned char*)calloc((size_t)w*h*4, 1);

  if (!data)
    return;

  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
      sSetPixelBGRA(data, w, x, y, pattern[(size_t)(h-1-y)*w + x]);

  sBuildPattern(ctxcanvas, data, w, h);
}

static int cdinteriorstyle(cdCtxCanvas* ctxcanvas, int style)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  /* the pattern bakes in the current colors, so it must be rebuilt
     whenever it may have gone stale */
  switch (style)
  {
  case CD_HATCH:
    cdhatch(ctxcanvas, canvas->hatch_style);
    break;
  case CD_STIPPLE:
    if (canvas->stipple)
      cdstipple(ctxcanvas, canvas->stipple_w, canvas->stipple_h, canvas->stipple);
    break;
  case CD_PATTERN:
    if (canvas->pattern)
      cdpattern(ctxcanvas, canvas->pattern_w, canvas->pattern_h, canvas->pattern);
    break;
  default:
    sKillPattern(ctxcanvas);
    break;
  }

  return style;
}

static long cdforeground(cdCtxCanvas* ctxcanvas, long color)
{
  ctxcanvas->canvas->foreground = color;

  /* hatch and stipple patterns are drawn in the foreground color */
  if (ctxcanvas->canvas->interior_style == CD_HATCH ||
      ctxcanvas->canvas->interior_style == CD_STIPPLE)
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);

  return color;
}

static long cdbackground(cdCtxCanvas* ctxcanvas, long color)
{
  ctxcanvas->canvas->background = color;

  if (ctxcanvas->canvas->interior_style == CD_HATCH ||
      ctxcanvas->canvas->interior_style == CD_STIPPLE)
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);

  return color;
}

static int cdbackopacity(cdCtxCanvas* ctxcanvas, int opacity)
{
  ctxcanvas->canvas->back_opacity = opacity;

  if (ctxcanvas->canvas->interior_style == CD_HATCH ||
      ctxcanvas->canvas->interior_style == CD_STIPPLE)
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);

  return opacity;
}

/*****************************************************************************\
* Line attributes                                                             *
\*****************************************************************************/

static void sKillDashes(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->dashes)
  {
    free(ctxcanvas->dashes);
    ctxcanvas->dashes = NULL;
  }
  ctxcanvas->dashes_count = 0;
}

static void sSetDashes(cdCtxCanvas* ctxcanvas, const int* pattern, int count)
{
  int i;

  sKillDashes(ctxcanvas);

  ctxcanvas->dashes = (CGFloat*)malloc(sizeof(CGFloat)*count);
  if (!ctxcanvas->dashes)
    return;

  for (i = 0; i < count; i++)
    ctxcanvas->dashes[i] = (CGFloat)pattern[i];

  ctxcanvas->dashes_count = count;
}

static int cdlinestyle(cdCtxCanvas* ctxcanvas, int style)
{
  static const int dashed[]      = { 6, 2 };
  static const int dotted[]      = { 2, 2 };
  static const int dash_dot[]    = { 6, 2, 2, 2 };
  static const int dash_dot_dot[]= { 6, 2, 2, 2, 2, 2 };

  switch (style)
  {
  case CD_DASHED:          sSetDashes(ctxcanvas, dashed, 2);       break;
  case CD_DOTTED:          sSetDashes(ctxcanvas, dotted, 2);       break;
  case CD_DASH_DOT:        sSetDashes(ctxcanvas, dash_dot, 4);     break;
  case CD_DASH_DOT_DOT:    sSetDashes(ctxcanvas, dash_dot_dot, 6); break;
  case CD_CUSTOM:
    if (ctxcanvas->canvas->line_dashes && ctxcanvas->canvas->line_dashes_count)
      sSetDashes(ctxcanvas, ctxcanvas->canvas->line_dashes, ctxcanvas->canvas->line_dashes_count);
    else
      sKillDashes(ctxcanvas);
    break;
  case CD_CONTINUOUS:
  default:
    sKillDashes(ctxcanvas);
    break;
  }

  return style;
}

static int cdlinewidth(cdCtxCanvas* ctxcanvas, int width)
{
  (void)ctxcanvas;
  return width;  /* applied by sUpdateStroke */
}

static int cdlinecap(cdCtxCanvas* ctxcanvas, int cap)
{
  (void)ctxcanvas;
  return cap;
}

static int cdlinejoin(cdCtxCanvas* ctxcanvas, int join)
{
  (void)ctxcanvas;
  return join;
}

/*****************************************************************************\
* Text                                                                        *
\*****************************************************************************/

static const char* sMapTypeFace(const char* type_face)
{
  if (cdStrEqualNoCase(type_face, "Courier") ||
      cdStrEqualNoCase(type_face, "Courier New") ||
      cdStrEqualNoCase(type_face, "Monospace"))
    return "Courier New";

  if (cdStrEqualNoCase(type_face, "Times") ||
      cdStrEqualNoCase(type_face, "Times New Roman") ||
      cdStrEqualNoCase(type_face, "Serif"))
    return "Times New Roman";

  if (cdStrEqualNoCase(type_face, "Helvetica") ||
      cdStrEqualNoCase(type_face, "Arial") ||
      cdStrEqualNoCase(type_face, "Sans"))
    return "Helvetica";

  if (cdStrEqualNoCase(type_face, "System"))
    return NULL;  /* use the UI font */

  return type_face;
}

static void sUpdateFontMetrics(cdCtxCanvas* ctxcanvas)
{
  CGRect box;

  if (!ctxcanvas->font)
    return;

  ctxcanvas->font_ascent  = CTFontGetAscent(ctxcanvas->font);
  ctxcanvas->font_descent = CTFontGetDescent(ctxcanvas->font);
  ctxcanvas->font_leading = CTFontGetLeading(ctxcanvas->font);

  box = CTFontGetBoundingBox(ctxcanvas->font);
  ctxcanvas->font_maxwidth = box.size.width;
}

static int cdfont(cdCtxCanvas* ctxcanvas, const char* type_face, int style, int size)
{
  const char* name = sMapTypeFace(type_face);
  CGFloat size_px = (CGFloat)cdGetFontSizePixels(ctxcanvas->canvas, size);
  CTFontRef font;

  if (name)
  {
    CFStringRef cfname = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    if (!cfname)
      return 0;
    font = CTFontCreateWithName(cfname, size_px, NULL);
    CFRelease(cfname);
  }
  else
    font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size_px, NULL);

  if (!font)
    return 0;

  if (style & (CD_BOLD|CD_ITALIC))
  {
    CTFontSymbolicTraits traits = 0;
    CTFontRef styled;

    if (style & CD_BOLD)   traits |= kCTFontTraitBold;
    if (style & CD_ITALIC) traits |= kCTFontTraitItalic;

    styled = CTFontCreateCopyWithSymbolicTraits(font, size_px, NULL, traits,
                                                kCTFontTraitBold|kCTFontTraitItalic);
    if (styled)
    {
      CFRelease(font);
      font = styled;
    }
  }

  if (ctxcanvas->font)
    CFRelease(ctxcanvas->font);
  ctxcanvas->font = font;

  sUpdateFontMetrics(ctxcanvas);
  return 1;
}

static CTLineRef sCreateLine(cdCtxCanvas* ctxcanvas, const char* s, int len, long color)
{
  CFStringRef str;
  CFMutableAttributedStringRef astr;
  CTLineRef line;
  CGColorRef cgcolor;
  CFRange range;

  if (!ctxcanvas->font)
    cdfont(ctxcanvas, ctxcanvas->canvas->font_type_face,
           ctxcanvas->canvas->font_style, ctxcanvas->canvas->font_size);

  if (!ctxcanvas->font)
    return NULL;

  str = CFStringCreateWithBytes(NULL, (const UInt8*)s, len, kCFStringEncodingUTF8, false);
  if (!str)  /* not valid UTF-8, fall back to Latin 1 */
    str = CFStringCreateWithBytes(NULL, (const UInt8*)s, len, kCFStringEncodingISOLatin1, false);
  if (!str)
    return NULL;

  astr = CFAttributedStringCreateMutable(NULL, 0);
  CFAttributedStringReplaceString(astr, CFRangeMake(0, 0), str);
  range = CFRangeMake(0, CFStringGetLength(str));

  CFAttributedStringSetAttribute(astr, range, kCTFontAttributeName, ctxcanvas->font);

  cgcolor = CGColorCreateGenericRGB(cdQuartzRed(color), cdQuartzGreen(color),
                                    cdQuartzBlue(color), cdQuartzAlpha(color));
  CFAttributedStringSetAttribute(astr, range, kCTForegroundColorAttributeName, cgcolor);
  CGColorRelease(cgcolor);

  line = CTLineCreateWithAttributedString(astr);

  CFRelease(astr);
  CFRelease(str);
  return line;
}

/* Moves the anchor point given by the CD alignment to the text origin,
   which in Core Text is the left end of the baseline. */
static void sAlignText(cdCtxCanvas* ctxcanvas, double* x, double* y, double width)
{
  double ascent = ctxcanvas->font_ascent;
  double descent = ctxcanvas->font_descent;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    *x -= width;
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    *x -= width/2;
    break;
  }

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_NORTH:
  case CD_NORTH_EAST:
  case CD_NORTH_WEST:
    *y -= ascent;
    break;
  case CD_SOUTH:
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
    *y += descent;
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    *y -= (ascent - descent)/2;
    break;
  /* CD_BASE_* already refer to the baseline */
  }
}

static void sDrawTextDecoration(cdCtxCanvas* ctxcanvas, double x, double y, double width)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  double thickness = ctxcanvas->font_ascent/14.0;

  if (thickness < 1.0)
    thickness = 1.0;

  if (!(canvas->font_style & (CD_UNDERLINE|CD_STRIKEOUT)))
    return;

  CGContextSaveGState(ctxcanvas->cgc);
  sSetFillColor(ctxcanvas, canvas->foreground);
  CGContextSetFillColorSpace(ctxcanvas->cgc, NULL);
  sSetFillColor(ctxcanvas, canvas->foreground);

  if (canvas->font_style & CD_UNDERLINE)
    CGContextFillRect(ctxcanvas->cgc,
                      CGRectMake((CGFloat)x, (CGFloat)(y - ctxcanvas->font_descent/2),
                                 (CGFloat)width, (CGFloat)thickness));

  if (canvas->font_style & CD_STRIKEOUT)
    CGContextFillRect(ctxcanvas->cgc,
                      CGRectMake((CGFloat)x, (CGFloat)(y + ctxcanvas->font_ascent/3),
                                 (CGFloat)width, (CGFloat)thickness));

  CGContextRestoreGState(ctxcanvas->cgc);
}

static void cdftext(cdCtxCanvas* ctxcanvas, double x, double y, const char* s, int len)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  CTLineRef line;
  double width, ascent, descent, leading;
  int rotated = (canvas->text_orientation != 0);

  line = sCreateLine(ctxcanvas, s, len, canvas->foreground);
  if (!line)
    return;

  width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

  CGContextSaveGState(ctxcanvas->cgc);
  CGContextSetTextMatrix(ctxcanvas->cgc, CGAffineTransformIdentity);

  if (rotated)
  {
    /* rotate around the alignment point, before it is moved to the baseline */
    CGContextTranslateCTM(ctxcanvas->cgc, (CGFloat)x, (CGFloat)y);
    CGContextRotateCTM(ctxcanvas->cgc, (CGFloat)(canvas->text_orientation*CD_DEG2RAD));
    CGContextTranslateCTM(ctxcanvas->cgc, (CGFloat)-x, (CGFloat)-y);
  }

  sAlignText(ctxcanvas, &x, &y, width);

  CGContextSetTextPosition(ctxcanvas->cgc, (CGFloat)x, (CGFloat)y);
  CTLineDraw(line, ctxcanvas->cgc);

  sDrawTextDecoration(ctxcanvas, x, y, width);

  CGContextRestoreGState(ctxcanvas->cgc);
  CFRelease(line);
}

static void cdtext(cdCtxCanvas* ctxcanvas, int x, int y, const char* s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdgetfontdim(cdCtxCanvas* ctxcanvas, int* max_width, int* height, int* ascent, int* descent)
{
  if (!ctxcanvas->font)
    cdfont(ctxcanvas, ctxcanvas->canvas->font_type_face,
           ctxcanvas->canvas->font_style, ctxcanvas->canvas->font_size);

  if (max_width) *max_width = _cdRound(ctxcanvas->font_maxwidth);
  if (height)    *height    = _cdRound(ctxcanvas->font_ascent + ctxcanvas->font_descent);
  if (ascent)    *ascent    = _cdRound(ctxcanvas->font_ascent);
  if (descent)   *descent   = _cdRound(ctxcanvas->font_descent);
}

static void cdgettextsize(cdCtxCanvas* ctxcanvas, const char* s, int len, int* width, int* height)
{
  CTLineRef line;
  double w, ascent, descent, leading;

  if (height)
    cdgetfontdim(ctxcanvas, NULL, height, NULL, NULL);

  if (!width)
    return;

  *width = 0;

  line = sCreateLine(ctxcanvas, s, len, ctxcanvas->canvas->foreground);
  if (!line)
    return;

  w = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
  *width = _cdRound(w);

  CFRelease(line);
}

static double cdtextorientation(cdCtxCanvas* ctxcanvas, double angle)
{
  (void)ctxcanvas;
  return angle;  /* applied by cdftext */
}

static int cdtextalignment(cdCtxCanvas* ctxcanvas, int alignment)
{
  (void)ctxcanvas;
  return alignment;
}

/*****************************************************************************\
* Images                                                                      *
\*****************************************************************************/

/* Packs a CD image rectangle, which is stored bottom-up in separate planes,
   into a top-down premultiplied BGRA CGImage. */
static CGImageRef sCreateImageFromPlanes(int iw, const unsigned char* r, const unsigned char* g,
                                         const unsigned char* b, const unsigned char* a,
                                         int xmin, int xmax, int ymin, int ymax)
{
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;
  int i, j;
  unsigned char* data = (unsigned char*)malloc((size_t)rw*rh*4);

  if (!data)
    return NULL;

  for (i = 0; i < rh; i++)
  {
    /* destination row 0 is the top, source row ymax is the top */
    const int src_line = (ymax - i)*iw;
    unsigned char* dst = data + (size_t)i*rw*4;

    for (j = 0; j < rw; j++)
    {
      int pos = src_line + xmin + j;
      unsigned char alpha = a? a[pos]: 255;

      dst[j*4+0] = (unsigned char)((b[pos]*alpha)/255);
      dst[j*4+1] = (unsigned char)((g[pos]*alpha)/255);
      dst[j*4+2] = (unsigned char)((r[pos]*alpha)/255);
      dst[j*4+3] = alpha;
    }
  }

  return sCreateImageFromBGRA(data, rw, rh);
}

static void sDrawImage(cdCtxCanvas* ctxcanvas, CGImageRef image, double x, double y, double w, double h)
{
  if (!image)
    return;

  CGContextSaveGState(ctxcanvas->cgc);
  CGContextSetInterpolationQuality(ctxcanvas->cgc, kCGInterpolationNone);
  CGContextDrawImage(ctxcanvas->cgc, CGRectMake((CGFloat)x, (CGFloat)y, (CGFloat)w, (CGFloat)h), image);
  CGContextRestoreGState(ctxcanvas->cgc);
  CGImageRelease(image);
}

static void cdfputimagerectrgb(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* r,
                               const unsigned char* g, const unsigned char* b,
                               double x, double y, double w, double h,
                               int xmin, int xmax, int ymin, int ymax)
{
  (void)ih;
  sDrawImage(ctxcanvas, sCreateImageFromPlanes(iw, r, g, b, NULL, xmin, xmax, ymin, ymax), x, y, w, h);
}

static void cdputimagerectrgb(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* r,
                              const unsigned char* g, const unsigned char* b,
                              int x, int y, int w, int h,
                              int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgb(ctxcanvas, iw, ih, r, g, b, (double)x, (double)y, (double)w, (double)h,
                     xmin, xmax, ymin, ymax);
}

static void cdfputimagerectrgba(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* r,
                                const unsigned char* g, const unsigned char* b, const unsigned char* a,
                                double x, double y, double w, double h,
                                int xmin, int xmax, int ymin, int ymax)
{
  (void)ih;
  sDrawImage(ctxcanvas, sCreateImageFromPlanes(iw, r, g, b, a, xmin, xmax, ymin, ymax), x, y, w, h);
}

static void cdputimagerectrgba(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* r,
                               const unsigned char* g, const unsigned char* b, const unsigned char* a,
                               int x, int y, int w, int h,
                               int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectrgba(ctxcanvas, iw, ih, r, g, b, a, (double)x, (double)y, (double)w, (double)h,
                      xmin, xmax, ymin, ymax);
}

static void cdfputimagerectmap(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* index,
                               const long* colors, double x, double y, double w, double h,
                               int xmin, int xmax, int ymin, int ymax)
{
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;
  int i, j;
  unsigned char* data;

  (void)ih;

  data = (unsigned char*)malloc((size_t)rw*rh*4);
  if (!data)
    return;

  for (i = 0; i < rh; i++)
  {
    const int src_line = (ymax - i)*iw;
    for (j = 0; j < rw; j++)
      sSetPixelBGRA(data, rw, j, i, colors[index[src_line + xmin + j]]);
  }

  sDrawImage(ctxcanvas, sCreateImageFromBGRA(data, rw, rh), x, y, w, h);
}

static void cdputimagerectmap(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char* index,
                              const long* colors, int x, int y, int w, int h,
                              int xmin, int xmax, int ymin, int ymax)
{
  cdfputimagerectmap(ctxcanvas, iw, ih, index, colors, (double)x, (double)y, (double)w, (double)h,
                     xmin, xmax, ymin, ymax);
}

/* Only available when the canvas is backed by a bitmap. */
static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char* r, unsigned char* g,
                          unsigned char* b, int x, int y, int w, int h)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  int i, j, bw = canvas->w;
  const unsigned char* data = ctxcanvas->data;

  if (!data)
    return;

  CGContextFlush(ctxcanvas->cgc);

  for (i = 0; i < h; i++)
  {
    /* the CD result is bottom-up, the bitmap rows are top-down */
    int src_y = canvas->h - 1 - (y + i);
    for (j = 0; j < w; j++)
    {
      int src_x = x + j;
      size_t pos = (size_t)i*w + j;
      const unsigned char* p;

      if (src_x < 0 || src_x >= bw || src_y < 0 || src_y >= canvas->h)
      {
        r[pos] = g[pos] = b[pos] = 0;
        continue;
      }

      p = data + ((size_t)src_y*bw + src_x)*4;

      /* undo the alpha premultiplication */
      if (p[3] == 0 || p[3] == 255)
      {
        b[pos] = p[0];
        g[pos] = p[1];
        r[pos] = p[2];
      }
      else
      {
        b[pos] = (unsigned char)((p[0]*255)/p[3]);
        g[pos] = (unsigned char)((p[1]*255)/p[3]);
        r[pos] = (unsigned char)((p[2]*255)/p[3]);
      }
    }
  }
}

/*****************************************************************************\
* Server images                                                               *
\*****************************************************************************/

static cdCtxImage* cdcreateimage(cdCtxCanvas* ctxcanvas, int w, int h)
{
  cdCtxImage* ctximage = (cdCtxImage*)malloc(sizeof(cdCtxImage));

  if (!ctximage)
    return NULL;

  memset(ctximage, 0, sizeof(cdCtxImage));

  ctximage->bitmap = cdquartzCreateBitmap(w, h, &ctximage->data);
  if (!ctximage->bitmap)
  {
    free(ctximage);
    return NULL;
  }

  ctximage->w = w;
  ctximage->h = h;
  ctximage->bpp = 32;
  ctximage->xres = ctxcanvas->canvas->xres;
  ctximage->yres = ctxcanvas->canvas->yres;
  ctximage->w_mm = w/ctximage->xres;
  ctximage->h_mm = h/ctximage->yres;

  return ctximage;
}

static void cdkillimage(cdCtxImage* ctximage)
{
  if (ctximage->bitmap)
    CGContextRelease(ctximage->bitmap);
  if (ctximage->data)
    free(ctximage->data);
  free(ctximage);
}

/* Copies a rectangle of the canvas into the server image. */
static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y)
{
  CGImageRef image;

  if (!ctxcanvas->data)
    return;

  CGContextFlush(ctxcanvas->cgc);

  image = CGBitmapContextCreateImage(ctxcanvas->cgc);
  if (!image)
    return;

  /* draw the canvas into the image so that (x,y) lands on the image origin */
  CGContextSaveGState(ctximage->bitmap);
  CGContextClipToRect(ctximage->bitmap, CGRectMake(0, 0, (CGFloat)ctximage->w, (CGFloat)ctximage->h));
  CGContextDrawImage(ctximage->bitmap,
                     CGRectMake((CGFloat)-x, (CGFloat)-y,
                                (CGFloat)ctxcanvas->canvas->w, (CGFloat)ctxcanvas->canvas->h),
                     image);
  CGContextRestoreGState(ctximage->bitmap);

  CGImageRelease(image);
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y,
                           int xmin, int xmax, int ymin, int ymax)
{
  CGImageRef image, sub;
  int rw = xmax-xmin+1;
  int rh = ymax-ymin+1;

  CGContextFlush(ctximage->bitmap);

  image = CGBitmapContextCreateImage(ctximage->bitmap);
  if (!image)
    return;

  if (rw != ctximage->w || rh != ctximage->h)
  {
    /* CGImage rows are top-down, the CD rectangle is bottom-up */
    sub = CGImageCreateWithImageInRect(image, CGRectMake((CGFloat)xmin,
                                                         (CGFloat)(ctximage->h - 1 - ymax),
                                                         (CGFloat)rw, (CGFloat)rh));
    CGImageRelease(image);
    image = sub;
  }

  sDrawImage(ctxcanvas, image, (double)x, (double)y, (double)rw, (double)rh);
}

static void cdscrollarea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  CGImageRef image, sub;
  int w = xmax-xmin+1;
  int h = ymax-ymin+1;

  if (!ctxcanvas->data)
    return;

  CGContextFlush(ctxcanvas->cgc);

  image = CGBitmapContextCreateImage(ctxcanvas->cgc);
  if (!image)
    return;

  sub = CGImageCreateWithImageInRect(image, CGRectMake((CGFloat)xmin,
                                                       (CGFloat)(ctxcanvas->canvas->h - 1 - ymax),
                                                       (CGFloat)w, (CGFloat)h));
  CGImageRelease(image);

  sDrawImage(ctxcanvas, sub, (double)(xmin+dx), (double)(ymin+dy), (double)w, (double)h);
}

/*****************************************************************************\
* Canvas                                                                      *
\*****************************************************************************/

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  CGContextSaveGState(ctxcanvas->cgc);
  CGContextSetBlendMode(ctxcanvas->cgc, kCGBlendModeCopy);
  CGContextSetFillColorSpace(ctxcanvas->cgc, NULL);
  sSetFillColor(ctxcanvas, canvas->background);
  CGContextFillRect(ctxcanvas->cgc, CGRectMake(0, 0, (CGFloat)canvas->w, (CGFloat)canvas->h));
  CGContextRestoreGState(ctxcanvas->cgc);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  CGContextFlush(ctxcanvas->cgc);
}

/*****************************************************************************\
* Custom attributes                                                           *
\*****************************************************************************/

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '1')
    CGContextSetShouldAntialias(ctxcanvas->cgc, true);
  else
    CGContextSetShouldAntialias(ctxcanvas->cgc, false);
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return NULL;
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
};

static void set_txtaa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '1')
    CGContextSetShouldSmoothFonts(ctxcanvas->cgc, true);
  else
    CGContextSetShouldSmoothFonts(ctxcanvas->cgc, false);
}

static cdAttribute txtaa_attrib =
{
  "TEXTANTIALIAS",
  set_txtaa_attrib,
  NULL
};

static void set_hatchboxsize_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  int size = 8;

  if (data)
    sscanf(data, "%d", &size);

  if (size < 2)
    size = 2;

  ctxcanvas->hatchboxsize = size;

  if (ctxcanvas->canvas->interior_style == CD_HATCH)
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
}

static char* get_hatchboxsize_attrib(cdCtxCanvas* ctxcanvas)
{
  static char size[10];
  sprintf(size, "%d", ctxcanvas->hatchboxsize);
  return size;
}

static cdAttribute hatchboxsize_attrib =
{
  "HATCHBOXSIZE",
  set_hatchboxsize_attrib,
  get_hatchboxsize_attrib
};

/* Gives the application the underlying CGContextRef so that it can mix
   its own Quartz drawing with CD. */
static char* get_cgcontext_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->cgc;
}

static cdAttribute cgcontext_attrib =
{
  "CGCONTEXT",
  NULL,
  get_cgcontext_attrib
};

/*****************************************************************************\
* Driver setup                                                                *
\*****************************************************************************/

double cdquartzGetResolution(void)
{
  /* 96 DPI is the common default and matches what the other drivers assume
     when they cannot query the display */
  return 96.0/25.4;
}

void cdquartzSetCanvasSize(cdCanvas* canvas, int w, int h)
{
  double res = cdquartzGetResolution();

  canvas->w = w;
  canvas->h = h;
  canvas->xres = res;
  canvas->yres = res;
  canvas->w_mm = w/res;
  canvas->h_mm = h/res;
  canvas->bpp = 32;
}

CGContextRef cdquartzCreateBitmap(int w, int h, unsigned char** data)
{
  CGColorSpaceRef space;
  CGContextRef cgc;
  unsigned char* buffer;

  if (w <= 0 || h <= 0)
    return NULL;

  buffer = (unsigned char*)calloc((size_t)w*h*4, 1);
  if (!buffer)
    return NULL;

  space = CGColorSpaceCreateDeviceRGB();
  cgc = CGBitmapContextCreate(buffer, w, h, 8, (size_t)w*4, space,
                              kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
  CGColorSpaceRelease(space);

  if (!cgc)
  {
    free(buffer);
    return NULL;
  }

  *data = buffer;
  return cgc;
}

void cdquartzUpdateCanvas(cdCtxCanvas* ctxcanvas, CGContextRef cgc)
{
  if (ctxcanvas->gstate_saved && ctxcanvas->cgc)
  {
    CGContextRestoreGState(ctxcanvas->cgc);
    ctxcanvas->gstate_saved = 0;
  }

  if (ctxcanvas->owns_cgc && ctxcanvas->cgc)
    CGContextRelease(ctxcanvas->cgc);

  ctxcanvas->cgc = cgc;

  if (cgc)
  {
    CGContextSetTextMatrix(cgc, CGAffineTransformIdentity);
    CGContextSetShouldAntialias(cgc, true);
    sUpdateClip(ctxcanvas);
  }
}

cdCtxCanvas* cdquartzCreateCanvas(cdCanvas* canvas, CGContextRef cgc)
{
  cdCtxCanvas* ctxcanvas = (cdCtxCanvas*)malloc(sizeof(cdCtxCanvas));

  if (!ctxcanvas)
    return NULL;

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->canvas = canvas;
  ctxcanvas->cgc = cgc;
  ctxcanvas->hatchboxsize = 8;

  canvas->ctxcanvas = ctxcanvas;
  canvas->invert_yaxis = 0;  /* Quartz is bottom-up, like CD */

  CGContextSetTextMatrix(cgc, CGAffineTransformIdentity);
  CGContextSetShouldAntialias(cgc, true);
  CGContextSetLineWidth(cgc, 1);

  /* the base state that clipping resets to */
  CGContextSaveGState(cgc);
  ctxcanvas->gstate_saved = 1;

  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &txtaa_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &cgcontext_attrib);

  return ctxcanvas;
}

void cdquartzKillCanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->gstate_saved && ctxcanvas->cgc)
    CGContextRestoreGState(ctxcanvas->cgc);

  sKillPattern(ctxcanvas);
  sKillDashes(ctxcanvas);

  if (ctxcanvas->font)
    CFRelease(ctxcanvas->font);

  if (ctxcanvas->owns_cgc && ctxcanvas->cgc)
    CGContextRelease(ctxcanvas->cgc);

  if (ctxcanvas->owns_data && ctxcanvas->data)
    free(ctxcanvas->data);

  free(ctxcanvas);
}

void cdquartzInitTable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;

  canvas->cxPixel  = cdpixel;
  canvas->cxLine   = cdline;
  canvas->cxPoly   = cdpoly;
  canvas->cxRect   = cdrect;
  canvas->cxBox    = cdbox;
  canvas->cxArc    = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord  = cdchord;
  canvas->cxText   = cdtext;

  canvas->cxFPixel  = cdfpixel;
  canvas->cxFLine   = cdfline;
  canvas->cxFPoly   = cdfpoly;
  canvas->cxFRect   = cdfrect;
  canvas->cxFBox    = cdfbox;
  canvas->cxFArc    = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord  = cdfchord;
  canvas->cxFText   = cdftext;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxTransform = cdtransform;

  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxForeground = cdforeground;
  canvas->cxBackground = cdbackground;

  canvas->cxFont = cdfont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxTextOrientation = cdtextorientation;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxScrollArea = cdscrollarea;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxFPutImageRectRGB = cdfputimagerectrgb;
  canvas->cxFPutImageRectRGBA = cdfputimagerectrgba;
  canvas->cxFPutImageRectMap = cdfputimagerectmap;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxKillImage = cdkillimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
}
