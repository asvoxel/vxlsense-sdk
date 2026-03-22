# VXL SDK 示例代码说明

## 示例列表

| 示例 | 说明 | 难度 |
|------|------|------|
| device_info.c | 设备枚举和信息查询 | 入门 |
| low_level_api.c | 完整的流控制示例 | 进阶 |

---

## device_info.c

### 功能

展示 VXL SDK 的设备枚举和信息查询功能：

1. 枚举所有连接的设备
2. 打印设备详细信息 (名称、序列号、VID/PID)
3. 列出每个设备的传感器和支持的 Profile
4. 打印相机参数 (内参/外参/深度参数)
5. 同时打开多个传感器并统计帧信息

### 编译

```bash
# macOS
gcc -o device_info device_info.c -I../include -L../lib/macos \
    -lvxl -lusb-1.0 -framework IOKit -framework CoreFoundation

# Linux
gcc -o device_info device_info.c -I../include -L../lib/linux \
    -lvxl -lusb-1.0 -lpthread

# Windows (MSVC)
cl device_info.c /I..\include /link ..\lib\windows\vxl.lib libusb-1.0.lib
```

### 运行

```bash
./device_info
```

### 输出示例

```
============================================================
  VXL SDK Device Info Test
============================================================

--- Device List ---
Found 1 device(s)

Device 0:
  Name: VXL435
  Serial: 12345678
  Manufacturer: ASFLY
  VID/PID: 2bdf:0001

--- Sensors ---
  [0] IR    - 3 profiles
  [1] DEPTH - 1 profiles
  [2] COLOR - 2 profiles

--- IR Profiles ---
  [0] GRAY8  640x800 @ 30fps
  [1] YUYV   640x400 @ 30fps
  ...

--- Stream Test (5 seconds) ---
  IR:    150 frames, 76.8 MB, 30.0 fps
  DEPTH: 150 frames, 38.4 MB, 30.0 fps
  COLOR: 150 frames, 23.1 MB, 30.0 fps
```

### 核心代码片段

```c
// 枚举设备
vxl_context_create(&ctx);
vxl_context_get_device_count(ctx, &count);

for (size_t i = 0; i < count; i++) {
    vxl_device_t *device;
    vxl_context_get_device(ctx, i, &device);

    vxl_device_info_t info;
    vxl_device_get_info(device, &info);
    printf("Device: %s, Serial: %s\n", info.name, info.serial_number);

    vxl_device_release(device);
}
```

---

## low_level_api.c

### 功能

展示完整的数据流控制：

1. 枚举 USB 设备
2. 初始化并打开设备
3. 测试指定传感器 (IR/Depth/Color)
4. 使用回调模式接收帧
5. 输出帧类型统计
6. 可选保存帧到文件

### 命令行选项

```
./low_level_api [选项]

选项:
  --ir      仅测试 IR 传感器
  --depth   仅测试 Depth 传感器
  --color   仅测试 Color 传感器
  --all     测试所有传感器 (默认)
  --save    保存帧到文件
  --time N  采集时间 (秒，默认 5)
```

### 编译

```bash
# macOS
gcc -o low_level_api low_level_api.c -I../include -L../lib/macos \
    -lvxl -lusb-1.0 -framework IOKit -framework CoreFoundation

# Linux
gcc -o low_level_api low_level_api.c -I../include -L../lib/linux \
    -lvxl -lusb-1.0 -lpthread

# Windows (MSVC)
cl low_level_api.c /I..\include /link ..\lib\windows\vxl.lib libusb-1.0.lib
```

### 运行示例

```bash
# 测试 IR 传感器，采集 10 秒
./low_level_api --ir --time 10

# 测试所有传感器并保存帧
./low_level_api --all --save

# 仅测试深度
./low_level_api --depth
```

### 输出示例

