# CD - Canvas Draw Library

A platform-independent 2D graphics library implemented for multiple platforms using native graphics libraries.

[![Linux CI](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-linux.yml)
[![macOS CI](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-macos.yml)
[![Windows CI](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/ci-windows.yml)
[![Security](https://github.com/lispnik/tecgraf-cd/actions/workflows/security.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/security.yml)

## About

CD (Canvas Draw) provides a unified interface for 2D graphics across multiple platforms and output formats. The library abstracts the underlying graphics systems, allowing applications to draw vector graphics, render text, and manipulate images using a consistent API regardless of the target platform or output device.

## Fork Differences from Upstream

This fork modernizes the original [Tecgraf CD library](http://www.tecgraf.puc-rio.br/cd) with significant improvements for contemporary development:

### Build System Modernization
- **CMake build system** - Replaced legacy "tecmake" build system with modern CMake (3.16+)
- **Dependency management** - Uses pre-built [IM library artifacts](https://github.com/lispnik/tecgraf-im) instead of complex source builds
- **Cross-platform builds** - Unified build process with comprehensive CI/CD across Linux, macOS, and Windows

### Platform Support Enhancements  
- **Apple Silicon macOS** - Native support for M1/M2 Macs with proper Homebrew integration
- **Modern Linux distributions** - Updated dependencies for Ubuntu 24.04+ and contemporary package names
- **Windows improvements** - Enhanced Visual Studio and vcpkg integration with proper DLL exports

### Development Infrastructure
- **GitHub Actions CI** - Comprehensive continuous integration with matrix builds across platforms and configurations
- **Automated testing** - Build verification, platform-specific compilation testing, and integration validation
- **Dependency optimization** - Streamlined external library management and caching strategies

### Graphics Backend Improvements
- **Cairo integration** - Enhanced Cairo backend with proper GTK/GDK conditional compilation
- **OpenGL modernization** - Updated OpenGL support with FTGL text rendering capabilities  
- **Cross-platform rendering** - Improved platform-specific backend selection and compilation
- **PDF/Vector output** - Enhanced PostScript, PDF, and SVG generation capabilities

### Compatibility and Reliability
- **Robust compilation** - Fixed numerous platform-specific compilation issues and header dependencies
- **Runtime stability** - Improved library loading and cross-platform compatibility
- **Modern compilers** - Support for contemporary C99/C11 compilers and toolchains
- **System integration** - Better integration with system package managers and dependency resolution

### Recent Improvements (2024)
- **Optimized CI/CD pipeline** - Enhanced build timeouts, improved caching, and parallel execution
- **Cairo compilation fixes** - Resolved platform-specific compilation issues for Linux/macOS
- **Dependency resolution** - Streamlined FTGL, GTK, and system library dependencies
- **Cross-platform consistency** - Unified behavior across different operating systems and graphics backends

## Features

- **Vector Graphics**: Complete 2D vector drawing capabilities
- **Multiple Backends**: Native implementations for Windows (GDI/GDI+/Direct2D), X11, Cairo, OpenGL
- **File Formats**: Support for PostScript, PDF, SVG, CGM, DGN, WMF/EMF, and more
- **Image Integration**: Seamless integration with the IM (Image Processing) library
- **Cross-Platform**: Builds on Windows, macOS, and Linux
- **Language Bindings**: Lua bindings included

## Supported Platforms

- **Windows**: GDI, GDI+, Direct2D backends
- **Linux**: X11, Cairo, XRender backends  
- **macOS**: X11, Core Graphics backends
- **All Platforms**: OpenGL, PDF, PostScript, SVG output

## Building

### Requirements

- CMake 3.16 or later
- C99-compatible compiler
- Platform-specific libraries (see below)

### Linux Dependencies

```bash
sudo apt-get install -y \
  build-essential \
  cmake \
  pkg-config \
  libx11-dev \
  libxext-dev \
  libxrender-dev \
  libxft-dev \
  libcairo2-dev \
  libpango1.0-dev \
  libfreetype6-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  liblua5.3-dev
```

### macOS Dependencies

```bash
brew install cmake pkg-config cairo pango freetype lua
```

### Windows Dependencies

- Visual Studio 2019 or later
- Windows SDK

### Build Instructions

```bash
# Clone with IM dependency
git clone https://github.com/lispnik/tecgraf-cd.git
cd tecgraf-cd

# Also clone the IM library dependency
git clone https://github.com/lispnik/tecgraf-im.git im-git

# Build IM first
cd im-git
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make -j$(nproc)
make install
cd ../..

# Build CD
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DIM_INCLUDE_DIR=../im-git/install/include \
  -DIM_LIBRARY=../im-git/install/lib/libim.so \
  -DCD_ENABLE_CAIRO=ON \
  -DCD_ENABLE_GL=ON \
  -DCD_ENABLE_PDF=ON \
  -DCD_ENABLE_IM=ON \
  -DCD_ENABLE_LUA=ON

make -j$(nproc)
make install
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CD_ENABLE_CAIRO` | ON | Enable Cairo backend (Linux/macOS) |
| `CD_ENABLE_XRENDER` | ON | Enable X11 XRender backend (Linux) |
| `CD_ENABLE_GL` | ON | Enable OpenGL backend |
| `CD_ENABLE_PDF` | ON | Enable PDF backend |
| `CD_ENABLE_IM` | ON | Enable IM integration |
| `CD_ENABLE_PPTX` | ON | Enable PowerPoint backend |
| `CD_ENABLE_DIRECT2D` | WIN32 | Enable Direct2D (Windows only) |
| `CD_ENABLE_GDIPLUS` | WIN32 | Enable GDI+ (Windows only) |
| `CD_ENABLE_LUA` | ON | Enable Lua bindings |
| `CD_BUILD_TESTS` | OFF | Build test suite |
| `BUILD_SHARED_LIBS` | ON | Build shared libraries |

## Quick Start

```c
#include <cd.h>

int main() {
    // Create a canvas (native window on each platform)
    cdCanvas* canvas = cdCreateCanvas(cdContextNativeWindow(), NULL);
    
    // Set foreground color to red
    cdCanvasForeground(canvas, CD_RED);
    
    // Draw a line
    cdCanvasLine(canvas, 0, 0, 100, 100);
    
    // Draw a filled rectangle
    cdCanvasBox(canvas, 10, 10, 50, 50);
    
    // Clean up
    cdKillCanvas(canvas);
    
    return 0;
}
```

## Integration with IM

CD seamlessly integrates with the [IM (Image Processing) library](https://github.com/lispnik/tecgraf-im) modernized fork:

```c
#include <cd.h>
#include <im.h>

// Load an image with IM
imImage* image = imFileLoadImage("photo.jpg", 0, NULL);

// Create CD canvas from IM image
cdCanvas* canvas = cdCreateCanvas(cdContextImage(), image);

// Draw on the image using CD
cdCanvasForeground(canvas, CD_BLUE);
cdCanvasText(canvas, 10, 10, "Hello World!");

// Save the modified image
imFileSaveImage("output.jpg", "JPEG", image);

cdKillCanvas(canvas);
imImageDestroy(image);
```

Both CD and IM have been modernized with CMake builds, GitHub Actions CI, and enhanced cross-platform support as part of the `lispnik` organization's effort to maintain these essential graphics libraries for contemporary development environments.

## Documentation

- **Original Documentation**: [Tecgraf CD Documentation](https://tecgraf.puc-rio.br/cd/)
- **API Reference**: Available in the `html/` directory
- **Programming Guide**: See `html/` directory for comprehensive guides
- **Examples**: See `test/` directory for usage examples
- **Build Instructions**: See build section above and platform-specific notes

## Original Authors

- **Tecgraf/PUC-Rio** - Computer Graphics Technology Group, Pontifical Catholic University of Rio de Janeiro
- **Website**: http://www.tecgraf.puc-rio.br/cd
- **Contact**: cd@tecgraf.puc-rio.br

## Fork Maintainer

This modernized fork is maintained as part of the `lispnik` GitHub organization, focusing on contemporary cross-platform support, enhanced CI/CD, and improved integration with modern development workflows.

## License

See [COPYRIGHT](COPYRIGHT) file for license information.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests if applicable
5. Ensure all CI checks pass
6. Submit a pull request

This fork welcomes contributions that maintain compatibility with the original Tecgraf CD API while enhancing cross-platform support, build system improvements, and modern development practices.

## Security

This library processes potentially untrusted input. Security-related issues should be reported to the maintainers.

## Dependencies

- **Required**: Platform graphics libraries (GDI/X11/etc.)
- **Optional**: Cairo, FreeType, OpenGL, Lua
- **IM Integration**: [tecgraf-im](https://github.com/lispnik/tecgraf-im) library