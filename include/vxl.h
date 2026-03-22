/**
 * @file vxl.h
 * @brief vxl SDK 统一头文件
 *
 * vxl SDK 是一个用于访问 UVC 相机的 C 语言库。
 *
 * 层次结构：
 * @code
 *   Context ──► Device ──► Sensor ──► Profile
 *                             │
 *                             ├──► Option
 *                             │
 *                             └──► Stream ──► Frame
 * @endcode
 *
 * 基本使用流程：
 * @code
 *   // 1. 创建上下文
 *   vxl_context_t *ctx;
 *   vxl_context_create(&ctx);
 *
 *   // 2. 获取设备
 *   vxl_device_t *device;
 *   vxl_context_get_device(ctx, 0, &device);
 *   vxl_device_open(device);
 *
 *   // 3. 获取传感器
 *   vxl_sensor_t *sensor;
 *   vxl_device_get_sensor(device, VXL_SENSOR_COLOR, &sensor);
 *
 *   // 4. 查找并创建流
 *   vxl_profile_t *profile;
 *   vxl_sensor_find_profile(sensor, VXL_FORMAT_YUYV, 640, 480, 30, &profile);
 *
 *   vxl_stream_t *stream;
 *   vxl_sensor_create_stream(sensor, profile, &stream);
 *
 *   // 5. 开始采集
 *   vxl_stream_start(stream, frame_callback, user_data);
 *
 *   // ... 在回调中处理帧 ...
 *
 *   // 6. 清理
 *   vxl_stream_stop(stream);
 *   vxl_stream_release(stream);
 *   vxl_profile_release(profile);
 *   vxl_sensor_release(sensor);
 *   vxl_device_release(device);
 *   vxl_context_destroy(ctx);
 * @endcode
 */

#ifndef __VXL_H__
#define __VXL_H__

/* 基础类型定义 */
#include "vxl_types.h"

/* 各模块头文件 */
#include "vxl_frame.h"
#include "vxl_profile.h"
#include "vxl_stream.h"
#include "vxl_sensor.h"
#include "vxl_device.h"
#include "vxl_context.h"

#endif /* __VXL_H__ */
