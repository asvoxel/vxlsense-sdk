/**
 * @file vxl615_depth_tuning.c
 * @brief VXL615 深度后处理参数调节示例
 *
 * 演示如何调节深度后处理参数以优化深度图质量：
 * 1. NCC阈值调节 (匹配质量控制)
 * 2. Patch大小调节 (匹配窗口大小)
 * 3. 离群点去除
 * 4. 降噪参数调节
 * 5. 中值滤波
 * 6. 去畸变
 *
 * 编译: make vxl615_depth_tuning
 * 运行: ./bin/vxl615_depth_tuning
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vxl_context.h"
#include "vxl_device.h"
#include "vxl_sensor.h"
#include "vxl_types.h"
#include "vxl615_types.h"

/* 深度后处理参数配置 */
typedef struct {
    /* NCC阈值 [0-255]，越高质量越好但有效像素越少 */
    float ncc_threshold;

    /* Patch大小 [0-6]，影响匹配窗口大小 */
    float patch_size;

    /* 离群点去除 (0=关闭, 1=开启) */
    float outlier_removal;

    /* 降噪 */
    float denoise_enable;  /* 0=关闭, 1=开启 */
    float denoise_level;   /* [1-3]，级别越高降噪越强 */

    /* 中值滤波 */
    float median_filter;      /* 0=关闭, 1=开启 */
    float median_kernel_size; /* [3,5,7,9,11,13,15]，奇数 */

    /* 去畸变 (0=关闭, 1=开启) */
    float undistortion;
} depth_processing_config_t;

/* 预定义配置 */
static const depth_processing_config_t PRESET_HIGH_QUALITY = {
    .ncc_threshold = 200,  /* 高阈值，质量优先 */
    .patch_size = 4,       /* 较大窗口，更准确 */
    .outlier_removal = 1,  /* 启用 */
    .denoise_enable = 1,   /* 启用 */
    .denoise_level = 3,    /* 最强降噪 */
    .median_filter = 1,    /* 启用 */
    .median_kernel_size = 5,
    .undistortion = 1      /* 启用 */
};

static const depth_processing_config_t PRESET_BALANCED = {
    .ncc_threshold = 128,  /* 中等阈值，平衡质量和覆盖 */
    .patch_size = 3,       /* 标准窗口 */
    .outlier_removal = 1,  /* 启用 */
    .denoise_enable = 1,   /* 启用 */
    .denoise_level = 2,    /* 中等降噪 */
    .median_filter = 1,    /* 启用 */
    .median_kernel_size = 3,
    .undistortion = 1      /* 启用 */
};

static const depth_processing_config_t PRESET_FAST = {
    .ncc_threshold = 80,   /* 低阈值，覆盖优先 */
    .patch_size = 2,       /* 小窗口，速度快 */
    .outlier_removal = 0,  /* 禁用 */
    .denoise_enable = 0,   /* 禁用 */
    .denoise_level = 1,
    .median_filter = 0,    /* 禁用 */
    .median_kernel_size = 3,
    .undistortion = 0      /* 禁用 */
};

/*============================================================================
 * 辅助函数
 *============================================================================*/

static vxl_sensor_t* open_vxl615_sensor(vxl_context_t **out_ctx, vxl_device_t **out_device)
{
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = NULL;

    vxl_error_t err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "Failed to create context\n");
        return NULL;
    }

    size_t count = 0;
    err = vxl_context_get_device_count(ctx, &count);
    if (err != VXL_SUCCESS || count == 0) {
        fprintf(stderr, "No devices found\n");
        vxl_context_destroy(ctx);
        return NULL;
    }

    /* 查找VXL615 */
    for (size_t i = 0; i < count; i++) {
        err = vxl_context_get_device(ctx, i, &device);
        if (err != VXL_SUCCESS) continue;

        vxl_device_info_t info;
        err = vxl_device_get_info(device, &info);
        if (err == VXL_SUCCESS &&
            info.vendor_id == VXL615_VENDOR_ID &&
            info.product_id == VXL615_PRODUCT_ID) {

            err = vxl_device_open(device);
            if (err != VXL_SUCCESS) continue;

            /* 获取Depth传感器 (通常是sensor 1) */
            err = vxl_device_get_sensor(device, 1, &sensor);
            if (err != VXL_SUCCESS) {
                /* 尝试sensor 0 */
                err = vxl_device_get_sensor(device, 0, &sensor);
            }

            if (err == VXL_SUCCESS) {
                *out_ctx = ctx;
                *out_device = device;
                printf("Opened VXL615: %s\n", info.name);
                return sensor;
            }

            vxl_device_close(device);
        }
    }

    vxl_context_destroy(ctx);
    return NULL;
}

