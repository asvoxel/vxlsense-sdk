cmake_minimum_required(VERSION 3.10)
project(vxl_examples VERSION 1.0.0 LANGUAGES C)

# C99 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#=============================================================================
# 平台检测
#=============================================================================

if(APPLE)
    set(VXL_PLATFORM "macos")
elseif(WIN32)
    set(VXL_PLATFORM "windows")
else()
    set(VXL_PLATFORM "linux")
endif()

#=============================================================================
# SDK 路径设置 (相对于 examples 目录)
#=============================================================================

set(SDK_ROOT ${CMAKE_SOURCE_DIR}/..)
set(VXL_INCLUDE_DIR ${SDK_ROOT}/include)
set(VXL_LIB_DIR ${SDK_ROOT}/lib/${VXL_PLATFORM})

#=============================================================================
# 查找 VXL 库
#=============================================================================

# 检测库文件 (优先 Release，支持 Debug)
if(EXISTS "${VXL_LIB_DIR}/libasvxl.a")
    set(VXL_LIBRARY "${VXL_LIB_DIR}/libasvxl.a")
    message(STATUS "Found VXL library: ${VXL_LIBRARY}")
elseif(EXISTS "${VXL_LIB_DIR}/libasvxl_dbg.a")
    set(VXL_LIBRARY "${VXL_LIB_DIR}/libasvxl_dbg.a")
    message(STATUS "Found VXL library (debug): ${VXL_LIBRARY}")
else()
    message(FATAL_ERROR "VXL library not found in ${VXL_LIB_DIR}\n"
                        "Expected: libasvxl.a or libasvxl_dbg.a")
endif()

#=============================================================================
# 查找依赖库
#=============================================================================

# libusb
find_library(LIBUSB_LIBRARY
    NAMES usb-1.0 libusb-1.0
    PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
          C:/msys64/mingw64/lib
)
if(NOT LIBUSB_LIBRARY)
    message(FATAL_ERROR "libusb-1.0 not found")
endif()

# libjpeg
find_library(JPEG_LIBRARY
    NAMES jpeg libjpeg
    PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
          C:/msys64/mingw64/lib
)

#=============================================================================
# 平台链接库
#=============================================================================

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    set(PLATFORM_LIBRARIES
        ${IOKIT_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${SECURITY_LIBRARY}
        pthread
    )
elseif(WIN32)
    # Windows: DirectShow + COM libraries
    set(PLATFORM_LIBRARIES
        strmiids
        ole32
        oleaut32
        uuid
    )
else()
    # Linux
    set(PLATFORM_LIBRARIES pthread rt)
endif()

#=============================================================================
# 公共链接库
#=============================================================================

set(VXL_LIBRARIES
    ${VXL_LIBRARY}
    ${LIBUSB_LIBRARY}
    ${JPEG_LIBRARY}
    ${PLATFORM_LIBRARIES}
)

#=============================================================================
# 示例程序
#=============================================================================

# device_info - 设备信息查询
add_executable(device_info device_info.c)
target_include_directories(device_info PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(device_info PRIVATE ${VXL_LIBRARIES})

# low_level_api - 底层 API 示例
add_executable(low_level_api low_level_api.c)
target_include_directories(low_level_api PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(low_level_api PRIVATE ${VXL_LIBRARIES})

# hotplug_test - 热插拔测试
add_executable(hotplug_test hotplug_test.c)
target_include_directories(hotplug_test PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(hotplug_test PRIVATE ${VXL_LIBRARIES})

# rgbd_fusion - RGB-D 融合示例
add_executable(rgbd_fusion rgbd_fusion.c)
target_include_directories(rgbd_fusion PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(rgbd_fusion PRIVATE ${VXL_LIBRARIES})

# vxl615_option_control - VXL615 Option控制基础示例
add_executable(vxl615_option_control vxl615_option_control.c)
target_include_directories(vxl615_option_control PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(vxl615_option_control PRIVATE ${VXL_LIBRARIES})

# vxl615_depth_tuning - VXL615 深度后处理参数调节示例
add_executable(vxl615_depth_tuning vxl615_depth_tuning.c)
target_include_directories(vxl615_depth_tuning PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(vxl615_depth_tuning PRIVATE ${VXL_LIBRARIES})

# vxl615_ae_adjustment - VXL615 AE参数动态调整示例
add_executable(vxl615_ae_adjustment vxl615_ae_adjustment.c)
target_include_directories(vxl615_ae_adjustment PRIVATE ${VXL_INCLUDE_DIR})
target_link_libraries(vxl615_ae_adjustment PRIVATE ${VXL_LIBRARIES})

#=============================================================================
# macOS 签名 (获取 USB 权限)
#=============================================================================

if(APPLE AND EXISTS "${CMAKE_SOURCE_DIR}/entitlements.plist")
    foreach(target device_info low_level_api hotplug_test rgbd_fusion
                   vxl615_option_control vxl615_depth_tuning vxl615_ae_adjustment)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND codesign --force --sign -
                    --entitlements ${CMAKE_SOURCE_DIR}/entitlements.plist
                    $<TARGET_FILE:${target}>
            COMMENT "Signing ${target} with USB entitlements"
        )
    endforeach()
endif()

#=============================================================================
# 配置摘要
#=============================================================================

message(STATUS "")
message(STATUS "=== VXL Examples Configuration ===")
message(STATUS "Platform: ${VXL_PLATFORM}")
message(STATUS "VXL library: ${VXL_LIBRARY}")
message(STATUS "libusb: ${LIBUSB_LIBRARY}")
message(STATUS "libjpeg: ${JPEG_LIBRARY}")
message(STATUS "Basic Examples: device_info, low_level_api, hotplug_test, rgbd_fusion")
message(STATUS "VXL615 Examples: vxl615_option_control, vxl615_depth_tuning, vxl615_ae_adjustment")
message(STATUS "==================================")
message(STATUS "")
