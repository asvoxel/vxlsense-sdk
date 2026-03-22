/**
 * @file vxl615_option_control.c
 * @brief VXL615 Option控制基础示例
 *
 * 演示如何：
 * 1. 检查Option支持
 * 2. 获取Option范围
 * 3. 读取和设置Option值
 * 4. 处理只读Option
 *
 * 编译: make vxl615_option_control
 * 运行: ./bin/vxl615_option_control
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vxl_context.h"
#include "vxl_device.h"
#include "vxl_sensor.h"
#include "vxl_types.h"
#include "vxl615_types.h"

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 查找并打开第一个VXL615设备
 */
static vxl_sensor_t* open_vxl615_sensor(vxl_context_t **out_ctx, vxl_device_t **out_device)
{
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = NULL;

    /* 创建上下文 */
    vxl_error_t err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "Failed to create context: %d\n", err);
        return NULL;
    }

    /* 获取设备列表 */
    size_t count = 0;
    err = vxl_context_get_device_count(ctx, &count);
    if (err != VXL_SUCCESS || count == 0) {
        fprintf(stderr, "No devices found\n");
        vxl_context_destroy(ctx);
        return NULL;
    }

    printf("Found %zu device(s)\n", count);

    /* 查找VXL615 */
    for (size_t i = 0; i < count; i++) {
        err = vxl_context_get_device(ctx, i, &device);
        if (err != VXL_SUCCESS) continue;

        vxl_device_info_t info;
        err = vxl_device_get_info(device, &info);
        if (err != VXL_SUCCESS) continue;

        printf("Device %zu: %s (VID:PID = %04X:%04X)\n",
               i, info.name, info.vendor_id, info.product_id);

        /* 检查是否是VXL615 */
        if (info.vendor_id == VXL615_VENDOR_ID &&
            info.product_id == VXL615_PRODUCT_ID) {
            printf("Found VXL615: %s\n", info.name);

            /* 打开设备 */
            err = vxl_device_open(device);
            if (err != VXL_SUCCESS) {
                fprintf(stderr, "Failed to open device: %d\n", err);
                continue;
            }

            /* 获取第一个传感器 */
            err = vxl_device_get_sensor(device, 0, &sensor);
            if (err != VXL_SUCCESS) {
                fprintf(stderr, "Failed to get sensor: %d\n", err);
                vxl_device_close(device);
                continue;
            }

            *out_ctx = ctx;
            *out_device = device;
            return sensor;
        }
    }

    fprintf(stderr, "No VXL615 device found\n");
    vxl_context_destroy(ctx);
    return NULL;
}

/*============================================================================
 * 示例：检查Option支持
 *============================================================================*/

void example_check_option_support(vxl_sensor_t *sensor)
{
    printf("\n=== Example 1: Check Option Support ===\n");

    struct {
        vxl_option_t option;
        const char *name;
    } options[] = {
        { VXL_OPTION_IR_ENABLE, "IR_ENABLE" },
        { VXL615_OPTION_NCC_THRESHOLD, "NCC_THRESHOLD" },
        { VXL615_OPTION_DENOISE_ENABLE, "DENOISE_ENABLE" },
        { VXL615_OPTION_2D_FMEAN_TARGET, "2D_FMEAN_TARGET" },
        { VXL615_OPTION_3D_GAIN_MIN, "3D_GAIN_MIN" },
    };

    for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
        bool supported = false;
        vxl_error_t err = vxl_sensor_is_option_supported(sensor, options[i].option, &supported);

        if (err == VXL_SUCCESS) {
            printf("  %-25s: %s\n", options[i].name, supported ? "Supported" : "Not supported");
        } else {
            printf("  %-25s: Error checking support (%d)\n", options[i].name, err);
        }
    }
}

/*============================================================================
 * 示例：获取Option范围
 *============================================================================*/

void example_get_option_range(vxl_sensor_t *sensor)
{
    printf("\n=== Example 2: Get Option Range ===\n");

    struct {
        vxl_option_t option;
        const char *name;
    } options[] = {
        { VXL615_OPTION_NCC_THRESHOLD, "NCC_THRESHOLD" },
        { VXL615_OPTION_PATCH_SIZE, "PATCH_SIZE" },
        { VXL615_OPTION_DENOISE_LEVEL, "DENOISE_LEVEL" },
        { VXL615_OPTION_MEDIAN_KERNEL_SIZE, "MEDIAN_KERNEL_SIZE" },
    };

    for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
        vxl_option_range_t range;
        vxl_error_t err = vxl_sensor_get_option_range(sensor, options[i].option, &range);

        if (err == VXL_SUCCESS) {
            printf("  %-25s: [%.0f - %.0f] step=%.0f default=%.0f\n",
                   options[i].name, range.min, range.max, range.step, range.def);
        } else {
            printf("  %-25s: Error getting range (%d)\n", options[i].name, err);
        }
    }
}

/*============================================================================
 * 示例：读取和设置Option
 *============================================================================*/

