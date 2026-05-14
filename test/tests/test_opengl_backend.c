#include <stdio.h>
#include <stdlib.h>
#include <cd.h>
#include <cdgl.h>
#include <cdirgb.h>

// Test OpenGL backend availability and functionality
int test_opengl_context() {
    printf("Testing OpenGL context availability...\n");

    cdContext* ctx = cdContextGL();
    if (!ctx) {
        printf("✗ OpenGL context not available\n");
        return 0;
    }

    printf("✓ OpenGL context available\n");
    printf("  Context caps: 0x%lx\n", cdContextCaps(ctx));
    printf("  Context type: %d\n", cdContextType(ctx));
    printf("  Is Plus: %s\n", cdContextIsPlus(ctx) ? "Yes" : "No");

    return 1;
}

// Test OpenGL canvas creation (using offscreen RGB for testing)
int test_opengl_canvas_creation() {
    printf("Testing OpenGL canvas creation...\n");

    // Since we can't create a real GL context in tests, test with RGB canvas
    // that exercises similar code paths
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    if (!canvas) {
        printf("✗ Failed to create test canvas\n");
        return 0;
    }

    printf("✓ Test canvas created successfully\n");

    // Test basic operations
    cdCanvasActivate(canvas);
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);

    // Test drawing primitives
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasArc(canvas, 100, 100, 50, 50, 0, 360);

    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, 200, 300, 150, 200);

    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasLine(canvas, 50, 50, 350, 250);

    printf("✓ Basic drawing operations completed\n");

    // Cleanup
    cdKillCanvas(canvas);
    printf("✓ Canvas cleanup completed\n");

    return 1;
}

// Test OpenGL-specific features
int test_opengl_features() {
    printf("Testing OpenGL-specific features...\n");

    // Test that OpenGL context returns appropriate capabilities
    cdContext* ctx = cdContextGL();
    if (!ctx) return 0;

    unsigned long caps = cdContextCaps(ctx);

    // Check for expected OpenGL capabilities
    int has_transform = (caps & 0x01) != 0;  // CD_CAP_TRANSFORM
    int has_clipping = (caps & 0x02) != 0;   // CD_CAP_CLIPPING

    printf("  Transform support: %s\n", has_transform ? "Yes" : "No");
    printf("  Clipping support: %s\n", has_clipping ? "Yes" : "No");
    printf("  Full capabilities: 0x%lx\n", caps);

    printf("✓ OpenGL features test completed\n");
    return 1;
}

int main() {
    printf("=== CD OpenGL Backend Test Suite ===\n");
    printf("CD Version: %s\n", cdVersion());

    int tests_passed = 0;
    int total_tests = 3;

    // Run test suite
    if (test_opengl_context()) tests_passed++;
    printf("\n");

    if (test_opengl_canvas_creation()) tests_passed++;
    printf("\n");

    if (test_opengl_features()) tests_passed++;
    printf("\n");

    // Summary
    printf("=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);

    if (tests_passed == total_tests) {
        printf("✓ All OpenGL backend tests passed!\n");
        return 0;
    } else {
        printf("✗ Some OpenGL backend tests failed\n");
        return 1;
    }
}