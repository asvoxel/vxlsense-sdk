/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_thread.h - Thread interface (pthread style)
 */

#ifndef __AS_THREAD_H__
#define __AS_THREAD_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque thread handle */
typedef struct as_thread as_thread_t;

/* Thread function type */
typedef void *(*as_thread_func_t)(void *arg);

/*
 * Create and start a thread
 *
 * @param thread_out  Output thread handle
 * @param func        Thread function
 * @param arg         Argument passed to thread function
 * @return AS_OK on success
 */
AS_API as_error_t as_thread_create(as_thread_t **thread_out,
                                    as_thread_func_t func,
                                    void *arg);

/*
 * Wait for thread to finish
 *
 * @param thread    Thread handle
 * @param retval    Output return value (can be NULL)
 * @return AS_OK on success
 */
AS_API as_error_t as_thread_join(as_thread_t *thread, void **retval);

/*
 * Detach thread (auto cleanup on exit)
 *
 * @param thread  Thread handle
 * @return AS_OK on success
 */
AS_API as_error_t as_thread_detach(as_thread_t *thread);

/*
 * Get current thread ID (for debugging)
 */
AS_API unsigned long as_thread_self(void);

/*
 * Sleep current thread
 *
 * @param ms  Milliseconds to sleep
 */
AS_API void as_thread_sleep_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __AS_THREAD_H__ */
