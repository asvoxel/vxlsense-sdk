/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_error.h - ASOS error codes
 */

#ifndef __AS_ERROR_H__
#define __AS_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Error type
 *============================================================================*/

typedef int as_error_t;

/*============================================================================
 * Error codes
 *============================================================================*/

#define AS_OK               0       /**< Success */
#define AS_ERR_PARAM        (-1)    /**< Invalid parameter */
#define AS_ERR_INVALID_ARG  (-1)    /**< Invalid argument (alias) */
#define AS_ERR_NOMEM        (-2)    /**< Out of memory */
#define AS_ERR_NO_MEMORY    (-2)    /**< Out of memory (alias) */
#define AS_ERR_TIMEOUT      (-3)    /**< Operation timeout */
#define AS_ERR_BUSY         (-4)    /**< Resource busy */
#define AS_ERR_PERM         (-5)    /**< Permission denied */
#define AS_ERR_NOTINIT      (-6)    /**< Not initialized */
#define AS_ERR_INVAL        (-7)    /**< Invalid operation */
#define AS_ERR_UNKNOWN      (-99)   /**< Unknown error */

/*============================================================================
 * Error check macros
 *============================================================================*/

/**
 * @brief Check if operation succeeded
 * @param err Error code
 * @return 1 if success, 0 if failed
 */
#define AS_SUCCEEDED(err)   ((err) >= 0)

/**
 * @brief Check if operation failed
 * @param err Error code
 * @return 1 if failed, 0 if success
 */
#define AS_FAILED(err)      ((err) < 0)

/**
 * @brief Return if pointer is NULL
 * @param ptr Pointer to check
 * @param err Error code to return
 */
#define AS_RETURN_IF_NULL(ptr, err) \
    do { \
        if ((ptr) == NULL) { \
            return (err); \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __AS_ERROR_H__ */
