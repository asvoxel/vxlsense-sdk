/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_mem.h - Memory allocation interface
 */

#ifndef __AS_MEM_H__
#define __AS_MEM_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Allocate memory
 * Returns NULL on failure
 */
AS_API void *as_malloc(size_t size);

/*
 * Allocate zeroed memory
 * Returns NULL on failure
 */
AS_API void *as_calloc(size_t count, size_t size);

/*
 * Reallocate memory
 * Returns NULL on failure (original ptr unchanged)
 */
AS_API void *as_realloc(void *ptr, size_t size);

/*
 * Free memory
 * ptr can be NULL
 */
AS_API void as_free(void *ptr);

/*
 * Allocate aligned memory
 * alignment must be power of 2
 * Returns NULL on failure
 */
AS_API void *as_aligned_alloc(size_t alignment, size_t size);

/*
 * Free aligned memory
 */
AS_API void as_aligned_free(void *ptr);

/*
 * Duplicate string
 * Returns NULL on failure
 */
AS_API char *as_strdup(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* __AS_MEM_H__ */
