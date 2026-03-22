/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_uvc.h - Cross-platform UVC abstraction layer
 *
 * Platform implementations:
 *   - Windows: DirectShow (asos/windows/as_uvc_ds.c)
 *   - macOS/Linux: libuvc wrapper (asos/posix/as_uvc_libuvc.c)
 *
 * This layer provides a unified API for UVC video capture across platforms.
 * The API is designed to be compatible with libuvc for easy porting.
 */

#ifndef __AS_UVC_H__
#define __AS_UVC_H__

#include "as_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Constants
 *============================================================================*/

#define AS_UVC_MAX_DEVICES          16
#define AS_UVC_MAX_STRING_LEN       256

/*============================================================================
 * Error codes (compatible with libuvc)
 *============================================================================*/

typedef enum as_uvc_error {
    AS_UVC_SUCCESS              = 0,
    AS_UVC_ERROR_IO             = -1,
    AS_UVC_ERROR_INVALID_PARAM  = -2,
    AS_UVC_ERROR_ACCESS         = -3,
    AS_UVC_ERROR_NO_DEVICE      = -4,
    AS_UVC_ERROR_NOT_FOUND      = -5,
    AS_UVC_ERROR_BUSY           = -6,
    AS_UVC_ERROR_TIMEOUT        = -7,
    AS_UVC_ERROR_OVERFLOW       = -8,
    AS_UVC_ERROR_PIPE           = -9,
    AS_UVC_ERROR_INTERRUPTED    = -10,
    AS_UVC_ERROR_NO_MEM         = -11,
    AS_UVC_ERROR_NOT_SUPPORTED  = -12,
    AS_UVC_ERROR_INVALID_DEVICE = -50,
    AS_UVC_ERROR_INVALID_MODE   = -51,
    AS_UVC_ERROR_CALLBACK_EXISTS= -52,
    AS_UVC_ERROR_OTHER          = -99
} as_uvc_error_t;

/*============================================================================
 * Frame formats (compatible with libuvc)
 *============================================================================*/

typedef enum as_uvc_frame_format {
    AS_UVC_FORMAT_UNKNOWN       = 0,
    AS_UVC_FORMAT_ANY           = 0,
    AS_UVC_FORMAT_UNCOMPRESSED  = 1,
    AS_UVC_FORMAT_COMPRESSED    = 2,
    AS_UVC_FORMAT_YUYV          = 3,
    AS_UVC_FORMAT_UYVY          = 4,
    AS_UVC_FORMAT_RGB           = 5,
    AS_UVC_FORMAT_BGR           = 6,
    AS_UVC_FORMAT_MJPEG         = 7,
    AS_UVC_FORMAT_H264          = 8,
    AS_UVC_FORMAT_GRAY8         = 10,
    AS_UVC_FORMAT_GRAY16        = 11,
    AS_UVC_FORMAT_BY8           = 12,
    AS_UVC_FORMAT_BA81          = 13,
    AS_UVC_FORMAT_SGRBG8        = 14,
    AS_UVC_FORMAT_SGBRG8        = 15,
    AS_UVC_FORMAT_SRGGB8        = 16,
    AS_UVC_FORMAT_SBGGR8        = 17,
    AS_UVC_FORMAT_NV12          = 18,
    AS_UVC_FORMAT_COUNT         = 19,
    AS_UVC_FORMAT_FRAME_FORMAT_UNKNOWN = 0,
} as_uvc_frame_format_t;

/*============================================================================
 * XU (Extension Unit) request codes (compatible with libuvc)
 *============================================================================*/

typedef enum as_uvc_req_code {
    AS_UVC_RC_UNDEFINED     = 0x00,
    AS_UVC_SET_CUR          = 0x01,
    AS_UVC_GET_CUR          = 0x81,
    AS_UVC_GET_MIN          = 0x82,
    AS_UVC_GET_MAX          = 0x83,
    AS_UVC_GET_RES          = 0x84,
    AS_UVC_GET_LEN          = 0x85,
    AS_UVC_GET_INFO         = 0x86,
    AS_UVC_GET_DEF          = 0x87
} as_uvc_req_code_t;

