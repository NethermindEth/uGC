/**
 * @file
 * @brief uGC - unit tests for the core handle store
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"
#include "ugc_test.h"
#include <stdint.h>

static int obj_a, obj_b, obj_c, extra_a;

static void
test_create_basic(void)
{
    void **h;

    ugc_handle_store_reset();
    h = ugc_handle_create(&obj_a, 1);
    CHECK(h != NULL);
    CHECK(*h == &obj_a);
    CHECK(ugc_handle_count == 1);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h)) == 1);
    CHECK(ugc_handle_index(h) == 0);
}

static void
test_create_sequential_slots(void)
{
    void **h1, **h2;

    ugc_handle_store_reset();
    h1 = ugc_handle_create(&obj_a, 1);
    h2 = ugc_handle_create(&obj_b, 2);
    CHECK(h1 != NULL && h2 != NULL);
    CHECK(h2 == h1 + 1);
    CHECK(ugc_handle_index(h2) == 1);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h1)) == 1);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h2)) == 2);
}

static void
test_create_with_extra(void)
{
    void **h;

    ugc_handle_store_reset();
    h = ugc_handle_create_with_extra(&obj_a, 5, &extra_a);
    CHECK(h != NULL);
    CHECK(*h == &obj_a);
    CHECK(ugc_handle_get_extra_at(ugc_handle_index(h)) == &extra_a);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h)) == 5);
}

static void
test_dependent_handles(void)
{
    void **h;
    void **slot;

    ugc_handle_store_reset();
    h = ugc_handle_create_dependent(&obj_a, &obj_b, 6);
    CHECK(h != NULL);
    CHECK(*h == &obj_a);
    /* Dependent handles must carry their HandleType (HNDTYPE_DEPENDENT) */
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h)) == 6);
    slot = ugc_handle_dependent_slot_at(ugc_handle_index(h));
    CHECK(slot != NULL);
    CHECK(*slot == &obj_b);
    ugc_handle_set_dependent_at(ugc_handle_index(h), &obj_c);
    CHECK(*slot == &obj_c);
}

static void
test_set_type_and_extra(void)
{
    void **h;

    ugc_handle_store_reset();
    h = ugc_handle_create(&obj_a, 1);
    CHECK(h != NULL);
    ugc_handle_set_type_at(ugc_handle_index(h), 7);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h)) == 7);
    ugc_handle_set_extra_at(ugc_handle_index(h), &extra_a);
    CHECK(ugc_handle_get_extra_at(ugc_handle_index(h)) == &extra_a);
}

static void
test_contains(void)
{
    void **h;
    int local;

    ugc_handle_store_reset();
    h = ugc_handle_create(&obj_a, 1);
    CHECK(ugc_handle_store_contains(h));
    CHECK(!ugc_handle_store_contains(&local));
    CHECK(!ugc_handle_store_contains(NULL));
    /* Any slot address is "contained", even unused ones (matches CLR
     * semantics of a store-range check) */
    CHECK(ugc_handle_store_contains(&ugc_handles[UGC_HANDLE_COUNT - 1]));
    /* One-past-the-end is not contained */
    CHECK(!ugc_handle_store_contains(&ugc_handles[0] + UGC_HANDLE_COUNT));
}

static void
test_store_exhaustion(void)
{
    int i;
    void **h = NULL;

    ugc_handle_store_reset();
    for (i = 0; i < UGC_HANDLE_COUNT; i++)
    {
        h = ugc_handle_create(&obj_a, 0);
        CHECK(h != NULL);
        if (h == NULL)
            break;
    }
    CHECK(ugc_handle_count == UGC_HANDLE_COUNT);
    /* All variants must fail once the store is full, without touching
     * the count */
    CHECK(ugc_handle_create(&obj_b, 0) == NULL);
    CHECK(ugc_handle_create_with_extra(&obj_b, 0, &extra_a) == NULL);
    CHECK(ugc_handle_create_dependent(&obj_b, &obj_c, 6) == NULL);
    CHECK(ugc_handle_count == UGC_HANDLE_COUNT);
    /* Destroying a handle makes room again: the freed slot is recycled */
    ugc_handle_destroy_at(0);
    h = ugc_handle_create(&obj_b, 3);
    CHECK(h == &ugc_handles[0]);
    CHECK(*h == &obj_b);
    CHECK(ugc_handle_create(&obj_c, 0) == NULL);
    ugc_handle_store_reset();
    CHECK(ugc_handle_count == 0);
}

