/**
 * @file
 * @brief uGC - formally specified core (allocation + handle store)
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "ugc_core.h"

int   ugc_handle_count = 0;
int   ugc_handle_free_count = 0;
void *ugc_handles[UGC_HANDLE_COUNT] = { 0 };
int   ugc_handle_types[UGC_HANDLE_COUNT] = { 0 };
void *ugc_handle_extra[UGC_HANDLE_COUNT] = { 0 };
void *ugc_handle_dependent[UGC_HANDLE_COUNT] = { 0 };
int   ugc_handle_free_stack[UGC_HANDLE_COUNT] = { 0 };

/* Direct path: allocate object + header straight from the zeroing
 * allocator, bypassing any allocation context. */
/*@
  requires 1 <= header_size;
  requires size <= SIZE_MAX - header_size;
  assigns \nothing;
  ensures \result == \null ||
          (\valid(\result + (0 .. size - 1)) &&
           (\forall integer i; 0 <= i < size ==> \result[i] == 0));
*/
static uint8_t *
ugc_alloc_direct(size_t size, size_t header_size)
{
    uint8_t *address = ugc_zalloc(size + header_size);
    if (address == (uint8_t *)0)
        return (uint8_t *)0; /* OOM: don't offset null to a bogus object */

    /*@ assert step_valid_block:
          \valid(address + (0 .. size + header_size - 1)); */
    /*@ assert step_valid_object:
          \valid((address + header_size) + (0 .. size - 1)); */
    /*@ assert step_zeroed_elem:
          \forall integer i; 0 <= i < size ==>
              (address + header_size)[i] == 0; */
    return address + header_size;
}

/* Refill path: hand the context a fresh zeroed quantum and carve the
 * requested object out of its beginning (after the plug-skew header). */
/*@
  requires 1 <= header_size <= UGC_ALLOC_QUANTUM / 2;
  requires size + header_size <= UGC_ALLOC_QUANTUM;
  requires \valid(alloc_ptr_p) && \valid(alloc_limit_p) &&
           \separated(alloc_ptr_p, alloc_limit_p);
  assigns *alloc_ptr_p, *alloc_limit_p;
  ensures \result == \null ==>
          *alloc_ptr_p == \old(*alloc_ptr_p) &&
          *alloc_limit_p == \old(*alloc_limit_p);
  ensures \result != \null ==>
          *alloc_ptr_p == \result + size &&
          *alloc_limit_p == \result + (UGC_ALLOC_QUANTUM - header_size) &&
          \valid(\result + (0 .. UGC_ALLOC_QUANTUM - header_size - 1)) &&
          (\forall integer i;
              0 <= i < UGC_ALLOC_QUANTUM - header_size ==>
                  \result[i] == 0);
*/
static uint8_t *
ugc_ctx_refill(uint8_t **alloc_ptr_p, uint8_t **alloc_limit_p,
               size_t size, size_t header_size)
{
    uint8_t *base = ugc_zalloc(UGC_ALLOC_QUANTUM);
    if (base == (uint8_t *)0)
        return (uint8_t *)0;

    /*@ assert step_valid_block:
          \valid(base + (0 .. UGC_ALLOC_QUANTUM - 1)); */
    /*@ assert step_valid_object:
          \valid((base + header_size) +
                 (0 .. UGC_ALLOC_QUANTUM - header_size - 1)); */
    /*@ assert step_zeroed_elem:
          \forall integer i; 0 <= i < UGC_ALLOC_QUANTUM - header_size ==>
              (base + header_size)[i] == 0; */
    /*@ assert step_ptr_shift:
          base + (header_size + size) == (base + header_size) + size; */
    /*@ assert step_limit_shift:
          base + UGC_ALLOC_QUANTUM ==
              (base + header_size) + (UGC_ALLOC_QUANTUM - header_size); */
    *alloc_ptr_p   = base + header_size + size;
    *alloc_limit_p = base + UGC_ALLOC_QUANTUM;
    return base + header_size;
}

