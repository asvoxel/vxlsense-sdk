/**
 * @file vxl_device.hpp
 * @brief VXL SDK Device C++ 封装
 */

#ifndef __VXL_DEVICE_HPP__
#define __VXL_DEVICE_HPP__

#include "vxl_device.h"
#include "vxl_types.hpp"
#include "vxl_sensor.hpp"
#include <memory>

namespace vxl {

class Device;
using DevicePtr = std::shared_ptr<Device>;

/**
 * @brief Device C++ 封装
 */
class Device : public std::enable_shared_from_this<Device> {
public:
    explicit Device(vxl_device_t* device, bool own = true);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    /**
     * @brief 打开设备
     */
    void open();

    /**
     * @brief 关闭设备
     */
    void close();

    /**
     * @brief 检查设备是否已打开
     */
    bool isOpen() const;

    /**
     * @brief 获取设备信息
     */
    DeviceInfo getInfo() const;

    /**
     * @brief 获取传感器数量
     */
    size_t sensorCount() const;

    /**
     * @brief 获取指定类型的传感器
     */
    SensorPtr getSensor(SensorType type);

    /**
     * @brief 获取指定索引的传感器
     */
    SensorPtr getSensorByIndex(size_t index);

    /**
     * @brief 获取内参
     */
    Intrinsics getIntrinsics(SensorType sensor) const;

    /**
     * @brief 获取外参
     */
    Extrinsics getExtrinsics(SensorType from, SensorType to) const;

    /**
     * @brief 获取深度计算参数
     */
    vxl_depth_param_t getDepthParam() const;

    /**
     * @brief 获取固件版本
     */
    std::string getFirmwareVersion() const;

    /**
     * @brief 硬件重启
     */
    void hwReset();

    /**
     * @brief 下载固件
     */
    void fwDownload(const void* data, size_t len,
                    uint32_t run_addr, uint32_t base_addr);

    /**
     * @brief 读取标定数据
     */
    std::vector<uint8_t> calibRead(vxl_calib_type_t type) const;

    /**
     * @brief 写入标定数据
     */
    void calibWrite(vxl_calib_type_t type, const void* data, size_t len);

    vxl_device_t* handle() const { return device_; }

    static DevicePtr create(vxl_device_t* device, bool own = true);

private:
    vxl_device_t* device_;
    bool own_;
};

} // namespace vxl

#endif // __VXL_DEVICE_HPP__
