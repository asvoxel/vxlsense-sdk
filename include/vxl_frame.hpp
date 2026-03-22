/**
 * @file vxl_frame.hpp
 * @brief VXL SDK Frame C++ 封装
 */

#ifndef __VXL_FRAME_HPP__
#define __VXL_FRAME_HPP__

#include "vxl_frame.h"
#include "vxl_types.hpp"
#include <memory>

namespace vxl {

class Frame;
using FramePtr = std::shared_ptr<Frame>;

/**
 * @brief Frame C++ 封装
 *
 * RAII 管理 vxl_frame_t 生命周期
 */
class Frame {
public:
    /**
     * @brief 从 C 句柄构造
     * @param frame C 帧句柄
     * @param own 是否拥有所有权 (默认 true，会调用 release)
     */
    explicit Frame(vxl_frame_t* frame, bool own = true);

    ~Frame();

    // 禁止拷贝
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    // 允许移动
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    /**
     * @brief 获取帧宽度
     */
    uint32_t width() const;

    /**
     * @brief 获取帧高度
     */
    uint32_t height() const;

    /**
     * @brief 获取帧格式
     */
    Format format() const;

    /**
     * @brief 获取行跨度 (stride)
     */
    uint32_t stride() const;

    /**
     * @brief 获取数据指针
     */
    const void* data() const;

    /**
     * @brief 获取数据大小 (字节)
     */
    size_t dataSize() const;

    /**
     * @brief 获取时间戳 (微秒)
     */
    uint64_t timestamp() const;

    /**
     * @brief 获取帧序号
     */
    uint32_t sequence() const;

    /**
     * @brief 获取设备帧计数 (用于帧同步)
     */
    uint16_t frameCount() const;

    /**
     * @brief 获取来源传感器类型
     */
    SensorType sensorType() const;

    /**
     * @brief 获取帧元数据
     */
    vxl_frame_metadata_t metadata() const;

    /**
     * @brief 转换为指定格式
     * @param dst_format 目标格式
     * @return 转换后的新帧
     */
    FramePtr convert(Format dst_format) const;

    /**
     * @brief 复制帧
     * @return 复制的新帧
     */
    FramePtr copy() const;

    /**
     * @brief 检查是否为双 IR 帧
     */
    bool isDualIR() const;

    /**
     * @brief 分离双 IR 帧
     * @param left 输出左 IR 帧
     * @param right 输出右 IR 帧
     */
    void splitIR(FramePtr& left, FramePtr& right) const;

    /**
     * @brief 检查与另一帧是否同步
     * @param other 另一帧
     * @return true 同步 (frame_count 相同)
     */
    bool isSynchronizedWith(const Frame& other) const;

    /**
     * @brief 检查Frame是否有效
     * @return true表示Frame句柄有效且可以使用
     */
    bool isValid() const { return frame_ != nullptr; }

    /**
     * @brief 获取格式的每像素字节数
     * @param fmt 格式
     * @return 每像素字节数，未知格式返回0
     */
    static uint32_t getBytesPerPixel(Format fmt);

    /**
     * @brief 获取当前帧格式的每像素字节数
     * @return 每像素字节数
     */
    uint32_t bytesPerPixel() const;

    /**
     * @brief 获取原始 C 句柄
     */
    vxl_frame_t* handle() const { return frame_; }

    /**
     * @brief 释放所有权，返回原始指针
     */
    vxl_frame_t* release();

    /**
     * @brief 从 C 句柄创建 shared_ptr
     * @param frame C 帧句柄
     * @param own 是否拥有所有权
     */
    static FramePtr create(vxl_frame_t* frame, bool own = true);

private:
    vxl_frame_t* frame_;
    bool own_;
};

} // namespace vxl

#endif // __VXL_FRAME_HPP__
