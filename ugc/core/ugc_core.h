/**
 * @file
 * @brief uGC - formally specified core (allocation + handle store)
 *
 * This module contains all the non-trivial logic of uGC in plain C with
 * ACSL contracts so it can be analyzed by Frama-C (WP for functional
 * correctness of the contracts, Eva for absence of undefined behavior).
 * The C++ classes (uGCHeap, uGCHandleStore, uGCHandleManager) are thin
 * wrappers over these functions.
 *
 * Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Allocation-context quantum. Refilling the context lets the runtime's own
 * fast paths (coreclr/runtime/<arch>/AllocFast.S: RhpNewFast & friends) bump
 * inline on alloc_ptr/combined_limit without calling into the GC again; the
 * runtime refreshes combined_limit itself after every GCHeap::Alloc
 * (GCHelpers.cpp: GcAllocInternal -> UpdateCombinedLimit). Must stay below
 * RH_LARGE_OBJECT_SIZE (85000): GcAllocInternal asserts in _DEBUG that
 * alloc_limit - alloc_ptr never exceeds it. */
#ifndef UGC_ALLOC_QUANTUM
#define UGC_ALLOC_QUANTUM (64 * 1024)
#endif

/** Capacity of the global handle store */
#define UGC_HANDLE_COUNT 65535

/*
 * ---------------------------------------------------------------------------
 * Abstract zeroing allocator
 * ---------------------------------------------------------------------------
 * The production implementation (ugc_zalloc.c) forwards to calloc. For WP
 * verification only this contract is used: the function either fails (NULL)
 * or returns a block of n valid, zero-initialized bytes.
 */
/*@
  assigns \nothing;
  ensures zalloc_result:
    \result == \null ||
    (\valid(\result + (0 .. n - 1)) &&
     (\forall integer i; 0 <= i < n ==> \result[i] == 0));
*/
uint8_t *ugc_zalloc(size_t n);

/*
 * ---------------------------------------------------------------------------
 * Allocation
 * ---------------------------------------------------------------------------
 * ugc_core_alloc implements IGCHeap::Alloc:
 *  - rejects a size whose header-extended value overflows size_t;
 *  - serves large objects, GC_ALLOC_USER_OLD_HEAP objects and calls without
 *    an allocation context directly from the zeroing allocator;
 *  - otherwise bump-allocates from the thread allocation context window
 *    [*alloc_ptr_p, *alloc_limit_p), refilling it with a fresh zeroed
 *    quantum when the request does not fit. The first object's header
 *    occupies the first header_size bytes of a fresh quantum (plug skew);
 *    every object's base size already pre-pays the header of the object
 *    that follows it, so a plain alloc_ptr bump keeps headers intact.
 *    The remainder of the previous quantum is abandoned - this GC never
 *    frees memory anyway.
 */

/*@
  requires header_size_range: 1 <= header_size <= UGC_ALLOC_QUANTUM / 2;
  requires context_ptrs:
    alloc_ptr_p == \null ||
    (\valid(alloc_ptr_p) && \valid(alloc_limit_p) &&
     \separated(alloc_ptr_p, alloc_limit_p));
  requires context_window:
    alloc_ptr_p == \null || *alloc_ptr_p == \null ||
    (\base_addr(*alloc_ptr_p) == \base_addr(*alloc_limit_p) &&
     *alloc_ptr_p <= *alloc_limit_p &&
     *alloc_limit_p - *alloc_ptr_p <= UGC_ALLOC_QUANTUM &&
     \valid(*alloc_ptr_p + (0 .. *alloc_limit_p - *alloc_ptr_p - 1)));

  assigns *alloc_ptr_p, *alloc_limit_p;

  behavior overflow:
    assumes size > SIZE_MAX - header_size;
    assigns \nothing;
    ensures \result == \null;

  behavior direct:
    assumes size <= SIZE_MAX - header_size;
    assumes alloc_ptr_p == \null || bypass_context ||
            size + header_size > UGC_ALLOC_QUANTUM;
    assigns \nothing;
    ensures \result == \null ||
            (\valid(\result + (0 .. size - 1)) &&
             (\forall integer i; 0 <= i < size ==> \result[i] == 0));

  behavior bump:
    assumes size <= SIZE_MAX - header_size;
    assumes alloc_ptr_p != \null && !bypass_context &&
            size + header_size <= UGC_ALLOC_QUANTUM;
    assumes *alloc_ptr_p != \null &&
            *alloc_limit_p - *alloc_ptr_p >= size;
    assigns *alloc_ptr_p;
    ensures \result == \old(*alloc_ptr_p);
    ensures *alloc_ptr_p == \old(*alloc_ptr_p) + size;
    ensures \valid(\result + (0 .. size - 1));

  behavior refill:
    assumes size <= SIZE_MAX - header_size;
    assumes alloc_ptr_p != \null && !bypass_context &&
            size + header_size <= UGC_ALLOC_QUANTUM;
    assumes !(*alloc_ptr_p != \null &&
              *alloc_limit_p - *alloc_ptr_p >= size);
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

  complete behaviors;
  disjoint behaviors;
*/
uint8_t *ugc_core_alloc(uint8_t **alloc_ptr_p, uint8_t **alloc_limit_p,
                        size_t size, bool bypass_context,
                        size_t header_size);