static void
test_destroy_and_reuse(void)
{
    void **h1, **h2, **h3;

    ugc_handle_store_reset();
    h1 = ugc_handle_create_with_extra(&obj_a, 1, &extra_a);
    h2 = ugc_handle_create_dependent(&obj_b, &obj_c, 6);
    CHECK(h1 != NULL && h2 != NULL);

    /* Destroy scrubs the slot completely */
    ugc_handle_destroy_at(ugc_handle_index(h1));
    CHECK(*h1 == NULL);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h1)) ==
        UGC_HANDLE_TYPE_FREE);
    CHECK(ugc_handle_get_extra_at(ugc_handle_index(h1)) == NULL);
    CHECK(ugc_handle_free_count == 1);

    /* The freed slot is recycled (LIFO) and carries no stale state */
    h3 = ugc_handle_create(&obj_c, 2);
    CHECK(h3 == h1);
    CHECK(*h3 == &obj_c);
    CHECK(ugc_handle_get_type_at(ugc_handle_index(h3)) == 2);
    CHECK(ugc_handle_get_extra_at(ugc_handle_index(h3)) == NULL);
    CHECK(*ugc_handle_dependent_slot_at(ugc_handle_index(h3)) == NULL);
    CHECK(ugc_handle_free_count == 0);
    CHECK(ugc_handle_count == 2);
}

static void
test_double_destroy(void)
{
    void **h1, **h2;

    ugc_handle_store_reset();
    h1 = ugc_handle_create(&obj_a, 1);
    CHECK(h1 != NULL);

    ugc_handle_destroy_at(ugc_handle_index(h1));
    CHECK(ugc_handle_free_count == 1);
    /* A double destroy must not push the same index twice */
    ugc_handle_destroy_at(ugc_handle_index(h1));
    CHECK(ugc_handle_free_count == 1);

    /* ... otherwise two creates would alias the same slot */
    h1 = ugc_handle_create(&obj_b, 2);
    h2 = ugc_handle_create(&obj_c, 3);
    CHECK(h1 != NULL && h2 != NULL);
    CHECK(h1 != h2);
    CHECK(*h1 == &obj_b);
    CHECK(*h2 == &obj_c);
}

static void
test_destroy_lifo_order(void)
{
    void **h1, **h2, **h3, **r1, **r2;

    ugc_handle_store_reset();
    h1 = ugc_handle_create(&obj_a, 1);
    h2 = ugc_handle_create(&obj_b, 1);
    h3 = ugc_handle_create(&obj_c, 1);
    CHECK(h1 != NULL && h2 != NULL && h3 != NULL);

    ugc_handle_destroy_at(ugc_handle_index(h1));
    ugc_handle_destroy_at(ugc_handle_index(h3));
    CHECK(ugc_handle_free_count == 2);

    /* Most recently destroyed slot is reused first */
    r1 = ugc_handle_create(&obj_a, 1);
    CHECK(r1 == h3);
    r2 = ugc_handle_create(&obj_a, 1);
    CHECK(r2 == h1);
    CHECK(ugc_handle_free_count == 0);
    CHECK(ugc_handle_count == 3);
}

static void
test_slot_store(void)
{
    void *slot = NULL;

    ugc_handle_slot_store(&slot, &obj_a);
    CHECK(slot == &obj_a);
    ugc_handle_slot_store(&slot, NULL);
    CHECK(slot == NULL);
}

static void
test_slot_store_if_null(void)
{
    void *slot = NULL;

    CHECK(ugc_handle_slot_store_if_null(&slot, &obj_a));
    CHECK(slot == &obj_a);
    CHECK(!ugc_handle_slot_store_if_null(&slot, &obj_b));
    CHECK(slot == &obj_a);
}

static void
test_slot_cas(void)
{
    void *slot = &obj_a;

    /* Mismatch: slot kept, current value returned */
    CHECK(ugc_handle_slot_cas(&slot, &obj_c, &obj_b) == &obj_a);
    CHECK(slot == &obj_a);
    /* Match: swapped; this implementation returns the new value */
    CHECK(ugc_handle_slot_cas(&slot, &obj_c, &obj_a) == &obj_c);
    CHECK(slot == &obj_c);
}

int
main(void)
{
    RUN_TEST(test_create_basic);
    RUN_TEST(test_create_sequential_slots);
    RUN_TEST(test_create_with_extra);
    RUN_TEST(test_dependent_handles);
    RUN_TEST(test_set_type_and_extra);
    RUN_TEST(test_contains);
    RUN_TEST(test_store_exhaustion);
    RUN_TEST(test_destroy_and_reuse);
    RUN_TEST(test_double_destroy);
    RUN_TEST(test_destroy_lifo_order);
    RUN_TEST(test_slot_store);
    RUN_TEST(test_slot_store_if_null);
    RUN_TEST(test_slot_cas);
    return ugc_test_summary();
}
