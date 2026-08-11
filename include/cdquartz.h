/** \file
 * \brief Quartz Bitmap driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CD_QUARTZ_H
#define __CD_QUARTZ_H

#ifdef __cplusplus
extern "C" {
#endif

/* Offscreen Quartz canvas, available on macOS without a window server.
   The creation data is "widthxheight [resolution]", for instance
   "800x600" or "800x600 3.8", where the resolution is in pixels/mm. */
cdContext* cdContextQuartzBitmap(void);

#define CD_QUARTZBITMAP cdContextQuartzBitmap()

#ifdef __cplusplus
}
#endif

#endif /* ifndef __CD_QUARTZ_H */
