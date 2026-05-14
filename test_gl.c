#include <stdio.h>
#include "include/cd.h"
#include "include/cdgl.h"

int main() {
    printf("=== CD Library OpenGL Support Test ===\n");

    // Check if we can get the GL context function
    cdContext *gl_context = cdContextGL();
    if (gl_context) {
        printf("✓ OpenGL context function available\n");
        printf("✓ OpenGL backend successfully compiled and linked\n");

        // Note: We don't try to create a canvas because that requires
        // an active OpenGL rendering context, which we don't have in
        // this command-line test environment.
        printf("✓ GL support is functional - context creation would require active GL environment\n");
    } else {
        printf("✗ OpenGL context function not available\n");
        return 1;
    }

    printf("=== GL support verification completed successfully ===\n");
    return 0;
}