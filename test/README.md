# CD Library Test Suite

This comprehensive test suite exercises all major features of the CD graphics library including drawing primitives, text rendering, image operations, color management, and backend functionality.

## Structure

```
test/
├── CMakeLists.txt          # CMake test configuration
├── README.md              # This file
├── basic_test.c           # Simple functionality test
├── utils/                 # Test utilities and framework
│   ├── test_utils.h       # Test framework macros and utilities
│   └── test_utils.c       # Common test functions
├── tests/                 # Individual test suites
│   ├── run_all_tests.c    # Master test runner
│   ├── test_primitives.c  # Drawing primitives (lines, arcs, polygons)
│   ├── test_attributes.c  # Colors, styles, patterns, line attributes
│   ├── test_text.c        # Font rendering, alignment, orientation
│   ├── test_images.c      # Image operations (RGB, RGBA, indexed)
│   ├── test_backends.c    # Backend-specific functionality
│   ├── test_comprehensive.c # Complex integration test
│   ├── test_edge_cases.c  # Error conditions and boundary cases
│   └── [additional tests] # Performance, transforms, clipping, etc.
└── output/                # Generated test files (created during testing)
```

## Building Tests

Tests are disabled by default. Enable them during CMake configuration:

```bash
cd build
cmake -DCD_BUILD_TESTS=ON ..
make
```

## Running Tests

### Individual Test Suites

Run specific test categories:

```bash
# Basic functionality test
./test/cd_basic_test

# Individual test suites
./test/test_primitives
./test/test_attributes
./test/test_text
./test/test_backends
```

### Complete Test Suite

Run all tests with the master runner:

```bash
# Run all tests
make test_cd

# Or run the test runner directly
./test/run_all_tests

# With options
./test/run_all_tests --verbose
./test/run_all_tests --stop-on-failure
```

### Visual Tests Only

Generate visual output files without full testing:

```bash
make test_cd_visual
```

### CTest Integration

Use CMake's built-in test runner:

```bash
make test
# or
ctest --output-on-failure
```

## Test Output

Tests generate visual output files to verify rendering correctness:

- **SVG files**: Vector graphics output for manual inspection
- **PNG files**: Raster graphics (if IM backend available)  
- **HTML report**: Comprehensive test results with links to generated files

Output files are organized by test suite in the `output/` directory:

```
output/
├── primitives/
│   ├── test_lines.svg
│   ├── test_rectangles.svg
│   └── test_arcs.svg
├── attributes/
│   ├── test_line_attributes.svg
│   └── test_colors.svg
└── test_report.html
```

## Test Categories

### Core Functionality

- **Primitives**: Lines, rectangles, arcs, sectors, chords, polygons, pixels
- **Attributes**: Colors, line styles, fill patterns, transparency
- **Text**: Fonts, alignment, orientation, measurement, Unicode support
- **Images**: RGB/RGBA/indexed image operations, scaling, clipping

### Backend Testing

- **SVG**: Vector graphics output with XML validation
- **RGB Image**: Pixel buffer operations and image retrieval
- **Debug**: Operation logging and tracing
- **Double Buffer**: Buffered rendering for smooth updates
- **Platform Backends**: X11, Cairo, OpenGL (when available)

### Advanced Features

- **Transformations**: Matrix operations, coordinate mapping
- **Clipping**: Rectangular and polygonal clipping regions  
- **Paths**: Complex paths and Bezier curves
- **Performance**: Benchmarking and optimization verification

### Quality Assurance

- **Edge Cases**: Boundary conditions, invalid parameters, error handling
- **Memory Management**: Resource cleanup, leak detection
- **Thread Safety**: Concurrent canvas operations
- **Regression**: Tests for previously fixed bugs

## Test Framework

The test suite includes a lightweight testing framework with:

- **Assertions**: `TEST_ASSERT`, `TEST_ASSERT_NOT_NULL`, `TEST_ASSERT_EQ`
- **Test Runner**: `RUN_TEST` macro with statistics tracking
- **Utilities**: Canvas creation helpers, drawing test patterns
- **Performance Measurement**: Timing and operations-per-second metrics

Example test structure:

```c
#include "test_utils.h"

int test_example_feature(void) {
    cdCanvas* canvas = test_create_canvas("SVG", 400, 300, "test_output.svg");
    TEST_ASSERT_NOT_NULL(canvas, "Canvas creation failed");
    
    // Test operations
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasLine(canvas, 0, 0, 100, 100);
    
    test_destroy_canvas(canvas);
    return 1;
}

int main(void) {
    printf("Running Example Tests...\n");
    RUN_TEST(test_example_feature);
    
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);
    return (tests_failed == 0) ? 0 : 1;
}
```

## Continuous Integration

The test suite is designed for CI/CD integration:

- **Exit codes**: 0 for success, non-zero for failures
- **JUnit output**: Compatible with CI systems (planned)
- **Coverage reports**: Integration with coverage tools
- **Visual regression**: Automated comparison of generated images

## Platform Support

Tests run on all platforms supported by CD:

- **Linux**: Full backend testing including X11, Cairo
- **macOS**: Core Graphics integration, X11 compatibility
- **Windows**: GDI, GDI+, Direct2D backend testing

## Dependencies

Test dependencies are automatically detected:

- **Required**: CD library itself
- **Optional**: IM library (for PNG image testing)
- **Optional**: Cairo library (for Cairo backend tests)  
- **Optional**: OpenGL (for OpenGL backend tests)

## Contributing

When adding new features to CD:

1. Add corresponding test cases to appropriate test file
2. Create new test file for major feature additions
3. Update test documentation
4. Ensure tests pass on all target platforms

Test file naming convention: `test_<feature>.c`

## Troubleshooting

**Tests fail to build**: Ensure CD library builds successfully first
**Missing output files**: Check write permissions in build directory  
**Segmentation faults**: Run tests under debugger or with AddressSanitizer
**Visual differences**: Compare generated SVG files with reference outputs