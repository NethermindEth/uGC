/**
 * @file
 * @brief uGC - production zeroing allocator (calloc-backed)
 *
 * Kept in a separate translation unit so that Frama-C/WP verifies
 * ugc_core.c against the ACSL contract of ugc_zalloc only.
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"
#include <stdlib.h>

uint8_t *
ugc_zalloc(size_t n)
{
    return (uint8_t *)calloc(n, sizeof(char));
}
