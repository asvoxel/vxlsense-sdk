/**
 * @file vxl_dip.h
 * @brief vxl SDK 数字图像处理 (Digital Image Processing) 伴生库
 *
 * 提供独立的图像格式转换函数，输入输出均为 vxl_frame_t 类型。
 */

#ifndef __VXL_DIP_H__
#define __VXL_DIP_H__

#include "vxl_types.h"
#include "vxl_frame.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * RGB 分量排列常量
 *============================================================================*/

#define VXL_RGB_R_OFFSET    0       /* RGB 格式中 R 分量偏移 */
#define VXL_RGB_G_OFFSET    1       /* RGB 格式中 G 分量偏移 */
#define VXL_RGB_B_OFFSET    2       /* RGB 格式中 B 分量偏移 */
#define VXL_RGB_BPP         3       /* RGB 每像素字节数 */

#define VXL_BGR_B_OFFSET    0       /* BGR 格式中 B 分量偏移 */
#define VXL_BGR_G_OFFSET    1       /* BGR 格式中 G 分量偏移 */
#define VXL_BGR_R_OFFSET    2       /* BGR 格式中 R 分量偏移 */
#define VXL_BGR_BPP         3       /* BGR 每像素字节数 */

#define VXL_YUYV_BPP        2       /* YUYV 每像素字节数 (平均) */
#define VXL_UYVY_BPP        2       /* UYVY 每像素字节数 (平均) */

/*============================================================================
 * YUYV (YUV422) 转换函数
 *============================================================================*/

/**
 * @brief 将 YUYV 格式帧转换为 RGB 格式
 *
 * YUYV 格式：每4字节表示2个像素，排列为 [Y0 U0 Y1 V0]
 *
 * @param src   源帧 (YUYV 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_yuyv_to_rgb(const vxl_frame_t *src, vxl_frame_t **dst);

/**
 * @brief 将 YUYV 格式帧转换为 BGR 格式
 *
 * @param src   源帧 (YUYV 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_yuyv_to_bgr(const vxl_frame_t *src, vxl_frame_t **dst);

/**
 * @brief 将 YUYV 格式帧转换为灰度 (只取 Y 分量)
 *
 * @param src   源帧 (YUYV 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_yuyv_to_gray(const vxl_frame_t *src, vxl_frame_t **dst);

/*============================================================================
 * UYVY (YUV422) 转换函数
 *============================================================================*/

/**
 * @brief 将 UYVY 格式帧转换为 RGB 格式
 *
 * UYVY 格式：每4字节表示2个像素，排列为 [U0 Y0 V0 Y1]
 *
 * @param src   源帧 (UYVY 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_uyvy_to_rgb(const vxl_frame_t *src, vxl_frame_t **dst);

/**
 * @brief 将 UYVY 格式帧转换为 BGR 格式
 *
 * @param src   源帧 (UYVY 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_uyvy_to_bgr(const vxl_frame_t *src, vxl_frame_t **dst);

/*============================================================================
 * MJPEG 转换函数
 *============================================================================*/

/**
 * @brief 将 MJPEG 格式帧解码为 RGB 格式
 *
 * @param src   源帧 (MJPEG 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 *
 * @note 需要 libjpeg 或 libjpeg-turbo 支持
 */
vxl_error_t vxl_dip_mjpeg_to_rgb(const vxl_frame_t *src, vxl_frame_t **dst);

/**
 * @brief 将 MJPEG 格式帧解码为 BGR 格式
 *
 * @param src   源帧 (MJPEG 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_mjpeg_to_bgr(const vxl_frame_t *src, vxl_frame_t **dst);

/**
 * @brief 将 MJPEG 格式帧解码为灰度格式
 *
 * @param src   源帧 (MJPEG 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_mjpeg_to_gray(const vxl_frame_t *src, vxl_frame_t **dst);

/*============================================================================
 * NV12 转换函数
 *============================================================================*/

/**
 * @brief 将 NV12 格式帧转换为 RGB 格式
 *
 * NV12 格式：Y 平面 + 交错 UV 平面
 *
 * @param src   源帧 (NV12 格式)
 * @param dst   输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 */
vxl_error_t vxl_dip_nv12_to_rgb(const vxl_frame_t *src, vxl_frame_t **dst);

/*============================================================================
 * 通用转换函数
 *============================================================================*/

/**
 * @brief 通用格式转换函数
 *
 * 根据源帧格式自动选择转换方法
 *
 * @param src        源帧
 * @param dst_format 目标格式
 * @param dst        输出转换后的帧 (需调用 vxl_frame_release 释放)
 * @return 错误码
 *
 * @note 支持的转换：
 *   - YUYV -> RGB, BGR, GRAY8
 *   - UYVY -> RGB, BGR
 *   - MJPEG -> RGB, BGR, GRAY8
 *   - NV12 -> RGB
 */
