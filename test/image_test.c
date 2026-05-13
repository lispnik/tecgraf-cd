/*
 * CD Image canvas test
 * Tests integration with IM library for image-based canvases
 */

#include <stdio.h>
#include <stdlib.h>
#include <cd.h>

#ifdef CD_ENABLE_IM
#include <im.h>
#include <im_image.h>
#endif

int main() {
    printf("CD Image Test\n");
    printf("=============\n");

#ifdef CD_ENABLE_IM
    // Create a simple RGB image
    imImage* image = imImageCreate(200, 200, IM_RGB, IM_BYTE);
    if (!image) {
        printf("ERROR: Failed to create IM image\n");
        return 1;
    }

    printf("IM image created: %dx%d\n", image->width, image->height);

    // Create CD canvas from IM image
    cdCanvas* canvas = cdCreateCanvas(cdContextImage(), image);
    if (!canvas) {
        printf("ERROR: Failed to create image canvas\n");
        imImageDestroy(image);
        return 1;
    }

    printf("Image canvas created successfully\n");

    // Activate and clear
    cdCanvasActivate(canvas);
    cdCanvasClear(canvas);

    // Set background color
    cdCanvasBackground(canvas, cdEncodeColor(255, 255, 255)); // White
    cdCanvasForeground(canvas, cdEncodeColor(255, 0, 0));     // Red

    // Draw some shapes
    cdCanvasLine(canvas, 10, 10, 190, 190);
    cdCanvasRect(canvas, 50, 50, 150, 150);

    // Draw filled rectangle
    cdCanvasForeground(canvas, cdEncodeColor(0, 255, 0)); // Green
    cdCanvasBox(canvas, 75, 75, 125, 125);

    // Draw text
    cdCanvasForeground(canvas, cdEncodeColor(0, 0, 255)); // Blue
    cdCanvasText(canvas, 100, 100, "CD+IM");

    printf("Drawing operations completed\n");

    // Test image access
    if (image->data[0]) {
        printf("Image data is accessible\n");
    }

    // Cleanup
    cdKillCanvas(canvas);
    imImageDestroy(image);

    printf("Resources cleaned up successfully\n");
    printf("\nImage test passed!\n");
    return 0;

#else
    printf("SKIPPED: CD was built without IM support\n");
    return 0;
#endif
}