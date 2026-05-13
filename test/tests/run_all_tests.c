/*
 * CD Library Test Suite - Master Test Runner
 * Runs all tests and generates a comprehensive report
 */

#include "test_utils.h"
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    const char* name;
    const char* description;
    const char* executable;
    int critical;  /* If 1, failure stops testing */
} test_suite;

/* Test suite definitions */
static test_suite test_suites[] = {
    {"primitives", "Basic drawing primitives (lines, arcs, polygons)", "test_primitives", 1},
    {"attributes", "Drawing attributes (colors, styles, patterns)", "test_attributes", 1},
    {"text", "Text rendering and fonts", "test_text", 1},
    {"backends", "Backend functionality and capabilities", "test_backends", 1},
    {"images", "Image operations and formats", "test_images", 0},
    {"transforms", "Coordinate transformations", "test_transforms", 0},
    {"clipping", "Clipping operations", "test_clipping", 0},
    {"paths", "Path and bezier operations", "test_paths", 0},
    {"colors", "Color management and palettes", "test_colors", 0},
    {"svg", "SVG-specific functionality", "test_svg", 0},
    {"irgb", "RGB image backend", "test_irgb", 0},
    {"comprehensive", "Comprehensive integration test", "test_comprehensive", 0},
    {"edge_cases", "Edge cases and error conditions", "test_edge_cases", 0},
    {"performance", "Performance benchmarks", "test_performance", 0},
    {"regression", "Regression tests for known issues", "test_regression", 0},
    {NULL, NULL, NULL, 0}
};

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int skipped_tests;
    double total_time;
    char error_log[4096];
} test_results;

int run_single_test(const test_suite* suite, test_results* results) {
    printf("Running %s tests: %s...\n", suite->name, suite->description);

    time_t start_time = time(NULL);

    /* Fork and execute test */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process - execute test */
        execl(suite->executable, suite->executable, (char*)NULL);
        /* If execl fails */
        printf("FAILED: Could not execute %s\n", suite->executable);
        exit(1);
    } else if (pid > 0) {
        /* Parent process - wait for completion */
        int status;
        waitpid(pid, &status, 0);

        time_t end_time = time(NULL);
        double elapsed = difftime(end_time, start_time);
        results->total_time += elapsed;

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                printf("PASSED (%s) - %.2f seconds\n", suite->name, elapsed);
                results->passed_tests++;
                return 1;
            } else {
                printf("FAILED (%s) - Exit code: %d - %.2f seconds\n",
                       suite->name, exit_code, elapsed);
                results->failed_tests++;

                /* Add to error log */
                char error_entry[256];
                snprintf(error_entry, sizeof(error_entry),
                        "Test '%s' failed with exit code %d\n", suite->name, exit_code);
                strncat(results->error_log, error_entry,
                       sizeof(results->error_log) - strlen(results->error_log) - 1);

                return 0;
            }
        } else {
            printf("FAILED (%s) - Abnormal termination - %.2f seconds\n", suite->name, elapsed);
            results->failed_tests++;
            return 0;
        }
    } else {
        /* Fork failed */
        printf("FAILED: Could not fork process for %s\n", suite->name);
        results->failed_tests++;
        return 0;
    }
}

void print_test_summary(const test_results* results) {
    printf("\n" "="*60 "\n");
    printf("CD LIBRARY TEST SUITE SUMMARY\n");
    printf("="*60 "\n");
    printf("Total test suites: %d\n", results->total_tests);
    printf("Passed: %d\n", results->passed_tests);
    printf("Failed: %d\n", results->failed_tests);
    printf("Skipped: %d\n", results->skipped_tests);
    printf("Total execution time: %.2f seconds\n", results->total_time);

    if (results->total_tests > 0) {
        double success_rate = (100.0 * results->passed_tests) / results->total_tests;
        printf("Success rate: %.1f%%\n", success_rate);
    }

    if (results->failed_tests > 0) {
        printf("\nFAILED TESTS:\n");
        printf("%s", results->error_log);
    }

    printf("="*60 "\n");
}