/*
 * ---------------------------------------------------------------------------
 * Handle store
 * ---------------------------------------------------------------------------
 * A handle is a pointer to a slot of ugc_handles. Parallel arrays keep the
 * handle type, the extra info and the dependent-handle secondary object.
 * Handles are never destroyed (this GC never frees anything), so the store
 * is a monotonically growing array.
 */
extern int   ugc_handle_count;
extern void *ugc_handles[UGC_HANDLE_COUNT];
extern int   ugc_handle_types[UGC_HANDLE_COUNT];
extern void *ugc_handle_extra[UGC_HANDLE_COUNT];
extern void *ugc_handle_dependent[UGC_HANDLE_COUNT];

/*@
  predicate ugc_valid_slot(void **h) =
    \exists integer i; 0 <= i < UGC_HANDLE_COUNT && h == &ugc_handles[i];

  predicate ugc_store_consistent =
    0 <= ugc_handle_count <= UGC_HANDLE_COUNT;
*/

/**
 * Check whether hndl points into the handle array.
 *
 * The membership test is performed on integer representations of the
 * pointers (as the CLR contract requires answering for arbitrary pointers),
 * which is outside what can be expressed portably in ACSL for pointers not
 * derived from the array, hence the intentionally weak contract.
 */
/*@
  assigns \nothing;
*/
bool ugc_handle_store_contains(const void *hndl);

/*@
  requires ugc_store_consistent;
  assigns ugc_handle_count,
          ugc_handles[ugc_handle_count],
          ugc_handle_types[ugc_handle_count];

  behavior full:
    assumes ugc_handle_count == UGC_HANDLE_COUNT;
    assigns \nothing;
    ensures \result == \null;

  behavior ok:
    assumes ugc_handle_count < UGC_HANDLE_COUNT;
    ensures \result == &ugc_handles[\old(ugc_handle_count)];
    ensures ugc_handles[\old(ugc_handle_count)] == object;
    ensures ugc_handle_types[\old(ugc_handle_count)] == type;
    ensures ugc_handle_count == \old(ugc_handle_count) + 1;

  complete behaviors;
  disjoint behaviors;
*/
void **ugc_handle_create(void *object, int type);

/*@
  requires ugc_store_consistent;
  assigns ugc_handle_count,
          ugc_handles[ugc_handle_count],
          ugc_handle_types[ugc_handle_count],
          ugc_handle_extra[ugc_handle_count];

  behavior full:
    assumes ugc_handle_count == UGC_HANDLE_COUNT;
    assigns \nothing;
    ensures \result == \null;

  behavior ok:
    assumes ugc_handle_count < UGC_HANDLE_COUNT;
    ensures \result == &ugc_handles[\old(ugc_handle_count)];
    ensures ugc_handles[\old(ugc_handle_count)] == object;
    ensures ugc_handle_types[\old(ugc_handle_count)] == type;
    ensures ugc_handle_extra[\old(ugc_handle_count)] == extra;
    ensures ugc_handle_count == \old(ugc_handle_count) + 1;

  complete behaviors;
  disjoint behaviors;
*/
void **ugc_handle_create_with_extra(void *object, int type, void *extra);

