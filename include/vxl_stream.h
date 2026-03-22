/**
 * @file vxl_stream.h
 * @brief vxl SDK Stream 接口定义
 */

#ifndef __VXL_STREAM_H__
#define __VXL_STREAM_H__

#include "vxl_types.h"
#include "vxl_frame.h"
#include "vxl_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Stream 控制
 *============================================================================*/

/**
 * @brief 启动流 (异步回调模式)
 * @param stream 流对象
 * @param callback 帧回调函数
 * @param user_data 用户数据，传递给回调
 * @return 错误码
 */
vxl_error_t vxl_stream_start(vxl_stream_t *stream,
                             vxl_frame_cbfn callback,
                             void *user_data);

/**
 * @brief 停止流
 * @param stream 流对象
 * @return 错误码
 */
vxl_error_t vxl_stream_stop(vxl_stream_t *stream);

/**
 * @brief 检查流是否正在运行
 * @param stream 流对象
 * @param running 输出运行状态
 * @return 错误码
 */
vxl_error_t vxl_stream_is_running(const vxl_stream_t *stream, bool *running);

/*============================================================================
 * Stream 同步帧获取
 *============================================================================*/

/**
 * @brief 等待下一帧 (阻塞)
 * @param stream 流对象
 * @param timeout_ms 超时时间 (毫秒)，0 表示永久等待
 * @param frame 输出帧对象 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_stream_wait_for_frame(vxl_stream_t *stream,
                                      uint32_t timeout_ms,
                                      vxl_frame_t **frame);

/**
 * @brief 尝试获取帧 (非阻塞)
 * @param stream 流对象
 * @param frame 输出帧对象 (需调用 vxl_frame_release 释放)
 * @return 错误码，无帧可用时返回 VXL_ERROR_FRAME_NOT_AVAILABLE
 */
vxl_error_t vxl_stream_poll_for_frame(vxl_stream_t *stream,
                                      vxl_frame_t **frame);

/*============================================================================
 * Stream 属性
 *============================================================================*/

/**
 * @brief 获取流的当前 Profile
 * @param stream 流对象
 * @param profile 输出 Profile 对象
 * @return 错误码
 */
vxl_error_t vxl_stream_get_profile(const vxl_stream_t *stream,
                                   const vxl_profile_t **profile);

/**
 * @brief 获取已接收的帧数
 * @param stream 流对象
 * @param count 输出帧数
 * @return 错误码
 */
vxl_error_t vxl_stream_get_frame_count(const vxl_stream_t *stream,
                                       uint64_t *count);

/**
 * @brief 获取丢弃的帧数 (缓冲区满时丢弃)
 * @param stream 流对象
 * @param count 输出帧数
 * @return 错误码
 */
vxl_error_t vxl_stream_get_dropped_count(const vxl_stream_t *stream,
                                         uint64_t *count);

/*============================================================================
 * Stream 释放
 *============================================================================*/

/**
 * @brief 释放流对象
 * @param stream 流对象
 * @return 错误码
 *
 * @note 如果流正在运行，会先停止流
 */
vxl_error_t vxl_stream_release(vxl_stream_t *stream);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_STREAM_H__ */
