/** \file
 * \brief Quartz Bitmap Driver
 *
 * An offscreen canvas backed by a CGBitmapContext. Needs no window server,
 * so it also serves as the way to render with Quartz from a headless process.
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdquartzctx.h"
#include "cdquartz.h"


static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdquartzKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  CGContextRef cgc;
  unsigned char* buffer = NULL;
  char* str = (char*)data;
  int w = 0, h = 0;
  double res = 0;

  if (!str)
    return;

  if (sscanf(str, "%dx%d %lg", &w, &h, &res) < 2)
    return;

  if (w <= 0 || h <= 0)
    return;

  cgc = cdquartzCreateBitmap(w, h, &buffer);
  if (!cgc)
    return;

  ctxcanvas = cdquartzCreateCanvas(canvas, cgc);
  if (!ctxcanvas)
  {
    CGContextRelease(cgc);
    free(buffer);
    return;
  }

  ctxcanvas->owns_cgc = 1;
  ctxcanvas->data = buffer;
  ctxcanvas->owns_data = 1;

  cdquartzSetCanvasSize(canvas, w, h);

  if (res > 0)
  {
    canvas->xres = res;
    canvas->yres = res;
    canvas->w_mm = w/res;
    canvas->h_mm = h/res;
  }
}

static void cdinittable(cdCanvas* canvas)
{
  cdquartzInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdQuartzBitmapContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  CD_CTX_IMAGE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextQuartzBitmap(void)
{
  return &cdQuartzBitmapContext;
}
