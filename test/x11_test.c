/*
 * CD X11 canvas test
 * Tests X11-specific functionality (Linux/Unix only)
 */

#include <stdio.h>
#include <stdlib.h>
#include <cd.h>

#ifdef CD_X11
#include <X11/Xlib.h>

int main() {
    printf("CD X11 Test\n");
    printf("===========\n");

    // Try to open X11 display
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        printf("SKIPPED: No X11 display available\n");
        return 0;
    }

    printf("X11 display opened successfully\n");

    // We can't create a real window in CI, so just test that the context exists
    cdContext* x11_context = cdContextX11();
    if (!x11_context) {
        printf("ERROR: X11 context not available\n");
        XCloseDisplay(display);
        return 1;
    }

    printf("X11 context is available\n");

    // Test double buffer context too
    cdContext* x11_db_context = cdContextDBuffer();
    if (!x11_db_context) {
        printf("WARNING: Double buffer context not available\n");
    } else {
        printf("Double buffer context is available\n");
    }

    XCloseDisplay(display);
    printf("\nX11 test passed!\n");
    return 0;
}

#else

int main() {
    printf("SKIPPED: CD was built without X11 support\n");
    return 0;
}

#endif