void example_get_set_option(vxl_sensor_t *sensor)
{
    printf("\n=== Example 3: Get and Set Option ===\n");

    /* 1. NCC Threshold */
    printf("\nNCC Threshold:\n");
    float value = 0.0f;
    vxl_error_t err = vxl_sensor_get_option(sensor, VXL615_OPTION_NCC_THRESHOLD, &value);
    if (err == VXL_SUCCESS) {
        printf("  Current value: %.0f\n", value);

        /* 设置新值 */
        float new_value = 150.0f;
        err = vxl_sensor_set_option(sensor, VXL615_OPTION_NCC_THRESHOLD, new_value);
        if (err == VXL_SUCCESS) {
            printf("  Set to: %.0f\n", new_value);

            /* 读取验证 */
            err = vxl_sensor_get_option(sensor, VXL615_OPTION_NCC_THRESHOLD, &value);
            if (err == VXL_SUCCESS) {
                printf("  Read back: %.0f\n", value);
            }

            /* 恢复原值 (演示完成后恢复) */
            // vxl_sensor_set_option(sensor, VXL615_OPTION_NCC_THRESHOLD, original_value);
        }
    }

    /* 2. Boolean Option - Denoise Enable */
    printf("\nDenoise Enable (Boolean):\n");
    err = vxl_sensor_get_option(sensor, VXL615_OPTION_DENOISE_ENABLE, &value);
    if (err == VXL_SUCCESS) {
        printf("  Current value: %s\n", value > 0.5f ? "ON" : "OFF");

        /* 切换状态 */
        float new_state = (value > 0.5f) ? 0.0f : 1.0f;
        err = vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_ENABLE, new_state);
        if (err == VXL_SUCCESS) {
            printf("  Set to: %s\n", new_state > 0.5f ? "ON" : "OFF");

            /* 读取验证 */
            err = vxl_sensor_get_option(sensor, VXL615_OPTION_DENOISE_ENABLE, &value);
            if (err == VXL_SUCCESS) {
                printf("  Read back: %s\n", value > 0.5f ? "ON" : "OFF");
            }
        }
    }
}

/*============================================================================
 * 示例：处理只读Option
 *============================================================================*/

void example_readonly_option(vxl_sensor_t *sensor)
{
    printf("\n=== Example 4: Read-only Options ===\n");

    /* 只读Option: 当前AE状态 */
    struct {
        vxl_option_t option;
        const char *name;
    } readonly_options[] = {
        { VXL615_OPTION_2D_FMEAN_CURRENT, "2D_FMEAN_CURRENT" },
        { VXL615_OPTION_2D_GAIN_CURRENT, "2D_GAIN_CURRENT" },
        { VXL615_OPTION_2D_EXPOSURE_CURRENT, "2D_EXPOSURE_CURRENT" },
    };

    printf("\nReading current AE status (read-only):\n");
    for (size_t i = 0; i < sizeof(readonly_options) / sizeof(readonly_options[0]); i++) {
        float value = 0.0f;
        vxl_error_t err = vxl_sensor_get_option(sensor, readonly_options[i].option, &value);
        if (err == VXL_SUCCESS) {
            printf("  %-25s: %.0f\n", readonly_options[i].name, value);
        }
    }

    /* 尝试设置只读Option (应该失败) */
    printf("\nTrying to set read-only option (should fail):\n");
    vxl_error_t err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_CURRENT, 100.0f);
    if (err != VXL_SUCCESS) {
        printf("  Set 2D_GAIN_CURRENT: Failed as expected (error %d)\n", err);
        printf("  This is correct behavior - read-only options cannot be set\n");
    } else {
        printf("  Set 2D_GAIN_CURRENT: Unexpectedly succeeded (this should not happen)\n");
    }
}

/*============================================================================
 * 示例：批量读取相关参数
 *============================================================================*/

void example_batch_read_params(vxl_sensor_t *sensor)
{
    printf("\n=== Example 5: Batch Read Related Parameters ===\n");

    /* 读取2D AE完整参数 */
    printf("\n2D Auto Exposure Parameters:\n");

    float fmean_target, fmean_boundary, fmean_current;
    float gain_min, gain_max, gain_current;
    float exp_min, exp_mid, exp_max, exp_current;

    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_FMEAN_TARGET, &fmean_target);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_FMEAN_BOUNDARY, &fmean_boundary);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_FMEAN_CURRENT, &fmean_current);

    printf("  Fmean: target=%.0f, boundary=%.0f, current=%.0f\n",
           fmean_target, fmean_boundary, fmean_current);

    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_GAIN_MIN, &gain_min);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_GAIN_MAX, &gain_max);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_GAIN_CURRENT, &gain_current);

    printf("  Gain: min=%.0f, max=%.0f, current=%.0f\n",
           gain_min, gain_max, gain_current);

    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_EXPOSURE_MIN, &exp_min);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_EXPOSURE_MID, &exp_mid);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_EXPOSURE_MAX, &exp_max);
    vxl_sensor_get_option(sensor, VXL615_OPTION_2D_EXPOSURE_CURRENT, &exp_current);

    printf("  Exposure: min=%.0f, mid=%.0f, max=%.0f, current=%.0f\n",
           exp_min, exp_mid, exp_max, exp_current);
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main(int argc, char **argv)
{
    printf("VXL615 Option Control Example\n");
    printf("==============================\n");

    /* 打开VXL615设备 */
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = open_vxl615_sensor(&ctx, &device);

    if (!sensor) {
        fprintf(stderr, "Failed to open VXL615 sensor\n");
        return EXIT_FAILURE;
    }

    /* 运行示例 */
    example_check_option_support(sensor);
    example_get_option_range(sensor);
    example_get_set_option(sensor);
    example_readonly_option(sensor);
    example_batch_read_params(sensor);

    printf("\n=== Summary ===\n");
    printf("Demonstrated:\n");
    printf("  1. Checking Option support\n");
    printf("  2. Getting Option ranges\n");
    printf("  3. Reading and setting Options\n");
    printf("  4. Handling read-only Options\n");
    printf("  5. Batch reading related parameters\n");

    /* 清理 */
    vxl_sensor_release(sensor);
    vxl_device_close(device);
    vxl_context_destroy(ctx);

    printf("\nExample completed successfully!\n");
    return EXIT_SUCCESS;
}
