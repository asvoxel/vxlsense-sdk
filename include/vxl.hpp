/**
 * @file vxl.hpp
 * @brief VXL SDK C++ API 总入口
 *
 * 包含所有 High Level C++ API 头文件
 *
 * 使用示例:
 * @code
 * #include <vxl.hpp>
 *
 * int main() {
 *     // 创建 Context 和 Device
 *     auto ctx = vxl::Context::create();
 *     auto device = ctx->getDevice(0);
 *     device->open();
 *
 *     // 创建 Pipeline
 *     vxl::Pipeline pipeline(device);
 *     pipeline.enableStream(vxl::SensorType::Depth);
 *     pipeline.enableStream(vxl::SensorType::Color);
 *     pipeline.start();
 *
 *     // 获取同步帧集
 *     auto frameset = pipeline.waitForFrameSet(1000);
 *     if (frameset) {
 *         auto depth = frameset->getDepthFrame();
 *         auto color = frameset->getColorFrame();
 *         // 处理同步的 depth + color
 *     }
 *
 *     // 应用滤波器
 *     vxl::FilterPipeline filters;
 *     filters.add<vxl::DecimationFilter>()
 *            .add<vxl::SpatialFilter>()
 *            .add<vxl::HoleFillingFilter>();
 *
 *     auto filtered_depth = filters.process(depth);
 *
 *     pipeline.stop();
 *     return 0;
 * }
 * @endcode
 */

#ifndef __VXL_HPP__
#define __VXL_HPP__

// Low Level C API
#include "vxl.h"

// C++ Types and Utilities
#include "vxl_types.hpp"

// C++ Wrapper for Low Level API
#include "vxl_frame.hpp"
#include "vxl_sensor.hpp"
#include "vxl_device.hpp"
#include "vxl_context.hpp"

// High Level API
#include "vxl_pipeline.hpp"
#include "vxl_frameset.hpp"
#include "vxl_filter.hpp"

// 高级功能按需包含，不放入总入口
// #include "vxl_body_tracker.hpp"
// #include "vxl_spatial_mapper.hpp"
// #include "vxl_object_detector.hpp"

#endif // __VXL_HPP__
