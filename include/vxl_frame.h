/**
 * @file vxl_frame.h
 * @brief vxl SDK Frame 接口定义
 */

#ifndef __VXL_FRAME_H__
#define __VXL_FRAME_H__

#include "vxl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Frame 内存池管理
 *============================================================================*/

/**
 * @brief 初始化帧内存池
 * @return 错误码
 *
 * @note 在使用 Frame 之前必须调用此函数
 *       通常由 vxl_context_create 自动调用
 */
vxl_error_t vxl_frame_pool_init(void);

/**
 * @brief 销毁帧内存池
 *
 * @note 通常由 vxl_context_destroy 自动调用
 */
void vxl_frame_pool_deinit(void);

/*============================================================================
 * Frame 生命周期管理
 *============================================================================*/

/**
 * @brief 增加帧引用计数
 * @param frame 帧对象
 * @return 错误码
 */
vxl_error_t vxl_frame_add_ref(vxl_frame_t *frame);

/**
 * @brief 释放帧引用
 * @param frame 帧对象
 * @return 错误码
 */
vxl_error_t vxl_frame_release(vxl_frame_t *frame);

/*============================================================================
 * Frame 属性访问
 *============================================================================*/

/**
 * @brief 获取帧数据指针
 * @param frame 帧对象
 * @param data 输出数据指针
 * @return 错误码
 */
vxl_error_t vxl_frame_get_data(const vxl_frame_t *frame, const void **data);

/**
 * @brief 获取帧数据大小
 * @param frame 帧对象
 * @param size 输出数据大小 (字节)
 * @return 错误码
 */
vxl_error_t vxl_frame_get_data_size(const vxl_frame_t *frame, size_t *size);

/**
 * @brief 获取帧宽度
 * @param frame 帧对象
 * @param width 输出宽度
 * @return 错误码
 */
vxl_error_t vxl_frame_get_width(const vxl_frame_t *frame, uint32_t *width);

/**
 * @brief 获取帧高度
 * @param frame 帧对象
 * @param height 输出高度
 * @return 错误码
 */
vxl_error_t vxl_frame_get_height(const vxl_frame_t *frame, uint32_t *height);

/**
 * @brief 获取帧格式
 * @param frame 帧对象
 * @param format 输出格式
 * @return 错误码
 */
vxl_error_t vxl_frame_get_format(const vxl_frame_t *frame, vxl_format_t *format);

/**
 * @brief 获取帧行跨度 (stride)
 * @param frame 帧对象
 * @param stride 输出行跨度 (字节)
 * @return 错误码
 */
vxl_error_t vxl_frame_get_stride(const vxl_frame_t *frame, uint32_t *stride);

/**
 * @brief 获取帧元数据
 * @param frame 帧对象
 * @param metadata 输出元数据
 * @return 错误码
 */
vxl_error_t vxl_frame_get_metadata(const vxl_frame_t *frame,
                                   vxl_frame_metadata_t *metadata);

/**
 * @brief 获取帧时间戳
 * @param frame 帧对象
 * @param timestamp_us 输出时间戳 (微秒)
 * @return 错误码
 */
vxl_error_t vxl_frame_get_timestamp(const vxl_frame_t *frame,
                                    uint64_t *timestamp_us);

/**
 * @brief 获取帧序号
 * @param frame 帧对象
 * @param sequence 输出序号
 * @return 错误码
 */
vxl_error_t vxl_frame_get_sequence(const vxl_frame_t *frame, uint32_t *sequence);

/**
 * @brief 获取设备帧计数 (用于帧同步)
 * @param frame 帧对象
 * @param frame_count 输出帧计数
 * @return 错误码
 *
 * @note 帧计数来自设备嵌入元数据，可用于匹配不同流的同步帧
 *       - Y16 流: 来自嵌入行 embedded_line.frame_count
 *       - RGB 流: 来自 MJPEG APP2 标签
 */
vxl_error_t vxl_frame_get_frame_count(const vxl_frame_t *frame, uint16_t *frame_count);

/**
 * @brief 获取帧来源传感器类型
 * @param frame 帧对象
 * @param sensor_type 输出传感器类型
 * @return 错误码
 */
vxl_error_t vxl_frame_get_sensor_type(const vxl_frame_t *frame,
                                       vxl_sensor_type_t *sensor_type);

/*============================================================================
 * 帧同步工具
 *============================================================================*/

/**
 * @brief 检查两帧是否同步 (frame_count 匹配)
 * @param frame_a 帧 A
 * @param frame_b 帧 B
 * @return true 同步 (frame_count 相同), false 不同步
 *
 * @note 用于匹配来自不同流的同步帧 (如 DEPTH 和 RGB)
 */
bool vxl_frames_synchronized(const vxl_frame_t *frame_a, const vxl_frame_t *frame_b);

/**
 * @brief 计算两帧的 frame_count 差值
 * @param frame_a 帧 A
 * @param frame_b 帧 B
 * @return frame_count 差值 (有符号), 用于判断帧先后
 *
 * @note 返回值:
 *   - 0: 同步帧
 *   - >0: frame_a 比 frame_b 新
 *   - <0: frame_a 比 frame_b 旧
 */
int32_t vxl_frame_count_diff(const vxl_frame_t *frame_a, const vxl_frame_t *frame_b);

/*============================================================================
 * Frame 格式转换
 *============================================================================*/

/**
 * @brief 将帧转换为指定格式
 * @param src 源帧
 * @param dst_format 目标格式
 * @param dst 输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 *
 * @note 支持的转换：
 *   - YUYV -> RGB, BGR, GRAY8
 *   - UYVY -> RGB, BGR, GRAY8
 *   - MJPEG -> RGB, BGR, GRAY8
 */
vxl_error_t vxl_frame_convert(const vxl_frame_t *src,
                              vxl_format_t dst_format,
                              vxl_frame_t **dst);

/**
 * @brief 复制帧
 * @param src 源帧
 * @param dst 输出复制后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_frame_copy(const vxl_frame_t *src, vxl_frame_t **dst);

/*============================================================================
 * Frame 工具函数
 *============================================================================*/

/**
 * @brief 分离双 IR 帧为左右两帧
 *
 * 某些立体相机 (如 VXL435) 将左右 IR 图像合并在一个帧中传输：
 *   - 帧大小: width * height * 2
 *   - 前半部分: 右 IR (offset = 0)
 *   - 后半部分: 左 IR (offset = width * height)
 *
 * @param src 源 IR 帧 (必须是 VXL_SENSOR_IR 类型且大小为 width*height*2)
 * @param left 输出左 IR 帧 (需调用 vxl_frame_release 释放)，可为 NULL
 * @param right 输出右 IR 帧 (需调用 vxl_frame_release 释放)，可为 NULL
 * @return 错误码
 *
 * @example
 *   vxl_frame_t *left = NULL, *right = NULL;
 *   if (vxl_frame_split_ir(ir_frame, &left, &right) == VXL_SUCCESS) {
 *       // 使用 left 和 right 帧...
 *       vxl_frame_release(left);
 *       vxl_frame_release(right);
 *   }
 */
vxl_error_t vxl_frame_split_ir(const vxl_frame_t *src,
                                vxl_frame_t **left,
                                vxl_frame_t **right);

/**
 * @brief 检查 IR 帧是否为双帧格式
 * @param frame IR 帧
 * @return true 为双帧格式，false 为单帧格式
 */
bool vxl_frame_is_dual_ir(const vxl_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_FRAME_H__ */
