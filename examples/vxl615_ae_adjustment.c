/**
 * @file vxl615_ae_adjustment.c
 * @brief VXL615 AE (自动曝光) 参数动态调整示例
 *
 * 演示如何调节AE参数以优化图像曝光：
 * 1. Fmean目标调节 (目标亮度控制)
 * 2. Gain范围控制 (增益限制)
 * 3. Exposure范围控制 (曝光时间限制)
 * 4. 监控当前AE状态
 * 5. 2D (NIR) 和 3D (Depth) AE独立控制
 *
 * 编译: make vxl615_ae_adjustment
 * 运行: ./bin/vxl615_ae_adjustment
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vxl_context.h"
#include "vxl_device.h"
#include "vxl_sensor.h"
#include "vxl_types.h"
#include "vxl615_types.h"

/* AE参数配置 */
typedef struct {
    /* Fmean (亮度目标) */
    float fmean_target;    /* 目标亮度值 [0-65535] */
    float fmean_boundary;  /* 边界容差 [0-255] */

    /* Gain (增益) 范围 */
    float gain_min;        /* 最小增益 */
    float gain_max;        /* 最大增益 */

    /* Exposure (曝光时间) 范围 */
    float exposure_min;    /* 最小曝光时间 */
    float exposure_mid;    /* 中间曝光时间 (参考值) */
    float exposure_max;    /* 最大曝光时间 */
} ae_config_t;

/* AE状态 (只读) */
typedef struct {
    float fmean_current;      /* 当前亮度 */
    float gain_current;       /* 当前增益 */
    float exposure_current;   /* 当前曝光时间 */
} ae_status_t;

/* 预定义配置 */
static const ae_config_t AE_BRIGHT = {
    .fmean_target = 12000,   /* 较亮 */
    .fmean_boundary = 50,
    .gain_min = 100,
    .gain_max = 800,
    .exposure_min = 100,
    .exposure_mid = 1000,
    .exposure_max = 5000
};

static const ae_config_t AE_NORMAL = {
    .fmean_target = 8000,    /* 正常 */
    .fmean_boundary = 50,
    .gain_min = 100,
    .gain_max = 1200,
    .exposure_min = 100,
    .exposure_mid = 2000,
    .exposure_max = 10000
};

static const ae_config_t AE_DARK = {
    .fmean_target = 5000,    /* 较暗 */
    .fmean_boundary = 50,
    .gain_min = 100,
    .gain_max = 2000,
    .exposure_min = 100,
    .exposure_mid = 5000,
    .exposure_max = 20000
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
    if (err != VXL_SUCCESS) return NULL;

    size_t count = 0;
    err = vxl_context_get_device_count(ctx, &count);
    if (err != VXL_SUCCESS || count == 0) {
        vxl_context_destroy(ctx);
        return NULL;
    }

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

            /* 获取IR/Depth传感器 */
            err = vxl_device_get_sensor(device, 0, &sensor);
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
 * AE配置函数
 *============================================================================*/

vxl_error_t apply_2d_ae_config(vxl_sensor_t *sensor, const ae_config_t *config)
{
    vxl_error_t err;

    printf("Applying 2D AE configuration...\n");

    /* Fmean Target */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_FMEAN_TARGET, config->fmean_target);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Fmean target\n");
        return err;
    }

    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_FMEAN_BOUNDARY, config->fmean_boundary);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Fmean boundary\n");
        return err;
    }
    printf("  Fmean: target=%.0f, boundary=%.0f\n", config->fmean_target, config->fmean_boundary);

    /* Gain Range */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MIN, config->gain_min);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Gain min\n");
        return err;
    }

    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MAX, config->gain_max);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Gain max\n");
        return err;
    }
    printf("  Gain range: [%.0f - %.0f]\n", config->gain_min, config->gain_max);

    /* Exposure Range */
    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MIN, config->exposure_min);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Exposure min\n");
        return err;
    }

    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MID, config->exposure_mid);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Exposure mid\n");
        return err;
    }

    err = vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MAX, config->exposure_max);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "  Failed to set Exposure max\n");
        return err;
    }
    printf("  Exposure range: [%.0f - %.0f - %.0f]\n",
           config->exposure_min, config->exposure_mid, config->exposure_max);

    printf("2D AE configuration applied!\n");
    return VXL_SUCCESS;
}

vxl_error_t get_2d_ae_status(vxl_sensor_t *sensor, ae_status_t *status)
{
    vxl_error_t err;

    err = vxl_sensor_get_option(sensor, VXL615_OPTION_2D_FMEAN_CURRENT, &status->fmean_current);
    if (err != VXL_SUCCESS) return err;

    err = vxl_sensor_get_option(sensor, VXL615_OPTION_2D_GAIN_CURRENT, &status->gain_current);
    if (err != VXL_SUCCESS) return err;

    err = vxl_sensor_get_option(sensor, VXL615_OPTION_2D_EXPOSURE_CURRENT, &status->exposure_current);
    if (err != VXL_SUCCESS) return err;

    return VXL_SUCCESS;
}

void print_2d_ae_status(vxl_sensor_t *sensor)
{
    ae_status_t status;

    if (get_2d_ae_status(sensor, &status) == VXL_SUCCESS) {
        printf("  Current Status: Fmean=%.0f, Gain=%.0f, Exposure=%.0f\n",
               status.fmean_current, status.gain_current, status.exposure_current);
    }
}

/*============================================================================
 * 示例场景
 *============================================================================*/

