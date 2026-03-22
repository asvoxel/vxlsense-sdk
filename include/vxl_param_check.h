/**
 * @file vxl_param_check.h
 * @brief VXL SDK统一参数验证宏
 *
 * 提供一致的参数验证和错误处理机制，用于所有C API函数
 */

#ifndef __VXL_PARAM_CHECK_H__
#define __VXL_PARAM_CHECK_H__

#include "vxl_types.h"
#include "as_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 基础参数验证宏
 *============================================================================*/

/**
 * @brief 检查参数非NULL
 * @param ptr 待检查的指针
 * @return 如果为NULL，返回VXL_ERROR_INVALID_PARAM
 */
#define CHECK_PARAM_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            AS_LOG_E("Parameter '" #ptr "' is NULL"); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/**
 * @brief 检查多个参数非NULL
 * 使用示例: CHECK_PARAMS_NOT_NULL(ctx, device, sensor)
 */
#define CHECK_PARAMS_NOT_NULL(...) \
    do { \
        void* _ptrs[] = { __VA_ARGS__ }; \
        const char* _names[] = { #__VA_ARGS__ }; \
        size_t _count = sizeof(_ptrs) / sizeof(_ptrs[0]); \
        for (size_t _i = 0; _i < _count; ++_i) { \
            if (_ptrs[_i] == NULL) { \
                AS_LOG_E("Parameter '%s' is NULL", _names[_i]); \
                return VXL_ERROR_INVALID_PARAM; \
            } \
        } \
    } while(0)

/**
 * @brief 检查句柄有效（非NULL）
 * @param handle 待检查的句柄
 * @return 如果无效，返回VXL_ERROR_INVALID_HANDLE
 */
#define CHECK_HANDLE_VALID(handle) \
    do { \
        if ((handle) == NULL) { \
            AS_LOG_E("Handle '" #handle "' is invalid (NULL)"); \
            return VXL_ERROR_INVALID_HANDLE; \
        } \
    } while(0)

/**
 * @brief 检查缓冲区大小
 * @param buf 缓冲区指针
 * @param size 缓冲区大小
 * @return 如果buf为NULL或size为0，返回VXL_ERROR_INVALID_PARAM
 */
#define CHECK_BUFFER_SIZE(buf, size) \
    do { \
        if ((buf) == NULL) { \
            AS_LOG_E("Buffer '" #buf "' is NULL"); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
        if ((size) == 0) { \
            AS_LOG_E("Buffer size '" #size "' is zero"); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/*============================================================================
 * 范围和值验证宏
 *============================================================================*/

/**
 * @brief 检查值在范围内
 * @param val 待检查的值
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 */
#define CHECK_VALUE_RANGE(val, min, max) \
    do { \
        if ((val) < (min) || (val) > (max)) { \
            AS_LOG_E("Value '" #val "' (%d) out of range [%d, %d]", \
                     (int)(val), (int)(min), (int)(max)); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/**
 * @brief 检查值大于0
 */
#define CHECK_VALUE_POSITIVE(val) \
    do { \
        if ((val) <= 0) { \
            AS_LOG_E("Value '" #val "' must be positive, got %d", (int)(val)); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/**
 * @brief 检查值非负
 */
#define CHECK_VALUE_NON_NEGATIVE(val) \
    do { \
        if ((val) < 0) { \
            AS_LOG_E("Value '" #val "' must be non-negative, got %d", (int)(val)); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/*============================================================================
 * 字符串验证宏
 *============================================================================*/

/**
 * @brief 检查字符串非空
 */
#define CHECK_STRING_NOT_EMPTY(str) \
    do { \
        if ((str) == NULL || (str)[0] == '\0') { \
            AS_LOG_E("String '" #str "' is NULL or empty"); \
            return VXL_ERROR_INVALID_PARAM; \
        } \
    } while(0)

/**
 * @brief 安全字符串拷贝（保证NULL结尾）
 * @param dst 目标缓冲区
 * @param src 源字符串
 * @param size 目标缓冲区大小
 */
#define SAFE_STRCPY(dst, src, size) \
    do { \
        if ((src) && (dst) && (size) > 0) { \
            strncpy((dst), (src), (size) - 1); \
            (dst)[(size) - 1] = '\0'; \
        } \
    } while(0)

/**
 * @brief 安全字符串拼接（保证NULL结尾）
 */
#define SAFE_STRCAT(dst, src, size) \
    do { \
        if ((src) && (dst) && (size) > 0) { \
            size_t _dst_len = strlen(dst); \
            if (_dst_len < (size) - 1) { \
                strncat((dst), (src), (size) - _dst_len - 1); \
                (dst)[(size) - 1] = '\0'; \
            } \
        } \
    } while(0)

/*============================================================================
 * 状态验证宏
 *============================================================================*/

/**
 * @brief 检查设备已打开
 * @param device 设备句柄
 * @param is_open_field 设备的is_open字段
 */
#define CHECK_DEVICE_OPEN(device, is_open_field) \
    do { \
        if (!(device)->is_open_field) { \
            AS_LOG_E("Device is not open"); \
            return VXL_ERROR_DEVICE_NOT_OPEN; \
        } \
    } while(0)

/**
 * @brief 检查设备未打开（用于open操作）
 */
#define CHECK_DEVICE_NOT_OPEN(device, is_open_field) \
    do { \
        if ((device)->is_open_field) { \
            AS_LOG_E("Device is already open"); \
            return VXL_ERROR_DEVICE_BUSY; \
        } \
    } while(0)

/**
 * @brief 检查流已启动
 */
#define CHECK_STREAM_STARTED(stream, is_running_field) \
    do { \
        if (!(stream)->is_running_field) { \
            AS_LOG_E("Stream is not started"); \
            return VXL_ERROR_STREAM_NOT_STARTED; \
        } \
    } while(0)

/**
 * @brief 检查流未启动（用于start操作）
 */
#define CHECK_STREAM_NOT_STARTED(stream, is_running_field) \
    do { \
        if ((stream)->is_running_field) { \
            AS_LOG_E("Stream is already started"); \
            return VXL_ERROR_STREAM_ALREADY_STARTED; \
        } \
    } while(0)

/*============================================================================
 * 资源生命周期验证
 *============================================================================*/

/**
 * @brief Magic number用于对象有效性验证
 */
#define VXL_MAGIC_NUMBER_CONTEXT    0x56584C43  /* 'VXLC' */
#define VXL_MAGIC_NUMBER_DEVICE     0x56584C44  /* 'VXLD' */
#define VXL_MAGIC_NUMBER_SENSOR     0x56584C53  /* 'VXLS' */
#define VXL_MAGIC_NUMBER_STREAM     0x56584C54  /* 'VXLT' */
#define VXL_MAGIC_NUMBER_FRAME      0x56584C46  /* 'VXLF' */
#define VXL_MAGIC_NUMBER_PROFILE    0x56584C50  /* 'VXLP' */

/**
 * @brief 检查对象magic number
 * 注意：需要对象结构体中有uint32_t magic字段
 */
#define CHECK_OBJECT_MAGIC(obj, expected_magic) \
    do { \
        if ((obj) == NULL) { \
            AS_LOG_E("Object '" #obj "' is NULL"); \
            return VXL_ERROR_INVALID_HANDLE; \
        } \
        if ((obj)->magic != (expected_magic)) { \
            AS_LOG_E("Object '" #obj "' has invalid magic number (0x%08X, expected 0x%08X)", \
                     (obj)->magic, (expected_magic)); \
            return VXL_ERROR_INVALID_HANDLE; \
        } \
    } while(0)

/**
 * @brief 初始化对象magic number
 */
#define INIT_OBJECT_MAGIC(obj, magic_value) \
    do { \
        if (obj) { \
            (obj)->magic = (magic_value); \
        } \
    } while(0)

/**
 * @brief 清除对象magic number（用于释放时）
 */
#define CLEAR_OBJECT_MAGIC(obj) \
    do { \
        if (obj) { \
            (obj)->magic = 0; \
        } \
    } while(0)

/*============================================================================
 * Backend验证宏
 *============================================================================*/

/**
 * @brief 检查Backend有效
 */
#define CHECK_BACKEND_VALID(backend) \
    do { \
        if ((backend) == NULL) { \
            AS_LOG_E("Backend is NULL"); \
            return VXL_ERROR_BACKEND; \
        } \
        if ((backend)->name == NULL) { \
            AS_LOG_E("Backend name is NULL"); \
            return VXL_ERROR_BACKEND; \
        } \
    } while(0)

/**
 * @brief 检查Backend函数指针存在
 */
#define CHECK_BACKEND_FUNCTION(backend, func) \
    do { \
        if ((backend)->func == NULL) { \
            AS_LOG_E("Backend '%s' does not implement '%s'", \
                     (backend)->name, #func); \
            return VXL_ERROR_NOT_SUPPORTED; \
        } \
    } while(0)

/*============================================================================
 * 错误处理辅助宏
 *============================================================================*/

/**
 * @brief 检查函数返回值
 */
#define CHECK_RETURN(expr) \
    do { \
        vxl_error_t _err = (expr); \
        if (_err != VXL_SUCCESS) { \
            AS_LOG_E("Expression '" #expr "' failed with error %d", _err); \
            return _err; \
        } \
    } while(0)

/**
 * @brief 检查函数返回值，失败时跳转到cleanup标签
 */
#define CHECK_RETURN_GOTO(expr, cleanup_label) \
    do { \
        vxl_error_t _err = (expr); \
        if (_err != VXL_SUCCESS) { \
            AS_LOG_E("Expression '" #expr "' failed with error %d", _err); \
            err = _err; \
            goto cleanup_label; \
        } \
    } while(0)

/*============================================================================
 * 调试辅助宏
 *============================================================================*/

#ifdef NDEBUG
    #define VXL_ASSERT(condition) ((void)0)
#else
    #define VXL_ASSERT(condition) \
        do { \
            if (!(condition)) { \
                AS_LOG_E("Assertion failed: " #condition " at %s:%d", \
                         __FILE__, __LINE__); \
                abort(); \
            } \
        } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VXL_PARAM_CHECK_H__ */