uint8_t *
ugc_core_alloc(uint8_t **alloc_ptr_p, uint8_t **alloc_limit_p,
               size_t size, bool bypass_context, size_t header_size)
{
    /* size is size_t; truncating it into a narrower type could wrap for a
     * large object and under-allocate. Keep the full width and reject an
     * addition that would overflow size_t. */
    if (size > SIZE_MAX - header_size)
        return (uint8_t *)0; /* overflow: signal OOM, don't under-allocate */

    size_t sizeWithHeader = size + header_size;

    /* Large or old-heap-flagged objects bypass the allocation context, like
     * the real GC does: the context quantum is deliberately smaller than the
     * LOH threshold, and GC_ALLOC_USER_OLD_HEAP objects must not land in the
     * ephemeral context region. */
    if (alloc_ptr_p == (uint8_t **)0 || bypass_context ||
        sizeWithHeader > UGC_ALLOC_QUANTUM)
    {
        return ugc_alloc_direct(size, header_size);
    }

    /* Serve from the thread's context when the request still fits. (We only
     * get here if the runtime's inline fast path failed, e.g. right after the
     * context was created or exhausted, or from paths that skip it.) */
    uint8_t *ptr = *alloc_ptr_p;
    if (ptr != (uint8_t *)0 && (size_t)(*alloc_limit_p - ptr) >= size)
    {
        *alloc_ptr_p = ptr + size;
        return ptr;
    }

    return ugc_ctx_refill(alloc_ptr_p, alloc_limit_p, size, header_size);
}

bool
ugc_handle_store_contains(const void *hndl)
{
    uintptr_t handle = (uintptr_t)hndl;
    uintptr_t handleStart = (uintptr_t)&ugc_handles[0];
    uintptr_t handleEnd = handleStart + sizeof(ugc_handles);

    return handle >= handleStart && handle < handleEnd;
}

/* Take the index of a slot for a new handle: pop the most recently
 * destroyed slot if one exists (LIFO keeps the touched-memory footprint
 * small - a property that matters in a zkVM, where every newly touched
 * page has a proving cost), otherwise grow the array. */
/*@
  requires ugc_store_consistent;
  assigns ugc_handle_count, ugc_handle_free_count;
  ensures ugc_store_consistent;

  behavior full:
    assumes ugc_handle_free_count == 0 &&
            ugc_handle_count == UGC_HANDLE_COUNT;
    assigns \nothing;
    ensures \result == -1;
    ensures ugc_handle_count == \old(ugc_handle_count);
    ensures ugc_handle_free_count == \old(ugc_handle_free_count);

  behavior recycled:
    assumes ugc_handle_free_count > 0;
    ensures ugc_handle_free_count == \old(ugc_handle_free_count) - 1;
    ensures ugc_handle_count == \old(ugc_handle_count);
    ensures \result == ugc_handle_free_stack[ugc_handle_free_count];
    ensures 0 <= \result < ugc_handle_count;

  behavior fresh:
    assumes ugc_handle_free_count == 0 &&
            ugc_handle_count < UGC_HANDLE_COUNT;
    ensures \result == \old(ugc_handle_count);
    ensures ugc_handle_count == \old(ugc_handle_count) + 1;
    ensures ugc_handle_free_count == 0;
    ensures 0 <= \result < ugc_handle_count;

  complete behaviors;
  disjoint behaviors;
*/
static int
ugc_handle_slot_acquire(void)
{
    if (ugc_handle_free_count > 0)
        return ugc_handle_free_stack[--ugc_handle_free_count];
    if (ugc_handle_count < UGC_HANDLE_COUNT)
        return ugc_handle_count++;
    return -1;
}

void **
ugc_handle_create(void *object, int type)
{
    int idx = ugc_handle_slot_acquire();
    if (idx < 0)
        return (void **)0;

    ugc_handles[idx] = object;
    ugc_handle_types[idx] = type;
    ugc_handle_extra[idx] = (void *)0;      /* scrub recycled slot state */
    ugc_handle_dependent[idx] = (void *)0;
    return &ugc_handles[idx];
}