/*============================================================================
 * 配置应用函数
 *============================================================================*/

vxl_error_t apply_depth_processing_config(vxl_sensor_t *sensor,
                                           const depth_processing_config_t *config)
{
    vxl_error_t err;

    printf("Applying depth processing configuration...\n");

    /* NCC Threshold */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_NCC_THRESHOLD, config->ncc_threshold);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set NCC threshold\n");
        return err;
    }
    printf("  NCC threshold: %.0f\n", config->ncc_threshold);

    /* Patch Size */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_PATCH_SIZE, config->patch_size);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set patch size\n");
        return err;
    }
    printf("  Patch size: %.0f\n", config->patch_size);

    /* Outlier Removal */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_OUTLIER_REMOVAL, config->outlier_removal);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set outlier removal\n");
        return err;
    }
    printf("  Outlier removal: %s\n", config->outlier_removal > 0.5f ? "ON" : "OFF");

    /* Denoise */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_ENABLE, config->denoise_enable);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set denoise enable\n");
        return err;
    }
    printf("  Denoise: %s\n", config->denoise_enable > 0.5f ? "ON" : "OFF");

    if (config->denoise_enable > 0.5f) {
        err = vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_LEVEL, config->denoise_level);
        if (err != VXL_SUCCESS) {
            fprintf(stderr, "  Failed to set denoise level\n");
            return err;
        }
        printf("  Denoise level: %.0f\n", config->denoise_level);
    }

    /* Median Filter */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_FILTER, config->median_filter);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set median filter\n");
        return err;
    }
    printf("  Median filter: %s\n", config->median_filter > 0.5f ? "ON" : "OFF");

    if (config->median_filter > 0.5f) {
        err = vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_KERNEL_SIZE, config->median_kernel_size);
        if (err != VXL_SUCCESS) {
            fprintf(stderr, "  Failed to set median kernel size\n");
            return err;
        }
        printf("  Median kernel size: %.0f\n", config->median_kernel_size);
    }

    /* Undistortion */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_UNDISTORTION, config->undistortion);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set undistortion\n");
        return err;
    }
    printf("  Undistortion: %s\n", config->undistortion > 0.5f ? "ON" : "OFF");

    printf("Configuration applied successfully!\n");
    return VXL_SUCCESS;
}

void print_current_config(vxl_sensor_t *sensor)
{
    float value;

    printf("\nCurrent Depth Processing Configuration:\n");

    vxl_sensor_get_option(sensor, VXL615_OPTION_NCC_THRESHOLD, &value);
    printf("  NCC threshold: %.0f\n", value);

    vxl_sensor_get_option(sensor, VXL615_OPTION_PATCH_SIZE, &value);
    printf("  Patch size: %.0f\n", value);

    vxl_sensor_get_option(sensor, VXL615_OPTION_OUTLIER_REMOVAL, &value);
    printf("  Outlier removal: %s\n", value > 0.5f ? "ON" : "OFF");

    vxl_sensor_get_option(sensor, VXL615_OPTION_DENOISE_ENABLE, &value);
    printf("  Denoise: %s", value > 0.5f ? "ON" : "OFF");
    if (value > 0.5f) {
        vxl_sensor_get_option(sensor, VXL615_OPTION_DENOISE_LEVEL, &value);
        printf(" (level=%.0f)\n", value);
    } else {
        printf("\n");
    }

    vxl_sensor_get_option(sensor, VXL615_OPTION_MEDIAN_FILTER, &value);
    printf("  Median filter: %s", value > 0.5f ? "ON" : "OFF");
    if (value > 0.5f) {
        vxl_sensor_get_option(sensor, VXL615_OPTION_MEDIAN_KERNEL_SIZE, &value);
        printf(" (kernel=%.0f)\n", value);
    } else {
        printf("\n");
    }

    vxl_sensor_get_option(sensor, VXL615_OPTION_UNDISTORTION, &value);
    printf("  Undistortion: %s\n", value > 0.5f ? "ON" : "OFF");
}

/*============================================================================
 * 示例场景
 *============================================================================*/

