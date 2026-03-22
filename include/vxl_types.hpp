/**
 * @file vxl_types.hpp
 * @brief VXL SDK C++ 类型定义
 */

#ifndef __VXL_TYPES_HPP__
#define __VXL_TYPES_HPP__

#include "vxl_types.h"
#include <stdexcept>
#include <string>

namespace vxl {

/**
 * @brief 传感器类型
 */
enum class SensorType {
    Unknown = VXL_SENSOR_UNKNOWN,
    Color   = VXL_SENSOR_COLOR,
    Depth   = VXL_SENSOR_DEPTH,
    IR      = VXL_SENSOR_IR
};

/**
 * @brief 帧格式
 */
enum class Format {
    Unknown = VXL_FORMAT_UNKNOWN,
    Any     = VXL_FORMAT_ANY,
    YUYV    = VXL_FORMAT_YUYV,
    UYVY    = VXL_FORMAT_UYVY,
    RGB     = VXL_FORMAT_RGB,
    BGR     = VXL_FORMAT_BGR,
    MJPEG   = VXL_FORMAT_MJPEG,
    H264    = VXL_FORMAT_H264,
    Gray8   = VXL_FORMAT_GRAY8,
    Gray16  = VXL_FORMAT_GRAY16,
    NV12    = VXL_FORMAT_NV12,
    Z16     = VXL_FORMAT_Z16
};

/**
 * @brief Pipeline 同步模式
 */
enum class SyncMode {
    Strict,      // 严格同步，frame_count 必须完全匹配
    Approximate, // 近似同步，允许 1-2 帧误差
    None         // 不同步，各流独立
};

/**
 * @brief 设备信息
 */
struct DeviceInfo {
    std::string name;
    std::string serial_number;
    std::string manufacturer;
    std::string fw_version;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t uvc_version;

    DeviceInfo() : vendor_id(0), product_id(0), uvc_version(0) {}

    DeviceInfo(const vxl_device_info_t& info)
        : name(info.name)
        , serial_number(info.serial_number)
        , manufacturer(info.manufacturer)
        , fw_version(info.fw_version)
        , vendor_id(info.vendor_id)
        , product_id(info.product_id)
        , uvc_version(info.uvc_version)
    {}
};

/**
 * @brief 相机内参
 */
struct Intrinsics {
    float fx, fy;           // 焦距
    float cx, cy;           // 主点
    float coeffs[5];        // 畸变系数
    uint32_t width, height; // 图像尺寸

    Intrinsics() : fx(0), fy(0), cx(0), cy(0), width(0), height(0) {
        for (int i = 0; i < 5; i++) coeffs[i] = 0;
    }

    Intrinsics(const vxl_intrinsics_t& intrin)
        : fx(intrin.fx), fy(intrin.fy)
        , cx(intrin.cx), cy(intrin.cy)
        , width(intrin.width), height(intrin.height)
    {
        for (int i = 0; i < 5; i++) coeffs[i] = intrin.coeffs[i];
    }
};

/**
 * @brief 相机外参
 */
struct Extrinsics {
    float rotation[9];      // 3x3 旋转矩阵
    float translation[3];   // 平移向量 (mm)

    Extrinsics() {
        for (int i = 0; i < 9; i++) rotation[i] = (i % 4 == 0) ? 1.0f : 0.0f;
        for (int i = 0; i < 3; i++) translation[i] = 0;
    }

    Extrinsics(const vxl_extrinsics_t& extrin) {
        for (int i = 0; i < 9; i++) rotation[i] = extrin.rotation[i];
        for (int i = 0; i < 3; i++) translation[i] = extrin.translation[i];
    }
};

/**
 * @brief VXL 异常类
 */
class Error : public std::runtime_error {
public:
    explicit Error(vxl_error_t code)
        : std::runtime_error(vxl_error_string(code))
        , code_(code)
    {}

    Error(vxl_error_t code, const std::string& msg)
        : std::runtime_error(msg + ": " + vxl_error_string(code))
        , code_(code)
    {}

    vxl_error_t code() const { return code_; }

private:
    vxl_error_t code_;
};

/**
 * @brief 检查错误码并抛出异常
 */
inline void check_error(vxl_error_t err) {
    if (err != VXL_SUCCESS) {
        throw Error(err);
    }
}

/**
 * @brief 检查错误码并抛出异常 (带自定义消息)
 */
inline void check_error(vxl_error_t err, const std::string& msg) {
    if (err != VXL_SUCCESS) {
        throw Error(err, msg);
    }
}

// 类型转换辅助函数
inline vxl_sensor_type_t to_c(SensorType type) {
    return static_cast<vxl_sensor_type_t>(type);
}

inline SensorType from_c(vxl_sensor_type_t type) {
    return static_cast<SensorType>(type);
}

inline vxl_format_t to_c(Format fmt) {
    return static_cast<vxl_format_t>(fmt);
}

inline Format from_c(vxl_format_t fmt) {
    return static_cast<Format>(fmt);
}

} // namespace vxl

#endif // __VXL_TYPES_HPP__
