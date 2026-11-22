#pragma once

// A minimal test harness shared by the test programs: CHECK() counts every
// check and prints the failing ones, and check_result() gives main()'s exit
// status.

#include <stdio.h>
#include <stdlib.h>

static unsigned long long checks;
static unsigned long long failures;

#define CHECK(cond, ...)                                               \
    do {                                                               \
        checks++;                                                      \
        if (!(cond)) {                                                 \
            failures++;                                                \
            fprintf(stderr, "%s:%d: %s: ", __FILE__, __LINE__, #cond); \
            fprintf(stderr, __VA_ARGS__);                              \
            fputc('\n', stderr);                                       \
        }                                                              \
    } while (0)

static int check_result(void) {
    if (failures > 0) {
        fprintf(stderr, "%llu of %llu checks failed\n", failures, checks);
        return EXIT_FAILURE;
    }
    printf("All %llu checks passed\n", checks);
    return EXIT_SUCCESS;
}
