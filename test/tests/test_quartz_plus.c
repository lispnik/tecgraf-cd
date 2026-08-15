/* Tests for CD's "Plus" contexts on macOS.
 *
 * cdInitContextPlus registers the anti-aliased drivers an application gets after
 * cdUseContextPlus(1) -- GDI+ on Windows, Cairo or XRender on X11. Quartz has no such driver
 * because it needs none: CoreGraphics anti-aliases already. So what has to be true here is not
 * that a different context appears, but that asking for one changes nothing and breaks nothing:
 * the calls exist, canvases still get created, and the drawing is still anti-aliased.
 *
 * Before this, libcd on macOS exported neither function and anything calling them failed to
 * link, which is a different and much louder failure than the one being tested for.
 *
 * Headless throughout, against the offscreen Quartz bitmap canvas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cd.h"
#include "cdquartz.h"
#include "test_utils.h"

#define CANVAS_W 100
#define CANVAS_H 100

static cdCanvas* create_quartz(void)
{
    char data[64];
    cdCanvas* canvas;

    sprintf(data, "%dx%d", CANVAS_W, CANVAS_H);
    canvas = cdCreateCanvas(cdContextQuartzBitmap(), data);

    if (canvas) {
        cdCanvasBackground(canvas, CD_WHITE);
        cdCanvasClear(canvas);
    }

    return canvas;
}

/* Counts distinct grey levels along a row crossing a black diagonal on white. Anti-aliasing is
   what puts anything between the two extremes there; without it the row is black and white
   only. */
static int intermediate_shades(cdCanvas* canvas, int y)
{
    unsigned char seen[256];
    int x, count = 0;

    memset(seen, 0, sizeof(seen));

    for (x = 0; x < CANVAS_W; x++) {
        unsigned char r = 0, g = 0, b = 0;
        cdCanvasGetImageRGB(canvas, &r, &g, &b, x, y, 1, 1);
        if (r > 8 && r < 247 && !seen[r]) {
            seen[r] = 1;
            count++;
        }
    }

    return count;
}

static int draw_and_count(cdCanvas* canvas)
{
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 1);
    cdCanvasLine(canvas, 0, 0, CANVAS_W - 1, CANVAS_H - 1);
    cdCanvasFlush(canvas);
    return intermediate_shades(canvas, CANVAS_H / 2);
}

static int test_query_starts_off(void)
{
    int use = cdUseContextPlus(CD_QUERY);
    TEST_ASSERT(use == 0, "context plus starts disabled");
    return 1;
}

/* The functions this file exists for. Calling them must be safe, and must leave the Plus list
   empty -- there is no Quartz Plus driver to register. */
static int test_init_registers_nothing(void)
{
    cdInitContextPlus();

    TEST_ASSERT(cdUseContextPlus(CD_QUERY) == 0,
                "cdInitContextPlus does not turn the plus contexts on by itself");

    cdFinishContextPlus();
    return 1;
}

/* The switch itself: turning it on reports the previous setting, and turning it back off
   restores it. */
static int test_use_toggles(void)
{
    int previous;

    previous = cdUseContextPlus(1);
    TEST_ASSERT(previous == 0, "enabling reports the previous setting");
    TEST_ASSERT(cdUseContextPlus(CD_QUERY) == 1, "and the setting sticks");

    previous = cdUseContextPlus(0);
    TEST_ASSERT(previous == 1, "disabling reports the previous setting");
    TEST_ASSERT(cdUseContextPlus(CD_QUERY) == 0, "and the setting sticks");

    return 1;
}

/* The one that matters. With the plus contexts asked for, a canvas must still be created -- a
   registered-but-broken context would return NULL here -- and must still anti-alias, because on
   Quartz the ordinary context is what an application asking for Plus is meant to get. */
static int test_canvas_still_works_and_antialiases(void)
{
    cdCanvas* plain;
    cdCanvas* plus;
    int plain_shades, plus_shades;

    plain = create_quartz();
    TEST_ASSERT(plain != NULL, "a canvas is created with the plus contexts off");
    plain_shades = draw_and_count(plain);
    cdKillCanvas(plain);

    cdInitContextPlus();
    cdUseContextPlus(1);

    plus = create_quartz();
    TEST_ASSERT(plus != NULL, "a canvas is created with the plus contexts on");
    plus_shades = draw_and_count(plus);
    cdKillCanvas(plus);

    cdUseContextPlus(0);
    cdFinishContextPlus();

    printf("    intermediate shades: %d without plus, %d with\n", plain_shades, plus_shades);

    TEST_ASSERT(plain_shades > 0, "the ordinary Quartz canvas anti-aliases");
    TEST_ASSERT(plus_shades > 0, "so does the one asked for through the plus contexts");
    TEST_ASSERT(plus_shades == plain_shades,
                "and it is the same drawing -- Quartz has no separate plus driver");

    return 1;
}

int main(void)
{
    printf("Quartz Context Plus Tests\n");
    printf("=========================\n");

    RUN_TEST(test_query_starts_off);
    RUN_TEST(test_init_registers_nothing);
    RUN_TEST(test_use_toggles);
    RUN_TEST(test_canvas_still_works_and_antialiases);

    printf("\nQuartz Context Plus Summary:\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}
