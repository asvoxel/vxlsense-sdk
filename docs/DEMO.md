# VXL Demo 应用说明

VXL Demo 是一个基于 Qt 的图形化演示程序，展示 VXL SDK 的主要功能。

## 功能特性

- 四视图布局：Left IR、Right IR、Depth、RGB
- 四种显示模式：IR+Depth、RGB、IR、Depth
- 设备枚举和选择
- 实时视频流显示
- OpenGL 硬件加速渲染

## 界面布局

```
┌─────────────────────────────────────────────┐
│ Device: [选择设备 ▼]  [Refresh] [Start] [Stop] │
├─────────────────────────────────────────────┤
│ Mode: ○ IR+Depth  ○ RGB  ○ IR  ○ Depth      │
├──────────────────┬──────────────────────────┤
│    Left IR       │      Right IR            │
├──────────────────┼──────────────────────────┤
│    Depth         │      RGB                 │
└──────────────────┴──────────────────────────┘
```

## 显示模式说明

| 模式 | 启动传感器 | 显示视图 |
|------|-----------|---------|
| IR+Depth | IR, Depth | Left IR, Right IR, Depth |
| RGB | Color | RGB |
| IR | IR | Left IR, Right IR |
| Depth | Depth | Depth |

未启用的视图显示 "本模式无图像" 占位符。

## 依赖

- Qt 6.8.2 (推荐) 或 Qt 5.12+
- OpenGL 2.1+
- libjpeg (MJPEG 解码)
- VXL SDK

### Windows 环境

推荐使用 MSYS2 MinGW64 环境:

```bash
# 安装 Qt 6 和依赖
pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-cmake mingw-w64-x86_64-libjpeg-turbo
```

Qt 版本信息:
- **Qt 6.8.2** (MinGW 64-bit)
- 路径: `C:/msys64/mingw64/lib`

## 编译

### Windows (MSYS2 MinGW64)

```bash
cd demo
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

### macOS / Linux

```bash
cd demo
mkdir build && cd build
cmake ..
make -j4
```

### 运行

```bash
# Windows
./bin/vxl_demo.exe

# macOS
./bin/vxl_demo.app/Contents/MacOS/vxl_demo

# Linux
./bin/vxl_demo
```

## 源码结构

```
demo/
├── CMakeLists.txt      # CMake 构建配置
├── src/
│   ├── main.cpp        # 程序入口
│   ├── mainwindow.h    # 主窗口定义
│   ├── mainwindow.cpp  # 主窗口实现
│   ├── videowidget.h   # 视频控件定义
│   └── videowidget.cpp # 视频控件实现 (OpenGL)
```

## 核心代码说明

### MainWindow

主窗口类，负责：
- VXL SDK 初始化和清理
- 设备枚举和管理
- 传感器和流的创建/启动/停止
- 帧数据分发到各视图

关键成员：
```cpp
vxl_context_t *m_context;    // VXL 上下文
vxl_device_t  *m_device;     // 当前设备
DisplayMode    m_displayMode; // 显示模式
std::vector<StreamInfo> m_streams;  // 流列表
ViewInfo m_views[VIEW_COUNT];       // 四个视图
```

### VideoWidget

视频显示控件，基于 QOpenGLWidget：
- 使用 OpenGL 纹理显示 RGB 图像
- 支持保持宽高比缩放
- 支持占位符文字显示
- 线程安全的帧更新

关键方法：
```cpp
void updateFrame(const uint8_t *data, int width, int height);
void setPlaceholderText(const QString &text);
void clear();
```

### 帧回调处理

```cpp
// 静态回调函数 (从 libuvc 线程调用)
static void frameCallback(vxl_frame_t *frame, void *user_data);

// 帧处理逻辑
void handleFrame(vxl_frame_t *frame, int streamIndex)
{
    vxl_sensor_type_t sensorType;
    vxl_frame_get_sensor_type(frame, &sensorType);

    switch (sensorType) {
        case VXL_SENSOR_IR:
            // 分离双 IR 帧
            if (vxl_frame_is_dual_ir(frame)) {
                vxl_frame_split_ir(frame, &left, &right);
                // 更新左右 IR 视图
            }
            break;
        case VXL_SENSOR_DEPTH:
            // 更新 Depth 视图
            break;
        case VXL_SENSOR_COLOR:
            // 更新 RGB 视图
            break;
    }
}
```

### 格式转换

Demo 内部将各种格式转换为 RGB 显示：
- MJPEG: 使用 libjpeg 解码
- YUYV: 提取 Y 分量转灰度
- GRAY8: 直接扩展为 RGB
- Z16: 16位深度映射到灰度 (近白远黑)

## USB 权限

### macOS

需要签名和授权：
```xml
<!-- entitlements.plist -->
<key>com.apple.security.device.usb</key>
<true/>
<key>com.apple.security.device.camera</key>
<true/>
```

### Linux

添加 udev 规则：
```bash
# /etc/udev/rules.d/99-vxl.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="2bdf", MODE="0666"
```

然后重载：
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```
