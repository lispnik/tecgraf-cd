#ifndef CD_TEST_UTILS_H
#define CD_TEST_UTILS_H

#include <cd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Test framework macros */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s - %s\n", __func__, message); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s - %s (expected: %d, actual: %d)\n", __func__, message, (int)(expected), (int)(actual)); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr, message) \
    TEST_ASSERT((ptr) == NULL, message)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    TEST_ASSERT((ptr) != NULL, message)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        if (test_func()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            tests_failed++; \
        } \
        tests_total++; \
    } while(0)

/* Test statistics */
extern int tests_total;
extern int tests_passed;
extern int tests_failed;

/* Common test utilities */
typedef struct {
    const char* name;
    cdContext* (*context_func)(void);
    int requires_display;
    int requires_file;
} backend_test_info;

/* Canvas creation helpers */
cdCanvas* test_create_canvas(const char* backend_name, int width, int height, const char* filename);
void test_destroy_canvas(cdCanvas* canvas);

/* Drawing test helpers */
void test_draw_reference_grid(cdCanvas* canvas, int width, int height);
void test_draw_color_palette(cdCanvas* canvas, int x, int y);
void test_draw_line_styles(cdCanvas* canvas, int x, int y);
void test_draw_fill_patterns(cdCanvas* canvas, int x, int y);
void test_draw_text_samples(cdCanvas* canvas, int x, int y);
void test_draw_primitives_grid(cdCanvas* canvas);

/* Backend testing */
backend_test_info* test_get_available_backends(int* count);
int test_backend_canvas_creation(const backend_test_info* backend);
int test_backend_basic_drawing(const backend_test_info* backend);

/* Image comparison utilities */
int test_create_reference_image(const char* filename, int width, int height);
int test_compare_images(const char* reference, const char* generated);

/* Performance testing */
typedef struct {
    const char* test_name;
    double elapsed_time;
    int operations_count;
    double ops_per_second;
} performance_result;

performance_result test_measure_performance(const char* test_name, void (*test_func)(cdCanvas*), cdCanvas* canvas, int iterations);

/* Math utilities */
#define TEST_PI 3.14159265358979323846
#define TEST_DEG2RAD (TEST_PI / 180.0)
#define TEST_RAD2DEG (180.0 / TEST_PI)

double test_distance(int x1, int y1, int x2, int y2);
int test_color_difference(long color1, long color2);

/* Test data generation */
void test_generate_test_image_rgb(int width, int height, unsigned char** r, unsigned char** g, unsigned char** b);
void test_generate_test_image_rgba(int width, int height, unsigned char** r, unsigned char** g, unsigned char** b, unsigned char** a);
void test_generate_gradient_pattern(int width, int height, long** pattern);

/* File utilities */
int test_file_exists(const char* filename);
void test_cleanup_files(void);

/* Memory management */
void test_memory_usage_start(void);
void test_memory_usage_report(const char* test_name);

/* Platform-specific helpers */
#ifdef _WIN32
#define TEST_PATH_SEP "\\"
#else
#define TEST_PATH_SEP "/"
#endif

#ifdef __cplusplus
}
#endif

#endif /* CD_TEST_UTILS_H */