void **
ugc_handle_create_with_extra(void *object, int type, void *extra)
{
    int idx = ugc_handle_slot_acquire();
    if (idx < 0)
        return (void **)0;

    ugc_handles[idx] = object;
    ugc_handle_types[idx] = type;
    ugc_handle_extra[idx] = extra;
    ugc_handle_dependent[idx] = (void *)0;
    return &ugc_handles[idx];
}

void **
ugc_handle_create_dependent(void *primary, void *secondary, int type)
{
    int idx = ugc_handle_slot_acquire();
    if (idx < 0)
        return (void **)0;

    ugc_handles[idx] = primary;
    ugc_handle_types[idx] = type;
    ugc_handle_extra[idx] = (void *)0;
    ugc_handle_dependent[idx] = secondary;
    return &ugc_handles[idx];
}

void
ugc_handle_destroy_at(size_t idx)
{
    if (ugc_handle_types[idx] < 0 ||
        ugc_handle_free_count == UGC_HANDLE_COUNT)
        return; /* double destroy: never push the same index twice */

    ugc_handles[idx] = (void *)0;
    ugc_handle_types[idx] = UGC_HANDLE_TYPE_FREE;
    ugc_handle_extra[idx] = (void *)0;
    ugc_handle_dependent[idx] = (void *)0;
    ugc_handle_free_stack[ugc_handle_free_count++] = (int)idx;
}

size_t
ugc_handle_index(void **hndl)
{
    return (size_t)((uintptr_t)hndl - (uintptr_t)&ugc_handles[0]) /
        sizeof(ugc_handles[0]);
}

void **
ugc_handle_dependent_slot_at(size_t idx)
{
    return &ugc_handle_dependent[idx];
}

void
ugc_handle_set_dependent_at(size_t idx, void *secondary)
{
    ugc_handle_dependent[idx] = secondary;
}

int
ugc_handle_get_type_at(size_t idx)
{
    return ugc_handle_types[idx];
}

void
ugc_handle_set_type_at(size_t idx, int type)
{
    ugc_handle_types[idx] = type;
}

void *
ugc_handle_get_extra_at(size_t idx)
{
    return ugc_handle_extra[idx];
}

void
ugc_handle_set_extra_at(size_t idx, void *extra)
{
    ugc_handle_extra[idx] = extra;
}

void
ugc_handle_store_reset(void)
{
    /*@
      loop invariant 0 <= i <= UGC_HANDLE_COUNT;
      loop invariant \forall integer k; 0 <= k < i ==>
          ugc_handles[k] == \null &&
          ugc_handle_types[k] == 0 &&
          ugc_handle_extra[k] == \null &&
          ugc_handle_dependent[k] == \null &&
          ugc_handle_free_stack[k] == 0;
      loop assigns i,
          ugc_handles[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_types[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_extra[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_dependent[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_free_stack[0 .. UGC_HANDLE_COUNT - 1];
      loop variant UGC_HANDLE_COUNT - i;
    */
    for (int i = 0; i < UGC_HANDLE_COUNT; i++)
    {
        ugc_handles[i] = (void *)0;
        ugc_handle_types[i] = 0;
        ugc_handle_extra[i] = (void *)0;
        ugc_handle_dependent[i] = (void *)0;
        ugc_handle_free_stack[i] = 0;
    }
    ugc_handle_count = 0;
    ugc_handle_free_count = 0;
}

void
ugc_handle_slot_store(void **slot, void *object)
{
    *slot = object;
}

bool
ugc_handle_slot_store_if_null(void **slot, void *object)
{
    if (*slot == (void *)0)
    {
        *slot = object;
        return true;
    }
    return false;
}

void *
ugc_handle_slot_cas(void **slot, void *object, void *comparand)
{
    if (*slot == comparand)
    {
        *slot = object;
    }
    return *slot;
}
