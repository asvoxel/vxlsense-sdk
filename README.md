# VxlSense SDK

VxlSense depth camera SDK — prebuilt binaries and headers.

## Usage

### With VxlROS2

```bash
cd /path/to/VxlROS2/
git clone https://github.com/asvoxel/vxlsense-sdk.git vxlsense-sdk
colcon build
```

### Standalone

```bash
export VXL_SDK_DIR=/path/to/vxlsense-sdk
gcc -o demo demo.c -I${VXL_SDK_DIR}/include -L${VXL_SDK_DIR}/lib/linux -lasvxl -lusb-1.0 -lpthread
```

## Structure

```
vxlsense-sdk/
├── include/          # C/C++ API headers
├── lib/
│   ├── linux/        # Linux static library
│   └── darwin/       # macOS static library
├── examples/         # Example programs
├── docs/             # Documentation
├── VERSION           # Build metadata
└── LICENSE
```

## Links

- VxlROS2: https://github.com/asvoxel/vxlros2
- Website: https://asvoxel.com