void generate_html_report(const test_results* results) {
    FILE* report = fopen("test_report.html", "w");
    if (!report) return;

    fprintf(report, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(report, "<title>CD Library Test Report</title>\n");
    fprintf(report, "<style>\n");
    fprintf(report, "body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(report, ".pass { color: green; }\n");
    fprintf(report, ".fail { color: red; }\n");
    fprintf(report, ".skip { color: orange; }\n");
    fprintf(report, "table { border-collapse: collapse; width: 100%%; }\n");
    fprintf(report, "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    fprintf(report, "th { background-color: #f2f2f2; }\n");
    fprintf(report, "</style>\n</head>\n<body>\n");

    time_t now = time(NULL);
    fprintf(report, "<h1>CD Library Test Report</h1>\n");
    fprintf(report, "<p>Generated: %s</p>\n", ctime(&now));

    fprintf(report, "<h2>Summary</h2>\n");
    fprintf(report, "<p>Total test suites: %d</p>\n", results->total_tests);
    fprintf(report, "<p class='pass'>Passed: %d</p>\n", results->passed_tests);
    fprintf(report, "<p class='fail'>Failed: %d</p>\n", results->failed_tests);
    fprintf(report, "<p class='skip'>Skipped: %d</p>\n", results->skipped_tests);
    fprintf(report, "<p>Total execution time: %.2f seconds</p>\n", results->total_time);

    if (results->total_tests > 0) {
        double success_rate = (100.0 * results->passed_tests) / results->total_tests;
        fprintf(report, "<p>Success rate: %.1f%%</p>\n", success_rate);
    }

    fprintf(report, "<h2>Visual Test Outputs</h2>\n");
    fprintf(report, "<p>Generated test images can be found in the output directories:</p>\n");
    fprintf(report, "<ul>\n");

    /* List generated files */
    system("find output -name '*.svg' -o -name '*.png' > generated_files.txt");
    FILE* files = fopen("generated_files.txt", "r");
    if (files) {
        char line[256];
        while (fgets(line, sizeof(line), files)) {
            line[strcspn(line, "\n")] = 0;  /* Remove newline */
            fprintf(report, "<li><a href='%s'>%s</a></li>\n", line, line);
        }
        fclose(files);
        unlink("generated_files.txt");
    }

    fprintf(report, "</ul>\n");
    fprintf(report, "</body>\n</html>\n");
    fclose(report);

    printf("HTML report generated: test_report.html\n");
}

int check_test_executable(const char* executable) {
    /* Check if test executable exists and is executable */
    return access(executable, X_OK) == 0;
}

void setup_test_environment(void) {
    /* Create output directories for each test */
    system("mkdir -p output/primitives");
    system("mkdir -p output/attributes");
    system("mkdir -p output/text");
    system("mkdir -p output/backends");
    system("mkdir -p output/images");
    system("mkdir -p output/transforms");
    system("mkdir -p output/clipping");
    system("mkdir -p output/paths");
    system("mkdir -p output/colors");
    system("mkdir -p output/svg");
    system("mkdir -p output/irgb");
    system("mkdir -p output/comprehensive");
    system("mkdir -p output/edge_cases");
    system("mkdir -p output/performance");
    system("mkdir -p output/regression");

    printf("Test environment setup complete.\n");
}

int main(int argc, char* argv[]) {
    printf("CD Library Comprehensive Test Suite\n");
    printf("====================================\n\n");

    /* Setup test environment */
    setup_test_environment();

    test_results results = {0};
    int stop_on_failure = 0;
    int verbose = 0;

    /* Parse command line arguments */
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--stop-on-failure") == 0) {
            stop_on_failure = 1;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --stop-on-failure  Stop testing on first failure\n");
            printf("  --verbose          Show detailed output\n");
            printf("  --help             Show this help\n");
            return 0;
        }
    }

    /* Run all test suites */
    test_suite* suite = test_suites;
    while (suite->name != NULL) {
        results.total_tests++;

        if (!check_test_executable(suite->executable)) {
            printf("SKIPPED (%s) - Executable not found: %s\n",
                   suite->name, suite->executable);
            results.skipped_tests++;
        } else {
            int success = run_single_test(suite, &results);

            if (!success && suite->critical && stop_on_failure) {
                printf("Critical test failed. Stopping execution.\n");
                break;
            }
        }

        suite++;
        printf("\n");
    }

    /* Generate reports */
    print_test_summary(&results);
    generate_html_report(&results);

    /* Return appropriate exit code */
    if (results.failed_tests > 0) {
        printf("\nSome tests failed. Check the error log above.\n");
        return 1;
    } else if (results.passed_tests == 0) {
        printf("\nNo tests were run successfully.\n");
        return 2;
    } else {
        printf("\nAll tests passed successfully!\n");
        return 0;
    }
}