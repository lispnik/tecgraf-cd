/** \file
 * \brief Quartz as Context Plus
 *
 * See Copyright Notice in cd.h
 */

#include "cd.h"
#include "cd_private.h"

/* CD's "Plus" contexts are its anti-aliased drivers, standing in for the plain ones when an
 * application asks for them: GDI+ for GDI on Windows, Cairo or XRender for X11. Quartz needs no
 * stand-in. CoreGraphics anti-aliases by default and the driver asks for it explicitly when it
 * creates a canvas (CGContextSetShouldAntialias, cdquartz.c), and transparency, line styles,
 * dashes and patterns are all in the plain driver already. There is nothing a Plus driver would
 * add here that the ordinary one does not do.
 *
 * So registering nothing is the whole implementation, and it is not a stub standing in for
 * missing work. An application may call cdInitContextPlus and then cdUseContextPlus(1), and
 * every context accessor in the Quartz driver goes on returning its ordinary context -- which
 * is the anti-aliased drawing the application was asking for. cdGetContextPlus answers NULL for
 * each slot, which is what the drivers that do consult the list (cd0prn.c, cd0emf.c) expect on
 * a platform with no Plus driver.
 *
 * What this does provide is the symbols. Without them, every application that calls
 * cdInitContextPlus -- CD's own cdtest among them -- fails to link on macOS, which is why IUP's
 * sample build had to define a stub of its own.
 *
 * If a Cairo build is ever enabled on macOS, src/cairo/cdcairoplus.c defines these same two
 * functions and the two files must not both be compiled.
 */

void cdInitContextPlus(void)
{
}

void cdFinishContextPlus(void)
{
}
