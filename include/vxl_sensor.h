/**
 * @file vxl_sensor.h
 * @brief vxl SDK Sensor 接口定义
 *
 * @note 生命周期与所有权说明:
 *       Sensor 对象由 vxl_device_get_sensor() 创建，持有 Device 的引用但不拥有其所有权。
 *       调用者必须按以下顺序释放资源:
 *         1. 停止并释放所有 Stream (vxl_stream_stop + vxl_stream_release)
 *         2. 释放所有 Sensor (vxl_sensor_release)
 *         3. 关闭/释放 Device (vxl_device_close 或 vxl_device_release)
 *         4. 销毁 Context (vxl_context_destroy)
 *       若不按此顺序释放，可能导致未定义行为。
 */

#ifndef __VXL_SENSOR_H__
#define __VXL_SENSOR_H__

#include "vxl_types.h"
#include "vxl_stream.h"
#include "vxl_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Sensor 属性
 *============================================================================*/

/**
 * @brief 获取传感器类型
 * @param sensor 传感器对象
 * @param type 输出传感器类型
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_type(const vxl_sensor_t *sensor,
                                vxl_sensor_type_t *type);

/**
 * @brief 获取传感器名称
 * @param sensor 传感器对象
 * @param name 输出名称缓冲区
 * @param size 缓冲区大小
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_name(const vxl_sensor_t *sensor,
                                char *name, size_t size);

/*============================================================================
 * Profile 枚举
 *============================================================================*/

/**
 * @brief 获取支持的 Profile 数量
 * @param sensor 传感器对象
 * @param count 输出数量
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_profile_count(const vxl_sensor_t *sensor,
                                         size_t *count);

/**
 * @brief 获取指定索引的 Profile
 * @param sensor 传感器对象
 * @param index Profile 索引
 * @param profile 输出 Profile 对象 (需调用 vxl_profile_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_profile(const vxl_sensor_t *sensor,
                                   size_t index,
                                   vxl_profile_t **profile);

/**
 * @brief 查找匹配的 Profile
 * @param sensor 传感器对象
 * @param format 期望格式 (VXL_FORMAT_ANY 表示任意)
 * @param width 期望宽度 (0 表示任意)
 * @param height 期望高度 (0 表示任意)
 * @param fps 期望帧率 (0 表示任意)
 * @param profile 输出 Profile 对象 (需调用 vxl_profile_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_sensor_find_profile(const vxl_sensor_t *sensor,
                                    vxl_format_t format,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t fps,
                                    vxl_profile_t **profile);

/*============================================================================
 * Stream 创建
 *============================================================================*/

/**
 * @brief 创建流对象
 * @param sensor 传感器对象
 * @param profile Profile 对象 (传感器会复制 Profile)
 * @param stream 输出流对象 (需调用 vxl_stream_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_sensor_create_stream(vxl_sensor_t *sensor,
                                     const vxl_profile_t *profile,
                                     vxl_stream_t **stream);

/*============================================================================
 * Option 控制
 *============================================================================*/

/**
 * @brief 检查选项是否支持
 * @param sensor 传感器对象
 * @param option 选项
 * @param supported 输出是否支持
 * @return 错误码
 */
vxl_error_t vxl_sensor_is_option_supported(const vxl_sensor_t *sensor,
                                           vxl_option_t option,
                                           bool *supported);

/**
 * @brief 获取选项范围
 * @param sensor 传感器对象
 * @param option 选项
 * @param range 输出范围
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_option_range(const vxl_sensor_t *sensor,
                                        vxl_option_t option,
                                        vxl_option_range_t *range);

/**
 * @brief 获取选项值
 * @param sensor 传感器对象
 * @param option 选项
 * @param value 输出值
 * @return 错误码
 */
vxl_error_t vxl_sensor_get_option(const vxl_sensor_t *sensor,
                                  vxl_option_t option,
                                  float *value);

/**
 * @brief 设置选项值
 * @param sensor 传感器对象
 * @param option 选项
 * @param value 值
 * @return 错误码
 */
vxl_error_t vxl_sensor_set_option(vxl_sensor_t *sensor,
                                  vxl_option_t option,
                                  float value);

/*============================================================================
 * Sensor 释放
 *============================================================================*/

/**
 * @brief 释放传感器对象
 * @param sensor 传感器对象
 * @return 错误码
 */
vxl_error_t vxl_sensor_release(vxl_sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_SENSOR_H__ */
