/* Stubs for tecgraf-specific FTGL helpers that aren't in upstream FTGL.
 *
 * cdgl.c expects the following symbols:
 *   ftglGetFontMaxWidth(font)       -> float
 *   ftglSetNearestFilter(font, on)  -> void
 *
 * These exist in tecgraf's bundled FTGL fork but not in vanilla FTGL
 * (e.g. Homebrew's 2.1.3-rc5). Compile this file only when linking against
 * an upstream FTGL build (default on macOS). On platforms that ship the
 * tecgraf FTGL, leave CD_FTGL_NEEDS_STUBS unset so these don't collide
 * with the real exports.
 */

#ifdef CD_FTGL_NEEDS_STUBS

#include <FTGL/ftgl.h>
#include <stdio.h>

/* ftglGetFontMaxWidth: tecgraf's helper returns the font's maximum glyph
 * advance width. Upstream FTGL has no direct equivalent. Use the advance
 * of a wide capital letter ("M" is conventional for em-width estimates).
 * Returns 0 if the font is invalid (matches tecgraf behavior). */
float ftglGetFontMaxWidth(FTGLfont* font)
{
  #ifdef CD_DEBUG_STUBS
  fprintf(stderr, "[STUB] ftglGetFontMaxWidth called with font=%p\n", (void*)font);
  #endif
  if (!font) return 0.0f;
  return ftglGetFontAdvance(font, "M");
}

/* ftglSetNearestFilter: tecgraf adds this to switch the texture filter to
 * GL_NEAREST for non-rotated text rendering. With linear filtering text is
 * marginally softer but still legible. No-op stub. */
void ftglSetNearestFilter(FTGLfont* font, int nearest)
{
  #ifdef CD_DEBUG_STUBS
  fprintf(stderr, "[STUB] ftglSetNearestFilter called with font=%p, nearest=%d\n", (void*)font, nearest);
  #endif
  (void)font; (void)nearest;
}

#endif /* CD_FTGL_NEEDS_STUBS */
