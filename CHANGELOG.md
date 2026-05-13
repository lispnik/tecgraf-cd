# Changelog

All notable changes to the CD library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [5.14.0] - 2024-01-15

### Added
- CMake-based build system for cross-platform compatibility
- GitHub Actions CI/CD pipeline for automated testing
- Support for building on macOS, Linux, and Windows
- Integration with lispnik/tecgraf-im fork for IM support
- Comprehensive test suite with platform-specific tests
- Security scanning with CodeQL and sanitizers
- pkg-config and CMake configuration files for easy integration
- Modern project structure with proper include directories

### Changed
- Migrated from legacy Tecmake build system to CMake
- Updated dependency handling to use system package managers
- Improved cross-platform compatibility
- Enhanced documentation with Markdown format

### Dependencies
- Links to lispnik/tecgraf-im instead of original IM library
- Uses system libraries where available (Cairo, FreeType, etc.)
- Optional Lua bindings with automatic detection

### Platform Support
- **Linux**: X11, Cairo, XRender backends with full feature support
- **macOS**: X11, native Core Graphics integration planned  
- **Windows**: GDI, GDI+, Direct2D backends with full feature support

### Build Options
- `CD_ENABLE_CAIRO`: Cairo backend (default: ON)
- `CD_ENABLE_XRENDER`: X11 XRender backend (default: ON)
- `CD_ENABLE_GL`: OpenGL backend (default: ON) 
- `CD_ENABLE_PDF`: PDF export (default: ON)
- `CD_ENABLE_IM`: IM integration (default: ON)
- `CD_ENABLE_PPTX`: PowerPoint export (default: ON)
- `CD_ENABLE_DIRECT2D`: Direct2D Windows backend (default: WIN32)
- `CD_ENABLE_GDIPLUS`: GDI+ Windows backend (default: WIN32)
- `CD_ENABLE_LUA`: Lua bindings (default: ON)
- `BUILD_SHARED_LIBS`: Build shared libraries (default: ON)

### Security
- Added automated security scanning with CodeQL
- Memory sanitizer testing in CI
- Static analysis with cppcheck and clang-tidy
- Vulnerability reporting process established

## [5.13.x and earlier]

Previous versions used the original Tecgraf build system. 
See the original CD documentation for historical changelog information.