/*============================================================================
 * Opaque handle types
 *============================================================================*/

typedef struct as_uvc_context as_uvc_context_t;
typedef struct as_uvc_device as_uvc_device_t;
typedef struct as_uvc_device_handle as_uvc_device_handle_t;
typedef struct as_uvc_stream_handle as_uvc_stream_handle_t;

/*============================================================================
 * Data structures
 *============================================================================*/

/**
 * @brief Extension Unit descriptor
 *
 * Represents a UVC Extension Unit (XU) for vendor-specific controls.
 */
typedef struct as_uvc_extension_unit {
    uint8_t     bUnitID;                    /* Unit ID */
    uint8_t     guidExtensionCode[16];      /* Extension GUID */
    uint64_t    bmControls;                 /* Bitmap of available controls */
    struct as_uvc_extension_unit *next;     /* Linked list pointer */
} as_uvc_extension_unit_t;

/**
 * @brief Device descriptor
 */
typedef struct as_uvc_device_descriptor {
    uint16_t    idVendor;
    uint16_t    idProduct;
    uint16_t    bcdUVC;
    char        serialNumber[AS_UVC_MAX_STRING_LEN];
    char        manufacturer[AS_UVC_MAX_STRING_LEN];
    char        product[AS_UVC_MAX_STRING_LEN];
} as_uvc_device_descriptor_t;

/**
 * @brief Frame structure (compatible with uvc_frame_t)
 */
typedef struct as_uvc_frame {
    void       *data;               /* Frame data */
    size_t      data_bytes;         /* Data size */
    uint32_t    width;              /* Width */
    uint32_t    height;             /* Height */
    as_uvc_frame_format_t frame_format; /* Format */
    size_t      step;               /* Row stride */
    uint32_t    sequence;           /* Frame sequence number */
    struct {
        int64_t tv_sec;
        int64_t tv_usec;
    } capture_time;                 /* Capture time (start) */
    struct {
        int64_t tv_sec;
        int64_t tv_nsec;
    } capture_time_finished;        /* Capture time (finished) - for libuvc compat */
    void       *source;             /* Device handle reference */
    uint8_t     library_owns_data;  /* Data ownership flag */
    void       *metadata;           /* Metadata (optional) */
    size_t      metadata_bytes;     /* Metadata size */
} as_uvc_frame_t;

/**
 * @brief Stream control parameters (compatible with uvc_stream_ctrl_t)
 */
typedef struct as_uvc_stream_ctrl {
    uint16_t    bmHint;
    uint8_t     bFormatIndex;
    uint8_t     bFrameIndex;
    uint32_t    dwFrameInterval;
    uint16_t    wKeyFrameRate;
    uint16_t    wPFrameRate;
    uint16_t    wCompQuality;
    uint16_t    wCompWindowSize;
    uint16_t    wDelay;
    uint32_t    dwMaxVideoFrameSize;
    uint32_t    dwMaxPayloadTransferSize;
    uint32_t    dwClockFrequency;
    uint8_t     bmFramingInfo;
    uint8_t     bPreferredVersion;
    uint8_t     bMinVersion;
    uint8_t     bMaxVersion;
    uint8_t     bInterfaceNumber;
} as_uvc_stream_ctrl_t;

/**
 * @brief Format descriptor (stub for Windows compatibility)
 *
 * On non-Windows platforms, use libuvc's uvc_format_desc_t directly.
 * This struct is provided for type compatibility only.
 */
typedef struct as_uvc_format_desc {
    uint8_t     bFormatIndex;
    uint8_t     bNumFrameDescriptors;
    uint8_t     bmFlags;
    as_uvc_frame_format_t format;
    /* Linked list - not used on Windows */
    struct as_uvc_format_desc *prev, *next;
    struct as_uvc_frame_desc *frame_descs;
} as_uvc_format_desc_t;

