# VXL SDK 构建指南

## 目录

- [依赖项](#依赖项)
- [库文件说明](#库文件说明)
- [链接方式](#链接方式)
- [编译示例](#编译示例)
- [编译 Demo](#编译-demo)
- [libusb 获取方式](#libusb-获取方式)
- [常见问题](#常见问题)

---

## 依赖项

### 运行时依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| libusb | 1.0.x | USB 通信库 (运行时需要) |

### Demo 额外依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| Qt | 5.12+ / 6.x | Demo 界面 |
| libjpeg | 任意 | MJPEG 解码 |

---

## 库文件说明

SDK 提供以下预编译库：

### libvxl.a / vxl.lib

VXL SDK 主库，已合并以下模块：

| 模块 | 说明 |
|------|------|
| vxl | VXL SDK 核心 |
| asos | OS 抽象层 |
| vxl435_usb | VXL435 设备驱动 |
| libuvc | UVC 协议实现 |

### libusb 库

SDK 同时提供 libusb 的静态库和动态库：

| 文件 | 类型 | 说明 |
|------|------|------|
| libusb-1.0.a | 静态库 | 推荐，无需额外部署 |
| libusb-1.0.dylib | 动态库 | macOS，需部署动态库 |
| libusb-1.0.so | 动态库 | Linux，需部署动态库 |

### 库文件位置

```
lib/
└── <platform>/
    ├── libvxl.a          # VXL SDK 主库
    ├── libusb-1.0.a      # libusb 静态库
    ├── libusb-1.0.dylib  # libusb 动态库 (macOS)
    └── libusb-1.0.so     # libusb 动态库 (Linux)
```

---

## 链接方式

### 方式一：静态链接 libusb (推荐)

使用 SDK 提供的静态 libusb，无需额外部署：

```bash
# macOS
gcc -o myapp myapp.c -I./include -L./lib/macos \
    -lvxl -lusb-1.0 \
    -framework IOKit -framework CoreFoundation

# Linux
gcc -o myapp myapp.c -I./include -L./lib/linux \
    -lvxl -lusb-1.0 -lpthread
```

### 方式二：动态链接 libusb (需部署)

使用 SDK 提供的动态 libusb，需要部署动态库：

```bash
# macOS (设置 rpath)
gcc -o myapp myapp.c -I./include -L./lib/macos \
    -lvxl -lusb-1.0 \
    -Wl,-rpath,@executable_path \
    -framework IOKit -framework CoreFoundation

# Linux (设置 rpath)
gcc -o myapp myapp.c -I./include -L./lib/linux \
    -lvxl -lusb-1.0 -lpthread \
    -Wl,-rpath,'$ORIGIN'
```

部署时需将动态库复制到可执行文件目录，详见 [DEPLOY.md](DEPLOY.md)。

### 方式三：使用系统 libusb

如系统已安装 libusb，可直接使用：

```bash
# macOS (需 brew install libusb)
gcc -o myapp myapp.c -I./include -L./lib/macos \
    -lvxl $(pkg-config --libs libusb-1.0) \
    -framework IOKit -framework CoreFoundation

# Linux (需 apt install libusb-1.0-0-dev)
gcc -o myapp myapp.c -I./include -L./lib/linux \
    -lvxl $(pkg-config --libs libusb-1.0) -lpthread
```

---

## 编译示例

### 方法一：直接编译

```bash
cd examples

# macOS
gcc -o device_info device_info.c \
    -I../include \
    -L../lib/macos -lvxl \
    -lusb-1.0 \
    -framework IOKit -framework CoreFoundation

# Linux
gcc -o device_info device_info.c \
    -I../include \
    -L../lib/linux -lvxl \
    -lusb-1.0 -lpthread -lrt
```

### 方法二：使用 CMake

创建 `examples/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(vxl_examples C)

set(CMAKE_C_STANDARD 99)

# 检测平台
if(APPLE)
    set(PLATFORM "macos")
    set(EXTRA_LIBS "-framework IOKit -framework CoreFoundation")
elseif(WIN32)
    set(PLATFORM "windows")
    set(EXTRA_LIBS "")
else()
    set(PLATFORM "linux")
    set(EXTRA_LIBS "pthread rt")
endif()

# SDK 路径
set(VXL_INCLUDE ${CMAKE_SOURCE_DIR}/../include)
set(VXL_LIB ${CMAKE_SOURCE_DIR}/../lib/${PLATFORM})

include_directories(${VXL_INCLUDE})
link_directories(${VXL_LIB})

# 示例程序
add_executable(device_info device_info.c)
target_link_libraries(device_info vxl usb-1.0 ${EXTRA_LIBS})

add_executable(low_level_api low_level_api.c)
target_link_libraries(low_level_api vxl usb-1.0 ${EXTRA_LIBS})
```

构建：

```bash
cd examples
mkdir build && cd build
cmake ..
make -j4
```

---

## 编译 Demo

Demo 需要 Qt 和 libjpeg。

### 安装依赖

```bash
# macOS
brew install qt@5 jpeg

# Ubuntu/Debian
sudo apt-get install qt5-default libjpeg-dev

# Fedora
sudo dnf install qt5-qtbase-devel libjpeg-devel
```

### 构建

```bash
cd demo
mkdir build && cd build

# 指定 Qt 路径 (如果需要)
# export CMAKE_PREFIX_PATH=/usr/local/opt/qt@5

cmake ..
make -j4
```

### 运行

```bash
# macOS
./bin/vxl_demo.app/Contents/MacOS/vxl_demo

# Linux
./bin/vxl_demo
```

---

## libusb 获取方式

SDK 已提供预编译的 libusb 库，通常无需额外安装。

### 方式一：使用 SDK 提供的库 (推荐)

SDK 的 `lib/<platform>/` 目录已包含 libusb 静态库和动态库，直接使用即可。

### 方式二：从 SDK 源码编译

SDK 的 `3rds/libusb-1.0.28/` 目录包含 libusb 源码：

```bash
cd 3rds/libusb-1.0.28
mkdir build && cd build
../configure --enable-static --enable-shared
make -j4
# 库文件在 libusb/.libs/ 目录
```

### 方式三：系统安装 (开发调试)

```bash
# macOS
brew install libusb

# Ubuntu/Debian
sudo apt-get install libusb-1.0-0-dev

# Fedora/RHEL
sudo dnf install libusb1-devel

# Windows (vcpkg)
vcpkg install libusb:x64-windows
```

---

## 常见问题

### 1. 找不到 libusb

```
fatal error: libusb.h: No such file or directory
```

解决：安装 libusb 开发包

```bash
# macOS
brew install libusb

# Ubuntu
sudo apt-get install libusb-1.0-0-dev
```

### 2. USB 权限问题 (Linux)

```
libusb: error [_get_usbfs_fd] libusb requires write access to USB device nodes
```

解决：添加 udev 规则

```bash
# 创建规则文件
sudo tee /etc/udev/rules.d/99-vxl.rules << 'EOF'
# VXL 深度相机
SUBSYSTEM=="usb", ATTR{idVendor}=="2bdf", MODE="0666"
EOF

# 重载规则
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### 3. macOS 签名问题

运行程序时提示无法访问 USB 设备。

解决：为程序签名

```bash
codesign --force --sign - --entitlements entitlements.plist ./myapp
```

entitlements.plist:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.device.usb</key>
    <true/>
</dict>
</plist>
```

### 4. Windows 驱动问题

Windows 需要安装 WinUSB 驱动。

推荐使用 Zadig 工具：
1. 下载 Zadig: https://zadig.akeo.ie/
2. 连接设备
3. 选择设备，安装 WinUSB 驱动

### 5. 链接错误

```
undefined reference to `vxl_context_create'
```

解决：检查库文件路径和链接顺序

```bash
# 正确顺序: 先目标文件，后库文件
gcc -o myapp myapp.c -I./include -L./lib/linux -lvxl -lusb-1.0

# 静态库完整链接 (如果有未解析符号)
gcc -o myapp myapp.c -I./include -Wl,--whole-archive ./lib/linux/libvxl.a -Wl,--no-whole-archive -lusb-1.0
```
