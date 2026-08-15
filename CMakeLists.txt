cmake_minimum_required(VERSION 3.16)

# Objective-C is only needed by the macOS native window driver
if(APPLE)
    project(CD VERSION 5.14.0 LANGUAGES C CXX OBJC)
else()
    project(CD VERSION 5.14.0 LANGUAGES C CXX)
endif()

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Installed libraries must be able to find their siblings. cdlua links against
# cd, so without an install rpath the installed cdlua cannot be loaded at all.
# Relative to the loader so that the install tree stays relocatable.
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

# Options for different backends
option(CD_ENABLE_CAIRO "Enable Cairo backend" ON)
option(CD_ENABLE_XRENDER "Enable X11 XRender backend" ON)
option(CD_ENABLE_GL "Enable OpenGL backend" ON)
option(CD_ENABLE_PDF "Enable PDF backend" OFF)
option(CD_ENABLE_IM "Enable IM integration" ON)
option(CD_ENABLE_PPTX "Enable PPTX backend" OFF)
option(CD_ENABLE_DIRECT2D "Enable Direct2D backend (Windows only)" WIN32)
option(CD_ENABLE_GDIPLUS "Enable GDI+ backend (Windows only)" WIN32)
option(CD_ENABLE_GDK "Enable GDK backend (Linux)" ${CD_PLATFORM_LINUX})
option(CD_ENABLE_HAIKU "Enable Haiku backend (Haiku OS)" ${CD_PLATFORM_HAIKU})
option(CD_ENABLE_QUARTZ "Enable Quartz backend (macOS only)" ON)
option(CD_ENABLE_LUA "Enable Lua bindings" ON)
option(CD_ENABLE_CGM "Enable CGM backend" ON)
option(CD_ENABLE_PS "Enable PostScript backend" ON)
option(CD_ENABLE_DXF "Enable DXF backend" ON)
option(CD_ENABLE_DGN "Enable MicroStation DGN backend" ON)
option(CD_ENABLE_SVG "Enable SVG backend" ON)
option(CD_ENABLE_SIM "Enable SIM backend" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Platform detection
if(WIN32)
    set(CD_PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(CD_PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
    set(CD_PLATFORM_HAIKU TRUE)
elseif(UNIX)
    set(CD_PLATFORM_LINUX TRUE)
endif()

# Find dependencies
find_package(PkgConfig)

# Cairo
if(CD_ENABLE_CAIRO AND NOT WIN32)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CAIRO cairo)
        pkg_check_modules(PANGOCAIRO pangocairo)
        pkg_check_modules(FONTCONFIG fontconfig)
    endif()
endif()

# GDK
if(CD_ENABLE_GDK AND CD_PLATFORM_LINUX)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GDK gdk-3.0)
        if(NOT GDK_FOUND)
            # Try GTK+ 2.0 as fallback
            pkg_check_modules(GDK gdk-2.0)
        endif()
    endif()
endif()

# X11 and XRender
if(CD_ENABLE_XRENDER AND CD_PLATFORM_LINUX)
    find_package(X11)
    if(X11_FOUND)
        find_path(XRENDER_INCLUDE_DIR X11/extensions/Xrender.h ${X11_INCLUDE_DIR})
        find_library(XRENDER_LIBRARY Xrender)
    endif()
endif()

# OpenGL
if(CD_ENABLE_GL)
    find_package(OpenGL)
    if(APPLE)
        find_library(OPENGL_FRAMEWORK OpenGL)
    endif()
endif()

# FreeType (for text rendering)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FREETYPE freetype2)
    if(FREETYPE_FOUND)
        message(STATUS "pkg-config FreeType LDFLAGS: ${FREETYPE_LDFLAGS}")
        message(STATUS "pkg-config FreeType LIBRARY_DIRS: ${FREETYPE_LIBRARY_DIRS}")
    endif()
endif()

# On macOS, help find Homebrew-installed FreeType
if(APPLE AND NOT FREETYPE_FOUND)
    # Try using environment variable from CI
    if(DEFINED ENV{FREETYPE_DIR})
        set(FREETYPE_ROOT_DIR $ENV{FREETYPE_DIR})
        find_path(FREETYPE_INCLUDE_DIR ft2build.h
            PATHS ${FREETYPE_ROOT_DIR}/include/freetype2
            NO_DEFAULT_PATH)
        find_library(FREETYPE_LIBRARY freetype
            PATHS ${FREETYPE_ROOT_DIR}/lib
            NO_DEFAULT_PATH)
    else()
        # Fallback to standard Homebrew paths
        find_path(FREETYPE_INCLUDE_DIR ft2build.h
            PATHS
                /opt/homebrew/include/freetype2
                /usr/local/include/freetype2
            NO_DEFAULT_PATH)
        find_library(FREETYPE_LIBRARY freetype
            PATHS
                /opt/homebrew/lib
                /usr/local/lib
            NO_DEFAULT_PATH)
    endif()

    if(FREETYPE_INCLUDE_DIR AND FREETYPE_LIBRARY)
        set(FREETYPE_FOUND TRUE)
        set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})
        set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
        message(STATUS "Found FreeType (macOS fallback): ${FREETYPE_LIBRARY}")
    else()
        message(WARNING "FreeType not found on macOS. Include dir: ${FREETYPE_INCLUDE_DIR}, Library: ${FREETYPE_LIBRARY}")
    endif()
