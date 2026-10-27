/**
 * @file
 * @brief uGC - zeroing allocator of the unit tests
 *
 * Takes the place of ugc_zalloc.c in the tests. The core never frees what it
 * allocates (the GC of a zkVM guest gives memory back only at exit), so this
 * allocator records each block, and the test frees all of them at the end
 * with ugc_test_free_all(). LeakSanitizer then reports only a real leak.
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"
#include "ugc_test.h"
#include <stdlib.h>

#define UGC_TEST_MAX_BLOCKS 64

static uint8_t *ugc_test_blocks[UGC_TEST_MAX_BLOCKS];
static size_t ugc_test_nblocks = 0;

uint8_t *
ugc_zalloc(size_t n)
{
    uint8_t *block;

    if (ugc_test_nblocks == UGC_TEST_MAX_BLOCKS)
        return (uint8_t *)0;
    block = (uint8_t *)calloc(n, sizeof(char));
    if (block != (uint8_t *)0)
        ugc_test_blocks[ugc_test_nblocks++] = block;
    return block;
}

void
ugc_test_free_all(void)
{
    while (ugc_test_nblocks > 0)
        free(ugc_test_blocks[--ugc_test_nblocks]);
}
