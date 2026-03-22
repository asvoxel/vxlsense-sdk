/**
 * @file device_info.c
 * @brief VXL SDK 设备信息测试程序
 *
 * 功能:
 * 1. 枚举所有设备，打印完整设备信息
 * 2. 列出每个设备的传感器和 Profile
 * 3. 打印相机参数 (内参/外参/深度参数)
 * 4. 同时打开多个传感器的流并统计帧信息
 *
 * 用法:
 *   ./device_info [选项]
 *
 * 选项:
 *   --mode M  流组合模式:
 *             id = IR + Depth (默认)
 *             rd = RGB + Depth
 *             ir = IR + RGB
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#include <time.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#include "vxl.h"

/*============================================================================
 * 配置
 *============================================================================*/

#define STREAM_TEST_DURATION_SEC    5       /* 流测试时长 (秒) */
#define MAX_SENSORS                 3       /* 最大传感器数 */

/*============================================================================
 * 流模式配置
 *============================================================================*/

static bool g_test_ir    = true;   /* 测试 IR */
static bool g_test_depth = true;   /* 测试 Depth */
static bool g_test_color = false;  /* 测试 Color */
static const char *g_mode_str = "id";

/*============================================================================
 * 帧统计 (每个传感器独立统计)
 *============================================================================*/

typedef struct {
    vxl_sensor_type_t   type;
    uint64_t            frame_count;
    uint64_t            total_bytes;
    uint32_t            min_size;
    uint32_t            max_size;
    uint32_t            last_width;
    uint32_t            last_height;
    vxl_format_t        last_format;
} sensor_stats_t;

static sensor_stats_t g_sensor_stats[MAX_SENSORS] = {0};
static double g_start_time = 0;

static double get_time_sec(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
#endif
}

/*============================================================================
 * 打印辅助函数
 *============================================================================*/

