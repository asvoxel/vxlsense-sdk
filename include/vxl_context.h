/**
 * @file vxl_context.h
 * @brief vxl SDK Context 接口定义
 */

#ifndef __VXL_CONTEXT_H__
#define __VXL_CONTEXT_H__

#include "vxl_types.h"
#include "vxl_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Context 生命周期
 *============================================================================*/

/**
 * @brief 创建 vxl 上下文
 * @param ctx 输出上下文对象
 * @return 错误码
 *
 * @note 使用完毕后需调用 vxl_context_destroy 释放
 */
vxl_error_t vxl_context_create(vxl_context_t **ctx);

/**
 * @brief 销毁 vxl 上下文
 * @param ctx 上下文对象
 * @return 错误码
 */
vxl_error_t vxl_context_destroy(vxl_context_t *ctx);

/*============================================================================
 * 设备枚举
 *============================================================================*/

/**
 * @brief 获取设备数量
 * @param ctx 上下文对象
 * @param count 输出设备数量
 * @return 错误码
 */
vxl_error_t vxl_context_get_device_count(vxl_context_t *ctx, size_t *count);

/**
 * @brief 获取指定索引的设备
 * @param ctx 上下文对象
 * @param index 设备索引
 * @param device 输出设备对象 (需调用 vxl_device_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_context_get_device(vxl_context_t *ctx,
                                   size_t index,
                                   vxl_device_t **device);

/**
 * @brief 查找指定 VID/PID 的设备
 * @param ctx 上下文对象
 * @param vid Vendor ID (0 表示任意)
 * @param pid Product ID (0 表示任意)
 * @param device 输出设备对象 (需调用 vxl_device_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_context_find_device(vxl_context_t *ctx,
                                    uint16_t vid,
                                    uint16_t pid,
                                    vxl_device_t **device);

/**
 * @brief 查找指定序列号的设备
 * @param ctx 上下文对象
 * @param serial_number 序列号
 * @param device 输出设备对象 (需调用 vxl_device_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_context_find_device_by_serial(vxl_context_t *ctx,
                                              const char *serial_number,
                                              vxl_device_t **device);

/**
 * @brief 刷新设备列表
 * @param ctx 上下文对象
 * @return 错误码
 */
vxl_error_t vxl_context_refresh_devices(vxl_context_t *ctx);

/*============================================================================
 * 设备事件
 *============================================================================*/

/**
 * @brief 注册设备事件回调
 * @param ctx 上下文对象
 * @param callback 设备事件回调函数
 * @param user_data 用户数据，传递给回调
 * @return 错误码
 *
 * @note 设备插拔事件将通过回调通知
 */
vxl_error_t vxl_context_set_device_event_callback(
    vxl_context_t *ctx,
    vxl_device_event_cbfn callback,
    void *user_data);

/*============================================================================
 * 日志配置
 *============================================================================*/

/**
 * @brief 日志级别
 */
typedef enum vxl_log_level {
    VXL_LOG_LEVEL_NONE  = -1,  /**< 禁用所有日志 */
    VXL_LOG_LEVEL_ERROR = 0,   /**< 仅错误 */
    VXL_LOG_LEVEL_WARN  = 1,   /**< 错误和警告 */
    VXL_LOG_LEVEL_INFO  = 2,   /**< 错误、警告和信息 */
    VXL_LOG_LEVEL_DEBUG = 3,   /**< 所有日志包括调试 */
    VXL_LOG_LEVEL_TRACE = 4,   /**< 所有日志包括跟踪 */
} vxl_log_level_t;

/**
 * @brief 设置日志级别
 * @param level 日志级别
 *
 * @note 默认值:
 *       - Release 版本: VXL_LOG_LEVEL_WARN
 *       - Debug 版本: VXL_LOG_LEVEL_INFO
 *
 * 示例:
 * @code
 * // 只显示错误和警告
 * vxl_set_log_level(VXL_LOG_LEVEL_WARN);
 *
 * // 显示所有调试信息
 * vxl_set_log_level(VXL_LOG_LEVEL_DEBUG);
 *
 * // 禁用所有日志
 * vxl_set_log_level(VXL_LOG_LEVEL_NONE);
 * @endcode
 */
void vxl_set_log_level(vxl_log_level_t level);

/**
 * @brief 获取当前日志级别
 * @return 当前日志级别
 */
vxl_log_level_t vxl_get_log_level(void);

/*============================================================================
 * 版本信息
 *============================================================================*/

/**
 * @brief 获取 SDK 版本字符串
 * @return 版本字符串 (格式: "X.Y.Z")
 */
const char* vxl_get_version_string(void);

/**
 * @brief 获取 SDK 版本号
 * @param major 输出主版本号
 * @param minor 输出次版本号
 * @param patch 输出补丁版本号
 * @return 错误码
 */
vxl_error_t vxl_get_version(int *major, int *minor, int *patch);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_CONTEXT_H__ */
