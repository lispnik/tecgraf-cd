/** \file
 * \brief Cairo as Context Plus
 *
 * See Copyright Notice in cd.h
 */
 
#include "cd.h"
#include "cd_private.h"
#include "cdcairo.h"
#include <stdlib.h>
#include <memory.h>

/* For GTK 3, only provides dummy functions,
   because Cairo is the main drawing toolkit on it. */

void cdFinishContextPlus(void)
{
}

void cdInitContextPlus(void)
{
#ifndef USE_GTK3
  cdContext* ctx_list[CD_CTXPLUS_COUNT];
  memset(ctx_list, 0, sizeof(ctx_list));

#ifndef CD_CAIRO_HEADLESS
  /* The headless build (macOS without GTK/X11) has no native window or
   * printer surface — Cocoa drawing is done by IUP's Cocoa backend, not
   * through Cairo here. Off-screen image, double-buffer and vector
   * surfaces (PDF/SVG/PS) still register below. */
  ctx_list[CD_CTXPLUS_NATIVEWINDOW] = cdContextCairoNativeWindow();
#endif
  ctx_list[CD_CTXPLUS_IMAGE] = cdContextCairoImage();
  ctx_list[CD_CTXPLUS_DBUFFER] = cdContextCairoDBuffer();
#if !defined(CAIRO_X11) && !defined(CD_CAIRO_HEADLESS)
  ctx_list[CD_CTXPLUS_PRINTER] = cdContextCairoPrinter();
#endif
#ifdef WIN32
  ctx_list[CD_CTXPLUS_EMF] = cdContextCairoEMF();  /* only available in Win32 */
#endif

  cdInitContextPlusList(ctx_list);
#endif
}
