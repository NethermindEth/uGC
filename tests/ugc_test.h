/**
 * @file
 * @brief uGC - minimal unit-test harness (no external dependencies)
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#pragma once

#include <stdio.h>

/* Frees every block that ugc_zalloc gave in the test (test_zalloc.c). */
void ugc_test_free_all(void);

static int ugc_test_checks = 0;
static int ugc_test_failures = 0;

#define CHECK(cond)                                                         \
    do                                                                      \
    {                                                                       \
        ugc_test_checks++;                                                  \
        if (!(cond))                                                        \
        {                                                                   \
            ugc_test_failures++;                                            \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                   \
    } while (0)

#define RUN_TEST(fn)                                                        \
    do                                                                      \
    {                                                                       \
        int before = ugc_test_failures;                                     \
        fn();                                                               \
        printf("%-40s %s\n", #fn,                                           \
               ugc_test_failures == before ? "ok" : "FAILED");              \
    } while (0)

static inline int
ugc_test_summary(void)
{
    ugc_test_free_all();
    printf("%d checks, %d failures\n", ugc_test_checks, ugc_test_failures);
    return ugc_test_failures != 0;
}
