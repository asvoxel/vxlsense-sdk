/**
 * @file vxl_types.h
 * @brief vxl SDK 基础类型定义
 */

#ifndef __VXL_TYPES_H__
#define __VXL_TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 版本信息
 *============================================================================*/

#define VXL_VERSION_MAJOR   0
#define VXL_VERSION_MINOR   1
#define VXL_VERSION_PATCH   0

/*============================================================================
 * 错误码定义
 *============================================================================*/

typedef enum vxl_error {
    VXL_SUCCESS                     = 0,
    VXL_ERROR_UNKNOWN               = -1,
    VXL_ERROR_INVALID_PARAM         = -2,
    VXL_ERROR_INVALID_HANDLE        = -3,
    VXL_ERROR_NO_DEVICE             = -4,
    VXL_ERROR_DEVICE_BUSY           = -5,
    VXL_ERROR_DEVICE_NOT_OPEN       = -6,
    VXL_ERROR_NOT_SUPPORTED         = -7,
    VXL_ERROR_NO_MEM                = -8,
    VXL_ERROR_IO                    = -9,
    VXL_ERROR_TIMEOUT               = -10,
    VXL_ERROR_STREAM_NOT_STARTED    = -11,
    VXL_ERROR_STREAM_ALREADY_STARTED = -12,
    VXL_ERROR_FRAME_NOT_AVAILABLE   = -13,
    VXL_ERROR_INVALID_PROFILE       = -14,
    VXL_ERROR_OPTION_NOT_SUPPORTED  = -15,
    VXL_ERROR_OPTION_READ_ONLY      = -16,
    VXL_ERROR_USB                   = -17,
    VXL_ERROR_BACKEND               = -18,
    VXL_ERROR_ACCESS                = -19,
} vxl_error_t;

const char* vxl_error_string(vxl_error_t error);

/*============================================================================
 * 传感器类型
 *============================================================================*/

typedef enum vxl_sensor_type {
    VXL_SENSOR_UNKNOWN      = 0,
    VXL_SENSOR_COLOR        = 1,    /* 彩色相机 (UVC) */
    VXL_SENSOR_DEPTH        = 2,    /* 深度相机 */
    VXL_SENSOR_IR           = 3,    /* 红外相机 */
    VXL_SENSOR_COUNT
} vxl_sensor_type_t;

const char* vxl_sensor_type_string(vxl_sensor_type_t type);

/*============================================================================
 * 帧格式定义 - 映射到 libuvc uvc_frame_format
 *============================================================================*/

typedef enum vxl_format {
    VXL_FORMAT_UNKNOWN      = 0,
    VXL_FORMAT_ANY          = 0,
    VXL_FORMAT_YUYV         = 1,    /* YUYV/YUV2/YUV422 */
    VXL_FORMAT_UYVY         = 2,
    VXL_FORMAT_RGB          = 3,    /* 24-bit RGB */
    VXL_FORMAT_BGR          = 4,    /* 24-bit BGR */
    VXL_FORMAT_MJPEG        = 5,    /* Motion-JPEG */
    VXL_FORMAT_H264         = 6,
    VXL_FORMAT_GRAY8        = 7,    /* 8-bit 灰度 */
    VXL_FORMAT_GRAY16       = 8,    /* 16-bit 灰度 */
    VXL_FORMAT_NV12         = 9,    /* YUV420: NV12 */
    VXL_FORMAT_Z16          = 10,   /* 16-bit 深度 */
    VXL_FORMAT_COUNT
} vxl_format_t;

const char* vxl_format_string(vxl_format_t format);
int vxl_format_bytes_per_pixel(vxl_format_t format);

/*============================================================================
 * Option 定义
 *============================================================================*/

