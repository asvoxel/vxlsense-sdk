/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_log.h - Minimal logging interface
 *
 * Release vs Debug behavior:
 * - Release (NDEBUG defined): Only ERROR/WARN, no file/line info
 * - Debug: All levels, with file/line info
 */

#ifndef __AS_LOG_H__
#define __AS_LOG_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels */
typedef enum as_log_level {
    AS_LOG_LEVEL_NONE  = -1,  /* Disable all logging */
    AS_LOG_LEVEL_ERROR = 0,
    AS_LOG_LEVEL_WARN  = 1,
    AS_LOG_LEVEL_INFO  = 2,
    AS_LOG_LEVEL_DEBUG = 3,
    AS_LOG_LEVEL_TRACE = 4,
} as_log_level_t;

/* Backward compatibility aliases */
#define AS_LOG_ERROR  AS_LOG_LEVEL_ERROR
#define AS_LOG_WARN   AS_LOG_LEVEL_WARN
#define AS_LOG_INFO   AS_LOG_LEVEL_INFO
#define AS_LOG_DEBUG  AS_LOG_LEVEL_DEBUG

/* Log callback function type */
typedef void (*as_log_callback_t)(as_log_level_t level,
                                   const char *file,
                                   int line,
                                   const char *fmt,
                                   ...);

/*
 * Set global log level
 * Messages below this level will be ignored
 */
AS_API void as_log_set_level(as_log_level_t level);

/*
 * Get current log level
 */
AS_API as_log_level_t as_log_get_level(void);

/*
 * Set whether to show file/line in log output
 * Default: true in Debug, false in Release
 */
AS_API void as_log_set_show_location(bool show);

/*
 * Get current show location setting
 */
AS_API bool as_log_get_show_location(void);

/*
 * Set custom log callback
 * If callback is NULL, use default stderr output
 */
AS_API void as_log_set_callback(as_log_callback_t callback);

/*
 * Core log function (internal use)
 */
AS_API void as_log_output(as_log_level_t level,
                          const char *file,
                          int line,
                          const char *fmt,
                          ...);

/*
 * Log macros
 *
 * In Release mode (NDEBUG defined):
 * - Default level: WARN (only ERROR and WARN are logged)
 * - File/line info: disabled
 *
 * In Debug mode:
 * - Default level: INFO
 * - File/line info: enabled
 */

#ifdef NDEBUG
/* Release mode: no file/line info, compile-time filter for TRACE */

#define AS_LOG_E(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_ERROR, NULL, 0, fmt, ##__VA_ARGS__)

#define AS_LOG_W(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_WARN, NULL, 0, fmt, ##__VA_ARGS__)

#define AS_LOG_I(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_INFO, NULL, 0, fmt, ##__VA_ARGS__)

#define AS_LOG_D(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_DEBUG, NULL, 0, fmt, ##__VA_ARGS__)

/* TRACE is completely compiled out in Release */
#define AS_LOG_T(fmt, ...) ((void)0)

#else
/* Debug mode: include file/line info */

#define AS_LOG_E(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define AS_LOG_W(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define AS_LOG_I(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define AS_LOG_D(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define AS_LOG_T(fmt, ...) \
    as_log_output(AS_LOG_LEVEL_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* NDEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __AS_LOG_H__ */
