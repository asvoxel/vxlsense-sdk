# VXL SDK API 参考

## 目录

- [错误码](#错误码)
- [数据类型](#数据类型)
- [Context API](#context-api)
- [Device API](#device-api)
- [Sensor API](#sensor-api)
- [Stream API](#stream-api)
- [Frame API](#frame-api)
- [Profile API](#profile-api)

---

## 错误码

```c
typedef enum vxl_error {
    VXL_SUCCESS                      =  0,   // 成功
    VXL_ERROR_UNKNOWN                = -1,   // 未知错误
    VXL_ERROR_INVALID_PARAM          = -2,   // 无效参数
    VXL_ERROR_INVALID_HANDLE         = -3,   // 无效句柄
    VXL_ERROR_NO_DEVICE              = -4,   // 无设备
    VXL_ERROR_DEVICE_BUSY            = -5,   // 设备忙
    VXL_ERROR_DEVICE_NOT_OPEN        = -6,   // 设备未打开
    VXL_ERROR_NOT_SUPPORTED          = -7,   // 不支持
    VXL_ERROR_NO_MEM                 = -8,   // 内存不足
    VXL_ERROR_IO                     = -9,   // IO 错误
    VXL_ERROR_TIMEOUT                = -10,  // 超时
    VXL_ERROR_STREAM_NOT_STARTED     = -11,  // 流未启动
    VXL_ERROR_STREAM_ALREADY_STARTED = -12,  // 流已启动
    VXL_ERROR_USB                    = -17,  // USB 错误
    VXL_ERROR_BACKEND                = -18,  // 后端错误
} vxl_error_t;

// 获取错误描述
const char* vxl_error_string(vxl_error_t error);
```

---

## 数据类型

### 传感器类型

```c
typedef enum vxl_sensor_type {
    VXL_SENSOR_UNKNOWN = 0,
    VXL_SENSOR_COLOR   = 1,   // 彩色相机
    VXL_SENSOR_DEPTH   = 2,   // 深度相机
    VXL_SENSOR_IR      = 3,   // 红外相机
} vxl_sensor_type_t;
```

### 帧格式

```c
typedef enum vxl_format {
    VXL_FORMAT_UNKNOWN = 0,
    VXL_FORMAT_YUYV    = 1,   // YUYV/YUV422
    VXL_FORMAT_UYVY    = 2,
    VXL_FORMAT_RGB     = 3,   // 24-bit RGB
    VXL_FORMAT_BGR     = 4,   // 24-bit BGR
    VXL_FORMAT_MJPEG   = 5,   // Motion-JPEG
    VXL_FORMAT_GRAY8   = 7,   // 8-bit 灰度
    VXL_FORMAT_GRAY16  = 8,   // 16-bit 灰度
    VXL_FORMAT_Z16     = 10,  // 16-bit 深度
} vxl_format_t;
```

### 设备信息

```c
typedef struct vxl_device_info {
    char     name[256];           // 设备名称
    char     serial_number[256];  // 序列号
    char     manufacturer[256];   // 制造商
    char     fw_version[256];     // 固件版本
    uint16_t vendor_id;           // USB VID
    uint16_t product_id;          // USB PID
} vxl_device_info_t;
```

### 相机内参

```c
typedef struct vxl_intrinsics {
    float    fx, fy;          // 焦距
    float    cx, cy;          // 主点
    float    coeffs[5];       // 畸变系数 k1,k2,p1,p2,k3
    uint32_t width, height;   // 图像尺寸
} vxl_intrinsics_t;
```

### 回调函数

```c
// 帧回调
typedef void (*vxl_frame_cbfn)(vxl_frame_t *frame, void *user_data);

// 设备事件回调
typedef void (*vxl_device_event_cbfn)(
    const vxl_device_info_t *device_info,
    bool added,
    void *user_data
);
```

---

## Context API

### vxl_context_create

创建 VXL 上下文。

```c
vxl_error_t vxl_context_create(vxl_context_t **ctx);
```

**参数:**
- `ctx`: 输出上下文指针

**返回:** 错误码

### vxl_context_destroy

销毁上下文。

```c
vxl_error_t vxl_context_destroy(vxl_context_t *ctx);
```

### vxl_context_get_device_count

获取设备数量。

```c
vxl_error_t vxl_context_get_device_count(vxl_context_t *ctx, size_t *count);
```

### vxl_context_get_device

获取指定索引的设备。

```c
vxl_error_t vxl_context_get_device(vxl_context_t *ctx, size_t index, vxl_device_t **device);
```

**注意:** 返回的 device 需要调用 `vxl_device_release()` 释放。

### vxl_context_refresh_devices

刷新设备列表。

```c
vxl_error_t vxl_context_refresh_devices(vxl_context_t *ctx);
```

---

## Device API

### vxl_device_open / vxl_device_close

打开/关闭设备。

```c
vxl_error_t vxl_device_open(vxl_device_t *device);
vxl_error_t vxl_device_close(vxl_device_t *device);
```

### vxl_device_release

释放设备对象。

```c
vxl_error_t vxl_device_release(vxl_device_t *device);
```

### vxl_device_get_info

获取设备信息。

```c
vxl_error_t vxl_device_get_info(const vxl_device_t *device, vxl_device_info_t *info);
```

### vxl_device_get_sensor_count

获取传感器数量。

```c
vxl_error_t vxl_device_get_sensor_count(const vxl_device_t *device, size_t *count);
```

### vxl_device_get_sensor

获取指定类型的传感器。

```c
vxl_error_t vxl_device_get_sensor(vxl_device_t *device, vxl_sensor_type_t type, vxl_sensor_t **sensor);
```

### vxl_device_get_sensor_by_index

获取指定索引的传感器。

```c
vxl_error_t vxl_device_get_sensor_by_index(vxl_device_t *device, size_t index, vxl_sensor_t **sensor);
```

---

## Sensor API

### vxl_sensor_get_type

获取传感器类型。

```c
vxl_error_t vxl_sensor_get_type(const vxl_sensor_t *sensor, vxl_sensor_type_t *type);
```

### vxl_sensor_get_profile_count

获取 Profile 数量。

```c
vxl_error_t vxl_sensor_get_profile_count(const vxl_sensor_t *sensor, size_t *count);
```

### vxl_sensor_get_profile

获取指定索引的 Profile。

```c
vxl_error_t vxl_sensor_get_profile(vxl_sensor_t *sensor, size_t index, vxl_profile_t **profile);
```

### vxl_sensor_find_profile

查找匹配条件的 Profile。

```c
vxl_error_t vxl_sensor_find_profile(
    vxl_sensor_t *sensor,
    vxl_format_t format,    // VXL_FORMAT_ANY 表示任意
    uint32_t width,         // 0 表示任意
    uint32_t height,        // 0 表示任意
    uint32_t fps,           // 0 表示任意
    vxl_profile_t **profile
);
```

### vxl_sensor_create_stream

创建数据流。

```c
vxl_error_t vxl_sensor_create_stream(vxl_sensor_t *sensor, vxl_profile_t *profile, vxl_stream_t **stream);
```

### vxl_sensor_release

释放传感器对象。

```c
vxl_error_t vxl_sensor_release(vxl_sensor_t *sensor);
```

---

## Stream API

### vxl_stream_start

启动数据流。

```c
vxl_error_t vxl_stream_start(vxl_stream_t *stream, vxl_frame_cbfn callback, void *user_data);
```

**参数:**
- `stream`: 流对象
- `callback`: 帧回调函数
- `user_data`: 传递给回调的用户数据

### vxl_stream_stop

停止数据流。

```c
vxl_error_t vxl_stream_stop(vxl_stream_t *stream);
```

### vxl_stream_release

释放流对象。

```c
vxl_error_t vxl_stream_release(vxl_stream_t *stream);
```

---

## Frame API

### vxl_frame_get_data

获取帧数据指针。

```c
vxl_error_t vxl_frame_get_data(const vxl_frame_t *frame, const void **data);
```

### vxl_frame_get_data_size

获取帧数据大小。

```c
vxl_error_t vxl_frame_get_data_size(const vxl_frame_t *frame, size_t *size);
```

### vxl_frame_get_width / vxl_frame_get_height

获取帧尺寸。

```c
vxl_error_t vxl_frame_get_width(const vxl_frame_t *frame, uint32_t *width);
vxl_error_t vxl_frame_get_height(const vxl_frame_t *frame, uint32_t *height);
```

### vxl_frame_get_format

获取帧格式。

```c
vxl_error_t vxl_frame_get_format(const vxl_frame_t *frame, vxl_format_t *format);
```

### vxl_frame_get_sensor_type

获取帧来源传感器类型。

```c
vxl_error_t vxl_frame_get_sensor_type(const vxl_frame_t *frame, vxl_sensor_type_t *sensor_type);
```

### vxl_frame_get_timestamp

获取帧时间戳。

```c
vxl_error_t vxl_frame_get_timestamp(const vxl_frame_t *frame, uint64_t *timestamp_us);
```

### vxl_frame_split_ir

分离双 IR 帧为左右两帧。

```c
vxl_error_t vxl_frame_split_ir(const vxl_frame_t *src, vxl_frame_t **left, vxl_frame_t **right);
```

**说明:** VXL435 将左右 IR 合并在一个帧中传输。使用此函数分离。

### vxl_frame_is_dual_ir

检查是否为双 IR 帧。

```c
bool vxl_frame_is_dual_ir(const vxl_frame_t *frame);
```

### vxl_frame_release

释放帧引用。

```c
vxl_error_t vxl_frame_release(vxl_frame_t *frame);
```

---

## Profile API

### vxl_profile_get_format

获取 Profile 格式。

```c
vxl_error_t vxl_profile_get_format(const vxl_profile_t *profile, vxl_format_t *format);
```

### vxl_profile_get_resolution

获取 Profile 分辨率。

```c
vxl_error_t vxl_profile_get_resolution(const vxl_profile_t *profile, uint32_t *width, uint32_t *height);
```

### vxl_profile_get_fps

获取 Profile 帧率。

```c
vxl_error_t vxl_profile_get_fps(const vxl_profile_t *profile, uint32_t *fps);
```

### vxl_profile_release

释放 Profile 对象。

```c
vxl_error_t vxl_profile_release(vxl_profile_t *profile);
```