typedef enum vxl_option {
    VXL_OPTION_UNKNOWN                  = 0,

    /*========================================================================
     * 标准 UVC 选项 (1-99) - 基于 UVC 1.5 规范
     *========================================================================*/

    /* 曝光和增益控制 (所有相机) */
    VXL_OPTION_EXPOSURE                 = 1,    /* 曝光时间 (us) */
    VXL_OPTION_GAIN                     = 2,    /* 增益 */
    VXL_OPTION_AUTO_EXPOSURE            = 8,    /* 自动曝光开关 */

    /* RGB 相机图像处理 (仅彩色相机，如VXL435的RGB传感器) */
    VXL_OPTION_BRIGHTNESS               = 3,    /* 亮度 */
    VXL_OPTION_CONTRAST                 = 4,    /* 对比度 */
    VXL_OPTION_SATURATION               = 5,    /* 饱和度 */
    VXL_OPTION_SHARPNESS                = 6,    /* 锐度 */
    VXL_OPTION_WHITE_BALANCE            = 7,    /* 白平衡 */
    VXL_OPTION_AUTO_WHITE_BALANCE       = 9,    /* 自动白平衡 */
    VXL_OPTION_GAMMA                    = 10,   /* Gamma 校正 */
    VXL_OPTION_HUE                      = 11,   /* 色调 */

    /*========================================================================
     * 深度相机通用选项 (100-199)
     *========================================================================*/

    VXL_OPTION_DEPTH_UNITS              = 100,  /* 深度单位 (mm) */
    VXL_OPTION_MIN_DISTANCE             = 101,  /* 最小工作距离 (mm) */
    VXL_OPTION_MAX_DISTANCE             = 102,  /* 最大工作距离 (mm) */

    /*========================================================================
     * IR/投影控制通用 (200-299)
     *========================================================================*/

    VXL_OPTION_IR_ENABLE                = 200,  /* IR/投影开关 (通用) */

    /*========================================================================
     * VXL435 产品特定选项 (1000-1099)
     *========================================================================*/

    /* IR 控制 */
    VXL435_OPTION_IR_CURRENT            = 1001, /* IR 电流档位 [0-7] */

    /* RGB ISP 控制 */
    VXL435_OPTION_RGB_DENOISE           = 1010, /* RGB 降噪级别 */
    VXL435_OPTION_RGB_SHARPEN           = 1011, /* RGB 锐化级别 */

    /* 通道开关控制 */
    VXL435_OPTION_LEFT_IR               = 1020, /* 左 IR 通道开关 */
    VXL435_OPTION_RIGHT_IR              = 1021, /* 右 IR 通道开关 */
    VXL435_OPTION_DEPTH                 = 1022, /* 深度通道开关 */
    VXL435_OPTION_RGB                   = 1023, /* RGB 通道开关 */

    /* 其他 */
    VXL435_OPTION_TWEAK_MODE            = 1030, /* 调试模式开关 */

    /*========================================================================
     * VXL615 产品特定选项 (2000-2099)
     *========================================================================*/

    /* IR 高级控制 (2000-2009) */
    VXL615_OPTION_IR_PERIOD             = 2001, /* IR 投影周期 */
    VXL615_OPTION_IR_DUTY_CYCLE         = 2002, /* IR 占空比 */

    /* 2D (NIR) AE 参数 (2010-2029) */
    VXL615_OPTION_2D_FMEAN_TARGET       = 2010, /* 2D 目标 Fmean */
    VXL615_OPTION_2D_FMEAN_BOUNDARY     = 2011, /* 2D Fmean 边界 */
    VXL615_OPTION_2D_FMEAN_CURRENT      = 2012, /* 2D 当前 Fmean (只读) */
    VXL615_OPTION_2D_GAIN_MIN           = 2013, /* 2D 最小增益 */
    VXL615_OPTION_2D_GAIN_MAX           = 2014, /* 2D 最大增益 */
    VXL615_OPTION_2D_GAIN_CURRENT       = 2015, /* 2D 当前增益 (只读) */
    VXL615_OPTION_2D_EXPOSURE_MIN       = 2016, /* 2D 最小曝光 */
    VXL615_OPTION_2D_EXPOSURE_MID       = 2017, /* 2D 中间曝光 */
    VXL615_OPTION_2D_EXPOSURE_MAX       = 2018, /* 2D 最大曝光 */
    VXL615_OPTION_2D_EXPOSURE_CURRENT   = 2019, /* 2D 当前曝光 (只读) */

    /* 深度后处理参数 (2030-2049) */
    VXL615_OPTION_NCC_THRESHOLD         = 2030, /* NCC 阈值 [0-255] */
    VXL615_OPTION_PATCH_SIZE            = 2031, /* Patch 大小 [0-6] */
    VXL615_OPTION_OUTLIER_REMOVAL       = 2032, /* 异常值移除开关 */
    VXL615_OPTION_DENOISE_ENABLE        = 2033, /* 降噪开关 */
    VXL615_OPTION_DENOISE_LEVEL         = 2034, /* 降噪级别 [1-3] */
    VXL615_OPTION_MEDIAN_FILTER         = 2035, /* 中值滤波开关 */
    VXL615_OPTION_MEDIAN_KERNEL_SIZE    = 2036, /* 中值滤波核大小 */
    VXL615_OPTION_UNDISTORTION          = 2037, /* 去畸变开关 */

    /* 3D (深度) AE 参数 (2050-2069) */
    VXL615_OPTION_3D_FMEAN_TARGET       = 2050, /* 3D 目标 Fmean */
    VXL615_OPTION_3D_FMEAN_BOUNDARY     = 2051, /* 3D Fmean 边界 */
    VXL615_OPTION_3D_FMEAN_CURRENT      = 2052, /* 3D 当前 Fmean (只读) */
    VXL615_OPTION_3D_GAIN_MIN           = 2053, /* 3D 最小增益 */
    VXL615_OPTION_3D_GAIN_MAX           = 2054, /* 3D 最大增益 */
    VXL615_OPTION_3D_GAIN_CURRENT       = 2055, /* 3D 当前增益 (只读) */
    VXL615_OPTION_3D_EXPOSURE_MIN       = 2056, /* 3D 最小曝光 */
    VXL615_OPTION_3D_EXPOSURE_MID       = 2057, /* 3D 中间曝光 */
    VXL615_OPTION_3D_EXPOSURE_MAX       = 2058, /* 3D 最大曝光 */
    VXL615_OPTION_3D_EXPOSURE_CURRENT   = 2059, /* 3D 当前曝光 (只读) */

    VXL_OPTION_COUNT
} vxl_option_t;