/*@
  requires ugc_store_consistent;
  assigns ugc_handle_count,
          ugc_handles[ugc_handle_count],
          ugc_handle_dependent[ugc_handle_count];

  behavior full:
    assumes ugc_handle_count == UGC_HANDLE_COUNT;
    assigns \nothing;
    ensures \result == \null;

  behavior ok:
    assumes ugc_handle_count < UGC_HANDLE_COUNT;
    ensures \result == &ugc_handles[\old(ugc_handle_count)];
    ensures ugc_handles[\old(ugc_handle_count)] == primary;
    ensures ugc_handle_dependent[\old(ugc_handle_count)] == secondary;
    ensures ugc_handle_count == \old(ugc_handle_count) + 1;

  complete behaviors;
  disjoint behaviors;
*/
void **ugc_handle_create_dependent(void *primary, void *secondary);

/**
 * Convert a handle (a pointer to a slot of ugc_handles) into its index.
 *
 * The conversion is a pointer subtraction against the array base. Like
 * ugc_handle_store_contains, relating an externally supplied pointer to
 * the array base is outside what the WP memory model can reason about,
 * hence the intentionally weak contract; the pointer/index round trip is
 * covered by unit tests instead. For a hndl satisfying ugc_valid_slot the
 * result is the index i such that hndl == &ugc_handles[i].
 */
/*@
  requires valid_handle: ugc_valid_slot(hndl);
  assigns \nothing;
*/
size_t ugc_handle_index(void **hndl);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns \nothing;
  ensures \result == &ugc_handle_dependent[idx];
*/
void **ugc_handle_dependent_slot_at(size_t idx);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns ugc_handle_dependent[idx];
  ensures ugc_handle_dependent[idx] == secondary;
*/
void ugc_handle_set_dependent_at(size_t idx, void *secondary);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns \nothing;
  ensures \result == ugc_handle_types[idx];
*/
int ugc_handle_get_type_at(size_t idx);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns ugc_handle_types[idx];
  ensures ugc_handle_types[idx] == type;
*/
void ugc_handle_set_type_at(size_t idx, int type);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns \nothing;
  ensures \result == ugc_handle_extra[idx];
*/
void *ugc_handle_get_extra_at(size_t idx);

/*@
  requires valid_index: 0 <= idx < UGC_HANDLE_COUNT;
  assigns ugc_handle_extra[idx];
  ensures ugc_handle_extra[idx] == extra;
*/
void ugc_handle_set_extra_at(size_t idx, void *extra);

/*@
  assigns ugc_handle_count,
          ugc_handles[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_types[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_extra[0 .. UGC_HANDLE_COUNT - 1],
          ugc_handle_dependent[0 .. UGC_HANDLE_COUNT - 1];
  ensures ugc_handle_count == 0;
  ensures \forall integer i; 0 <= i < UGC_HANDLE_COUNT ==>
          ugc_handles[i] == \null &&
          ugc_handle_types[i] == 0 &&
          ugc_handle_extra[i] == \null &&
          ugc_handle_dependent[i] == \null;
*/
void ugc_handle_store_reset(void);

/*
 * ---------------------------------------------------------------------------
 * Handle slot operations (IGCHandleManager)
 * ---------------------------------------------------------------------------
 * These operate on the object slot a handle points to.
 */

/*@
  requires \valid(slot);
  assigns *slot;
  ensures *slot == object;
*/
void ugc_handle_slot_store(void **slot, void *object);

/*@
  requires \valid(slot);
  assigns *slot;

  behavior was_null:
    assumes *slot == \null;
    ensures *slot == object;
    ensures \result == \true;

  behavior was_set:
    assumes *slot != \null;
    ensures *slot == \old(*slot);
    ensures \result == \false;

  complete behaviors;
  disjoint behaviors;
*/
bool ugc_handle_slot_store_if_null(void **slot, void *object);

/**
 * Non-atomic compare-and-swap over a handle slot (uGC is single-threaded
 * with respect to handle mutation). Note: unlike the CLR interlocked
 * primitive, this returns the value of the slot after the operation - the
 * behavior is inherited from the original implementation.
 */
/*@
  requires \valid(slot);
  assigns *slot;

  behavior swapped:
    assumes *slot == comparand;
    ensures *slot == object;
    ensures \result == object;

  behavior kept:
    assumes *slot != comparand;
    ensures *slot == \old(*slot);
    ensures \result == \old(*slot);

  complete behaviors;
  disjoint behaviors;
*/
void *ugc_handle_slot_cas(void **slot, void *object, void *comparand);

#ifdef __cplusplus
}
#endif
