/**
 * @file
 * @brief uGC - Frama-C/Eva driver: exercises the core with unconstrained
 *        inputs to verify absence of runtime errors (undefined behavior)
 *
 * This file is only compiled by Frama-C (formal/verify.sh); it is not part
 * of the production library or the unit tests.
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"
#include "__fc_builtin.h"

static int object_a, object_b;

int
main(void)
{
    /* --- Allocation: no context, arbitrary size (overflow + direct) --- */
    size_t size = Frama_C_size_t_interval(0, SIZE_MAX);
    (void)ugc_core_alloc((uint8_t **)0, (uint8_t **)0, size, false, 8);

    /* --- Allocation: context paths (refill, then bump / direct) --- */
    uint8_t *alloc_ptr = (uint8_t *)0;
    uint8_t *alloc_limit = (uint8_t *)0;
    size_t s1 = Frama_C_size_t_interval(0, 2 * UGC_ALLOC_QUANTUM);
    size_t s2 = Frama_C_size_t_interval(0, 2 * UGC_ALLOC_QUANTUM);
    bool bypass = Frama_C_interval(0, 1);

    (void)ugc_core_alloc(&alloc_ptr, &alloc_limit, s1, false, 8);

    /* Re-establish the context invariant for the non-relational domain:
     * at runtime alloc_ptr/alloc_limit are either both null or both point
     * into the same quantum (the context_window precondition). */
    if (alloc_ptr == (uint8_t *)0 || alloc_limit == (uint8_t *)0)
    {
        alloc_ptr = (uint8_t *)0;
        alloc_limit = (uint8_t *)0;
    }
    (void)ugc_core_alloc(&alloc_ptr, &alloc_limit, s2, bypass, 8);

    /* --- Handle store: creation up to exhaustion --- */
    ugc_handle_store_reset();

    void **h1 = ugc_handle_create(&object_a, Frama_C_interval(0, 12));
    void **h2 = ugc_handle_create_with_extra(&object_a,
        Frama_C_interval(0, 12), &object_b);
    void **h3 = ugc_handle_create_dependent(&object_a, &object_b,
        Frama_C_interval(0, 12));

    if (h1 != (void **)0)
    {
        (void)ugc_handle_store_contains(h1);
        (void)ugc_handle_index(h1);

        /* --- Destroy + recycle: double destroy must be harmless --- */
        ugc_handle_destroy_at(ugc_handle_index(h1));
        ugc_handle_destroy_at(ugc_handle_index(h1));
        h1 = ugc_handle_create(&object_b, Frama_C_interval(0, 12));
    }
    (void)ugc_handle_store_contains(&object_a);
    (void)ugc_handle_store_contains((const void *)0);

    /* --- Handle accessors over the whole index range --- */
    size_t idx = Frama_C_size_t_interval(0, UGC_HANDLE_COUNT - 1);
    ugc_handle_set_type_at(idx, Frama_C_interval(0, 12));
    (void)ugc_handle_get_type_at(idx);
    ugc_handle_set_extra_at(idx, &object_b);
    (void)ugc_handle_get_extra_at(idx);
    ugc_handle_set_dependent_at(idx, &object_b);
    (void)ugc_handle_dependent_slot_at(idx);

    /* --- Handle slot operations --- */
    if (h2 != (void **)0)
    {
        ugc_handle_slot_store(h2, &object_b);
        (void)ugc_handle_slot_store_if_null(h2, &object_a);
        (void)ugc_handle_slot_cas(h2, &object_a, &object_b);
    }
    if (h3 != (void **)0)
    {
        (void)ugc_handle_slot_store_if_null(h3, &object_a);
    }

    ugc_handle_store_reset();
    return 0;
}