```
------------------------------------------------------------
[Step 1] Enumerate devices
------------------------------------------------------------
Found 1 device(s)

------------------------------------------------------------
[Step 2] Open device
------------------------------------------------------------
Device opened: VXL435

------------------------------------------------------------
[Step 3] Start IR stream
------------------------------------------------------------
Profile: GRAY8 640x800 @ 30fps
Stream started

------------------------------------------------------------
[Step 4] Capturing frames (5 seconds)
------------------------------------------------------------
Frame 1: 512000 bytes, IR
Frame 2: 512000 bytes, IR
...

------------------------------------------------------------
[Step 5] Statistics
------------------------------------------------------------
Total frames: 150
  IR:    150
  Depth: 0
  Color: 0
Average FPS: 30.0
Total data: 76.8 MB
```

### 核心代码片段

```c
// 帧回调函数
void frame_callback(vxl_frame_t *frame, void *user_data)
{
    uint32_t width, height;
    size_t size;
    vxl_sensor_type_t type;

    vxl_frame_get_width(frame, &width);
    vxl_frame_get_height(frame, &height);
    vxl_frame_get_data_size(frame, &size);
    vxl_frame_get_sensor_type(frame, &type);

    // 更新统计
    g_stats.frame_count++;
    g_stats.total_bytes += size;

    // 按类型分类
    switch (type) {
        case VXL_SENSOR_IR:    g_stats.ir_count++; break;
        case VXL_SENSOR_DEPTH: g_stats.depth_count++; break;
        case VXL_SENSOR_COLOR: g_stats.color_count++; break;
    }

    // 可选：保存帧
    if (g_stats.save_frames && g_stats.frame_count % SAVE_FRAME_INTERVAL == 0) {
        save_frame_to_file(frame);
    }
}

// 启动流
vxl_stream_start(stream, frame_callback, &g_stats);
sleep(capture_time);
vxl_stream_stop(stream);
```

---

## 编写自己的程序

### 最小示例

```c
#include "vxl.h"
#include <stdio.h>

void my_callback(vxl_frame_t *frame, void *user)
{
    uint32_t w, h;
    vxl_frame_get_width(frame, &w);
    vxl_frame_get_height(frame, &h);
    printf("Frame: %ux%u\n", w, h);
}

int main()
{
    vxl_context_t *ctx;
    vxl_device_t *dev;
    vxl_sensor_t *sensor;
    vxl_stream_t *stream;
    vxl_profile_t *profile;

    vxl_context_create(&ctx);
    vxl_context_get_device(ctx, 0, &dev);
    vxl_device_open(dev);

    vxl_device_get_sensor(dev, VXL_SENSOR_IR, &sensor);
    vxl_sensor_get_profile(sensor, 0, &profile);
    vxl_sensor_create_stream(sensor, profile, &stream);

    vxl_stream_start(stream, my_callback, NULL);
    sleep(5);
    vxl_stream_stop(stream);

    // 清理
    vxl_stream_release(stream);
    vxl_profile_release(profile);
    vxl_sensor_release(sensor);
    vxl_device_close(dev);
    vxl_device_release(dev);
    vxl_context_destroy(ctx);

    return 0;
}
```

### CMakeLists.txt 模板

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_vxl_app C)

set(CMAKE_C_STANDARD 99)

# VXL SDK 路径
set(VXL_SDK_DIR "/path/to/vxl-sdk")

# 检测平台
if(APPLE)
    set(PLATFORM "macos")
    set(EXTRA_LIBS "-framework IOKit -framework CoreFoundation")
elseif(WIN32)
    set(PLATFORM "windows")
    set(EXTRA_LIBS "")
else()
    set(PLATFORM "linux")
    set(EXTRA_LIBS "pthread")
endif()

include_directories(${VXL_SDK_DIR}/include)
link_directories(${VXL_SDK_DIR}/lib/${PLATFORM})

add_executable(my_app main.c)
target_link_libraries(my_app vxl usb-1.0 ${EXTRA_LIBS})
```
