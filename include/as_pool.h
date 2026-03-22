/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_pool.h - Memory pool and buffer management
 *
 * Two-level abstraction:
 *   - Pool: manages fixed-size buffer blocks, maintains used/free lists
 *   - Buffer: memory block with reference counting
 *
 * Usage:
 *   as_pool_t *pool;
 *   as_pool_create(1024, 10, &pool);  // 10 blocks of 1024 bytes
 *
 *   as_buf_t *buf = as_pool_alloc(pool);  // get from free list, move to used
 *   void *data = as_buf_data(buf);
 *
 *   as_buf_ref(buf);    // increase ref count
 *   as_buf_unref(buf);  // decrease, when 0 returns to free list
 *
 *   as_pool_destroy(pool);
 */

#ifndef __AS_POOL_H__
#define __AS_POOL_H__

#include "as_types.h"
#include "as_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct as_pool as_pool_t;
typedef struct as_buf  as_buf_t;

/* Buffer structure (public for embedding, but treat as opaque) */
struct as_buf {
    as_list_node_t  node;       /* List linkage (in used or free list) */
    as_pool_t      *pool;       /* Owner pool */
    volatile int    ref;        /* Reference count */
    size_t          size;       /* Data size */
    uint8_t         data[];     /* Flexible array member for user data */
};

/*
 * Create a memory pool
 *
 * @param block_size   Size of each buffer's data area
 * @param block_count  Number of buffers to pre-allocate
 * @param pool_out     Output pool handle
 * @return AS_OK on success
 */
AS_API as_error_t as_pool_create(size_t block_size,
                                  size_t block_count,
                                  as_pool_t **pool_out);

/*
 * Destroy a memory pool
 * Warning: all buffers should be returned to free list before destruction
 *
 * @param pool  Pool handle
 */
AS_API void as_pool_destroy(as_pool_t *pool);

/*
 * Allocate a buffer from pool
 * Moves buffer from free list to used list
 * Initial reference count is 1
 *
 * @param pool  Pool handle
 * @return Buffer pointer, or NULL if pool exhausted
 */
AS_API as_buf_t *as_pool_alloc(as_pool_t *pool);

/*
 * Get pool statistics
 *
 * @param pool        Pool handle
 * @param total_out   Total buffers in pool (optional)
 * @param used_out    Buffers in used list (optional)
 * @param free_out    Buffers in free list (optional)
 */
AS_API void as_pool_stats(as_pool_t *pool,
                           size_t *total_out,
                           size_t *used_out,
                           size_t *free_out);

/*
 * Get buffer data pointer
 *
 * @param buf  Buffer handle
 * @return Pointer to data area
 */
AS_INLINE void *as_buf_data(as_buf_t *buf)
{
    return buf ? buf->data : NULL;
}

/*
 * Get buffer data size
 *
 * @param buf  Buffer handle
 * @return Data size in bytes
 */
AS_INLINE size_t as_buf_size(as_buf_t *buf)
{
    return buf ? buf->size : 0;
}

/*
 * Increase buffer reference count
 *
 * @param buf  Buffer handle
 */
AS_API void as_buf_ref(as_buf_t *buf);

/*
 * Decrease buffer reference count
 * When count reaches 0, buffer moves from used list to free list
 *
 * @param buf  Buffer handle
 */
AS_API void as_buf_unref(as_buf_t *buf);

/*
 * Get current reference count
 *
 * @param buf  Buffer handle
 * @return Reference count, or -1 if invalid
 */
AS_API int as_buf_get_ref(as_buf_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* __AS_POOL_H__ */