const char* vxl_option_string(vxl_option_t option);

/*============================================================================
 * Option 范围
 *============================================================================*/

typedef struct vxl_option_range {
    float min;
    float max;
    float step;
    float def;
} vxl_option_range_t;

/*============================================================================
 * 帧元数据
 *============================================================================*/

typedef struct vxl_frame_metadata {
    uint64_t    timestamp_us;       /* 时间戳 (微秒) */
    uint32_t    sequence;           /* 帧序号 */
    uint32_t    exposure_us;        /* 曝光时间 (微秒) */
    uint16_t    gain;               /* 增益值 */
} vxl_frame_metadata_t;

/*============================================================================
 * 设备信息
 *============================================================================*/

#define VXL_MAX_STRING_LEN  256

typedef struct vxl_device_info {
    char        name[VXL_MAX_STRING_LEN];
    char        serial_number[VXL_MAX_STRING_LEN];
    char        manufacturer[VXL_MAX_STRING_LEN];
    char        fw_version[VXL_MAX_STRING_LEN];     /* 固件版本 */
    char        rom_version[VXL_MAX_STRING_LEN];    /* ROM 版本（VXL615支持） */
    uint16_t    vendor_id;
    uint16_t    product_id;
    uint16_t    uvc_version;        /* UVC 版本 (如 0x0100 = 1.0) */
} vxl_device_info_t;

/*============================================================================
 * 标定类型
 *============================================================================*/

typedef enum vxl_calib_type {
    VXL_CALIB_INTRINSICS        = 0,    /* 内参 */
    VXL_CALIB_EXTRINSICS        = 1,    /* 外参 */
    VXL_CALIB_DEPTH             = 2,    /* 深度标定 */
    VXL_CALIB_IR_IR             = 3,    /* IR-IR 标定 */
    VXL_CALIB_IR_RGB            = 4,    /* IR-RGB 标定 */
} vxl_calib_type_t;

/*============================================================================
 * 相机参数
 *============================================================================*/

/**
 * @brief 相机内参
 */
typedef struct vxl_intrinsics {
    float       fx, fy;             /* 焦距 */
    float       cx, cy;             /* 主点 */
    float       coeffs[5];          /* 畸变系数 k1,k2,p1,p2,k3 */
    uint32_t    width, height;      /* 图像尺寸 */
} vxl_intrinsics_t;

/**
 * @brief 相机外参 (从 from 到 to 的变换)
 */
typedef struct vxl_extrinsics {
    float       rotation[9];        /* 3x3 旋转矩阵 (行优先) */
    float       translation[3];     /* 平移向量 (mm) */
} vxl_extrinsics_t;

/**
 * @brief 深度计算参数
 */
typedef struct vxl_depth_param {
    float       baseline;           /* 基线 (mm) */
    float       focus;              /* 焦距 */
    float       cx, cy;             /* 算法主点 */
} vxl_depth_param_t;

/*============================================================================
 * 句柄类型前向声明
 *============================================================================*/

typedef struct vxl_context vxl_context_t;
typedef struct vxl_device vxl_device_t;
typedef struct vxl_sensor vxl_sensor_t;
typedef struct vxl_profile vxl_profile_t;
typedef struct vxl_stream vxl_stream_t;
typedef struct vxl_frame vxl_frame_t;

/*============================================================================
 * 回调函数类型
 *============================================================================*/

typedef void (*vxl_frame_cbfn)(vxl_frame_t *frame, void *user_data);

typedef void (*vxl_device_event_cbfn)(
    const vxl_device_info_t *device_info,
    bool added,
    void *user_data
);

#ifdef __cplusplus
}
#endif

#endif /* __VXL_TYPES_H__ */
