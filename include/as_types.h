/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_types.h - Basic type definitions
 */

#ifndef __AS_TYPES_H__
#define __AS_TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64)
    #define AS_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define AS_PLATFORM_MACOS   1
#elif defined(__linux__)
    #define AS_PLATFORM_LINUX   1
#else
    #error "Unsupported platform"
#endif

/* Export/Import macros */
#if defined(AS_PLATFORM_WINDOWS)
    #ifdef AS_BUILD_SHARED
        #define AS_API __declspec(dllexport)
    #elif defined(AS_USE_SHARED)
        #define AS_API __declspec(dllimport)
    #else
        #define AS_API
    #endif
#else
    #if defined(AS_BUILD_SHARED) && defined(__GNUC__)
        #define AS_API __attribute__((visibility("default")))
    #else
        #define AS_API
    #endif
#endif

/* Inline hint */
#if defined(_MSC_VER)
    #define AS_INLINE __inline
#else
    #define AS_INLINE static inline
#endif

/* Include error codes */
#include "as_error.h"

#ifdef __cplusplus
}
#endif

#endif /* __AS_TYPES_H__ */