/**
 * @brief Frame descriptor (stub for Windows compatibility)
 *
 * On non-Windows platforms, use libuvc's uvc_frame_desc_t directly.
 * This struct is provided for type compatibility only.
 */
typedef struct as_uvc_frame_desc {
    uint8_t     bFrameIndex;
    uint16_t    wWidth;
    uint16_t    wHeight;
    uint32_t    dwMinBitRate;
    uint32_t    dwMaxBitRate;
    uint32_t    dwDefaultFrameInterval;
    uint8_t     bFrameIntervalType;
    uint32_t    dwBytesPerLine;
    uint32_t   *intervals;
    /* Linked list - not used on Windows */
    struct as_uvc_frame_desc *prev, *next;
    struct as_uvc_format_desc *parent;
} as_uvc_frame_desc_t;

/*============================================================================
 * Callback function types
 *============================================================================*/

typedef void (*as_uvc_frame_callback_t)(as_uvc_frame_t *frame, void *user_ptr);

/*============================================================================
 * Context management
 *============================================================================*/

/**
 * @brief Initialize UVC context
 * @param ctx Output context pointer
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_init(as_uvc_context_t **ctx);

/**
 * @brief Deinitialize UVC context
 * @param ctx Context to destroy
 */
AS_API void as_uvc_exit(as_uvc_context_t *ctx);

/*============================================================================
 * Device enumeration
 *============================================================================*/

/**
 * @brief Get list of UVC devices
 * @param ctx UVC context
 * @param list Output device list (NULL-terminated)
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_get_device_list(
    as_uvc_context_t *ctx,
    as_uvc_device_t ***list);

/**
 * @brief Free device list
 * @param list Device list to free
 * @param unref_devices If non-zero, also unref each device
 */
AS_API void as_uvc_free_device_list(
    as_uvc_device_t **list,
    uint8_t unref_devices);

/**
 * @brief Get device descriptor
 * @param dev Device
 * @param desc Output descriptor pointer
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_get_device_descriptor(
    as_uvc_device_t *dev,
    as_uvc_device_descriptor_t **desc);

/**
 * @brief Free device descriptor
 * @param desc Descriptor to free
 */
AS_API void as_uvc_free_device_descriptor(as_uvc_device_descriptor_t *desc);

/**
 * @brief Increase device reference count
 * @param dev Device
 */
AS_API void as_uvc_ref_device(as_uvc_device_t *dev);

/**
 * @brief Decrease device reference count
 * @param dev Device
 */
AS_API void as_uvc_unref_device(as_uvc_device_t *dev);

/*============================================================================
 * Device open/close
 *============================================================================*/

/**
 * @brief Open UVC device
 * @param dev Device to open
 * @param devh Output device handle
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_open(
    as_uvc_device_t *dev,
    as_uvc_device_handle_t **devh);

/**
 * @brief Close UVC device
 * @param devh Device handle to close
 */
AS_API void as_uvc_close(as_uvc_device_handle_t *devh);

/*============================================================================
 * Stream control
 *============================================================================*/

/**
 * @brief Format capability structure for format enumeration
 */
typedef struct as_uvc_format_cap {
    as_uvc_frame_format_t format;   /* Frame format */
    uint16_t    width;              /* Width in pixels */
    uint16_t    height;             /* Height in pixels */
    uint8_t     fps;                /* Frame rate */
    uint32_t    frame_interval;     /* Frame interval in 100ns units */
} as_uvc_format_cap_t;

/**
 * @brief Enumerate supported formats
 * @param devh Device handle
 * @param caps Output array of format capabilities (caller allocates)
 * @param max_caps Maximum number of entries in caps array
 * @return Number of formats found, or negative error code
 *
 * Call with caps=NULL to get the number of supported formats.
 * Then allocate array and call again to retrieve the formats.
 */
AS_API int as_uvc_enum_formats(
    as_uvc_device_handle_t *devh,
    as_uvc_format_cap_t *caps,
    int max_caps);

