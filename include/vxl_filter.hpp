/**
 * @file vxl_filter.hpp
 * @brief VXL SDK Filter - 深度优化滤波器
 */

#ifndef __VXL_FILTER_HPP__
#define __VXL_FILTER_HPP__

#include "vxl_types.hpp"
#include "vxl_frame.hpp"
#include <memory>
#include <vector>

namespace vxl {

/**
 * @brief Filter 基类
 *
 * Filter 是独立的处理单元：
 * - 作用于 Frame，输出新的 Frame
 * - 用户自己组合 Filter 链
 * - SDK 只提供 Filter 实现，不强制使用
 */
class Filter {
public:
    virtual ~Filter() = default;

    /**
     * @brief 处理帧
     * @param input 输入帧
     * @return 处理后的新帧（失败返回nullptr）
     */
    virtual FramePtr process(FramePtr input) = 0;

    /**
     * @brief 链式调用支持
     */
    FramePtr operator()(FramePtr input) {
        return process(input);
    }

    /**
     * @brief 检查Filter状态是否有效
     * @return true表示Filter配置正确，可以正常工作
     */
    virtual bool isValid() const { return true; }

    /**
     * @brief 获取最后一次错误信息
     * @return 错误描述字符串（无错误时返回空字符串）
     */
    virtual std::string getLastError() const { return ""; }
};

using FilterPtr = std::shared_ptr<Filter>;

/**
 * @brief 降采样滤波器
 *
 * 降低深度图分辨率以减少噪声并提高处理速度
 */
class DecimationFilter : public Filter {
public:
    DecimationFilter();
    ~DecimationFilter();

    /**
     * @brief 设置降采样倍数
     * @param scale 降采样倍数 (2, 4, 8)
     */
    void setScale(int scale);
    int getScale() const;

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 空间滤波器
 *
 * 边缘保持平滑，减少空间噪声
 */
class SpatialFilter : public Filter {
public:
    SpatialFilter();
    ~SpatialFilter();

    /**
     * @brief 设置滤波强度
     * @param magnitude 强度 (1-5)
     */
    void setMagnitude(int magnitude);
    int getMagnitude() const;

    /**
     * @brief 设置平滑系数
     * @param alpha 系数 (0.25-1.0)
     */
    void setAlpha(float alpha);
    float getAlpha() const;

    /**
     * @brief 设置深度差异阈值
     * @param delta 阈值 (1-50)
     */
    void setDelta(float delta);
    float getDelta() const;

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 时间滤波器
 *
 * 多帧时间平均，减少时间噪声
 */
class TemporalFilter : public Filter {
public:
    TemporalFilter();
    ~TemporalFilter();

    /**
     * @brief 设置时间平滑系数
     * @param alpha 系数 (0.0-1.0)，越小越平滑
     */
    void setAlpha(float alpha);
    float getAlpha() const;

    /**
     * @brief 设置深度差异阈值
     * @param delta 阈值 (1-100)
     */
    void setDelta(float delta);
    float getDelta() const;

    /**
     * @brief 重置历史帧
     */
    void reset();

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 空洞填充滤波器
 *
 * 填充深度图中的空洞 (无效像素)
 */
class HoleFillingFilter : public Filter {
public:
    enum class Mode {
        FillFromLeft,       // 从左侧填充
        FarestFromAround,   // 使用周围最远值
        NearestFromAround   // 使用周围最近值
    };

    HoleFillingFilter();
    ~HoleFillingFilter();

    /**
     * @brief 设置填充模式
     */
    void setMode(Mode mode);
    Mode getMode() const;

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 阈值滤波器
 *
 * 过滤超出距离范围的像素
 */
class ThresholdFilter : public Filter {
public:
    ThresholdFilter();
    ~ThresholdFilter();

    /**
     * @brief 设置最小距离
     * @param min_mm 最小距离 (毫米)
     * @throws Error 如果min_mm > max_distance
     */
    void setMinDistance(uint16_t min_mm);
    uint16_t getMinDistance() const;

    /**
     * @brief 设置最大距离
     * @param max_mm 最大距离 (毫米)
     * @throws Error 如果max_mm < min_distance
     */
    void setMaxDistance(uint16_t max_mm);
    uint16_t getMaxDistance() const;

    FramePtr process(FramePtr input) override;
    bool isValid() const override;
    std::string getLastError() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 深度-视差转换滤波器
 *
 * 将深度图转换为视差图
 * disparity = baseline * focal / depth
 */
class DepthToDisparityFilter : public Filter {
public:
    DepthToDisparityFilter();
    ~DepthToDisparityFilter();

    /**
     * @brief 设置基线距离
     * @param baseline_mm 基线距离 (毫米)
     */
    void setBaseline(float baseline_mm);
    float getBaseline() const;

    /**
     * @brief 设置焦距
     * @param focal_pixels 焦距 (像素)
     */
    void setFocal(float focal_pixels);
    float getFocal() const;

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief 视差-深度转换滤波器
 *
 * 将视差图转换回深度图
 * depth = baseline * focal / disparity
 */
class DisparityToDepthFilter : public Filter {
public:
    DisparityToDepthFilter();
    ~DisparityToDepthFilter();

    /**
     * @brief 设置基线距离
     * @param baseline_mm 基线距离 (毫米)
     */
    void setBaseline(float baseline_mm);
    float getBaseline() const;

    /**
     * @brief 设置焦距
     * @param focal_pixels 焦距 (像素)
     */
    void setFocal(float focal_pixels);
    float getFocal() const;

    FramePtr process(FramePtr input) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Filter 管道
 *
 * 便捷的 Filter 链管理
 *
 * 使用示例:
 * @code
 * vxl::FilterPipeline pipe;
 * pipe.add<vxl::DecimationFilter>()
 *     .add<vxl::SpatialFilter>()
 *     .add<vxl::HoleFillingFilter>();
 *
 * auto filtered = pipe.process(depth_frame);
 * @endcode
 */
class FilterPipeline {
public:
    FilterPipeline() = default;
    ~FilterPipeline() = default;

    /**
     * @brief 添加 Filter 到管道
     */
    FilterPipeline& add(FilterPtr filter);

    /**
     * @brief 添加 Filter (模板版本)
     */
    template<typename T, typename... Args>
    FilterPipeline& add(Args&&... args) {
        return add(std::make_shared<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief 处理帧
     */
    FramePtr process(FramePtr input);

    /**
     * @brief 链式调用支持
     */
    FramePtr operator()(FramePtr input) {
        return process(input);
    }

    /**
     * @brief 获取 Filter 数量
     */
    size_t size() const { return filters_.size(); }

    /**
     * @brief 检查是否为空
     */
    bool empty() const { return filters_.empty(); }

    /**
     * @brief 清空管道
     */
    void clear() { filters_.clear(); }

    /**
     * @brief 获取指定索引的 Filter
     */
    FilterPtr get(size_t index) const;

private:
    std::vector<FilterPtr> filters_;
};

} // namespace vxl

#endif // __VXL_FILTER_HPP__
