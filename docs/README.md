# VXL SDK

VXL SDK 是一个用于访问深度相机的跨平台 C 语言库，支持 IR、Depth、RGB 多种数据流。

## 支持平台

- macOS (x86_64, arm64)
- Linux (x86_64, arm64, 嵌入式)
- Windows (x86_64)

## 支持设备

- VXL435 立体深度相机

## 目录结构

```
vxl-sdk-<platform>-v<version>/
├── include/          # 头文件
├── lib/              # 库文件
│   └── <platform>/   # 平台特定库 (libvxl.a)
├── docs/             # 文档
├── demo/             # Qt Demo 源码
└── examples/         # C 示例代码
```

## 依赖说明

libvxl.a 已包含以下库的代码：
- libuvc (UVC 协议实现)
- asos (OS 抽象层)
- vxl435_usb (VXL435 驱动)

运行时仍需系统安装 **libusb-1.0**：

```bash
# macOS
brew install libusb

# Ubuntu/Debian
sudo apt-get install libusb-1.0-0-dev

# Fedora
sudo dnf install libusb1-devel
```

## 快速开始

### 1. 编译示例

```bash
cd examples
gcc -o device_info device_info.c -I../include -L../lib/<platform> -lvxl -lusb-1.0
```

### 2. 运行

```bash
./device_info
```

## 基本使用流程

```c
#include "vxl.h"

// 帧回调函数
void frame_callback(vxl_frame_t *frame, void *user_data)
{
    uint32_t width, height;
    vxl_frame_get_width(frame, &width);
    vxl_frame_get_height(frame, &height);
    printf("收到帧: %dx%d\n", width, height);
}

int main()
{
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = NULL;
    vxl_stream_t *stream = NULL;
    vxl_profile_t *profile = NULL;

    // 1. 创建上下文
    vxl_context_create(&ctx);

    // 2. 获取并打开设备
    vxl_context_get_device(ctx, 0, &device);
    vxl_device_open(device);

    // 3. 获取传感器
    vxl_device_get_sensor(device, VXL_SENSOR_COLOR, &sensor);

    // 4. 获取 Profile 并创建流
    vxl_sensor_get_profile(sensor, 0, &profile);
    vxl_sensor_create_stream(sensor, profile, &stream);

    // 5. 开始采集
    vxl_stream_start(stream, frame_callback, NULL);

    // ... 等待数据 ...
    sleep(5);

    // 6. 停止并清理
    vxl_stream_stop(stream);
    vxl_stream_release(stream);
    vxl_profile_release(profile);
    vxl_sensor_release(sensor);
    vxl_device_close(device);
    vxl_device_release(device);
    vxl_context_destroy(ctx);

    return 0;
}
```

## 对象层次结构

```
Context ──► Device ──► Sensor ──► Profile
                          │
                          ├──► Option
                          │
                          └──► Stream ──► Frame
```

- **Context**: SDK 上下文，管理设备枚举
- **Device**: 物理设备，包含多个传感器
- **Sensor**: 传感器 (IR/Depth/Color)
- **Profile**: 流配置 (分辨率、帧率、格式)
- **Stream**: 数据流，产生帧数据
- **Frame**: 帧数据，包含图像和元数据

## 更多文档

- [API 参考](API.md)
- [Demo 说明](DEMO.md)
- [示例说明](EXAMPLES.md)
- [构建指南](BUILD.md)
- [部署指南](DEPLOY.md)
- [Linux 设备配置](LINUX.md)
- [Windows 驱动安装](WINDOWS.md)
- [更新日志](CHANGELOG.md)

## 技术支持

如有问题，请联系技术支持。
