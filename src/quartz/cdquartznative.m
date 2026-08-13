/** \file
 * \brief Quartz Native Window Driver
 *
 * The creation data is an NSView*. This is the only file of the driver that
 * needs AppKit, and therefore Objective-C: everything CD draws goes through
 * the CGContextRef obtained from the view.
 *
 * See Copyright Notice in cd.h
 */

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdquartzctx.h"
#include "cdnative.h"

/* CD lets an application draw into a canvas at any time, not only from
   drawRect:. Focusing the view is the only way to honour that, so the
   deprecated focus API is used deliberately. Drawing from drawRect:, which
   is the recommended path, never reaches it: there is already a current
   graphics context then. */
#pragma clang diagnostic ignored "-Wdeprecated-declarations"


/* Obtains the CGContext to draw the view into.

   Inside drawRect: there already is a current graphics context and it must be
   used as is. Outside of it the view has to be focused first, which is what
   the returned "locked" flag tracks. */
static CGContextRef sViewGetContext(NSView* view, int* locked)
{
  NSGraphicsContext* nsc = [NSGraphicsContext currentContext];

  *locked = 0;

  /* If nothing is focused for drawing then we are outside any draw cycle, and whatever gets
     drawn now has to be asked for on screen -- AppKit will not do it on its own.

     This is not the same as "there is no current context". A host whose view redirects focus
     locking at a persistent backing store (IUP's canvas does exactly that) can leave that
     context current after an unbalanced lock, in which case the branch below never runs again
     and the drawing lands correctly but silently. That is precisely how IupPlot behaved: every
     grid toggle, autoscale toggle and dial re-rendered the plot into the backing store, and
     none of it appeared until an unrelated repaint -- resizing the window -- blitted it. */
  if ([NSView focusView] != view)
    [view setNeedsDisplay:YES];

  if (!nsc)
  {
    if (![view lockFocusIfCanDraw])
      return NULL;

    nsc = [NSGraphicsContext currentContext];
    if (!nsc)
    {
      [view unlockFocus];
      return NULL;
    }

    *locked = 1;
  }

  return (CGContextRef)[nsc CGContext];
}

static void sUpdateSize(cdCtxCanvas* ctxcanvas)
{
  NSView* view = (NSView*)ctxcanvas->view;
  NSRect bounds = [view bounds];
  int w = (int)bounds.size.width;
  int h = (int)bounds.size.height;

  if (w <= 0) w = 1;
  if (h <= 0) h = 1;

  cdquartzSetCanvasSize(ctxcanvas->canvas, w, h);
}

/* AppKit views may use a top-down coordinate system, CD and Quartz do not. */
static void sFixViewOrientation(cdCtxCanvas* ctxcanvas, CGContextRef cgc)
{
  NSView* view = (NSView*)ctxcanvas->view;

  if ([view isFlipped])
  {
    CGContextTranslateCTM(cgc, 0, (CGFloat)ctxcanvas->canvas->h);
    CGContextScaleCTM(cgc, 1, -1);
  }
}

static int cdactivate(cdCtxCanvas* ctxcanvas)
{
  CGContextRef cgc;
  int locked = 0;

  if (!ctxcanvas->view)
    return CD_ERROR;

  sUpdateSize(ctxcanvas);

  cgc = sViewGetContext((NSView*)ctxcanvas->view, &locked);
  if (!cgc)
    return CD_ERROR;

  ctxcanvas->focus_locked = locked;

  sFixViewOrientation(ctxcanvas, cgc);
  cdquartzUpdateCanvas(ctxcanvas, cgc);

  return CD_OK;
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->focus_locked)
  {
    CGContextFlush(ctxcanvas->cgc);
    [(NSView*)ctxcanvas->view unlockFocus];
    ctxcanvas->focus_locked = 0;
  }
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  CGContextFlush(ctxcanvas->cgc);
  [[(NSView*)ctxcanvas->view window] flushWindow];
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  NSView* view = (NSView*)ctxcanvas->view;

  if (ctxcanvas->focus_locked)
  {
    [view unlockFocus];
    ctxcanvas->focus_locked = 0;
  }

  ctxcanvas->owns_cgc = 0;  /* the context belongs to the window server */
  ctxcanvas->view = NULL;

  cdquartzKillCanvas(ctxcanvas);

  if (view)
    CFRelease((CFTypeRef)view);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  NSView* view = (NSView*)data;
  cdCtxCanvas* ctxcanvas;
  CGContextRef cgc;
  int locked = 0;

  if (!view)
    return;

  cgc = sViewGetContext(view, &locked);
  if (!cgc)
    return;

  ctxcanvas = cdquartzCreateCanvas(canvas, cgc);
  if (!ctxcanvas)
  {
    if (locked)
      [view unlockFocus];
    return;
  }

  CFRetain((CFTypeRef)view);
  ctxcanvas->view = view;
  ctxcanvas->focus_locked = locked;

  sUpdateSize(ctxcanvas);
  sFixViewOrientation(ctxcanvas, cgc);
}

static void cdinittable(cdCanvas* canvas)
{
  cdquartzInitTable(canvas);

  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;

  /* a window canvas has no pixel buffer to read back from */
  canvas->cxGetImageRGB = NULL;
  canvas->cxScrollArea = NULL;
}

static cdContext cdNativeWindowContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_REGION | CD_CAP_WRITEMODE |
                 CD_CAP_PALETTE | CD_CAP_GETIMAGERGB),
  CD_CTX_WINDOW,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextNativeWindow(void)
{
  return &cdNativeWindowContext;
}

void cdGetScreenSize(int* width, int* height, double* width_mm, double* height_mm)
{
  NSScreen* screen = [NSScreen mainScreen];
  NSRect frame;
  double res = cdquartzGetResolution();

  if (!screen)
  {
    if (width)  *width  = 0;
    if (height) *height = 0;
    if (width_mm)  *width_mm  = 0;
    if (height_mm) *height_mm = 0;
    return;
  }

  frame = [screen frame];

  if (width)  *width  = (int)frame.size.width;
  if (height) *height = (int)frame.size.height;
  if (width_mm)  *width_mm  = frame.size.width/res;
  if (height_mm) *height_mm = frame.size.height/res;
}

int cdGetScreenColorPlanes(void)
{
  NSScreen* screen = [NSScreen mainScreen];

  if (!screen)
    return 24;

  return (int)NSBitsPerPixelFromDepth([screen depth]);
}

int cdBaseDriver(void)
{
  return CD_BASE_QUARTZ;
}