void example_ae_presets(vxl_sensor_t *sensor)
{
    printf("\n=== Example 1: AE Preset Configurations ===\n");

    printf("\n1. BRIGHT preset (for bright environments):\n");
    apply_2d_ae_config(sensor, &AE_BRIGHT);
    sleep(1);
    print_2d_ae_status(sensor);

    printf("\n2. NORMAL preset (default):\n");
    apply_2d_ae_config(sensor, &AE_NORMAL);
    sleep(1);
    print_2d_ae_status(sensor);

    printf("\n3. DARK preset (for dark environments):\n");
    apply_2d_ae_config(sensor, &AE_DARK);
    sleep(1);
    print_2d_ae_status(sensor);
}

void example_fmean_tuning(vxl_sensor_t *sensor)
{
    printf("\n=== Example 2: Fmean Target Tuning ===\n");

    printf("\nFmean controls image brightness:\n");
    printf("  - Lower target → darker image\n");
    printf("  - Higher target → brighter image\n");

    /* 暗图像 */
    printf("\n1. Dark image (Fmean target = 5000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_FMEAN_TARGET, 5000.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 中等亮度 */
    printf("\n2. Medium brightness (Fmean target = 10000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_FMEAN_TARGET, 10000.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 亮图像 */
    printf("\n3. Bright image (Fmean target = 15000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_FMEAN_TARGET, 15000.0f);
    sleep(1);
    print_2d_ae_status(sensor);
}

void example_gain_control(vxl_sensor_t *sensor)
{
    printf("\n=== Example 3: Gain Range Control ===\n");

    printf("\nGain controls sensor sensitivity:\n");
    printf("  - Lower gain → less noise, needs more light\n");
    printf("  - Higher gain → more noise, works in darker conditions\n");

    /* 低增益 (适合明亮环境) */
    printf("\n1. Low gain (for bright environment, max=500):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MIN, 100.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MAX, 500.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 中等增益 */
    printf("\n2. Medium gain (normal conditions, max=1200):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MAX, 1200.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 高增益 (适合暗环境) */
    printf("\n3. High gain (for dark environment, max=2500):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_GAIN_MAX, 2500.0f);
    sleep(1);
    print_2d_ae_status(sensor);
}

void example_exposure_control(vxl_sensor_t *sensor)
{
    printf("\n=== Example 4: Exposure Range Control ===\n");

    printf("\nExposure controls light integration time:\n");
    printf("  - Shorter exposure → less motion blur, darker in low light\n");
    printf("  - Longer exposure → more motion blur, brighter in low light\n");

    /* 短曝光 (适合快速运动) */
    printf("\n1. Short exposure (for fast motion, max=2000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MIN, 100.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MID, 500.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MAX, 2000.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 中等曝光 */
    printf("\n2. Medium exposure (normal, max=10000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MID, 2000.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MAX, 10000.0f);
    sleep(1);
    print_2d_ae_status(sensor);

    /* 长曝光 (适合静态场景) */
    printf("\n3. Long exposure (for static scenes, max=30000):\n");
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MID, 10000.0f);
    vxl_sensor_set_option(sensor, VXL615_OPTION_2D_EXPOSURE_MAX, 30000.0f);
    sleep(1);
    print_2d_ae_status(sensor);
}

void example_monitor_ae(vxl_sensor_t *sensor)
{
    printf("\n=== Example 5: Monitor AE Status ===\n");

    printf("\nMonitoring AE convergence (5 seconds)...\n");
    printf("Current AE status every second:\n\n");

    for (int i = 0; i < 5; i++) {
        printf("  [%d] ", i + 1);
        print_2d_ae_status(sensor);
        sleep(1);
    }

    printf("\nAE should converge to target Fmean value.\n");
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main(int argc, char **argv)
{
    printf("VXL615 AE Adjustment Example\n");
    printf("=============================\n\n");

    /* 打开VXL615设备 */
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;
    vxl_sensor_t *sensor = open_vxl615_sensor(&ctx, &device);

    if (!sensor) {
        fprintf(stderr, "Failed to open VXL615 sensor\n");
        fprintf(stderr, "Note: This example requires a VXL615 device\n");
        return EXIT_FAILURE;
    }

    /* 运行示例 */
    example_ae_presets(sensor);
    example_fmean_tuning(sensor);
    example_gain_control(sensor);
    example_exposure_control(sensor);
    example_monitor_ae(sensor);

    printf("\n=== AE Tuning Guidelines ===\n");
    printf("\nFmean Target:\n");
    printf("  - 3000-6000: Dark image (for low-light, high dynamic range)\n");
    printf("  - 8000-12000: Normal brightness\n");
    printf("  - 15000-20000: Bright image (for bright scenes)\n");

    printf("\nGain Range:\n");
    printf("  - max < 800: Low noise, needs good lighting\n");
    printf("  - max 1000-1500: Balanced\n");
    printf("  - max > 2000: High noise, works in dark\n");

    printf("\nExposure Range:\n");
    printf("  - max < 5000: Fast (1-5ms), for moving objects\n");
    printf("  - max 5000-15000: Normal (5-15ms)\n");
    printf("  - max > 20000: Slow (>20ms), for static scenes only\n");

    printf("\nBest Practices:\n");
    printf("  1. Start with NORMAL preset\n");
    printf("  2. Adjust Fmean target for desired brightness\n");
    printf("  3. Limit gain range to control noise\n");
    printf("  4. Limit exposure range to prevent motion blur\n");
    printf("  5. Monitor current AE status to verify convergence\n");

    printf("\nNote: This example demonstrated 2D AE control.\n");
    printf("      Use VXL615_OPTION_3D_* options for 3D (Depth) AE control.\n");

    /* 清理 */
    vxl_sensor_release(sensor);
    vxl_device_close(device);
    vxl_context_destroy(ctx);

    printf("\nExample completed!\n");
    return EXIT_SUCCESS;
}
