# CD - Canvas Draw Library

A platform-independent 2D graphics library implemented for multiple platforms using native graphics libraries.

[![Build](https://github.com/lispnik/tecgraf-cd/actions/workflows/build.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/build.yml)
[![Security](https://github.com/lispnik/tecgraf-cd/actions/workflows/security.yml/badge.svg)](https://github.com/lispnik/tecgraf-cd/actions/workflows/security.yml)

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

CD seamlessly integrates with the [IM (Image Processing) library](https://github.com/lispnik/tecgraf-im):

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

## Documentation

- [API Reference](https://tecgraf.puc-rio.br/cd/)
- [Programming Guide](html/)
- [Examples](test/)

## Original Authors

- **Tecgraf/PUC-Rio** - Computer Graphics Technology Group
- **Website**: http://www.tecgraf.puc-rio.br/cd

## License

See [COPYRIGHT](COPYRIGHT) file for license information.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Ensure all CI checks pass
6. Submit a pull request

## Security

This library processes potentially untrusted input. Security-related issues should be reported to the maintainers.

## Dependencies

- **Required**: Platform graphics libraries (GDI/X11/etc.)
- **Optional**: Cairo, FreeType, OpenGL, Lua
- **IM Integration**: [tecgraf-im](https://github.com/lispnik/tecgraf-im) library