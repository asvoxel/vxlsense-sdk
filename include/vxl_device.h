/**
 * @file vxl_device.h
 * @brief vxl SDK Device 接口定义
 *
 * @note 子对象管理说明:
 *       Device 不跟踪由其创建的 Sensor 和 Stream 对象。
 *       vxl_device_release() 不会自动释放已创建的 Sensor/Stream。
 *       调用者必须在关闭 Device 之前手动释放所有子对象，顺序如下:
 *         1. vxl_stream_stop() + vxl_stream_release() - 释放所有流
 *         2. vxl_sensor_release() - 释放所有传感器
 *         3. vxl_device_close() 或 vxl_device_release() - 关闭设备
 */

#ifndef __VXL_DEVICE_H__
#define __VXL_DEVICE_H__

#include "vxl_types.h"
#include "vxl_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Device 生命周期
 *============================================================================*/

/**
 * @brief 打开设备
 * @param device 设备对象
 * @return 错误码
 */
vxl_error_t vxl_device_open(vxl_device_t *device);

/**
 * @brief 关闭设备
 * @param device 设备对象
 * @return 错误码
 */
vxl_error_t vxl_device_close(vxl_device_t *device);

/**
 * @brief 检查设备是否已打开
 * @param device 设备对象
 * @param open 输出是否打开
 * @return 错误码
 */
vxl_error_t vxl_device_is_open(const vxl_device_t *device, bool *open);

/**
 * @brief 释放设备对象
 * @param device 设备对象
 * @return 错误码
 *
 * @note 如果设备已打开，会先关闭设备
 */
vxl_error_t vxl_device_release(vxl_device_t *device);

/*============================================================================
 * Device 信息
 *============================================================================*/

/**
 * @brief 获取设备信息
 * @param device 设备对象
 * @param info 输出设备信息
 * @return 错误码
 */
vxl_error_t vxl_device_get_info(const vxl_device_t *device,
                                vxl_device_info_t *info);

/*============================================================================
 * Sensor 枚举
 *============================================================================*/

/**
 * @brief 获取传感器数量
 * @param device 设备对象
 * @param count 输出传感器数量
 * @return 错误码
 *
 * @note 设备必须已打开
 */
vxl_error_t vxl_device_get_sensor_count(const vxl_device_t *device,
                                        size_t *count);

/**
 * @brief 获取指定类型的传感器
 * @param device 设备对象
 * @param type 传感器类型
 * @param sensor 输出传感器对象 (需调用 vxl_sensor_release 释放)
 * @return 错误码
 *
 * @note 设备必须已打开
 */
vxl_error_t vxl_device_get_sensor(vxl_device_t *device,
                                  vxl_sensor_type_t type,
                                  vxl_sensor_t **sensor);

/**
 * @brief 获取指定索引的传感器
 * @param device 设备对象
 * @param index 传感器索引
 * @param sensor 输出传感器对象 (需调用 vxl_sensor_release 释放)
 * @return 错误码
 *
 * @note 设备必须已打开
 */
vxl_error_t vxl_device_get_sensor_by_index(vxl_device_t *device,
                                           size_t index,
                                           vxl_sensor_t **sensor);

/*============================================================================
 * 固件操作
 *============================================================================*/

/**
 * @brief 下载固件
 * @param device 设备对象
 * @param data 固件数据
 * @param len 数据长度
 * @param run_addr 运行地址
 * @param base_addr 基地址
 * @return 错误码，不支持返回 VXL_ERROR_NOT_SUPPORTED
 */
vxl_error_t vxl_device_fw_download(vxl_device_t *device,
                                   const void *data, size_t len,
                                   uint32_t run_addr, uint32_t base_addr);

/**
 * @brief 获取固件版本
 * @param device 设备对象
 * @param version 输出版本字符串
 * @param len 缓冲区长度
 * @return 错误码
 */
vxl_error_t vxl_device_fw_version(const vxl_device_t *device,
                                  char *version, size_t len);

/*============================================================================
 * 标定数据操作
 *============================================================================*/

/**
 * @brief 下载标定数据
 * @param device 设备对象
 * @param data 标定数据
 * @param len 数据长度
 * @return 错误码
 */
vxl_error_t vxl_device_calib_download(vxl_device_t *device,
                                      const void *data, size_t len);

/**
 * @brief 读取标定参数
 * @param device 设备对象
 * @param type 标定类型
 * @param data 输出数据缓冲区
 * @param len 输入缓冲区大小，输出实际数据长度
 * @return 错误码
 */
vxl_error_t vxl_device_calib_read(vxl_device_t *device,
                                  vxl_calib_type_t type,
                                  void *data, size_t *len);

/**
 * @brief 写入标定参数
 * @param device 设备对象
 * @param type 标定类型
 * @param data 标定数据
 * @param len 数据长度
 * @return 错误码
 */
vxl_error_t vxl_device_calib_write(vxl_device_t *device,
                                   vxl_calib_type_t type,
                                   const void *data, size_t len);

/*============================================================================
 * 相机参数
 *============================================================================*/

/**
 * @brief 获取内参
 * @param device 设备对象
 * @param sensor 传感器类型
 * @param intrin 输出内参
 * @return 错误码
 */
vxl_error_t vxl_device_get_intrin(vxl_device_t *device,
                                  vxl_sensor_type_t sensor,
                                  vxl_intrinsics_t *intrin);

/**
 * @brief 获取外参
 * @param device 设备对象
 * @param from 源传感器
 * @param to 目标传感器
 * @param extrin 输出外参
 * @return 错误码
 */
vxl_error_t vxl_device_get_extrin(vxl_device_t *device,
                                  vxl_sensor_type_t from,
                                  vxl_sensor_type_t to,
                                  vxl_extrinsics_t *extrin);

/**
 * @brief 获取深度计算参数
 * @param device 设备对象
 * @param param 输出深度参数
 * @return 错误码
 */
vxl_error_t vxl_device_get_depth_param(vxl_device_t *device,
                                       vxl_depth_param_t *param);

/*============================================================================
 * 设备控制
 *============================================================================*/

/**
 * @brief 硬件重启
 * @param device 设备对象
 * @return 错误码
 */
vxl_error_t vxl_device_hw_reset(vxl_device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_DEVICE_H__ */