endif()

# On Windows, help find vcpkg-installed FreeType
if(WIN32 AND NOT FREETYPE_FOUND)
    find_package(Freetype QUIET)
    if(FREETYPE_FOUND)
        message(STATUS "Found FreeType (Windows): ${FREETYPE_LIBRARIES}")
    endif()
endif()

# ZLIB (needed for minizip support)
find_package(ZLIB)

# IM integration - look for our fork first
if(CD_ENABLE_IM)
    # Try to find our IM fork
    find_path(IM_INCLUDE_DIR im.h
        HINTS
            ${CMAKE_SOURCE_DIR}/../im-git/include
            ${CMAKE_SOURCE_DIR}/../im/include
            /usr/local/include
            /usr/include
    )
    find_library(IM_LIBRARY
        NAMES im libim
        HINTS
            ${CMAKE_SOURCE_DIR}/../im-git/lib
            ${CMAKE_SOURCE_DIR}/../im/lib
            /usr/local/lib
            /usr/lib
    )
    if(IM_INCLUDE_DIR AND IM_LIBRARY)
        set(IM_FOUND TRUE)
    endif()
endif()

# Lua
if(CD_ENABLE_LUA)
    find_package(Lua)
endif()

# Include directories
include_directories(include)
include_directories(src)
include_directories(src/sim)
include_directories(src/svg)
include_directories(src/x11)

# Core CD library sources
set(CD_CORE_SOURCES
    src/cd.c
    src/cd_active.c
    src/cd_attributes.c
    src/cd_bitmap.c
    src/cd_image.c
    src/cd_primitives.c
    src/cd_text.c
    src/cd_util.c
    src/cd_vectortext.c
    src/wd.c
    src/wdhdcpy.c
    src/rgb2map.c
)

# Driver sources
set(CD_DRIVER_SOURCES
    src/drv/cddebug.c
    src/drv/cdpicture.c
    src/drv/cdirgb.c
    src/drv/cdmf.c
)

