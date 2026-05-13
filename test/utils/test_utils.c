#include "test_utils.h"
#include <cdsvg.h>
#include <cdirgb.h>
#include <cddbuf.h>
#include <cddebug.h>
#include <sys/time.h>
#include <sys/stat.h>

/* Test statistics */
int tests_total = 0;
int tests_passed = 0;
int tests_failed = 0;

/* Canvas creation helpers */
cdCanvas* test_create_canvas(const char* backend_name, int width, int height, const char* filename) {
    cdCanvas* canvas = NULL;
    char canvas_data[1024];

    if (strcmp(backend_name, "SVG") == 0) {
        if (filename) {
            snprintf(canvas_data, sizeof(canvas_data), "%s %dx%d 3.78", filename, width, height);
            canvas = cdCreateCanvas(cdContextSVG(), canvas_data);
        }
    }
    else if (strcmp(backend_name, "RGB") == 0) {
        snprintf(canvas_data, sizeof(canvas_data), "%dx%d", width, height);
        canvas = cdCreateCanvas(cdContextImageRGB(), canvas_data);
    }
    else if (strcmp(backend_name, "DEBUG") == 0) {
        canvas = cdCreateCanvas(cdContextDebug(), NULL);
    }

    if (canvas) {
        cdCanvasActivate(canvas);
        cdCanvasBackground(canvas, CD_WHITE);
        cdCanvasClear(canvas);
    }

    return canvas;
}

void test_destroy_canvas(cdCanvas* canvas) {
    if (canvas) {
        cdKillCanvas(canvas);
    }
}

/* Drawing test helpers */
void test_draw_reference_grid(cdCanvas* canvas, int width, int height) {
    int i;

    /* Set grid color and style */
    cdCanvasForeground(canvas, CD_DARK_GRAY);
    cdCanvasLineStyle(canvas, CD_DOTTED);
    cdCanvasLineWidth(canvas, 1);

    /* Draw vertical lines every 50 pixels */
    for (i = 0; i < width; i += 50) {
        cdCanvasLine(canvas, i, 0, i, height);
    }

    /* Draw horizontal lines every 50 pixels */
    for (i = 0; i < height; i += 50) {
        cdCanvasLine(canvas, 0, i, width, i);
    }

    /* Draw coordinate axes */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineStyle(canvas, CD_CONTINUOUS);
    cdCanvasLineWidth(canvas, 2);

    /* X-axis */
    cdCanvasLine(canvas, 0, height/2, width, height/2);
    /* Y-axis */
    cdCanvasLine(canvas, width/2, 0, width/2, height);

    /* Draw origin marker */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasMarkType(canvas, CD_CIRCLE);
    cdCanvasMarkSize(canvas, 10);
    cdCanvasMark(canvas, width/2, height/2);
}

void test_draw_color_palette(cdCanvas* canvas, int x, int y) {
    long colors[] = {
        CD_RED, CD_GREEN, CD_BLUE, CD_YELLOW, CD_MAGENTA, CD_CYAN,
        CD_DARK_RED, CD_DARK_GREEN, CD_DARK_BLUE, CD_WHITE, CD_BLACK
    };
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    int i;
    int box_size = 30;

    for (i = 0; i < num_colors; i++) {
        cdCanvasForeground(canvas, colors[i]);
        cdCanvasBox(canvas, x + i * box_size, x + (i + 1) * box_size, y, y + box_size);
    }
}

void test_draw_line_styles(cdCanvas* canvas, int x, int y) {
    int styles[] = {CD_CONTINUOUS, CD_DASHED, CD_DOTTED, CD_DASH_DOT, CD_DASH_DOT_DOT};
    const char* names[] = {"CONTINUOUS", "DASHED", "DOTTED", "DASH_DOT", "DASH_DOT_DOT"};
    int num_styles = sizeof(styles) / sizeof(styles[0]);
    int i;
    int line_spacing = 25;

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasLineWidth(canvas, 3);

    for (i = 0; i < num_styles; i++) {
        cdCanvasLineStyle(canvas, styles[i]);
        cdCanvasLine(canvas, x, y + i * line_spacing, x + 200, y + i * line_spacing);

        /* Label the line style */
        cdCanvasText(canvas, x + 210, y + i * line_spacing, names[i]);
    }
}

void test_draw_fill_patterns(cdCanvas* canvas, int x, int y) {
    int patterns[] = {CD_SOLID, CD_HATCH, CD_STIPPLE, CD_PATTERN};
    int hatches[] = {CD_HORIZONTAL, CD_VERTICAL, CD_FDIAGONAL, CD_BDIAGONAL, CD_CROSS, CD_DIAGCROSS};
    int i, j;
    int box_size = 40;

    /* Solid fill */
    cdCanvasInteriorStyle(canvas, CD_SOLID);
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasBox(canvas, x, x + box_size, y, y + box_size);

    /* Hatch patterns */
    for (i = 0; i < 6; i++) {
        cdCanvasInteriorStyle(canvas, CD_HATCH);
        cdCanvasHatch(canvas, hatches[i]);
        cdCanvasForeground(canvas, CD_RED);
        cdCanvasBox(canvas, x + (i + 1) * (box_size + 5), x + (i + 2) * (box_size + 5), y, y + box_size);
    }
}

