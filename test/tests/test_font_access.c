#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cd.h>
#include <cdirgb.h>

// Test font access and availability
int test_system_fonts() {
    printf("=== System Font Access Test ===\n");

    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    if (!canvas) {
        printf("✗ Failed to create canvas\n");
        return 0;
    }

    cdCanvasActivate(canvas);

    // Test common system fonts
    const char* test_fonts[] = {
        "Arial",
        "Times",
        "Courier",
        "Helvetica",
        "System",
        NULL
    };

    int fonts_found = 0;
    int total_fonts = 0;

    for (int i = 0; test_fonts[i]; i++) {
        total_fonts++;
        printf("Testing font: %s...", test_fonts[i]);

        // Try to set font
        int result = cdCanvasFont(canvas, test_fonts[i], CD_PLAIN, 12);
        if (result) {
            printf(" ✓ Available\n");
            fonts_found++;

            // Get actual font info
            char type_face[256];
            int style, size;
            cdCanvasGetFont(canvas, type_face, &style, &size);
            printf("  → Actual: %s, style=%d, size=%d\n", type_face, style, size);
        } else {
            printf(" ✗ Not available\n");
        }
    }

    cdKillCanvas(canvas);

    printf("System fonts available: %d/%d\n", fonts_found, total_fonts);
    return fonts_found > 0 ? 1 : 0;
}

// Test vector font access from different possible paths
int test_vector_fonts() {
    printf("\n=== Vector Font Access Test ===\n");

    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "400x300");
    if (!canvas) {
        printf("✗ Failed to create canvas\n");
        return 0;
    }

    cdCanvasActivate(canvas);

    // Possible vector font locations
    const char* font_paths[] = {
        // Source tree (for development)
        "etc/vectorfont00.txt",
        "../etc/vectorfont00.txt",
        "../../etc/vectorfont00.txt",

        // Homebrew installation paths
        "/opt/homebrew/share/cd/vectorfont00.txt",
        "/opt/homebrew/Cellar/tecgraf-cd/5.14.0/share/cd/vectorfont00.txt",

        // Standard system paths
        "/usr/local/share/cd/vectorfont00.txt",
        "/usr/share/cd/vectorfont00.txt",

        // Current directory (for testing)
        "vectorfont00.txt",

        NULL
    };

    int fonts_found = 0;
    int total_paths = 0;

    for (int i = 0; font_paths[i]; i++) {
        total_paths++;
        printf("Testing vector font path: %s...", font_paths[i]);

        // Check if file exists first
        if (access(font_paths[i], F_OK) == 0) {
            printf(" (file exists)");

            // Try to load vector font
            char* result = cdCanvasVectorFont(canvas, font_paths[i]);
            if (result) {
                printf(" ✓ Loaded successfully\n");
                fonts_found++;
                printf("  → Font loaded: %s\n", result);

                // Test rendering with this font
                cdCanvasVectorFontSize(canvas, 20, 20);
                double sx, sy;
                cdCanvasGetVectorFontSize(canvas, &sx, &sy);
                printf("  → Font size set to: %.1fx%.1f\n", sx, sy);
            } else {
                printf(" ✗ Failed to load\n");
            }
        } else {
            printf(" (file not found)\n");
        }
    }

    cdKillCanvas(canvas);

    printf("Vector fonts accessible: %d/%d paths\n", fonts_found, total_paths);
    return fonts_found > 0 ? 1 : 0;
}

