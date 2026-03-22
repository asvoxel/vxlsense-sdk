/**
 * @file vxl_context.hpp
 * @brief VXL SDK Context C++ 封装
 */

#ifndef __VXL_CONTEXT_HPP__
#define __VXL_CONTEXT_HPP__

#include "vxl_context.h"
#include "vxl_types.hpp"
#include "vxl_device.hpp"
#include <memory>
#include <functional>

namespace vxl {

class Context;
using ContextPtr = std::shared_ptr<Context>;

/**
 * @brief Context C++ 封装
 */
class Context {
public:
    using DeviceEventCallback = std::function<void(const DeviceInfo&, bool added)>;

    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    /**
     * @brief 创建 Context
     */
    static ContextPtr create();

    /**
     * @brief 获取设备数量
     */
    size_t deviceCount() const;

    /**
     * @brief 获取指定索引的设备
     */
    DevicePtr getDevice(size_t index);

    /**
     * @brief 查找指定 VID/PID 的设备
     */
    DevicePtr findDevice(uint16_t vid, uint16_t pid);

    /**
     * @brief 查找指定序列号的设备
     */
    DevicePtr findDeviceBySerial(const std::string& serial_number);

    /**
     * @brief 刷新设备列表
     */
    void refreshDevices();

    /**
     * @brief 设置设备事件回调
     */
    void setDeviceEventCallback(DeviceEventCallback callback);

    vxl_context_t* handle() const { return ctx_; }

private:
    Context();

    static void device_event_callback_wrapper(
        const vxl_device_info_t* device_info,
        bool added,
        void* user_data);

    vxl_context_t* ctx_;
    DeviceEventCallback device_callback_;
};

/**
 * @brief 设置日志级别
 */
inline void setLogLevel(vxl_log_level_t level) {
    vxl_set_log_level(level);
}

/**
 * @brief 获取当前日志级别
 */
inline vxl_log_level_t getLogLevel() {
    return vxl_get_log_level();
}

/**
 * @brief 获取 SDK 版本字符串
 */
inline std::string getVersionString() {
    return vxl_get_version_string();
}

} // namespace vxl

#endif // __VXL_CONTEXT_HPP__
