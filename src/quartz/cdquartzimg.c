/** \file
 * \brief Quartz Server Image Driver
 *
 * Draws into a server image previously created with cdCanvasCreateImage.
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>

#include "cdquartzctx.h"
#include "cdimage.h"


static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  ctxcanvas->owns_cgc = 0;  /* the context belongs to the image */
  ctxcanvas->owns_data = 0;
  cdquartzKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxImage* ctximage;
  cdCtxCanvas* ctxcanvas;

  if (!data)
    return;

  ctximage = ((cdImage*)data)->ctximage;
  if (!ctximage || !ctximage->bitmap)
    return;

  ctxcanvas = cdquartzCreateCanvas(canvas, ctximage->bitmap);
  if (!ctxcanvas)
    return;

  ctxcanvas->data = ctximage->data;

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

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  CD_CTX_IMAGE,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextImage(void)
{
  return &cdImageContext;
}
