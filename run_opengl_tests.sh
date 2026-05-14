#!/bin/bash

echo "=== CD OpenGL Tests Runner ==="
echo "Building and running OpenGL backend tests..."

# Set library path
export DYLD_LIBRARY_PATH="$(pwd)/build:$DYLD_LIBRARY_PATH"

# Build tests manually (since main test suite has compilation issues)
echo
echo "Building OpenGL tests..."

echo "  Building test_opengl_simple..."
gcc -o test_opengl_simple test/tests/test_opengl_simple.c -Iinclude -Lbuild -lcd

echo "  Building test_opengl_graphics..."
gcc -o test_opengl_graphics test/tests/test_opengl_graphics.c -Iinclude -Lbuild -lcd

echo "  Building test_opengl_backend..."
gcc -o test_opengl_backend test/tests/test_opengl_backend.c -Iinclude -Lbuild -lcd

echo
echo "Running OpenGL tests..."

# Run tests
echo
echo "=== Running test_opengl_simple ==="
./test_opengl_simple
SIMPLE_RESULT=$?

echo
echo "=== Running test_opengl_graphics ==="
./test_opengl_graphics
GRAPHICS_RESULT=$?

echo
echo "=== Running test_opengl_backend ==="
./test_opengl_backend
BACKEND_RESULT=$?

# Summary
echo
echo "=== Test Results Summary ==="
echo "test_opengl_simple:  $([ $SIMPLE_RESULT -eq 0 ] && echo "PASSED" || echo "FAILED")"
echo "test_opengl_graphics: $([ $GRAPHICS_RESULT -eq 0 ] && echo "PASSED" || echo "FAILED")"
echo "test_opengl_backend:  $([ $BACKEND_RESULT -eq 0 ] && echo "PASSED" || echo "FAILED")"

# Cleanup
rm -f test_opengl_simple test_opengl_graphics test_opengl_backend

# Overall result
if [ $SIMPLE_RESULT -eq 0 ] && [ $GRAPHICS_RESULT -eq 0 ] && [ $BACKEND_RESULT -eq 0 ]; then
    echo
    echo "✓ All OpenGL tests PASSED!"
    exit 0
else
    echo
    echo "✗ Some OpenGL tests FAILED!"
    exit 1
fi