void example_preset_configs(vxl_sensor_t *sensor)
{
    printf("\n=== Example: Preset Configurations ===\n");

    printf("\n1. HIGH QUALITY preset (质量优先，处理较慢):\n");
    apply_depth_processing_config(sensor, &PRESET_HIGH_QUALITY);
    sleep(1);

    printf("\n2. BALANCED preset (质量和速度平衡):\n");
    apply_depth_processing_config(sensor, &PRESET_BALANCED);
    sleep(1);

    printf("\n3. FAST preset (速度优先，质量一般):\n");
    apply_depth_processing_config(sensor, &PRESET_FAST);
    sleep(1);
}

void example_manual_tuning(vxl_sensor_t *sensor)
{
    printf("\n=== Example: Manual Parameter Tuning ===\n");

    printf("\nScenario: Tuning NCC threshold for different environments\n");

    /* 室内低纹理场景 - 降低NCC阈值增加覆盖 */
    printf("\n1. Indoor low-texture (indoor, plain walls):\n");
    printf("   -> Lower NCC threshold for more coverage\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_NCC_THRESHOLD, 80.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_PATCH_SIZE, 4.0f);  /* 较大窗口补偿 */
    print_current_config(sensor);

    sleep(2);

    /* 室外高纹理场景 - 提高NCC阈值保证质量 */
    printf("\n2. Outdoor high-texture (outdoor, rich details):\n");
    printf("   -> Higher NCC threshold for better quality\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_NCC_THRESHOLD, 180.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_PATCH_SIZE, 3.0f);
    print_current_config(sensor);

    sleep(2);

    /* 动态场景 - 减少后处理加快速度 */
    printf("\n3. Dynamic scene (fast moving objects):\n");
    printf("   -> Disable heavy processing for speed\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_ENABLE, 0.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_FILTER, 0.0f);
    print_current_config(sensor);
}

void example_noise_reduction(vxl_sensor_t *sensor)
{
    printf("\n=== Example: Noise Reduction Tuning ===\n");

    printf("\n1. No noise reduction:\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_ENABLE, 0.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_FILTER, 0.0f);
    print_current_config(sensor);

    sleep(2);

    printf("\n2. Light noise reduction (denoise level 1):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_ENABLE, 1.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_LEVEL, 1.0f);
    print_current_config(sensor);

    sleep(2);

    printf("\n3. Medium noise reduction (denoise level 2 + median 3x3):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_LEVEL, 2.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_FILTER, 1.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_KERNEL_SIZE, 3.0f);
    print_current_config(sensor);

    sleep(2);

    printf("\n4. Strong noise reduction (denoise level 3 + median 5x5):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_DENOISE_LEVEL, 3.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_MEDIAN_KERNEL_SIZE, 5.0f);
    print_current_config(sensor);
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main(int argc, char **argv)
{
    printf("VXL615 Depth Processing Tuning Example\n");
    printf("=======================================\n\n");

    /* 打开VXL615设备 */
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = open_vxl615_sensor(&ctx, &device);

    if (!sensor) {
        fprintf(stderr, "Failed to open VXL615 sensor\n");
        return EXIT_FAILURE;
    }

    /* 显示当前配置 */
    print_current_config(sensor);

    /* 运行示例 */
    example_preset_configs(sensor);
    example_manual_tuning(sensor);
    example_noise_reduction(sensor);

    printf("\n=== Tuning Guidelines ===\n");
    printf("NCC Threshold:\n");
    printf("  - Lower (50-100): More coverage, lower quality, noisier\n");
    printf("  - Medium (100-150): Balanced\n");
    printf("  - Higher (150-255): Better quality, less coverage, cleaner\n");
    printf("\nPatch Size:\n");
    printf("  - Smaller (1-2): Faster, less accurate, good for high-texture\n");
    printf("  - Medium (3-4): Balanced\n");
    printf("  - Larger (5-6): Slower, more accurate, good for low-texture\n");
    printf("\nDenoise Level:\n");
    printf("  - 1: Light smoothing\n");
    printf("  - 2: Medium smoothing\n");
    printf("  - 3: Strong smoothing (may lose details)\n");
    printf("\nMedian Kernel:\n");
    printf("  - 3: Fast, light smoothing\n");
    printf("  - 5: Medium smoothing\n");
    printf("  - 7-15: Strong smoothing (slower)\n");

    /* 清理 */
    vxl_sensor_release(sensor);
    vxl_device_close(device);
    vxl_context_destroy(ctx);

    printf("\nExample completed!\n");
    return EXIT_SUCCESS;
}
