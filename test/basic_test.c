/*
 * Basic CD library test
 * Tests library initialization and basic functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <cd.h>
#include <cddebug.h>

int main() {
    printf("CD Basic Test\n");
    printf("=============\n");

    // Test version functions
    printf("CD Version: %s\n", cdVersion());
    printf("CD Version Date: %s\n", cdVersionDate());
    printf("CD Version Number: %d\n", cdVersionNumber());

    /* The debug driver logs every call to a file, so it needs a filename: passing NULL means
       there is nothing for it to open and the canvas comes back NULL. It is otherwise the one
       driver guaranteed to exist on every platform, which is what makes it the right one for a
       test that only asks whether the library works at all. */
    cdCanvas* canvas = cdCreateCanvas(CD_DEBUG, "basic_test.deb");
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

    remove("basic_test.deb");

    printf("\nAll basic tests passed!\n");
    return 0;
}