void test_draw_text_samples(cdCanvas* canvas, int x, int y) {
    const char* sample_text = "CD Test Suite";
    int alignments[] = {CD_NORTH, CD_SOUTH, CD_EAST, CD_WEST, CD_CENTER};
    const char* align_names[] = {"NORTH", "SOUTH", "EAST", "WEST", "CENTER"};
    int i;
    int text_spacing = 30;

    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_PLAIN, 12);

    for (i = 0; i < 5; i++) {
        cdCanvasTextAlignment(canvas, alignments[i]);
        cdCanvasText(canvas, x, y + i * text_spacing, sample_text);
        cdCanvasText(canvas, x + 200, y + i * text_spacing, align_names[i]);
    }
}

void test_draw_primitives_grid(cdCanvas* canvas) {
    int width, height;
    cdCanvasGetSize(canvas, &width, &height, NULL, NULL);

    /* Grid background */
    test_draw_reference_grid(canvas, width, height);

    /* Basic primitives */
    cdCanvasForeground(canvas, CD_BLUE);
    cdCanvasLineWidth(canvas, 2);

    /* Lines */
    cdCanvasLine(canvas, 50, 50, 150, 150);
    cdCanvasLine(canvas, 150, 50, 50, 150);

    /* Rectangle */
    cdCanvasForeground(canvas, CD_GREEN);
    cdCanvasRect(canvas, 200, 300, 50, 150);

    /* Filled box */
    cdCanvasForeground(canvas, CD_RED);
    cdCanvasBox(canvas, 350, 450, 50, 150);

    /* Arc */
    cdCanvasForeground(canvas, CD_MAGENTA);
    cdCanvasArc(canvas, 550, 100, 80, 80, 0, 180);

    /* Circle (sector) */
    cdCanvasForeground(canvas, CD_CYAN);
    cdCanvasSector(canvas, 650, 100, 60, 60, 0, 360);

    /* Text */
    cdCanvasForeground(canvas, CD_BLACK);
    cdCanvasFont(canvas, "Helvetica", CD_BOLD, 16);
    cdCanvasText(canvas, 50, height - 50, "CD Library Test Suite");
}

/* Backend testing */
static backend_test_info available_backends[] = {
    {"SVG", cdContextSVG, 0, 1},
    {"RGB", cdContextImageRGB, 0, 0},
    {"DEBUG", cdContextDebug, 0, 0},
    {NULL, NULL, 0, 0}
};

backend_test_info* test_get_available_backends(int* count) {
    *count = 0;
    while (available_backends[*count].name != NULL) {
        (*count)++;
    }
    return available_backends;
}

int test_backend_canvas_creation(const backend_test_info* backend) {
    cdCanvas* canvas = test_create_canvas(backend->name, 400, 300, "test_canvas_creation.svg");
    if (!canvas) {
        printf("Failed to create canvas for backend: %s\n", backend->name);
        return 0;
    }

    test_destroy_canvas(canvas);
    return 1;
}

int test_backend_basic_drawing(const backend_test_info* backend) {
    char filename[256];
    snprintf(filename, sizeof(filename), "test_basic_drawing_%s.svg", backend->name);

    cdCanvas* canvas = test_create_canvas(backend->name, 400, 300, filename);
    if (!canvas) {
        return 0;
    }

    /* Draw basic primitives */
    test_draw_primitives_grid(canvas);

    test_destroy_canvas(canvas);
    return 1;
}

/* Image generation utilities */
void test_generate_test_image_rgb(int width, int height, unsigned char** r, unsigned char** g, unsigned char** b) {
    int i, j;
    int size = width * height;

    *r = malloc(size);
    *g = malloc(size);
    *b = malloc(size);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            int index = i * width + j;
            (*r)[index] = (unsigned char)(255 * j / width);
            (*g)[index] = (unsigned char)(255 * i / height);
            (*b)[index] = (unsigned char)(128);
        }
    }
}

void test_generate_test_image_rgba(int width, int height, unsigned char** r, unsigned char** g, unsigned char** b, unsigned char** a) {
    test_generate_test_image_rgb(width, height, r, g, b);

    int i;
    int size = width * height;
    *a = malloc(size);

    for (i = 0; i < size; i++) {
        (*a)[i] = (unsigned char)(255 * (i % 256) / 255);
    }
}

void test_generate_gradient_pattern(int width, int height, long** pattern) {
    int i, j;
    int size = width * height;

    *pattern = malloc(size * sizeof(long));

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            int index = i * width + j;
            unsigned char red = (unsigned char)(255 * j / width);
            unsigned char green = (unsigned char)(255 * i / height);
            unsigned char blue = 128;
            (*pattern)[index] = cdEncodeColor(red, green, blue);
        }
    }
}

/* Performance testing */
performance_result test_measure_performance(const char* test_name, void (*test_func)(cdCanvas*), cdCanvas* canvas, int iterations) {
    performance_result result;
    struct timeval start, end;
    int i;

    result.test_name = test_name;
    result.operations_count = iterations;

    gettimeofday(&start, NULL);

    for (i = 0; i < iterations; i++) {
        test_func(canvas);
    }

    gettimeofday(&end, NULL);

    result.elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    result.ops_per_second = iterations / result.elapsed_time;

    return result;
}

/* File utilities */
int test_file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void test_cleanup_files(void) {
    /* Clean up generated test files */
    system("rm -f test_*.svg test_*.png test_*.jpg");
}

/* Math utilities */
double test_distance(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

int test_color_difference(long color1, long color2) {
    unsigned char r1, g1, b1, r2, g2, b2;
    cdDecodeColor(color1, &r1, &g1, &b1);
    cdDecodeColor(color2, &r2, &g2, &b2);

    return abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
}