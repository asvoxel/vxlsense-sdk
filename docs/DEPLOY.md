# VXL SDK 部署指南

本文档说明如何部署基于 VXL SDK 开发的应用程序。

## 目录

- [部署方式选择](#部署方式选择)
- [静态链接部署](#静态链接部署)
- [动态链接部署](#动态链接部署)
- [平台特定说明](#平台特定说明)
- [常见问题](#常见问题)

---

## 部署方式选择

| 方式 | 优点 | 缺点 | 推荐场景 |
|------|------|------|---------|
| 静态链接 | 单文件，无依赖 | 文件较大，无法更新库 | 嵌入式、独立应用 |
| 动态链接 | 文件小，可更新库 | 需部署动态库 | 桌面应用、需要更新 |

**推荐**：优先使用静态链接，简化部署流程。

---

## 静态链接部署

使用静态链接时，所有库代码都编译进可执行文件，部署只需复制可执行文件即可。

### 编译命令

```bash
# macOS
gcc -o myapp myapp.c -I./include -L./lib/macos \
    -lvxl -lusb-1.0 \
    -framework IOKit -framework CoreFoundation

# Linux
gcc -o myapp myapp.c -I./include -L./lib/linux \
    -lvxl -lusb-1.0 -lpthread
```

### 部署文件

```
部署目录/
└── myapp              # 可执行文件 (单文件即可)
```

---

## 动态链接部署

使用动态链接时，需要将动态库一同部署。

### 编译命令

```bash
# macOS (设置 rpath 为可执行文件目录)
gcc -o myapp myapp.c -I./include -L./lib/macos \
    -lvxl -lusb-1.0 \
    -Wl,-rpath,@executable_path \
    -framework IOKit -framework CoreFoundation

# Linux (设置 rpath 为可执行文件目录)
gcc -o myapp myapp.c -I./include -L./lib/linux \
    -lvxl -lusb-1.0 -lpthread \
    -Wl,-rpath,'$ORIGIN'
```

### 部署文件

```
# macOS
部署目录/
├── myapp                   # 可执行文件
├── libusb-1.0.dylib        # 符号链接
└── libusb-1.0.0.dylib      # 实际动态库

# Linux
部署目录/
├── myapp                   # 可执行文件
├── libusb-1.0.so           # 符号链接
└── libusb-1.0.so.0         # 实际动态库
```

### 部署脚本示例

```bash
#!/bin/bash
# deploy.sh - 部署脚本示例

APP_NAME="myapp"
DEPLOY_DIR="./deploy"
SDK_LIB="./lib/macos"  # 或 ./lib/linux

# 创建部署目录
mkdir -p "${DEPLOY_DIR}"

# 复制可执行文件
cp "${APP_NAME}" "${DEPLOY_DIR}/"

# 复制动态库 (macOS)
if [[ "$(uname)" == "Darwin" ]]; then
    cp "${SDK_LIB}/libusb-1.0.0.dylib" "${DEPLOY_DIR}/"
    cd "${DEPLOY_DIR}"
    ln -sf libusb-1.0.0.dylib libusb-1.0.dylib
fi

# 复制动态库 (Linux)
if [[ "$(uname)" == "Linux" ]]; then
    cp "${SDK_LIB}/libusb-1.0.so.0" "${DEPLOY_DIR}/"
    cd "${DEPLOY_DIR}"
    ln -sf libusb-1.0.so.0 libusb-1.0.so
fi

echo "部署完成: ${DEPLOY_DIR}/"
```

---

## 平台特定说明

### macOS

#### USB 权限

macOS 需要应用程序签名才能访问 USB 设备：

1. 创建 `entitlements.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
    "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.device.usb</key>
    <true/>
</dict>
</plist>
```

2. 签名应用:

```bash
codesign --force --sign - --entitlements entitlements.plist ./myapp
```

#### 动态库路径

macOS 查找动态库的顺序：
1. `@executable_path` - 可执行文件目录
2. `@loader_path` - 加载模块目录
3. `@rpath` - 运行时搜索路径
4. `/usr/local/lib`, `/usr/lib`

推荐使用 `@executable_path`，将动态库放在可执行文件同目录。

#### 验证动态库依赖

```bash
otool -L ./myapp
```

### Linux

#### USB 权限

Linux 需要 udev 规则授予 USB 访问权限：

1. 创建 `/etc/udev/rules.d/99-vxl.rules`:

```
# VXL 深度相机
SUBSYSTEM=="usb", ATTR{idVendor}=="2bdf", MODE="0666"
```

2. 重载规则:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

#### 动态库路径

Linux 查找动态库的顺序：
1. `$ORIGIN` - 可执行文件目录 (需在 rpath 中设置)
2. `LD_LIBRARY_PATH` 环境变量
3. `/etc/ld.so.conf` 配置的路径
4. `/lib`, `/usr/lib`

推荐使用 `$ORIGIN`，将动态库放在可执行文件同目录。

#### 验证动态库依赖

```bash
ldd ./myapp
```

### Windows

#### 动态库部署

Windows 将 DLL 放在可执行文件同目录即可：

```
部署目录/
├── myapp.exe
└── libusb-1.0.dll
```

#### USB 驱动

Windows 需要安装 WinUSB 驱动：
1. 下载 Zadig: https://zadig.akeo.ie/
2. 连接设备
3. 选择设备，安装 WinUSB 驱动

---

## 常见问题

### 1. dyld: Library not loaded (macOS)

```
dyld: Library not loaded: libusb-1.0.dylib
```

**原因**: 找不到动态库

**解决**:
- 确保动态库在可执行文件同目录
- 检查编译时是否设置了 rpath: `-Wl,-rpath,@executable_path`

### 2. error while loading shared libraries (Linux)

```
error while loading shared libraries: libusb-1.0.so: cannot open shared object file
```

**原因**: 找不到动态库

**解决**:
- 确保动态库在可执行文件同目录
- 检查编译时是否设置了 rpath: `-Wl,-rpath,'$ORIGIN'`
- 或设置环境变量: `export LD_LIBRARY_PATH=.`

### 3. USB 设备访问被拒绝

**macOS**: 检查应用是否签名，是否有 USB 授权
**Linux**: 检查 udev 规则是否配置正确
**Windows**: 检查是否安装 WinUSB 驱动

### 4. 静态链接时找不到符号

```
undefined reference to `libusb_xxx'
```

**解决**: 确保链接顺序正确，库在后面

```bash
gcc -o myapp myapp.c -lvxl -lusb-1.0  # 正确
gcc -lvxl -lusb-1.0 -o myapp myapp.c  # 错误
```

### 5. 如何确认使用的是静态库还是动态库

```bash
# macOS
otool -L ./myapp | grep libusb
# 如果输出为空，说明是静态链接

# Linux
ldd ./myapp | grep libusb
# 如果输出为空，说明是静态链接
```
