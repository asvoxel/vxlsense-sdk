/**
 * @file vxl_profile.h
 * @brief vxl SDK Profile 接口定义
 */

#ifndef __VXL_PROFILE_H__
#define __VXL_PROFILE_H__

#include "vxl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Profile 属性访问
 *============================================================================*/

/**
 * @brief 获取 Profile 所属的传感器类型
 * @param profile Profile 对象
 * @param type 输出传感器类型
 * @return 错误码
 */
vxl_error_t vxl_profile_get_sensor_type(const vxl_profile_t *profile,
                                        vxl_sensor_type_t *type);

/**
 * @brief 获取 Profile 的帧格式
 * @param profile Profile 对象
 * @param format 输出帧格式
 * @return 错误码
 */
vxl_error_t vxl_profile_get_format(const vxl_profile_t *profile,
                                   vxl_format_t *format);

/**
 * @brief 获取 Profile 的宽度
 * @param profile Profile 对象
 * @param width 输出宽度
 * @return 错误码
 */
vxl_error_t vxl_profile_get_width(const vxl_profile_t *profile,
                                  uint32_t *width);

/**
 * @brief 获取 Profile 的高度
 * @param profile Profile 对象
 * @param height 输出高度
 * @return 错误码
 */
vxl_error_t vxl_profile_get_height(const vxl_profile_t *profile,
                                   uint32_t *height);

/**
 * @brief 获取 Profile 的帧率
 * @param profile Profile 对象
 * @param fps 输出帧率
 * @return 错误码
 */
vxl_error_t vxl_profile_get_fps(const vxl_profile_t *profile, uint32_t *fps);

/*============================================================================
 * Profile 释放
 *============================================================================*/

/**
 * @brief 释放 Profile 对象
 * @param profile Profile 对象
 * @return 错误码
 */
vxl_error_t vxl_profile_release(vxl_profile_t *profile);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_PROFILE_H__ */