# Platform-specific sources
if(CD_PLATFORM_WINDOWS)
    # Note: Windows driver implementations are in src/win32/, not the cd0*.c stub files
    if(CD_ENABLE_GDIPLUS)
        file(GLOB GDIPLUS_SOURCES src/gdiplus/*.cpp)
        list(APPEND CD_DRIVER_SOURCES ${GDIPLUS_SOURCES})
    endif()
    if(CD_ENABLE_DIRECT2D)
        file(GLOB DIRECT2D_SOURCES src/direct2d/*.cpp)
        list(APPEND CD_DRIVER_SOURCES ${DIRECT2D_SOURCES})
    endif()
    if(CD_ENABLE_CAIRO AND CAIRO_FOUND)
        # Include Windows-specific Cairo EMF support
        file(GLOB CAIRO_SOURCES src/cairo/*.c)
        list(APPEND CD_DRIVER_SOURCES ${CAIRO_SOURCES})
    endif()
    file(GLOB WIN32_SOURCES src/win32/*.c)
    list(APPEND CD_DRIVER_SOURCES ${WIN32_SOURCES})
elseif(CD_PLATFORM_LINUX)
    file(GLOB X11_SOURCES src/x11/*.c)
    list(APPEND CD_DRIVER_SOURCES ${X11_SOURCES})
    if(CD_ENABLE_XRENDER AND X11_FOUND AND XRENDER_LIBRARY)
        file(GLOB XRENDER_SOURCES src/xrender/*.c)
        # Exclude XRender ContextPlus if Cairo is also enabled (Cairo takes precedence)
        if(CD_ENABLE_CAIRO AND CAIRO_FOUND)
            list(REMOVE_ITEM XRENDER_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/src/xrender/cdxrplus.c)
        endif()
        list(APPEND CD_DRIVER_SOURCES ${XRENDER_SOURCES})
    endif()
    if(CD_ENABLE_CAIRO AND CAIRO_FOUND)
        file(GLOB CAIRO_SOURCES src/cairo/*.c)
        # Exclude Windows-specific Cairo files on Linux
        list(REMOVE_ITEM CAIRO_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo/cdcairoemf.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo/cdcaironative_win32.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo/cdcairoprn_win32.c)
        # Exclude GDK-dependent Cairo files if GDK is not available
        if(NOT (CD_ENABLE_GDK AND GDK_FOUND))
            list(REMOVE_ITEM CAIRO_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo/cdcaironative_gdk.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo/cdcairoprn_gtk.c)
        endif()
        list(APPEND CD_DRIVER_SOURCES ${CAIRO_SOURCES})
    endif()
    if(CD_ENABLE_GDK AND GDK_FOUND)
        file(GLOB GDK_SOURCES src/gdk/*.c)
        list(APPEND CD_DRIVER_SOURCES ${GDK_SOURCES})
    endif()
elseif(CD_PLATFORM_MACOS)
    # X11 is disabled on macOS, the Quartz driver uses CoreGraphics/CoreText
    if(CD_ENABLE_QUARTZ)
        list(APPEND CD_DRIVER_SOURCES
            src/quartz/cdquartz.c
            src/quartz/cdquartzbitmap.c
            src/quartz/cdquartzimg.c
            src/quartz/cdquartzdbuf.c
            src/quartz/cdquartznative.m
            src/quartz/cdquartzclipboard.m
            # cdInitContextPlus/cdFinishContextPlus. Quartz has no separate anti-aliased driver
            # because it does not need one; these exist so that applications asking for the Plus
            # contexts link, and get the ordinary already anti-aliased ones. Do not compile this
            # together with src/cairo/cdcairoplus.c -- both define the same two functions.
            src/quartz/cdquartzplus.c
        )
        # Native PDF through CGPDFContext, so that CD_PDF exists without PDFlib.
        # Mutually exclusive with src/drv/cdpdf.c: both define cdContextPDF.
        if(NOT CD_ENABLE_PDF)
            list(APPEND CD_DRIVER_SOURCES src/quartz/cdquartzpdf.c)
            set(CD_QUARTZ_PDF TRUE)

            # The printer driver spools through the PDF driver, so it needs it. Without this
            # cdContextPrinter comes from src/drv/cd0prn.c and always returns NULL.
            list(APPEND CD_DRIVER_SOURCES src/quartz/cdquartzprn.m)
            set(CD_QUARTZ_PRINTER TRUE)
        endif()
    endif()
elseif(CD_PLATFORM_HAIKU)
    # Haiku platform support
    if(CD_ENABLE_HAIKU)
        file(GLOB HAIKU_SOURCES src/haiku/*.cpp)
        list(APPEND CD_DRIVER_SOURCES ${HAIKU_SOURCES})
    endif()
endif()

# Additional drivers
if(CD_ENABLE_GL AND OPENGL_FOUND)
    list(APPEND CD_DRIVER_SOURCES src/drv/cdgl.c)
    # Include FTGL stubs for all platforms since Homebrew FTGL lacks tecgraf extensions
    list(APPEND CD_DRIVER_SOURCES src/drv/cdgl_ftgl_stubs.c)
endif()

if(CD_ENABLE_PDF)
    list(APPEND CD_DRIVER_SOURCES src/drv/cdpdf.c)
endif()

if(CD_ENABLE_PPTX)
    list(APPEND CD_DRIVER_SOURCES
        src/drv/cdpptx.c
        src/drv/pptx.c
    )
    file(GLOB MINIZIP_SOURCES src/minizip/*.c)
    list(APPEND CD_DRIVER_SOURCES ${MINIZIP_SOURCES})
endif()

# CGM support. src/intcgm is the CGM *interpreter* (cdplayCGM); src/drv/cdcgm.c is the
# *writer* (cdContextCGM), and was in no source list at all -- so CD_CGM did not exist and
# every caller got a NULL canvas.
if(CD_ENABLE_CGM)
    file(GLOB CGM_SOURCES src/intcgm/*.c)
    # src/drv/cgm.c is the metafile writer cdcgm.c is built on, and was equally unlisted
    list(APPEND CD_DRIVER_SOURCES ${CGM_SOURCES} src/drv/cdcgm.c src/drv/cgm.c)
endif()

# PostScript, DXF and DGN: portable, dependency-free file drivers that were likewise present
# in src/drv but absent from the build, which is why CD_PS and CD_DXF resolved to nothing.
if(CD_ENABLE_PS)
    list(APPEND CD_DRIVER_SOURCES src/drv/cdps.c)
endif()

if(CD_ENABLE_DXF)
    list(APPEND CD_DRIVER_SOURCES src/drv/cddxf.c)
endif()

if(CD_ENABLE_DGN)
    list(APPEND CD_DRIVER_SOURCES src/drv/cddgn.c)
endif()

# SVG support
if(CD_ENABLE_SVG)
    file(GLOB SVG_SOURCES src/svg/*.c)
    list(APPEND CD_DRIVER_SOURCES ${SVG_SOURCES})
endif()

# SIM support
if(CD_ENABLE_SIM)
    file(GLOB SIM_SOURCES src/sim/*.c)
    list(APPEND CD_DRIVER_SOURCES ${SIM_SOURCES})
endif()

# IM integration
if(CD_ENABLE_IM AND IM_FOUND)
    list(APPEND CD_DRIVER_SOURCES src/drv/cdim.c)
endif()

# Create main CD library
add_library(cd ${CD_CORE_SOURCES} ${CD_DRIVER_SOURCES})

# Set up interface include directories
target_include_directories(cd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set library properties
set_target_properties(cd PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/cd.h;include/wd.h;include/cd_old.h"
)

# Windows .def file support for symbol export
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(cd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cd_windows.def)
endif()

# Compiler definitions
# Don't redefine CD_VERSION and CD_COPYRIGHT as they're in cd.h

if(BUILD_SHARED_LIBS)
    target_compile_definitions(cd PRIVATE CD_DLL_EXPORT)
endif()

# Platform-specific definitions and linking
if(CD_PLATFORM_WINDOWS)
    target_compile_definitions(cd PRIVATE WIN32 _WIN32)
    target_link_libraries(cd PRIVATE
        user32 gdi32 shell32 comdlg32 ole32 oleaut32 uuid
    )
    if(CD_ENABLE_GDIPLUS)
        target_link_libraries(cd PRIVATE gdiplus)
        target_compile_definitions(cd PRIVATE CD_ENABLE_GDIPLUS)
    endif()
    if(CD_ENABLE_DIRECT2D)
        target_link_libraries(cd PRIVATE d2d1 dwrite)
        target_compile_definitions(cd PRIVATE CD_ENABLE_DIRECT2D)
    endif()
elseif(CD_PLATFORM_LINUX)
    if(X11_FOUND)
        target_include_directories(cd PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(cd PRIVATE ${X11_LIBRARIES})
        target_compile_definitions(cd PRIVATE CD_X11)
    endif()
    if(CD_ENABLE_XRENDER AND XRENDER_LIBRARY)
        target_include_directories(cd PRIVATE ${XRENDER_INCLUDE_DIR})
        target_link_libraries(cd PRIVATE ${XRENDER_LIBRARY})
        target_compile_definitions(cd PRIVATE CD_ENABLE_XRENDER)
    endif()
elseif(CD_PLATFORM_MACOS)
    # Use native macOS frameworks only
    target_link_libraries(cd PRIVATE "-framework CoreGraphics" "-framework CoreText")
    if(CD_ENABLE_QUARTZ)
        target_include_directories(cd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/quartz)
        target_link_libraries(cd PRIVATE "-framework CoreFoundation" "-framework Cocoa")
        target_compile_definitions(cd PRIVATE CD_ENABLE_QUARTZ)
        # ARC is not used, the driver manages CoreFoundation lifetimes itself
        set_source_files_properties(
            src/quartz/cdquartznative.m
            src/quartz/cdquartzclipboard.m
            src/quartz/cdquartzprn.m
            PROPERTIES COMPILE_OPTIONS "-fno-objc-arc")
    endif()
elseif(CD_PLATFORM_HAIKU)
    # Haiku-specific libraries
    target_link_libraries(cd PRIVATE be)
    target_compile_definitions(cd PRIVATE CD_HAIKU)
endif()

# GDK backend (can be used on Linux and potentially other platforms)
if(CD_ENABLE_GDK AND GDK_FOUND)
    target_include_directories(cd PRIVATE ${GDK_INCLUDE_DIRS})
    target_link_libraries(cd PRIVATE ${GDK_LIBRARIES})
    target_compile_definitions(cd PRIVATE CD_ENABLE_GDK)
endif()

# Cairo backend
if(CD_ENABLE_CAIRO AND CAIRO_FOUND)
    target_include_directories(cd PRIVATE ${CAIRO_INCLUDE_DIRS})
    target_link_directories(cd PRIVATE ${CAIRO_LIBRARY_DIRS})
    target_link_libraries(cd PRIVATE ${CAIRO_LIBRARIES})
    target_compile_definitions(cd PRIVATE CD_ENABLE_CAIRO)
    if(PANGOCAIRO_FOUND)
        target_include_directories(cd PRIVATE ${PANGOCAIRO_INCLUDE_DIRS})
        target_link_directories(cd PRIVATE ${PANGOCAIRO_LIBRARY_DIRS})
        target_link_libraries(cd PRIVATE ${PANGOCAIRO_LIBRARIES})
    endif()
    if(FONTCONFIG_FOUND)
        target_include_directories(cd PRIVATE ${FONTCONFIG_INCLUDE_DIRS})
        target_link_directories(cd PRIVATE ${FONTCONFIG_LIBRARY_DIRS})
        target_link_libraries(cd PRIVATE ${FONTCONFIG_LIBRARIES})
    endif()
endif()

# OpenGL backend
if(CD_ENABLE_GL AND OPENGL_FOUND)
    # Find FTGL library for OpenGL text rendering
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FTGL REQUIRED ftgl)

    target_include_directories(cd PRIVATE ${OPENGL_INCLUDE_DIR} ${FTGL_INCLUDE_DIRS})
    target_link_directories(cd PRIVATE ${FTGL_LIBRARY_DIRS})

    if(APPLE)
        target_link_libraries(cd PRIVATE ${OPENGL_FRAMEWORK} ${FTGL_LIBRARIES})
    else()
        target_link_libraries(cd PRIVATE ${OPENGL_LIBRARIES} ${FTGL_LIBRARIES})
    endif()
    target_compile_definitions(cd PRIVATE CD_ENABLE_GL CD_FTGL_NEEDS_STUBS)
endif()

# FreeType
if(FREETYPE_FOUND)
    message(STATUS "FreeType include dirs: ${FREETYPE_INCLUDE_DIRS}")
    message(STATUS "FreeType libraries: ${FREETYPE_LIBRARIES}")
    message(STATUS "FreeType ldflags: ${FREETYPE_LDFLAGS}")
    target_include_directories(cd PRIVATE ${FREETYPE_INCLUDE_DIRS})
    if(FREETYPE_LDFLAGS)
        target_link_libraries(cd PRIVATE ${FREETYPE_LDFLAGS})
    else()
        target_link_libraries(cd PRIVATE ${FREETYPE_LIBRARIES})
    endif()
    # Also add library directories if available
    if(FREETYPE_LIBRARY_DIRS)
        target_link_directories(cd PRIVATE ${FREETYPE_LIBRARY_DIRS})
    endif()
endif()

# IM integration
# SIM: src/drv/cdirgb.c uses this to decide whether the simulation driver is
# available. Without it that driver sets cxLine/cxArc/cxSector to NULL and CD
# calls through the null pointer.
if(CD_ENABLE_SIM)
    target_compile_definitions(cd PRIVATE CD_ENABLE_SIM)
endif()

if(CD_ENABLE_IM AND IM_FOUND)
    target_include_directories(cd PRIVATE ${IM_INCLUDE_DIR})
    target_link_libraries(cd PRIVATE ${IM_LIBRARY})
    target_compile_definitions(cd PRIVATE CD_ENABLE_IM)
endif()

# Math library
if(UNIX)
    target_link_libraries(cd PRIVATE m)
endif()

# ZLIB library (for minizip support)
if(ZLIB_FOUND)
    target_link_libraries(cd PRIVATE ZLIB::ZLIB)
endif()

# Lua bindings
if(CD_ENABLE_LUA AND LUA_FOUND)
    # Base Lua source files
    set(CDLUA_SOURCES
        src/lua5/cdlua5.c
        src/lua5/cdlua5_active.c
        src/lua5/cdlua5_canvas.c
        src/lua5/cdlua5ctx.c
        src/lua5/cdluagl5.c
        src/lua5/cdvoid5.c
    )

    # Add backend-specific files based on enabled options
    if(CD_ENABLE_CAIRO)
        list(APPEND CDLUA_SOURCES src/lua5/cdluacairo5.c)
    endif()

    if(CD_ENABLE_PDF)
        list(APPEND CDLUA_SOURCES src/lua5/cdluapdf5.c)
    endif()

    # src/lua5/cdluaim5.c is the CD-IM bridge for Lua and calls into imlua, IM's own Lua
    # binding -- not into IM itself. It was compiled whenever CD_ENABLE_IM was on, but imlua was
    # only looked for after the library had been declared, so on any machine without imlua
    # installed the whole cdlua target failed to link on imlua_checkimage and friends. Look
    # first, then decide.
    # Find and link IM Lua libraries
    # Check if libraries were explicitly provided (e.g., by homebrew formula)
    if(NOT DEFINED IMLUA_LIBRARY)
        # First try to find them alongside the IM library
        get_filename_component(IM_LIB_DIR "${IM_LIBRARY}" DIRECTORY)

        # Try multiple approaches to find IM Lua libraries
        find_library(IMLUA_LIBRARY
            NAMES imlua libimlua
            HINTS ${IM_LIB_DIR}
            PATHS ${IM_LIBRARY_DIR} /opt/homebrew/lib /usr/local/lib /usr/lib
            PATH_SUFFIXES lib)

        # If not found, try explicit homebrew path based on dependency
        if(NOT IMLUA_LIBRARY)
            # Check if we can construct the path from HOMEBREW_PREFIX
            if(DEFINED ENV{HOMEBREW_PREFIX})
                set(HOMEBREW_PREFIX $ENV{HOMEBREW_PREFIX})
                find_library(IMLUA_LIBRARY
                    NAMES imlua libimlua
                    PATHS ${HOMEBREW_PREFIX}/lib ${HOMEBREW_PREFIX}/Cellar/tecgraf-im/*/lib
                    NO_DEFAULT_PATH)
            endif()
        endif()
    endif()

    if(NOT DEFINED IMLUA_PROCESS_LIBRARY)
        find_library(IMLUA_PROCESS_LIBRARY
            NAMES imlua_process libimlua_process
            HINTS ${IM_LIB_DIR}
            PATHS ${IM_LIBRARY_DIR} /opt/homebrew/lib /usr/local/lib /usr/lib
            PATH_SUFFIXES lib)

        # If not found, try explicit homebrew path
        if(NOT IMLUA_PROCESS_LIBRARY)
            if(DEFINED ENV{HOMEBREW_PREFIX})
                set(HOMEBREW_PREFIX $ENV{HOMEBREW_PREFIX})
                find_library(IMLUA_PROCESS_LIBRARY
                    NAMES imlua_process libimlua_process
                    PATHS ${HOMEBREW_PREFIX}/lib ${HOMEBREW_PREFIX}/Cellar/tecgraf-im/*/lib
                    NO_DEFAULT_PATH)
            endif()
        endif()
    endif()


    if(CD_ENABLE_IM AND IM_FOUND AND IMLUA_LIBRARY)
        list(APPEND CDLUA_SOURCES src/lua5/cdluaim5.c)
        set(CDLUA_WITH_IM TRUE)
    elseif(CD_ENABLE_IM)
        message(STATUS "cdlua: building without IM support (imlua not found)")
    endif()

    # Only add Windows-specific backends if enabled and on Windows
    if(CD_ENABLE_DIRECT2D AND WIN32)
        list(APPEND CDLUA_SOURCES src/lua5/cdluadirect2d5.c)
    endif()

    if(CD_ENABLE_GDIPLUS AND WIN32)
        list(APPEND CDLUA_SOURCES src/lua5/cdluacontextplus5.c)
    endif()

    add_library(cdlua ${CDLUA_SOURCES})
    target_include_directories(cdlua PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(cdlua PRIVATE cd ${LUA_LIBRARIES})

    # Link IM and its Lua binding only when the bridge that needs them is in the target
    if(CDLUA_WITH_IM)
        target_include_directories(cdlua PRIVATE ${IM_INCLUDE_DIR})
        target_link_libraries(cdlua PRIVATE ${IM_LIBRARY} ${IMLUA_LIBRARY})
        message(STATUS "cdlua: IM support through ${IMLUA_LIBRARY}")

        # optional: only the process bindings need it, and cdluaim5.c does not
        if(IMLUA_PROCESS_LIBRARY)
            target_link_libraries(cdlua PRIVATE ${IMLUA_PROCESS_LIBRARY})
        endif()
    endif()

    set_target_properties(cdlua PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS cd
    EXPORT CDTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(CD_ENABLE_LUA AND LUA_FOUND)
    install(TARGETS cdlua
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install additional headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Install fonts and data files
install(DIRECTORY etc/ DESTINATION ${CMAKE_INSTALL_DATADIR}/cd
    FILES_MATCHING
    PATTERN "*.ttf"
    PATTERN "*.txt"
    PATTERN "*.dgn"
)

# Install pkg-config file
configure_file(cd.pc.in cd.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cd.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Install CMake config files
write_basic_package_version_file(
    CDConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    CDConfig.cmake.in
    CDConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CD
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CDConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CDConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CD
)

# Export targets
install(EXPORT CDTargets
    FILE CDTargets.cmake
    NAMESPACE CD::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CD
)

# Add export to build tree
export(TARGETS cd
    FILE ${CMAKE_CURRENT_BINARY_DIR}/CDTargets.cmake
    NAMESPACE CD::
)

# Tests and examples (optional)
option(CD_BUILD_TESTS "Build CD tests" OFF)
if(CD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Print configuration summary
message(STATUS "CD Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Cairo backend: ${CD_ENABLE_CAIRO}")
message(STATUS "  XRender backend: ${CD_ENABLE_XRENDER}")
message(STATUS "  OpenGL backend: ${CD_ENABLE_GL}")
message(STATUS "  PDF backend: ${CD_ENABLE_PDF}")
if(CD_QUARTZ_PDF)
    message(STATUS "  PDF backend (Quartz/CGPDFContext): ON, so CD_PDF works without PDFlib")
endif()
if(CD_QUARTZ_PRINTER)
    message(STATUS "  Printer backend (Quartz/NSPrintOperation): ON")
endif()
message(STATUS "  CGM backend: ${CD_ENABLE_CGM}")
message(STATUS "  PS backend: ${CD_ENABLE_PS}")
message(STATUS "  DXF backend: ${CD_ENABLE_DXF}")
message(STATUS "  DGN backend: ${CD_ENABLE_DGN}")
message(STATUS "  SVG backend: ${CD_ENABLE_SVG}")
message(STATUS "  SIM backend: ${CD_ENABLE_SIM}")
message(STATUS "  IM integration: ${CD_ENABLE_IM}")
message(STATUS "  Lua bindings: ${CD_ENABLE_LUA}")
message(STATUS "  Test suite: ${CD_BUILD_TESTS}")
if(CD_PLATFORM_WINDOWS)
    message(STATUS "  GDI+ backend: ${CD_ENABLE_GDIPLUS}")
    message(STATUS "  Direct2D backend: ${CD_ENABLE_DIRECT2D}")
endif()
if(CD_PLATFORM_LINUX)
    message(STATUS "  GDK backend: ${CD_ENABLE_GDK}")
endif()
if(CD_PLATFORM_HAIKU)
    message(STATUS "  Haiku backend: ${CD_ENABLE_HAIKU}")
endif()
if(CD_PLATFORM_MACOS)
    message(STATUS "  Quartz backend: ${CD_ENABLE_QUARTZ}")
endif()

if(CD_BUILD_TESTS)
    message(STATUS "")
    message(STATUS "Test Suite Features:")
    message(STATUS "  - Comprehensive primitive testing")
    message(STATUS "  - Attributes and styling tests")
    message(STATUS "  - Text rendering validation")
    message(STATUS "  - Image operations testing")
    message(STATUS "  - Backend compatibility tests")
    message(STATUS "  - Edge cases and error handling")
    message(STATUS "  - Performance benchmarks")
    message(STATUS "  - Visual output generation")
    message(STATUS "")
    message(STATUS "To run tests: make test_cd")
    message(STATUS "Visual tests: make test_cd_visual")
endif()