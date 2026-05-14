#include <stdio.h>
#include <cd.h>
#include <cdgl.h>

int main() {
    printf("=== CD OpenGL Backend Test ===\n");
    printf("CD Version: %s\n", cdVersion());

    // Check if OpenGL context function is available
    cdContext* ctx = cdContextGL();
    if (ctx) {
        printf("✓ OpenGL context function available\n");
        printf("Context caps: 0x%lx\n", cdContextCaps(ctx));
        printf("Context type: %d\n", cdContextType(ctx));
        printf("Is Plus: %s\n", cdContextIsPlus(ctx) ? "Yes" : "No");
    } else {
        printf("✗ OpenGL context not available\n");
        return 1;
    }

    printf("OpenGL backend test completed successfully\n");
    return 0;
}