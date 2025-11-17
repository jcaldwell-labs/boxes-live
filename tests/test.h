#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test framework macros */
#define TEST_START() int test_passed = 0; int test_failed = 0
#define TEST_END() printf("\n=== Test Summary ===\n"); \
                   printf("Passed: %d\n", test_passed); \
                   printf("Failed: %d\n", test_failed); \
                   return test_failed > 0 ? 1 : 0

#define TEST(name) \
    printf("\n[TEST] %s\n", name); \
    if (1)

#define ASSERT(condition, message) \
    if (condition) { \
        printf("  ✓ %s\n", message); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s\n", message); \
        test_failed++; \
    }

#define ASSERT_EQ(actual, expected, message) \
    if ((actual) == (expected)) { \
        printf("  ✓ %s (expected: %d, got: %d)\n", message, (int)(expected), (int)(actual)); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s (expected: %d, got: %d)\n", message, (int)(expected), (int)(actual)); \
        test_failed++; \
    }

#define ASSERT_STR_EQ(actual, expected, message) \
    if (strcmp((actual), (expected)) == 0) { \
        printf("  ✓ %s\n", message); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s (expected: '%s', got: '%s')\n", message, (expected), (actual)); \
        test_failed++; \
    }

#define ASSERT_NEAR(actual, expected, epsilon, message) \
    if (fabs((actual) - (expected)) < (epsilon)) { \
        printf("  ✓ %s\n", message); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s (expected: %.2f, got: %.2f)\n", message, (double)(expected), (double)(actual)); \
        test_failed++; \
    }

#define ASSERT_NULL(ptr, message) \
    if ((ptr) == NULL) { \
        printf("  ✓ %s\n", message); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s (expected NULL, got non-NULL)\n", message); \
        test_failed++; \
    }

#define ASSERT_NOT_NULL(ptr, message) \
    if ((ptr) != NULL) { \
        printf("  ✓ %s\n", message); \
        test_passed++; \
    } else { \
        printf("  ✗ FAILED: %s (expected non-NULL, got NULL)\n", message); \
        test_failed++; \
    }

#endif /* TEST_H */