static void print_header(const char *title)
{
    printf("\n");
    printf("============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

static void print_section(const char *title)
{
    printf("\n--- %s ---\n", title);
}

static const char* sensor_type_str(vxl_sensor_type_t type)
{
    switch (type) {
        case VXL_SENSOR_COLOR: return "COLOR";
        case VXL_SENSOR_DEPTH: return "DEPTH";
        case VXL_SENSOR_IR:    return "IR";
        default:               return "UNKNOWN";
    }
}

static const char* format_str(vxl_format_t fmt)
{
    switch (fmt) {
        case VXL_FORMAT_YUYV:   return "YUYV";
        case VXL_FORMAT_UYVY:   return "UYVY";
        case VXL_FORMAT_RGB:    return "RGB";
        case VXL_FORMAT_BGR:    return "BGR";
        case VXL_FORMAT_MJPEG:  return "MJPEG";
        case VXL_FORMAT_GRAY8:  return "GRAY8";
        case VXL_FORMAT_GRAY16: return "GRAY16";
        case VXL_FORMAT_Z16:    return "Z16";
        default:                return "UNKNOWN";
    }
}

/*============================================================================
 * 设备信息打印
 *============================================================================*/

static void print_device_info(vxl_device_t *dev, size_t index)
{
    vxl_device_info_t info;
    vxl_error_t err;

    err = vxl_device_get_info(dev, &info);
    if (err != VXL_SUCCESS) {
        printf("  [%zu] Failed to get info: %s\n", index, vxl_error_string(err));
        return;
    }

    printf("  [%zu] %s\n", index, info.name);
    printf("      VID/PID:    0x%04X / 0x%04X\n", info.vendor_id, info.product_id);
    if (strlen(info.serial_number) > 0) {
        printf("      Serial:     %s\n", info.serial_number);
    }
    if (strlen(info.fw_version) > 0) {
        printf("      Firmware:   %s\n", info.fw_version);
    }
}

/*============================================================================
 * 相机参数打印
 *============================================================================*/

static void print_camera_params(vxl_device_t *dev)
{
    vxl_error_t err;
    char version[64] = {0};

    print_section("Camera Parameters");

    /* 固件版本 */
    err = vxl_device_fw_version(dev, version, sizeof(version));
    if (err == VXL_SUCCESS) {
        printf("  Firmware Version: %s\n", version);
    }

    /* 深度参数 */
    vxl_depth_param_t depth_param;
    err = vxl_device_get_depth_param(dev, &depth_param);
    if (err == VXL_SUCCESS) {
        printf("  Depth Parameters:\n");
        printf("    Baseline: %.2f mm\n", depth_param.baseline);
        printf("    Focus:    %.2f\n", depth_param.focus);
        printf("    Cx/Cy:    %.2f / %.2f\n", depth_param.cx, depth_param.cy);
    }

    /* IR 内参 */
    vxl_intrinsics_t intrin;
    err = vxl_device_get_intrin(dev, VXL_SENSOR_IR, &intrin);
    if (err == VXL_SUCCESS) {
        printf("  IR Intrinsics:\n");
        printf("    Size:     %u x %u\n", intrin.width, intrin.height);
        printf("    fx/fy:    %.2f / %.2f\n", intrin.fx, intrin.fy);
        printf("    cx/cy:    %.2f / %.2f\n", intrin.cx, intrin.cy);
        printf("    Distort:  [%.4f, %.4f, %.4f, %.4f, %.4f]\n",
               intrin.coeffs[0], intrin.coeffs[1], intrin.coeffs[2],
               intrin.coeffs[3], intrin.coeffs[4]);
    }

    /* RGB 内参 */
    err = vxl_device_get_intrin(dev, VXL_SENSOR_COLOR, &intrin);
    if (err == VXL_SUCCESS) {
        printf("  RGB Intrinsics:\n");
        printf("    Size:     %u x %u\n", intrin.width, intrin.height);
        printf("    fx/fy:    %.2f / %.2f\n", intrin.fx, intrin.fy);
        printf("    cx/cy:    %.2f / %.2f\n", intrin.cx, intrin.cy);
    }

    /* IR-RGB 外参 */
    vxl_extrinsics_t extrin;
    err = vxl_device_get_extrin(dev, VXL_SENSOR_IR, VXL_SENSOR_COLOR, &extrin);
    if (err == VXL_SUCCESS) {
        printf("  IR->RGB Extrinsics:\n");
        printf("    T: [%.2f, %.2f, %.2f] mm\n",
               extrin.translation[0], extrin.translation[1], extrin.translation[2]);
    }
}

/*============================================================================
 * 传感器信息打印
 *============================================================================*/

static void print_sensor_info(vxl_device_t *dev)
{
    vxl_error_t err;
    size_t sensor_count = 0;

    print_section("Sensors");

    err = vxl_device_get_sensor_count(dev, &sensor_count);
    if (err != VXL_SUCCESS) {
        printf("  Failed to get sensor count: %s\n", vxl_error_string(err));
        return;
    }

    printf("  Sensor Count: %zu\n\n", sensor_count);

    for (size_t i = 0; i < sensor_count; i++) {
        vxl_sensor_t *sensor = NULL;
        err = vxl_device_get_sensor_by_index(dev, i, &sensor);
        if (err != VXL_SUCCESS) continue;

        vxl_sensor_type_t type;
        vxl_sensor_get_type(sensor, &type);

        size_t profile_count = 0;
        vxl_sensor_get_profile_count(sensor, &profile_count);

        printf("  [%zu] %s Sensor (%zu profiles)\n", i, sensor_type_str(type), profile_count);

        /* 打印所有 Profile */
        for (size_t p = 0; p < profile_count; p++) {
            vxl_profile_t *profile = NULL;
            if (vxl_sensor_get_profile(sensor, p, &profile) == VXL_SUCCESS) {
                vxl_format_t fmt;
                uint32_t w, h, fps;
                vxl_profile_get_format(profile, &fmt);
                vxl_profile_get_width(profile, &w);
                vxl_profile_get_height(profile, &h);
                vxl_profile_get_fps(profile, &fps);
                printf("      [%zu] %s %ux%u @ %u fps%s\n",
                       p, format_str(fmt), w, h, fps,
                       (p == 0) ? " <-- will use" : "");
                vxl_profile_release(profile);
            }
        }

        vxl_sensor_release(sensor);
    }
}

/*============================================================================
 * 帧回调 (每个传感器独立)
 *============================================================================*/

static void color_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;
    sensor_stats_t *stats = &g_sensor_stats[0];
    size_t data_size = 0;

    vxl_frame_get_data_size(frame, &data_size);
    vxl_frame_get_width(frame, &stats->last_width);
    vxl_frame_get_height(frame, &stats->last_height);
    vxl_frame_get_format(frame, &stats->last_format);

    stats->frame_count++;
    stats->total_bytes += data_size;
    if (stats->min_size == 0 || data_size < stats->min_size) stats->min_size = (uint32_t)data_size;
    if (data_size > stats->max_size) stats->max_size = (uint32_t)data_size;
}

static void depth_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;
    sensor_stats_t *stats = &g_sensor_stats[1];
    size_t data_size = 0;

    vxl_frame_get_data_size(frame, &data_size);
    vxl_frame_get_width(frame, &stats->last_width);
    vxl_frame_get_height(frame, &stats->last_height);
    vxl_frame_get_format(frame, &stats->last_format);

    stats->frame_count++;
    stats->total_bytes += data_size;
    if (stats->min_size == 0 || data_size < stats->min_size) stats->min_size = (uint32_t)data_size;
    if (data_size > stats->max_size) stats->max_size = (uint32_t)data_size;
}