vxl_error_t vxl_dip_convert(const vxl_frame_t *src,
                            vxl_format_t dst_format,
                            vxl_frame_t **dst);

/*============================================================================
 * 工具函数
 *============================================================================*/

/**
 * @brief 计算指定格式的图像数据大小
 *
 * @param format    图像格式
 * @param width     图像宽度
 * @param height    图像高度
 * @return 数据大小 (字节)，格式不支持返回 0
 */
size_t vxl_dip_calc_image_size(vxl_format_t format,
                               uint32_t width,
                               uint32_t height);

/**
 * @brief 计算指定格式的默认行跨度
 *
 * @param format    图像格式
 * @param width     图像宽度
 * @return 行跨度 (字节)，格式不支持返回 0
 */
uint32_t vxl_dip_calc_stride(vxl_format_t format, uint32_t width);

/*============================================================================
 * 深度处理函数
 *============================================================================*/

/**
 * @brief 将深度帧转换为伪彩色 RGB 图像
 *
 * 使用 Jet 色彩映射表，将深度值映射到彩色
 *
 * @param src           源帧 (Z16 格式)
 * @param dst           输出转换后的帧 (RGB 格式)
 * @param min_depth     最小深度值 (用于归一化)
 * @param max_depth     最大深度值 (用于归一化)
 * @return 错误码
 */
vxl_error_t vxl_dip_depth_colormap(const vxl_frame_t *src,
                                   vxl_frame_t **dst,
                                   uint16_t min_depth,
                                   uint16_t max_depth);

/*============================================================================
 * RGB-D 融合接口
 *============================================================================*/

/* 注意: vxl_intrinsics_t 和 vxl_extrinsics_t 已在 vxl_types.h 中定义 */

/**
 * @brief 3D 点结构
 */
typedef struct vxl_point3d {
    float x;    /**< X 坐标 (毫米) */
    float y;    /**< Y 坐标 (毫米) */
    float z;    /**< Z 坐标 (毫米/深度值) */
} vxl_point3d_t;

/**
 * @brief 彩色 3D 点结构
 */
typedef struct vxl_point3d_rgb {
    float x, y, z;          /**< 3D 坐标 (毫米) */
    uint8_t r, g, b;        /**< RGB 颜色 */
    uint8_t reserved;       /**< 保留对齐 */
} vxl_point3d_rgb_t;

/**
 * @brief 点云结构
 */
typedef struct vxl_pointcloud {
    vxl_point3d_rgb_t *points;  /**< 点数组 */
    size_t count;               /**< 有效点数 */
    size_t capacity;            /**< 数组容量 */
} vxl_pointcloud_t;

/**
 * @brief 创建点云
 *
 * @param capacity  初始容量
 * @return 点云对象，失败返回 NULL
 */
vxl_pointcloud_t* vxl_pointcloud_create(size_t capacity);

/**
 * @brief 释放点云
 *
 * @param pc 点云对象
 */
void vxl_pointcloud_destroy(vxl_pointcloud_t *pc);

/**
 * @brief 将 2D 像素坐标反投影到 3D 点
 *
 * @param u         像素 X 坐标
 * @param v         像素 Y 坐标
 * @param depth     深度值 (毫米)
 * @param intrin    相机内参
 * @param point     输出 3D 点
 * @return 错误码
 */
vxl_error_t vxl_dip_deproject_point(uint32_t u, uint32_t v, float depth,
                                    const vxl_intrinsics_t *intrin,
                                    vxl_point3d_t *point);

/**
 * @brief 将 3D 点投影到 2D 像素坐标
 *
 * @param point     3D 点
 * @param intrin    相机内参
 * @param u         输出像素 X 坐标
 * @param v         输出像素 Y 坐标
 * @return 错误码
 */
vxl_error_t vxl_dip_project_point(const vxl_point3d_t *point,
                                  const vxl_intrinsics_t *intrin,
                                  float *u, float *v);

/**
 * @brief 从深度帧生成点云
 *
 * @param depth     深度帧 (Z16 格式)
 * @param intrin    深度相机内参
 * @param scale     深度比例因子 (raw_value / scale = 毫米)
 * @param pc        输出点云
 * @return 错误码
 */
vxl_error_t vxl_dip_depth_to_pointcloud(const vxl_frame_t *depth,
                                        const vxl_intrinsics_t *intrin,
                                        float scale,
                                        vxl_pointcloud_t *pc);

