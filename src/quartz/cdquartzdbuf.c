/** \file
 * \brief Quartz Double Buffer Driver
 *
 * Draws into an offscreen server image and blits it to the target canvas
 * on Flush.
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdquartzctx.h"
#include "cddbuf.h"


static void cdcreatecanvas(cdCanvas* canvas, void* data);

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdImage* image_dbuffer = ctxcanvas->image_dbuffer;

  ctxcanvas->owns_cgc = 0;   /* the context belongs to the image */
  ctxcanvas->owns_data = 0;
  cdquartzKillCanvas(ctxcanvas);

  if (image_dbuffer)
    cdKillImage(image_dbuffer);
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
  cdCanvasDeactivate(ctxcanvas->canvas_dbuffer);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  CGContextFlush(ctxcanvas->cgc);

  /* this is done in the target canvas context */
  cdCanvasPutImageRect(ctxcanvas->canvas_dbuffer, ctxcanvas->image_dbuffer, 0, 0, 0, 0, 0, 0);
}

static int cdactivate(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  int w, h;

  /* this will update the target canvas size */
  cdCanvasActivate(canvas_dbuffer);

  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w == 0) w = 1;
  if (h == 0) h = 1;

  if (w != ctxcanvas->image_dbuffer->w || h != ctxcanvas->image_dbuffer->h)
  {
    /* the image no longer matches the target, rebuild both it and the canvas */
    cdCanvas* canvas = ctxcanvas->canvas;
    cdImage* old_image_dbuffer = ctxcanvas->image_dbuffer;
    cdCtxCanvas* old_ctxcanvas = ctxcanvas;

    canvas->ctxcanvas = NULL;
    cdcreatecanvas(canvas, canvas_dbuffer);
    if (!canvas->ctxcanvas)
    {
      canvas->ctxcanvas = old_ctxcanvas;
      return CD_ERROR;
    }

    old_ctxcanvas->image_dbuffer = NULL;  /* killed below, not by KillCanvas */
    old_ctxcanvas->owns_cgc = 0;
    old_ctxcanvas->owns_data = 0;
    cdquartzKillCanvas(old_ctxcanvas);
    cdKillImage(old_image_dbuffer);

    cdUpdateAttributes(canvas);
  }

  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCanvas* canvas_dbuffer = (cdCanvas*)data;
  cdCtxCanvas* ctxcanvas;
  cdImage* image_dbuffer;
  cdCtxImage* ctximage;
  int w, h;

  if (!canvas_dbuffer)
    return;

  cdCanvasActivate(canvas_dbuffer);

  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w == 0) w = 1;
  if (h == 0) h = 1;

  /* this is done in the target canvas context */
  image_dbuffer = cdCanvasCreateImage(canvas_dbuffer, w, h);
  if (!image_dbuffer)
    return;

  ctximage = image_dbuffer->ctximage;

  ctxcanvas = cdquartzCreateCanvas(canvas, ctximage->bitmap);
  if (!ctxcanvas)
  {
    cdKillImage(image_dbuffer);
    return;
  }

  ctxcanvas->data = ctximage->data;
  ctxcanvas->image_dbuffer = image_dbuffer;
  ctxcanvas->canvas_dbuffer = canvas_dbuffer;

  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->w_mm = ctximage->w_mm;
  canvas->h_mm = ctximage->h_mm;
  canvas->bpp = ctximage->bpp;
  canvas->xres = ctximage->xres;
  canvas->yres = ctximage->yres;
}

static void cdinittable(cdCanvas* canvas)
{
  cdquartzInitTable(canvas);

  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdDBufferContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  CD_CTX_IMAGE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextDBuffer(void)
{
  return &cdDBufferContext;
}
