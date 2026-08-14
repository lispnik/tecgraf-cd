/** \file
 * \brief Quartz PDF page machinery, shared with the printer driver
 *
 * The printer driver draws exactly what the PDF driver draws -- it spools to a PDF and then
 * hands that to the print system -- so the page lifecycle lives here rather than being written
 * twice. The delicate parts are the CTM scale, which must be applied before the base graphics
 * state is saved, and the gstate unwind around CGPDFContextEndPage; both are easy to get subtly
 * wrong and are covered by test_quartz_pdf.
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDQUARTZPDF_H
#define __CDQUARTZPDF_H

#include "cdquartzctx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Creates a CGPDFContext writing to filename, sets the canvas size fields from the page size in
   millimetres at the given resolution, begins the first page, and attaches a Quartz context
   canvas to it. Returns NULL on failure, in which case nothing was created. */
cdCtxCanvas* cdquartzPDFCreateCanvas(cdCanvas* canvas, const char* filename,
                                     double w_mm, double h_mm, double dpi);

/* Ends the current page and begins the next, re-establishing the base graphics state and
   replaying the clip and transform onto it. This is what cxFlush means for a paged format. */
void cdquartzPDFNewPage(cdCtxCanvas* ctxcanvas);

/* Ends the current page and closes the document, so the file is complete on disk. Does not
   release the context canvas -- callers still owe a cdquartzKillCanvas. */
void cdquartzPDFClose(cdCtxCanvas* ctxcanvas);

/* Installs the Quartz function table with the entries a PDF page cannot honour removed. */
void cdquartzPDFInitTable(cdCanvas* canvas);

#ifdef __cplusplus
}
#endif

#endif
