/**
 * @file vxl_frameset.hpp
 * @brief VXL SDK FrameSet - 同步帧集合
 */

#ifndef __VXL_FRAMESET_HPP__
#define __VXL_FRAMESET_HPP__

#include "vxl_types.hpp"
#include "vxl_frame.hpp"
#include <memory>
#include <map>

namespace vxl {

class FrameSet;
using FrameSetPtr = std::shared_ptr<FrameSet>;

/**
 * @brief 同步帧集合
 *
 * 存储来自多个传感器的同步帧 (基于 frame_count 匹配)
 */
class FrameSet {
public:
    FrameSet();
    ~FrameSet();

    /**
     * @brief 获取深度帧
     * @return 深度帧或 nullptr
     */
    FramePtr getDepthFrame() const;

    /**
     * @brief 获取彩色帧
     * @return 彩色帧或 nullptr
     */
    FramePtr getColorFrame() const;

    /**
     * @brief 获取红外帧
     * @return 红外帧或 nullptr
     */
    FramePtr getIRFrame() const;

    /**
     * @brief 获取指定类型的帧
     * @param type 传感器类型
     * @return 帧或 nullptr
     */
    FramePtr getFrame(SensorType type) const;

    /**
     * @brief 检查是否包含指定类型的帧
     */
    bool hasFrame(SensorType type) const;

    /**
     * @brief 获取帧计数 (同步标识)
     */
    uint16_t getFrameCount() const;

    /**
     * @brief 获取时间戳 (取最早帧的时间戳)
     */
    uint64_t getTimestamp() const;

    /**
     * @brief 获取包含的帧数量
     */
    size_t size() const;

    /**
     * @brief 检查是否为空
     */
    bool empty() const;

    /**
     * @brief 添加帧到集合 (内部使用)
     */
    void addFrame(SensorType type, FramePtr frame);

    /**
     * @brief 设置帧计数 (内部使用)
     */
    void setFrameCount(uint16_t count);

    /**
     * @brief 创建空 FrameSet
     */
    static FrameSetPtr create();

private:
    std::map<SensorType, FramePtr> frames_;
    uint16_t frame_count_;
};

} // namespace vxl

#endif // __VXL_FRAMESET_HPP__