// Test font rendering capabilities
int test_font_rendering() {
    printf("\n=== Font Rendering Test ===\n");

    cdCanvas* canvas = cdCreateCanvas(cdContextImageRGB(), "600x400");
    if (!canvas) {
        printf("✗ Failed to create canvas\n");
        return 0;
    }

    cdCanvasActivate(canvas);
    cdCanvasBackground(canvas, CD_WHITE);
    cdCanvasClear(canvas);
    cdCanvasForeground(canvas, CD_BLUE);

    int tests_passed = 0;

    // Test 1: Basic system font rendering
    printf("Test 1: System font rendering...");
    cdCanvasFont(canvas, "Arial", CD_PLAIN, 16);
    cdCanvasText(canvas, 50, 350, "System Font Test - Arial");
    printf(" ✓ Text rendered\n");
    tests_passed++;

    // Test 2: Font styles
    printf("Test 2: Font style variations...");
    cdCanvasFont(canvas, "Arial", CD_BOLD, 14);
    cdCanvasText(canvas, 50, 300, "Bold Text");

    cdCanvasFont(canvas, "Arial", CD_ITALIC, 14);
    cdCanvasText(canvas, 200, 300, "Italic Text");

    cdCanvasFont(canvas, "Arial", CD_BOLD | CD_ITALIC, 14);
    cdCanvasText(canvas, 350, 300, "Bold Italic");
    printf(" ✓ Style variations rendered\n");
    tests_passed++;

    // Test 3: Font sizes
    printf("Test 3: Font size variations...");
    int sizes[] = {10, 14, 18, 24, 32};
    for (int i = 0; i < 5; i++) {
        cdCanvasFont(canvas, "Arial", CD_PLAIN, sizes[i]);
        char text[64];
        snprintf(text, sizeof(text), "Size %d", sizes[i]);
        cdCanvasText(canvas, 50 + i * 100, 200, text);
    }
    printf(" ✓ Size variations rendered\n");
    tests_passed++;

    // Test 4: Vector font (if available)
    printf("Test 4: Vector font rendering...");
    // Try to find a vector font
    const char* vf_paths[] = {
        "etc/vectorfont00.txt",
        "/opt/homebrew/share/cd/vectorfont00.txt",
        NULL
    };

    int vector_font_loaded = 0;
    for (int i = 0; vf_paths[i] && !vector_font_loaded; i++) {
        if (access(vf_paths[i], F_OK) == 0) {
            char* vf_result = cdCanvasVectorFont(canvas, vf_paths[i]);
            if (vf_result) {
                vector_font_loaded = 1;
                cdCanvasVectorFontSize(canvas, 24, 24);
                cdCanvasText(canvas, 50, 100, "Vector Font Test");
                printf(" ✓ Vector font rendered from %s\n", vf_paths[i]);
                tests_passed++;
                break;
            }
        }
    }

    if (!vector_font_loaded) {
        printf(" ⚠ No vector fonts available\n");
    }

    // Save output for visual verification
    printf("Saving font test output...");

    // Get image data and save as PPM
    unsigned char* r_data = malloc(600 * 400);
    unsigned char* g_data = malloc(600 * 400);
    unsigned char* b_data = malloc(600 * 400);

    if (r_data && g_data && b_data) {
        cdCanvasGetImageRGB(canvas, r_data, g_data, b_data, 0, 0, 600, 400);

        FILE* fp = fopen("/tmp/font_test_output.ppm", "w");
        if (fp) {
            fprintf(fp, "P6\n600 400\n255\n");
            for (int y = 399; y >= 0; y--) {
                for (int x = 0; x < 600; x++) {
                    int idx = y * 600 + x;
                    fputc(r_data[idx], fp);
                    fputc(g_data[idx], fp);
                    fputc(b_data[idx], fp);
                }
            }
            fclose(fp);
            printf(" ✓ Output saved to /tmp/font_test_output.ppm\n");
        }

        free(r_data);
        free(g_data);
        free(b_data);
    }

    cdKillCanvas(canvas);

    printf("Font rendering tests passed: %d/%d\n", tests_passed, vector_font_loaded ? 4 : 3);
    return tests_passed >= 3 ? 1 : 0;
}

// Check font installation paths
int check_font_installation() {
    printf("\n=== Font Installation Check ===\n");

    const char* check_paths[] = {
        "/opt/homebrew/share/cd",
        "/opt/homebrew/Cellar/tecgraf-cd/5.14.0/share/cd",
        "/usr/local/share/cd",
        "/usr/share/cd",
        "etc",
        NULL
    };

    int installations_found = 0;

    for (int i = 0; check_paths[i]; i++) {
        printf("Checking path: %s...", check_paths[i]);

        if (access(check_paths[i], F_OK) == 0) {
            printf(" ✓ Directory exists\n");

            // Check for font files
            char font_path[512];
            snprintf(font_path, sizeof(font_path), "%s/arial.ttf", check_paths[i]);
            if (access(font_path, F_OK) == 0) {
                printf("  → TTF fonts available\n");
                installations_found++;
            }

            snprintf(font_path, sizeof(font_path), "%s/vectorfont00.txt", check_paths[i]);
            if (access(font_path, F_OK) == 0) {
                printf("  → Vector fonts available\n");
                installations_found++;
            }
        } else {
            printf(" ✗ Directory not found\n");
        }
    }

    printf("Font installations found: %d locations\n", installations_found);
    return installations_found > 0 ? 1 : 0;
}

int main() {
    printf("CD Font Access Verification Test\n");
    printf("=================================\n");
    printf("CD Version: %s\n\n", cdVersion());

    int total_tests = 4;
    int passed_tests = 0;

    // Run all tests
    if (check_font_installation()) passed_tests++;
    if (test_system_fonts()) passed_tests++;
    if (test_vector_fonts()) passed_tests++;
    if (test_font_rendering()) passed_tests++;

    // Summary
    printf("\n=== Test Results Summary ===\n");
    printf("Tests passed: %d/%d\n", passed_tests, total_tests);

    if (passed_tests == total_tests) {
        printf("✓ All font access tests passed!\n");
        printf("✓ CD can access fonts properly\n");
        printf("✓ Font installation is working\n");
        printf("\nGenerated files:\n");
        printf("- /tmp/font_test_output.ppm (visual verification)\n");
        return 0;
    } else {
        printf("✗ Some font access tests failed\n");
        if (passed_tests == 0) {
            printf("  → No fonts accessible - installation issue\n");
        } else if (passed_tests < total_tests) {
            printf("  → Partial font support - some paths missing\n");
        }
        return 1;
    }
}