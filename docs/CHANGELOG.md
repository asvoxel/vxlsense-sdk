# VXL SDK 更新日志

## [1.0.0] - 2025-01-07

### 新增功能

- 首次发布
- Context/Device/Sensor/Stream/Frame 完整 API
- VXL435 立体深度相机支持
- IR/Depth/RGB 三种传感器类型
- 双 IR 帧分离功能 (`vxl_frame_split_ir`)
- 跨平台支持 (macOS, Linux, Windows)

### Demo 应用

- 四视图布局 (Left IR, Right IR, Depth, RGB)
- 四种显示模式 (IR+Depth, RGB, IR, Depth)
- OpenGL 硬件加速渲染
- MJPEG 解码支持

### 示例代码

- device_info.c: 设备枚举和信息查询
- low_level_api.c: 完整流控制示例

---

## 版本号说明

版本号格式: `MAJOR.MINOR.BUILD`

- **MAJOR**: 不兼容的 API 变更
- **MINOR**: 向后兼容的功能新增
- **BUILD**: 构建版本号 (每次发布自动递增)
