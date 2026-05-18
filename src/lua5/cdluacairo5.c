/** \file
 * \brief Cairo Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdcairo.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"


#ifdef CD_ENABLE_IMAGERGB
static void *cdimagergb_checkdata(lua_State* L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluaimagergbctx =
{
  0,
  "CAIRO_IMAGERGB",
  cdContextCairoImageRGB,
  cdimagergb_checkdata,
  NULL,
  0
};
#endif

#ifdef CD_ENABLE_PS
static void *cdps_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluapsctx =
{
  0,
  "CAIRO_PS",
  cdContextCairoPS,
  cdps_checkdata,
  NULL,
  0
};
#endif

#ifdef CD_ENABLE_SVG
static void *cdsvg_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluasvgctx =
{
  0,
  "CAIRO_SVG",
  cdContextCairoSVG,
  cdsvg_checkdata,
  NULL,
  0
};
#endif

#ifdef CD_ENABLE_PDF
static void *cdpdf_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluapdfctx =
{
  0,
  "CAIRO_PDF",
  cdContextCairoPDF,
  cdpdf_checkdata,
  NULL,
  0
};
#endif

#ifdef CD_ENABLE_DBUFFER
static void *cddbuf_checkdata(lua_State * L, int param)
{
  return cdlua_checkcanvas(L, param);
}

static cdluaContext cdluadbufctx =
{
  0,
  "CAIRO_DBUFFER",
  cdContextCairoDBuffer,
  cddbuf_checkdata,
  NULL,
  0
};
#endif

#ifdef WIN32
static void *cdemf_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluaemfctx = 
{
  0,
  "CAIRO_EMF",
  cdContextCairoEMF,
  cdemf_checkdata,
  NULL,
  1
};
#endif

#ifdef CD_ENABLE_PRINTER
static void *cdprinter_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluaprinterctx =
{
  0,
  "CAIRO_PRINTER",
  cdContextCairoPrinter,
  cdprinter_checkdata,
  NULL,
  0
};
#endif

static void *cdnativewindow_checkdata(lua_State *L, int param)
{
  if (lua_isnil(L,param))
    return NULL;
  if (lua_isuserdata(L,param))
    return lua_touserdata(L,param);
  if (lua_isstring(L, param))
    return (void *)lua_tostring(L,param);

  luaL_argerror(L, param, "data should be of type userdata or a string, or a nil value");
  return NULL;
}

#ifdef CD_ENABLE_NATIVEWINDOW
static cdluaContext cdluanativewindowctx =
{
  0,
  "CAIRO_NATIVEWINDOW",
  cdContextCairoNativeWindow,
  cdnativewindow_checkdata,
  NULL,
  0
};
#endif

#ifdef CD_ENABLE_IMAGE
static void *cdimage_checkdata(lua_State *L, int param)
{
  return cdlua_checkimage(L, param);
}

static cdluaContext cdluaimagectx =
{
  0,
  "CAIRO_IMAGE",
  cdContextCairoImage,
  cdimage_checkdata,
  NULL,
  0
};
#endif

static const luaL_Reg funcs[] = {
  { NULL, NULL },
};

static int cdluacairo_open (lua_State *L)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  cdlua_register_lib(L, funcs);  /* leave cd table at the top of the stack */
#ifdef CD_ENABLE_PDF
  cdlua_addcontext(L, cdL, &cdluapdfctx);
#endif
#ifdef CD_ENABLE_PS
  cdlua_addcontext(L, cdL, &cdluapsctx);
#endif
#ifdef CD_ENABLE_SVG
  cdlua_addcontext(L, cdL, &cdluasvgctx);
#endif
#ifdef CD_ENABLE_IMAGERGB
  cdlua_addcontext(L, cdL, &cdluaimagergbctx);
#endif
#ifdef CD_ENABLE_DBUFFER
  cdlua_addcontext(L, cdL, &cdluadbufctx);
#endif
#ifdef CD_ENABLE_IMAGE
  cdlua_addcontext(L, cdL, &cdluaimagectx);
#endif
#ifdef CD_ENABLE_NATIVEWINDOW
  cdlua_addcontext(L, cdL, &cdluanativewindowctx);
#endif
#ifdef CD_ENABLE_PRINTER
  cdlua_addcontext(L, cdL, &cdluaprinterctx);
#endif
#ifdef WIN32
  cdlua_addcontext(L, cdL, &cdluaemfctx);
#endif
  return 1;
}

int luaopen_cdluacairo(lua_State* L)
{
  return cdluacairo_open(L);
}