/**
 * @brief 从深度帧和 RGB 帧生成彩色点云
 *
 * 将深度图反投影到 3D 空间，并从 RGB 图像获取颜色
 *
 * @param depth         深度帧 (Z16 格式)
 * @param rgb           RGB 帧 (RGB24 格式)
 * @param depth_intrin  深度相机内参
 * @param rgb_intrin    RGB 相机内参
 * @param depth_to_rgb  深度到 RGB 的外参变换 (可为 NULL 表示已对齐)
 * @param scale         深度比例因子 (raw_value / scale = 毫米)
 * @param pc            输出彩色点云
 * @return 错误码
 */
vxl_error_t vxl_dip_rgbd_to_pointcloud(const vxl_frame_t *depth,
                                       const vxl_frame_t *rgb,
                                       const vxl_intrinsics_t *depth_intrin,
                                       const vxl_intrinsics_t *rgb_intrin,
                                       const vxl_extrinsics_t *depth_to_rgb,
                                       float scale,
                                       vxl_pointcloud_t *pc);

/**
 * @brief 将深度帧对齐到 RGB 帧
 *
 * 将深度帧重采样到 RGB 帧的视角
 *
 * @param depth         深度帧 (Z16 格式)
 * @param depth_intrin  深度相机内参
 * @param rgb_intrin    RGB 相机内参
 * @param depth_to_rgb  深度到 RGB 的外参变换
 * @param scale         深度比例因子
 * @param aligned       输出对齐后的深度帧
 * @return 错误码
 */
vxl_error_t vxl_dip_align_depth_to_rgb(const vxl_frame_t *depth,
                                       const vxl_intrinsics_t *depth_intrin,
                                       const vxl_intrinsics_t *rgb_intrin,
                                       const vxl_extrinsics_t *depth_to_rgb,
                                       float scale,
                                       vxl_frame_t **aligned);

/*============================================================================
 * RGBD 融合接口 (简化版)
 *============================================================================*/

/**
 * @brief RGBD 融合配置
 */
typedef struct vxl_rgbd_config {
    vxl_intrinsics_t    depth_intrin;   /**< 深度相机内参 */
    vxl_intrinsics_t    rgb_intrin;     /**< RGB 相机内参 */
    float               depth_scale;    /**< 深度比例因子 (raw/scale=mm, VXL615=8) */
} vxl_rgbd_config_t;

/**
 * @brief 将深度帧缩放到指定分辨率
 *
 * 使用最近邻插值缩放 16-bit 深度帧
 *
 * @param src       源深度帧 (Z16 格式)
 * @param dst_width 目标宽度
 * @param dst_height 目标高度
 * @param dst       输出缩放后的深度帧
 * @return 错误码
 */
vxl_error_t vxl_dip_resize_depth(const vxl_frame_t *src,
                                  uint32_t dst_width,
                                  uint32_t dst_height,
                                  vxl_frame_t **dst);

/**
 * @brief RGBD 融合 - 将深度帧对齐到 RGB 帧
 *
 * 执行步骤：
 * 1. 将深度帧缩放到 RGB 分辨率
 * 2. 根据相机内参进行坐标变换
 * 3. 输出与 RGB 分辨率相同的对齐深度帧
 *
 * @param depth     深度帧 (Z16 格式)
 * @param rgb       RGB 帧 (RGB24/BGR24/MJPEG 格式，用于获取目标分辨率)
 * @param config    融合配置 (内参和深度比例因子)
 * @param aligned   输出对齐后的深度帧 (Z16 格式，与 RGB 分辨率相同)
 * @return 错误码
 *
 * @note 对于 VXL615：depth_scale = 8.0f
 *       对于 VXL605：depth_scale = 16.0f
 *
 * @example
 *   vxl_rgbd_config_t config = {
 *       .depth_intrin = {...},  // 从设备获取
 *       .rgb_intrin = {...},    // 从设备获取
 *       .depth_scale = 8.0f     // VXL615
 *   };
 *   vxl_frame_t *aligned = NULL;
 *   if (vxl_dip_rgbd_align(depth, rgb, &config, &aligned) == VXL_SUCCESS) {
 *       // aligned 是与 RGB 分辨率相同的深度帧
 *       vxl_frame_release(aligned);
 *   }
 */
vxl_error_t vxl_dip_rgbd_align(const vxl_frame_t *depth,
                                const vxl_frame_t *rgb,
                                const vxl_rgbd_config_t *config,
                                vxl_frame_t **aligned);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_DIP_H__ */