static void ir_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;
    sensor_stats_t *stats = &g_sensor_stats[2];
    size_t data_size = 0;

    vxl_frame_get_data_size(frame, &data_size);
    vxl_frame_get_width(frame, &stats->last_width);
    vxl_frame_get_height(frame, &stats->last_height);
    vxl_frame_get_format(frame, &stats->last_format);

    stats->frame_count++;
    stats->total_bytes += data_size;
    if (stats->min_size == 0 || data_size < stats->min_size) stats->min_size = (uint32_t)data_size;
    if (data_size > stats->max_size) stats->max_size = (uint32_t)data_size;
}

/*============================================================================
 * 多传感器流测试
 *============================================================================*/

static void test_all_streams(vxl_device_t *dev)
{
    vxl_error_t err;
    vxl_sensor_t *sensors[MAX_SENSORS] = {NULL};
    vxl_stream_t *streams[MAX_SENSORS] = {NULL};
    vxl_profile_t *profiles[MAX_SENSORS] = {NULL};
    vxl_frame_cbfn callbacks[MAX_SENSORS] = {color_callback, depth_callback, ir_callback};
    vxl_sensor_type_t types[MAX_SENSORS] = {VXL_SENSOR_COLOR, VXL_SENSOR_DEPTH, VXL_SENSOR_IR};
    bool test_flags[MAX_SENSORS] = {g_test_color, g_test_depth, g_test_ir};
    int stream_count = 0;

    print_section("Multi-Sensor Stream Test");
    if (strcmp(g_mode_str, "id") == 0) {
        printf("  Mode: id (IR + Depth)\n\n");
    } else if (strcmp(g_mode_str, "rd") == 0) {
        printf("  Mode: rd (RGB + Depth)\n\n");
    } else if (strcmp(g_mode_str, "ir") == 0) {
        printf("  Mode: ir (IR + RGB)\n\n");
    }

    /* 重置统计 */
    memset(g_sensor_stats, 0, sizeof(g_sensor_stats));
    for (int i = 0; i < MAX_SENSORS; i++) {
        g_sensor_stats[i].type = types[i];
    }

    /* 为每个传感器创建流 */
    for (int i = 0; i < MAX_SENSORS; i++) {
        /* 跳过未选择的传感器 */
        if (!test_flags[i]) {
            continue;
        }

        /* 获取传感器 */
        err = vxl_device_get_sensor(dev, types[i], &sensors[i]);
        if (err != VXL_SUCCESS) {
            printf("  [%s] Sensor not available\n", sensor_type_str(types[i]));
            continue;
        }

        /* 获取 Profile
         * 所有传感器使用 profile[0]，这是默认推荐分辨率
         * (VXL615 已在 backend 中重排为 640x400@30fps 优先)
         */
        int profile_idx = 0;
        err = vxl_sensor_get_profile(sensors[i], profile_idx, &profiles[i]);
        if (err != VXL_SUCCESS) {
            printf("  [%s] No profile[%d] available\n", sensor_type_str(types[i]), profile_idx);
            vxl_sensor_release(sensors[i]);
            sensors[i] = NULL;
            continue;
        }

        /* 打印配置 */
        vxl_format_t fmt;
        uint32_t w, h, fps;
        vxl_profile_get_format(profiles[i], &fmt);
        vxl_profile_get_width(profiles[i], &w);
        vxl_profile_get_height(profiles[i], &h);
        vxl_profile_get_fps(profiles[i], &fps);
        printf("  [%s] Profile[%d]: %s %ux%u @ %u fps\n",
               sensor_type_str(types[i]), profile_idx, format_str(fmt), w, h, fps);

        /* 创建流 */
        err = vxl_sensor_create_stream(sensors[i], profiles[i], &streams[i]);
        if (err != VXL_SUCCESS) {
            printf("  [%s] Failed to create stream: %s\n",
                   sensor_type_str(types[i]), vxl_error_string(err));
            vxl_profile_release(profiles[i]);
            vxl_sensor_release(sensors[i]);
            profiles[i] = NULL;
            sensors[i] = NULL;
            continue;
        }

        stream_count++;
    }

    if (stream_count == 0) {
        printf("\n  No streams created!\n");
        return;
    }

    printf("\n  Starting %d stream(s) for %d seconds...\n\n",
           stream_count, STREAM_TEST_DURATION_SEC);

    /* 启动所有流 (先启动 Y16 流，再启动 RGB 流) */
    g_start_time = get_time_sec();
    /* 第一轮：启动非 COLOR 流 (Y16) */
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (streams[i] && types[i] != VXL_SENSOR_COLOR) {
            printf("  Starting %s stream first (Y16)...\n", sensor_type_str(types[i]));
            err = vxl_stream_start(streams[i], callbacks[i], NULL);
            if (err != VXL_SUCCESS) {
                printf("  [%s] Failed to start: %s\n",
                       sensor_type_str(types[i]), vxl_error_string(err));
            }
        }
    }
    /* 第二轮：启动 COLOR 流 (RGB) */
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (streams[i] && types[i] == VXL_SENSOR_COLOR) {
            printf("  Starting %s stream second (RGB)...\n", sensor_type_str(types[i]));
            err = vxl_stream_start(streams[i], callbacks[i], NULL);
            if (err != VXL_SUCCESS) {
                printf("  [%s] Failed to start: %s\n",
                       sensor_type_str(types[i]), vxl_error_string(err));
            }
        }
    }

    /* 等待并定期打印状态 */
    for (int t = 0; t < STREAM_TEST_DURATION_SEC; t++) {
        sleep_ms(1000);
        double elapsed = get_time_sec() - g_start_time;
        printf("  [%.1fs]", elapsed);
        for (int i = 0; i < MAX_SENSORS; i++) {
            if (streams[i] && g_sensor_stats[i].frame_count > 0) {
                printf(" %s:%llu",
                       sensor_type_str(types[i]),
                       (unsigned long long)g_sensor_stats[i].frame_count);
            }
        }
        printf("\n");
    }

    /* 停止所有流 */
    printf("\n  Stopping streams...\n");
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (streams[i]) {
            vxl_stream_stop(streams[i]);
        }
    }

    /* 打印统计 */
    double elapsed = get_time_sec() - g_start_time;
    print_section("Statistics");
    printf("  Duration: %.2f sec\n\n", elapsed);

    printf("  %-8s  %10s  %10s  %12s  %10s\n",
           "Sensor", "Frames", "FPS", "Total MB", "Frame Size");
    printf("  %-8s  %10s  %10s  %12s  %10s\n",
           "------", "------", "---", "--------", "----------");

    uint64_t total_frames = 0;
    uint64_t total_bytes = 0;

    for (int i = 0; i < MAX_SENSORS; i++) {
        sensor_stats_t *s = &g_sensor_stats[i];
        if (s->frame_count > 0) {
            double fps = s->frame_count / elapsed;
            double mb = s->total_bytes / (1024.0 * 1024.0);
            printf("  %-8s  %10llu  %10.2f  %12.2f  %u~%u\n",
                   sensor_type_str(s->type),
                   (unsigned long long)s->frame_count,
                   fps, mb, s->min_size, s->max_size);
            total_frames += s->frame_count;
            total_bytes += s->total_bytes;
        } else if (streams[i]) {
            printf("  %-8s  %10s  %10s  %12s  %s\n",
                   sensor_type_str(s->type), "0", "-", "-", "(no frames)");
        }
    }

    printf("  %-8s  %10s  %10s  %12s\n", "------", "------", "---", "--------");
    printf("  %-8s  %10llu  %10.2f  %12.2f\n",
           "TOTAL",
           (unsigned long long)total_frames,
           total_frames / elapsed,
           total_bytes / (1024.0 * 1024.0));

    /* 清理 */
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (streams[i]) vxl_stream_release(streams[i]);
        if (profiles[i]) vxl_profile_release(profiles[i]);
        if (sensors[i]) vxl_sensor_release(sensors[i]);
    }
}

