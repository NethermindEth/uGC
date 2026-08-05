/**
 * @file
 * @brief uGC - unit tests for the core allocator
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"
#include "ugc_test.h"
#include <stdint.h>
#include <string.h>

/* Header size used by uGCHeap on 64-bit hosts (ObjHeader = alignpad +
 * sync block value). */
#define HDR 8

typedef struct
{
    uint8_t *alloc_ptr;
    uint8_t *alloc_limit;
} test_context;

static void *
ctx_alloc(test_context *ctx, size_t size, bool bypass)
{
    return ugc_core_alloc(&ctx->alloc_ptr, &ctx->alloc_limit, size, bypass,
                          HDR);
}

static int
is_zeroed(const void *p, size_t n)
{
    const uint8_t *b = (const uint8_t *)p;
    size_t i;

    for (i = 0; i < n; i++)
    {
        if (b[i] != 0)
            return 0;
    }
    return 1;
}

static void
test_no_context_alloc(void)
{
    void *obj = ugc_core_alloc(NULL, NULL, 128, false, HDR);

    CHECK(obj != NULL);
    CHECK(is_zeroed(obj, 128));
    /* Object memory must be writable */
    memset(obj, 0xAA, 128);
}

static void
test_overflow_returns_null(void)
{
    test_context ctx = { 0 };

    CHECK(ugc_core_alloc(NULL, NULL, SIZE_MAX, false, HDR) == NULL);
    CHECK(ugc_core_alloc(NULL, NULL, SIZE_MAX - HDR + 1, false, HDR) == NULL);
    CHECK(ctx_alloc(&ctx, SIZE_MAX, false) == NULL);
    /* Boundary that does not overflow must succeed or fail only due to OOM;
     * SIZE_MAX - HDR would be a real OOM, so use a sane large size instead */
    CHECK(ctx_alloc(&ctx, UGC_ALLOC_QUANTUM, false) != NULL);
}

static void
test_refill_initializes_context(void)
{
    test_context ctx = { 0 };
    size_t size = 100;
    void *obj = ctx_alloc(&ctx, size, false);

    CHECK(obj != NULL);
    /* First object of a fresh quantum sits right after the plug-skew
     * header */
    CHECK(ctx.alloc_ptr == (uint8_t *)obj + size);
    CHECK(ctx.alloc_limit == (uint8_t *)obj - HDR + UGC_ALLOC_QUANTUM);
    CHECK(is_zeroed(obj, size));
    /* The whole handed-out window is zeroed */
    CHECK(is_zeroed(obj, (size_t)(ctx.alloc_limit - (uint8_t *)obj)));
}

static void
test_bump_allocation(void)
{
    test_context ctx = { 0 };
    void *first = ctx_alloc(&ctx, 64, false);
    uint8_t *ptr_after_first = ctx.alloc_ptr;
    void *second = ctx_alloc(&ctx, 32, false);

    CHECK(first != NULL);
    CHECK(second == ptr_after_first);
    CHECK(ctx.alloc_ptr == ptr_after_first + 32);
    /* Objects must not overlap */
    CHECK((uint8_t *)second >= (uint8_t *)first + 64);
    memset(first, 0x11, 64);
    memset(second, 0x22, 32);
    CHECK(((uint8_t *)first)[63] == 0x11);
    CHECK(((uint8_t *)second)[0] == 0x22);
}

static void
test_exact_fit_bump(void)
{
    test_context ctx = { 0 };
    void *first = ctx_alloc(&ctx, 16, false);
    size_t rest = (size_t)(ctx.alloc_limit - ctx.alloc_ptr);
    uint8_t *expected = ctx.alloc_ptr;
    void *second;

    CHECK(first != NULL);
    CHECK(rest <= UGC_ALLOC_QUANTUM - HDR - 16);
    second = ctx_alloc(&ctx, rest, false);
    CHECK(second == expected);
    CHECK(ctx.alloc_ptr == ctx.alloc_limit);
}

static void
test_exhaustion_triggers_refill(void)
{
    test_context ctx = { 0 };
    void *first = ctx_alloc(&ctx, 16, false);
    uint8_t *old_limit = ctx.alloc_limit;
    size_t rest = (size_t)(ctx.alloc_limit - ctx.alloc_ptr);
    /* Request one byte more than what remains: must come from a fresh
     * quantum */
    void *second = ctx_alloc(&ctx, rest + 1, false);

    CHECK(first != NULL);
    CHECK(second != NULL);
    CHECK(ctx.alloc_limit != old_limit);
    CHECK(ctx.alloc_ptr == (uint8_t *)second + rest + 1);
    CHECK(ctx.alloc_limit == (uint8_t *)second - HDR + UGC_ALLOC_QUANTUM);
}

static void
test_large_object_bypasses_context(void)
{
    test_context ctx = { 0 };
    void *seed = ctx_alloc(&ctx, 8, false);
    uint8_t *saved_ptr = ctx.alloc_ptr;
    uint8_t *saved_limit = ctx.alloc_limit;
    /* size + header > quantum => direct path, context untouched */
    void *large = ctx_alloc(&ctx, UGC_ALLOC_QUANTUM - HDR + 1, false);

    CHECK(seed != NULL);
    CHECK(large != NULL);
    CHECK(ctx.alloc_ptr == saved_ptr);
    CHECK(ctx.alloc_limit == saved_limit);
    CHECK(is_zeroed(large, UGC_ALLOC_QUANTUM - HDR + 1));
}

static void
test_boundary_size_uses_context(void)
{
    test_context ctx = { 0 };
    /* size + header == quantum: still a context allocation */
    void *obj = ctx_alloc(&ctx, UGC_ALLOC_QUANTUM - HDR, false);

    CHECK(obj != NULL);
    CHECK(ctx.alloc_ptr == ctx.alloc_limit);
}

static void
test_bypass_flag_skips_context(void)
{
    test_context ctx = { 0 };
    void *seed = ctx_alloc(&ctx, 8, false);
    uint8_t *saved_ptr = ctx.alloc_ptr;
    void *obj = ctx_alloc(&ctx, 24, true);

    CHECK(seed != NULL);
    CHECK(obj != NULL);
    CHECK(ctx.alloc_ptr == saved_ptr);
    CHECK(is_zeroed(obj, 24));
}

static void
test_zero_size_allocation(void)
{
    test_context ctx = { 0 };
    void *obj = ctx_alloc(&ctx, 0, false);

    CHECK(obj != NULL);
    CHECK(ctx.alloc_ptr == (uint8_t *)obj);
}

int
main(void)
{
    RUN_TEST(test_no_context_alloc);
    RUN_TEST(test_overflow_returns_null);
    RUN_TEST(test_refill_initializes_context);
    RUN_TEST(test_bump_allocation);
    RUN_TEST(test_exact_fit_bump);
    RUN_TEST(test_exhaustion_triggers_refill);
    RUN_TEST(test_large_object_bypasses_context);
    RUN_TEST(test_boundary_size_uses_context);
    RUN_TEST(test_bypass_flag_skips_context);
    RUN_TEST(test_zero_size_allocation);
    return ugc_test_summary();
}
