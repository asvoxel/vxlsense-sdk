/**
 * @file vxl_pipeline.hpp
 * @brief VXL SDK Pipeline - 多流同步管道
 */

#ifndef __VXL_PIPELINE_HPP__
#define __VXL_PIPELINE_HPP__

#include "vxl_types.hpp"
#include "vxl_device.hpp"
#include "vxl_sensor.hpp"
#include "vxl_frameset.hpp"
#include <memory>
#include <functional>

namespace vxl {

class Pipeline;
using PipelinePtr = std::shared_ptr<Pipeline>;

/**
 * @brief 流配置
 */
struct StreamConfig {
    SensorType type;
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t fps;

    StreamConfig()
        : type(SensorType::Unknown)
        , format(Format::Any)
        , width(0)
        , height(0)
        , fps(0)
    {}

    StreamConfig(SensorType t, Format f = Format::Any,
                 uint32_t w = 0, uint32_t h = 0, uint32_t r = 0)
        : type(t), format(f), width(w), height(h), fps(r)
    {}
};

/**
 * @brief Pipeline 配置
 */
struct PipelineConfig {
    SyncMode sync_mode = SyncMode::Strict;
    size_t frame_queue_size = 4;
    uint32_t sync_timeout_ms = 100;  // 等待同步帧的超时

    std::vector<StreamConfig> streams;

    PipelineConfig& enableStream(SensorType type,
                                  Format format = Format::Any,
                                  uint32_t width = 0,
                                  uint32_t height = 0,
                                  uint32_t fps = 0) {
        streams.emplace_back(type, format, width, height, fps);
        return *this;
    }
};

/**
 * @brief Pipeline - 多流同步管道
 *
 * Pipeline 是 Device 的上层聚合，负责：
 * - 管理多个 Stream 的启停
 * - 基于 frame_count 进行帧同步
 * - 输出 FrameSet（同步帧集合）
 *
 * 使用示例:
 * @code
 * auto device = ctx->getDevice(0);
 * device->open();
 *
 * vxl::Pipeline pipeline(device);
 * pipeline.enableStream(vxl::SensorType::Depth);
 * pipeline.enableStream(vxl::SensorType::Color);
 * pipeline.start();
 *
 * while (running) {
 *     auto frameset = pipeline.waitForFrameSet(1000);
 *     if (frameset) {
 *         auto depth = frameset->getDepthFrame();
 *         auto color = frameset->getColorFrame();
 *         // 处理同步的 depth + color
 *     }
 * }
 *
 * pipeline.stop();
 * @endcode
 *
 * 线程安全说明:
 * - enableStream/disableStream/setSyncMode/setFrameQueueSize 必须在 start() 之前调用
 * - 这些配置方法在 pipeline 运行时调用会抛出异常
 * - waitForFrameSet/pollForFrameSet 可以从多个线程并发调用（内部有锁保护）
 * - start/stop 不应该并发调用（由调用者保证串行）
 * - 回调模式下，callback 在内部同步线程中执行，不要在回调中调用 stop()
 *
 * 错误恢复:
 * - 如果 start() 失败（找不到 profile、创建 stream 失败等），
 *   Pipeline 会自动清理已创建的资源并恢复到 stopped 状态
 * - 可以修改配置后重新调用 start()
 */
class Pipeline {
public:
    using FrameSetCallback = std::function<void(FrameSetPtr)>;

    /**
     * @brief 构造 Pipeline
     * @param device 设备对象 (必须已打开)
     */
    explicit Pipeline(DevicePtr device);

    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    /**
     * @brief 启用指定类型的流
     * @param type 传感器类型
     * @param format 期望格式 (Any 表示自动选择)
     * @param width 期望宽度 (0 表示自动)
     * @param height 期望高度 (0 表示自动)
     * @param fps 期望帧率 (0 表示自动)
     */
    void enableStream(SensorType type,
                      Format format = Format::Any,
                      uint32_t width = 0,
                      uint32_t height = 0,
                      uint32_t fps = 0);

    /**
     * @brief 启用流 (使用配置)
     */
    void enableStream(const StreamConfig& config);

    /**
     * @brief 禁用指定类型的流
     */
    void disableStream(SensorType type);

    /**
     * @brief 使用配置启动
     */
    void start(const PipelineConfig& config);

    /**
     * @brief 启动 (使用已配置的流)
     */
    void start();

    /**
     * @brief 启动 (回调模式)
     */
    void start(FrameSetCallback callback);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 检查是否运行中
     */
    bool isRunning() const;

    /**
     * @brief 等待同步帧集
     * @param timeout_ms 超时时间 (毫秒)
     * @return FrameSet 或 nullptr (超时)
     */
    FrameSetPtr waitForFrameSet(uint32_t timeout_ms = 1000);

    /**
     * @brief 轮询同步帧集 (非阻塞)
     * @return FrameSet 或 nullptr
     */
    FrameSetPtr pollForFrameSet();

    /**
     * @brief 设置帧队列大小
     */
    void setFrameQueueSize(size_t size);

    /**
     * @brief 设置同步模式
     */
    void setSyncMode(SyncMode mode);

    /**
     * @brief 获取当前同步模式
     */
    SyncMode getSyncMode() const;

    /**
     * @brief 获取关联的设备
     */
    DevicePtr getDevice() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace vxl

#endif // __VXL_PIPELINE_HPP__