/**
 * @brief Get stream control parameters for given format
 * @param devh Device handle
 * @param ctrl Output stream control
 * @param format Desired frame format
 * @param width Desired width
 * @param height Desired height
 * @param fps Desired FPS
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_get_stream_ctrl_format_size(
    as_uvc_device_handle_t *devh,
    as_uvc_stream_ctrl_t *ctrl,
    as_uvc_frame_format_t format,
    int width, int height, int fps);

/**
 * @brief Open stream with control parameters
 * @param devh Device handle
 * @param strmh Output stream handle
 * @param ctrl Stream control parameters
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_stream_open_ctrl(
    as_uvc_device_handle_t *devh,
    as_uvc_stream_handle_t **strmh,
    as_uvc_stream_ctrl_t *ctrl);

/**
 * @brief Start streaming with callback
 * @param strmh Stream handle
 * @param cb Frame callback function
 * @param user_ptr User data pointer
 * @param flags Streaming flags
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_stream_start(
    as_uvc_stream_handle_t *strmh,
    as_uvc_frame_callback_t cb,
    void *user_ptr,
    uint8_t flags);

/**
 * @brief Stop streaming
 * @param strmh Stream handle
 * @return AS_UVC_SUCCESS on success
 */
AS_API as_uvc_error_t as_uvc_stream_stop(as_uvc_stream_handle_t *strmh);

/**
 * @brief Close stream handle
 * @param strmh Stream handle
 */
AS_API void as_uvc_stream_close(as_uvc_stream_handle_t *strmh);

/*============================================================================
 * Frame management
 *============================================================================*/

/**
 * @brief Allocate frame buffer
 * @param data_bytes Size of data buffer
 * @return Allocated frame or NULL
 */
AS_API as_uvc_frame_t* as_uvc_allocate_frame(size_t data_bytes);

/**
 * @brief Free frame buffer
 * @param frame Frame to free
 */
AS_API void as_uvc_free_frame(as_uvc_frame_t *frame);

/*============================================================================
 * Extension Unit (XU) control
 *============================================================================*/

/**
 * @brief Get list of extension units
 * @param devh Device handle
 * @return Pointer to linked list of extension units, or NULL if none
 *
 * The returned list is owned by the device handle and should not be freed.
 */
AS_API const as_uvc_extension_unit_t* as_uvc_get_extension_units(
    as_uvc_device_handle_t *devh);

/**
 * @brief Get control data length
 * @param devh Device handle
 * @param unit Unit ID
 * @param ctrl Control selector
 * @return Control length in bytes, or negative error code
 */
AS_API int as_uvc_xu_get_len(
    as_uvc_device_handle_t *devh,
    uint8_t unit,
    uint8_t ctrl);

/**
 * @brief Get control value
 * @param devh Device handle
 * @param unit Unit ID
 * @param ctrl Control selector
 * @param data Output data buffer
 * @param len Data length
 * @param req_code Request code (AS_UVC_GET_CUR, AS_UVC_GET_MIN, etc.)
 * @return Number of bytes read, or negative error code
 */
AS_API int as_uvc_xu_get(
    as_uvc_device_handle_t *devh,
    uint8_t unit,
    uint8_t ctrl,
    void *data,
    int len,
    as_uvc_req_code_t req_code);

/**
 * @brief Set control value
 * @param devh Device handle
 * @param unit Unit ID
 * @param ctrl Control selector
 * @param data Input data buffer
 * @param len Data length
 * @return Number of bytes written, or negative error code
 */
AS_API int as_uvc_xu_set(
    as_uvc_device_handle_t *devh,
    uint8_t unit,
    uint8_t ctrl,
    void *data,
    int len);

/*============================================================================
 * Error handling
 *============================================================================*/

/**
 * @brief Get error string
 * @param err Error code
 * @return Error description string
 */
AS_API const char* as_uvc_strerror(as_uvc_error_t err);

#ifdef __cplusplus
}
#endif

#endif /* __AS_UVC_H__ */
