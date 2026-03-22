/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_atomic.h - Atomic operations (minimal set)
 */

#ifndef __AS_ATOMIC_H__
#define __AS_ATOMIC_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Atomic integer type
 */
typedef volatile int as_atomic_t;

/*
 * Initialize atomic variable
 */
#define AS_ATOMIC_INIT(val) (val)

/*
 * Load value atomically
 */
AS_INLINE int as_atomic_load(as_atomic_t *v)
{
#if defined(_MSC_VER)
    return _InterlockedOr((volatile long *)v, 0);
#else
    return __sync_fetch_and_add(v, 0);
#endif
}

/*
 * Store value atomically
 */
AS_INLINE void as_atomic_store(as_atomic_t *v, int val)
{
#if defined(_MSC_VER)
    _InterlockedExchange((volatile long *)v, val);
#else
    __sync_lock_test_and_set(v, val);
    __sync_synchronize();
#endif
}

/*
 * Add and return new value
 */
AS_INLINE int as_atomic_add(as_atomic_t *v, int val)
{
#if defined(_MSC_VER)
    return _InterlockedExchangeAdd((volatile long *)v, val) + val;
#else
    return __sync_add_and_fetch(v, val);
#endif
}

/*
 * Subtract and return new value
 */
AS_INLINE int as_atomic_sub(as_atomic_t *v, int val)
{
#if defined(_MSC_VER)
    return _InterlockedExchangeAdd((volatile long *)v, -val) - val;
#else
    return __sync_sub_and_fetch(v, val);
#endif
}

/*
 * Increment and return new value
 */
AS_INLINE int as_atomic_inc(as_atomic_t *v)
{
    return as_atomic_add(v, 1);
}

/*
 * Decrement and return new value
 */
AS_INLINE int as_atomic_dec(as_atomic_t *v)
{
    return as_atomic_sub(v, 1);
}

/*
 * Compare and swap
 * If *v == old_val, set *v = new_val and return true
 * Otherwise return false
 */
AS_INLINE bool as_atomic_cas(as_atomic_t *v, int old_val, int new_val)
{
#if defined(_MSC_VER)
    return _InterlockedCompareExchange((volatile long *)v, new_val, old_val) == old_val;
#else
    return __sync_bool_compare_and_swap(v, old_val, new_val);
#endif
}

/*
 * Exchange and return old value
 */
AS_INLINE int as_atomic_exchange(as_atomic_t *v, int val)
{
#if defined(_MSC_VER)
    return _InterlockedExchange((volatile long *)v, val);
#else
    return __sync_lock_test_and_set(v, val);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __AS_ATOMIC_H__ */