/*============================================================================
 * 主程序
 *============================================================================*/

int main(int argc, char *argv[])
{
    vxl_error_t err;
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode_str = argv[++i];
            if (strcmp(g_mode_str, "id") == 0) {
                /* IR + Depth */
                g_test_ir = true;
                g_test_depth = true;
                g_test_color = false;
            } else if (strcmp(g_mode_str, "rd") == 0) {
                /* RGB + Depth */
                g_test_ir = false;
                g_test_depth = true;
                g_test_color = true;
            } else if (strcmp(g_mode_str, "ir") == 0) {
                /* IR + RGB */
                g_test_ir = true;
                g_test_depth = false;
                g_test_color = true;
            } else {
                fprintf(stderr, "Unknown mode: %s (use: id, rd, ir)\n", g_mode_str);
                return 1;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --mode M  Stream mode (default: id)\n");
            printf("            id = IR + Depth\n");
            printf("            rd = RGB + Depth\n");
            printf("            ir = IR + RGB\n");
            printf("  --help    Show this help\n");
            return 0;
        }
    }

    print_header("VXL Device Info Test");
    printf("  SDK Version: %s\n", vxl_get_version_string());

    /*========================================================================
     * 创建 Context
     *========================================================================*/
    err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create context: %s\n", vxl_error_string(err));
        return 1;
    }

    /*========================================================================
     * 枚举设备
     *========================================================================*/
    print_section("Devices");

    size_t device_count = 0;
    err = vxl_context_get_device_count(ctx, &device_count);
    if (err != VXL_SUCCESS || device_count == 0) {
        printf("  No devices found.\n");
        vxl_context_destroy(ctx);
        return 1;
    }

    printf("  Found %zu device(s):\n\n", device_count);

    for (size_t i = 0; i < device_count; i++) {
        vxl_device_t *dev = NULL;
        err = vxl_context_get_device(ctx, i, &dev);
        if (err == VXL_SUCCESS) {
            print_device_info(dev, i);
            vxl_device_release(dev);
        }
    }

    /*========================================================================
     * 打开第一个设备
     *========================================================================*/
    err = vxl_context_get_device(ctx, 0, &device);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to get device: %s\n", vxl_error_string(err));
        vxl_context_destroy(ctx);
        return 1;
    }

    err = vxl_device_open(device);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to open device: %s\n", vxl_error_string(err));
        vxl_device_release(device);
        vxl_context_destroy(ctx);
        return 1;
    }

    /*========================================================================
     * 打印详细信息
     *========================================================================*/
    print_camera_params(device);
    print_sensor_info(device);

    /*========================================================================
     * 多传感器流测试
     *========================================================================*/
    test_all_streams(device);

    /*========================================================================
     * 清理
     *========================================================================*/
    print_section("Cleanup");
    vxl_device_close(device);
    vxl_device_release(device);
    vxl_context_destroy(ctx);
    printf("  Done.\n\n");

    return 0;
}
