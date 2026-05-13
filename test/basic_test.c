/*
 * Basic CD library test
 * Tests library initialization and basic functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <cd.h>

int main() {
    printf("CD Basic Test\n");
    printf("=============\n");

    // Test version functions
    printf("CD Version: %s\n", cdVersion());
    printf("CD Version Date: %s\n", cdVersionDate());
    printf("CD Version Number: %d\n", cdVersionNumber());

    // Test debug canvas creation (should always work)
    cdCanvas* canvas = cdCreateCanvas(cdContextDebug(), NULL);
    if (!canvas) {
        printf("ERROR: Failed to create debug canvas\n");
        return 1;
    }

    printf("Debug canvas created successfully\n");

    // Test basic drawing operations
    cdCanvasActivate(canvas);

    // Test color functions
    long red = cdEncodeColor(255, 0, 0);
    cdCanvasForeground(canvas, red);

    unsigned char r, g, b;
    cdDecodeColor(red, &r, &g, &b);
    if (r != 255 || g != 0 || b != 0) {
        printf("ERROR: Color encoding/decoding failed\n");
        cdKillCanvas(canvas);
        return 1;
    }

    printf("Color functions work correctly\n");

    // Test basic primitives (these should not crash)
    cdCanvasLine(canvas, 0, 0, 100, 100);
    cdCanvasRect(canvas, 10, 10, 90, 90);
    cdCanvasBox(canvas, 20, 20, 80, 80);
    cdCanvasArc(canvas, 50, 50, 30, 30, 0, 360);

    printf("Basic drawing primitives executed\n");

    // Test text
    cdCanvasText(canvas, 25, 50, "Test");
    printf("Text drawing executed\n");

    // Cleanup
    cdKillCanvas(canvas);
    printf("Canvas destroyed successfully\n");

    printf("\nAll basic tests passed!\n");
    return 0;
}