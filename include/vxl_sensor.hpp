/**
 * @file vxl_sensor.hpp
 * @brief VXL SDK Sensor C++ 封装
 */

#ifndef __VXL_SENSOR_HPP__
#define __VXL_SENSOR_HPP__

#include "vxl_sensor.h"
#include "vxl_types.hpp"
#include "vxl_frame.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace vxl {

class Sensor;
class Stream;
class Profile;

using SensorPtr = std::shared_ptr<Sensor>;
using StreamPtr = std::shared_ptr<Stream>;
using ProfilePtr = std::shared_ptr<Profile>;

/**
 * @brief Profile 封装
 */
class Profile {
public:
    explicit Profile(vxl_profile_t* profile, bool own = true);
    ~Profile();

    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;
    Profile(Profile&& other) noexcept;
    Profile& operator=(Profile&& other) noexcept;

    Format format() const;
    uint32_t width() const;
    uint32_t height() const;
    uint32_t fps() const;

    vxl_profile_t* handle() const { return profile_; }

    static ProfilePtr create(vxl_profile_t* profile, bool own = true);

private:
    vxl_profile_t* profile_;
    bool own_;
};

/**
 * @brief Stream 封装
 */
class Stream {
public:
    using FrameCallback = std::function<void(FramePtr)>;

    explicit Stream(vxl_stream_t* stream, bool own = true);
    ~Stream();

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    /**
     * @brief 启动流 (回调模式)
     */
    void start(FrameCallback callback);

    /**
     * @brief 启动流 (轮询模式)
     */
    void start();

    /**
     * @brief 停止流
     */
    void stop();

    /**
     * @brief 检查是否运行中
     */
    bool isRunning() const;

    /**
     * @brief 等待帧 (阻塞)
     */
    FramePtr waitForFrame(uint32_t timeout_ms = 1000);

    /**
     * @brief 轮询帧 (非阻塞)
     * @return 帧或 nullptr
     */
    FramePtr pollForFrame();

    /**
     * @brief 获取当前 Profile
     */
    ProfilePtr getProfile() const;

    vxl_stream_t* handle() const { return stream_; }

    static StreamPtr create(vxl_stream_t* stream, bool own = true);

private:
    static void frame_callback_wrapper(vxl_frame_t* frame, void* user_data);

    vxl_stream_t* stream_;
    bool own_;
    FrameCallback callback_;
};

/**
 * @brief Sensor C++ 封装
 */
class Sensor {
public:
    explicit Sensor(vxl_sensor_t* sensor, bool own = true);
    ~Sensor();

    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;

    /**
     * @brief 获取传感器类型
     */
    SensorType type() const;

    /**
     * @brief 获取传感器名称
     */
    std::string name() const;

    /**
     * @brief 获取支持的 Profile 数量
     */
    size_t profileCount() const;

    /**
     * @brief 获取指定索引的 Profile
     */
    ProfilePtr getProfile(size_t index) const;

    /**
     * @brief 查找匹配的 Profile
     */
    ProfilePtr findProfile(Format format = Format::Any,
                           uint32_t width = 0,
                           uint32_t height = 0,
                           uint32_t fps = 0) const;

    /**
     * @brief 获取所有 Profile
     */
    std::vector<ProfilePtr> getProfiles() const;

    /**
     * @brief 创建流
     */
    StreamPtr createStream(const Profile& profile);
    StreamPtr createStream(ProfilePtr profile);

    /**
     * @brief 检查选项是否支持
     */
    bool isOptionSupported(vxl_option_t option) const;

    /**
     * @brief 获取选项范围
     */
    vxl_option_range_t getOptionRange(vxl_option_t option) const;

    /**
     * @brief 获取选项值
     */
    float getOption(vxl_option_t option) const;

    /**
     * @brief 设置选项值
     */
    void setOption(vxl_option_t option, float value);

    vxl_sensor_t* handle() const { return sensor_; }

    static SensorPtr create(vxl_sensor_t* sensor, bool own = true);

private:
    vxl_sensor_t* sensor_;
    bool own_;
};

} // namespace vxl

#endif // __VXL_SENSOR_HPP__
