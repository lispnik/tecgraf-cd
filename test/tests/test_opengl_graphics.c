#include <stdio.h>
#include <stdlib.h>
#include <cd.h>
#include <cdirgb.h>

int main() {
    printf("=== CD Graphics Test ===\n");
    printf("CD Version: %s\n", cdVersion());

    // Create an RGB image canvas (800x600)
    int width = 800;
    int height = 600;

    printf("Creating RGB canvas (%dx%d)...\n", width, height);
    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "800x600");

    if (!canvas) {
        printf("✗ Failed to create RGB canvas\n");
        return 1;
    }

    printf("✓ Canvas created successfully\n");

    // Activate the canvas
    cdCanvasActivate(canvas);

    // Set background
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);

    printf("Drawing graphics...\n");

    // Draw some colorful shapes

    // Red filled circle
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasSector(canvas, width/4, height/4, 80, 80, 0, 360);

    // Blue rectangle
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasRect(canvas, width/2 - 50, width/2 + 50, height/2 - 30, height/2 + 30);

    // Green thick line
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasLineWidth(canvas, 5);
    cdCanvasLine(canvas, 50, height - 50, width - 50, 50);

    // Yellow filled polygon (triangle)
    cdCanvasForeground(canvas, CD_YELLOW);
    cdCanvasBegin(canvas, CD_FILL);
    cdCanvasVertex(canvas, 3*width/4, height/4);
    cdCanvasVertex(canvas, 3*width/4 + 40, height/4 + 60);
    cdCanvasVertex(canvas, 3*width/4 - 40, height/4 + 60);
    cdCanvasEnd(canvas);

    // Magenta text
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 24);
    cdCanvasText(canvas, width/2, height/2 + 100, "CD Graphics Library!");

    // Create some additional decorative elements
    cdCanvasForeground(canvas, 0x00FF8800); // Orange
    for (int i = 0; i < 10; i++) {
        cdCanvasArc(canvas, 100 + i*60, 100, 20, 20, 0, 360);
    }

    // Get image data and save to file
    printf("Getting image data...\n");

    // Allocate separate RGB buffers
    unsigned char* r_data = malloc(width * height);
    unsigned char* g_data = malloc(width * height);
    unsigned char* b_data = malloc(width * height);

    if (!r_data || !g_data || !b_data) {
        printf("✗ Failed to allocate image buffers\n");
        if (r_data) free(r_data);
        if (g_data) free(g_data);
        if (b_data) free(b_data);
    } else {
        cdCanvasGetImageRGB(canvas, r_data, g_data, b_data, 0, 0, width, height);
        printf("✓ Image data retrieved\n");

        // Write PPM file (simple image format)
        FILE* fp = fopen("/tmp/cd_graphics_output.ppm", "w");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", width, height);

            // Convert RGB to PPM format (flip Y coordinate)
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    int idx = y * width + x;
                    fputc(r_data[idx], fp); // R
                    fputc(g_data[idx], fp); // G
                    fputc(b_data[idx], fp); // B
                }
            }
            fclose(fp);
            printf("✓ Graphics saved to /tmp/cd_graphics_output.ppm\n");
        } else {
            printf("✗ Failed to save image file\n");
        }

        free(r_data);
        free(g_data);
        free(b_data);
    }

    // Cleanup
    cdKillCanvas(canvas);
    printf("✓ Canvas cleaned up\n");

    printf("\n=== CD Graphics Test Completed ===\n");
    printf("View the output with: open /tmp/cd_graphics_output.ppm\n");

    return 0;
}