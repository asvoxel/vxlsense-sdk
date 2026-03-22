/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_mutex.h - Mutex interface
 */

#ifndef __AS_MUTEX_H__
#define __AS_MUTEX_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque mutex handle */
typedef struct as_mutex as_mutex_t;

/*
 * Create a mutex
 *
 * @param mutex_out  Output mutex handle
 * @return AS_OK on success
 */
AS_API as_error_t as_mutex_create(as_mutex_t **mutex_out);

/*
 * Destroy a mutex
 *
 * @param mutex  Mutex handle
 */
AS_API void as_mutex_destroy(as_mutex_t *mutex);

/*
 * Lock mutex (blocking)
 *
 * @param mutex  Mutex handle
 * @return AS_OK on success
 */
AS_API as_error_t as_mutex_lock(as_mutex_t *mutex);

/*
 * Try to lock mutex (non-blocking)
 *
 * @param mutex  Mutex handle
 * @return AS_OK if locked, AS_ERR_BUSY if already locked
 */
AS_API as_error_t as_mutex_trylock(as_mutex_t *mutex);

/*
 * Unlock mutex
 *
 * @param mutex  Mutex handle
 * @return AS_OK on success
 */
AS_API as_error_t as_mutex_unlock(as_mutex_t *mutex);

/*============================================================================
 * Condition Variable
 *============================================================================*/

/* Opaque condition variable handle */
typedef struct as_cond as_cond_t;

/*
 * Create a condition variable
 *
 * @param cond_out  Output condition variable handle
 * @return AS_OK on success
 */
AS_API as_error_t as_cond_create(as_cond_t **cond_out);

/*
 * Destroy a condition variable
 *
 * @param cond  Condition variable handle
 */
AS_API void as_cond_destroy(as_cond_t *cond);

/*
 * Wait on condition variable (blocking)
 * The mutex must be locked before calling this function.
 * The mutex is atomically released while waiting and re-acquired before returning.
 *
 * @param cond   Condition variable handle
 * @param mutex  Mutex handle (must be locked)
 * @return AS_OK on success
 */
AS_API as_error_t as_cond_wait(as_cond_t *cond, as_mutex_t *mutex);

/*
 * Wait on condition variable with timeout
 * The mutex must be locked before calling this function.
 *
 * @param cond       Condition variable handle
 * @param mutex      Mutex handle (must be locked)
 * @param timeout_ms Timeout in milliseconds
 * @return AS_OK on success, AS_ERR_TIMEOUT on timeout
 */
AS_API as_error_t as_cond_timedwait(as_cond_t *cond,
                                     as_mutex_t *mutex,
                                     uint32_t timeout_ms);

/*
 * Signal one waiting thread
 *
 * @param cond  Condition variable handle
 * @return AS_OK on success
 */
AS_API as_error_t as_cond_signal(as_cond_t *cond);

/*
 * Signal all waiting threads
 *
 * @param cond  Condition variable handle
 * @return AS_OK on success
 */
AS_API as_error_t as_cond_broadcast(as_cond_t *cond);

#ifdef __cplusplus
}
#endif

#endif /* __AS_MUTEX_H__ */
