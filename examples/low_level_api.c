/**
 * @file low_level_api.c
 * @brief VXL SDK Low Level API 示例
 *
 * 演示如何使用 VXL SDK Low Level API:
 * 1. 枚举 USB 设备
 * 2. 初始化设备
 * 3. 测试多传感器流组合
 * 4. 使用回调模式接收帧
 * 5. 输出帧类型统计
 *
 * 用法:
 *   ./low_level_api [选项]
 *
 * 选项:
 *   --mode M  流组合模式:
 *             id = IR + Depth (默认)
 *             rd = RGB + Depth
 *             ir = IR + RGB
 *   --save    保存帧到文件
 *   --time N  采集时间 (秒，默认 5)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#include <pthread.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#include "vxl.h"

/*============================================================================
 * 常量定义
 *============================================================================*/

#define DEFAULT_CAPTURE_TIME_SEC    5       /* 默认采集时长 (秒) */
#define FRAME_WAIT_TIMEOUT_MS       1000    /* 帧等待超时 (毫秒) */
#define SAVE_FRAME_INTERVAL         30      /* 每隔多少帧保存一次 */

/*============================================================================
 * 全局统计数据
 *============================================================================*/

typedef struct {
    uint64_t frame_count;
    uint64_t total_bytes;
    uint64_t saved_count;
    uint64_t ir_count;
    uint64_t depth_count;
    uint64_t color_count;
    uint64_t unknown_count;
    double   start_time;
    bool     save_frames;
    const char *sensor_name;
} frame_stats_t;

static frame_stats_t g_stats = {0};

#ifdef _WIN32
static CRITICAL_SECTION g_stats_lock;
#define STATS_LOCK()   EnterCriticalSection(&g_stats_lock)
#define STATS_UNLOCK() LeaveCriticalSection(&g_stats_lock)
#define STATS_INIT()   InitializeCriticalSection(&g_stats_lock)
#define STATS_DEINIT() DeleteCriticalSection(&g_stats_lock)
#else
static pthread_mutex_t g_stats_lock = PTHREAD_MUTEX_INITIALIZER;
#define STATS_LOCK()   pthread_mutex_lock(&g_stats_lock)
#define STATS_UNLOCK() pthread_mutex_unlock(&g_stats_lock)
#define STATS_INIT()
#define STATS_DEINIT()
#endif

/*============================================================================
 * 辅助函数
 *============================================================================*/

static void print_separator(void)
{
    printf("------------------------------------------------------------\n");
}

static void print_step(int step, const char *description)
{
    printf("\n");
    print_separator();
    printf("[Step %d] %s\n", step, description);
    print_separator();
}

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

static const char* sensor_type_string(vxl_sensor_type_t type)
{
    switch (type) {
        case VXL_SENSOR_COLOR: return "COLOR";
        case VXL_SENSOR_DEPTH: return "DEPTH";
        case VXL_SENSOR_IR:    return "IR";
        default:               return "UNKNOWN";
    }
}

/* 保存帧数据为文件 */
static int save_frame_to_file(vxl_frame_t *frame, const char *prefix, uint64_t frame_num)
{
    const void *data = NULL;
    size_t data_size = 0;
    uint32_t width = 0, height = 0;
    vxl_format_t format;

    vxl_frame_get_data(frame, &data);
    vxl_frame_get_data_size(frame, &data_size);
    vxl_frame_get_width(frame, &width);
    vxl_frame_get_height(frame, &height);
    vxl_frame_get_format(frame, &format);

    if (!data || data_size == 0) {
        return -1;
    }

    char filename[256];
    FILE *fp = NULL;

    switch (format) {
        case VXL_FORMAT_GRAY8:
            snprintf(filename, sizeof(filename), "%s_%04llu.pgm",
                     prefix, (unsigned long long)frame_num);
            fp = fopen(filename, "wb");
            if (fp) {
                fprintf(fp, "P5\n%u %u\n255\n", width, height);
                fwrite(data, 1, width * height, fp);
                fclose(fp);
                printf("    Saved: %s\n", filename);
                return 0;
            }
            break;

        case VXL_FORMAT_GRAY16:
        case VXL_FORMAT_Z16:
            snprintf(filename, sizeof(filename), "%s_%04llu.pgm",
                     prefix, (unsigned long long)frame_num);
            fp = fopen(filename, "wb");
            if (fp) {
                fprintf(fp, "P5\n%u %u\n65535\n", width, height);
                fwrite(data, 1, width * height * 2, fp);
                fclose(fp);
                printf("    Saved: %s\n", filename);
                return 0;
            }
            break;

        case VXL_FORMAT_MJPEG:
            snprintf(filename, sizeof(filename), "%s_%04llu.jpg",
                     prefix, (unsigned long long)frame_num);
            fp = fopen(filename, "wb");
            if (fp) {
                fwrite(data, 1, data_size, fp);
                fclose(fp);
                printf("    Saved: %s\n", filename);
                return 0;
            }
            break;

        default:
            snprintf(filename, sizeof(filename), "%s_%04llu.raw",
                     prefix, (unsigned long long)frame_num);
            fp = fopen(filename, "wb");
            if (fp) {
                fwrite(data, 1, data_size, fp);
                fclose(fp);
                printf("    Saved: %s\n", filename);
                return 0;
            }
            break;
    }

    return -1;
}

/*============================================================================
 * 帧回调函数
 *============================================================================*/

static void frame_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;

    if (!frame) return;

    size_t data_size = 0;
    uint32_t width = 0, height = 0;
    uint64_t timestamp = 0;
    vxl_format_t format;

    vxl_frame_get_data_size(frame, &data_size);
    vxl_frame_get_width(frame, &width);
    vxl_frame_get_height(frame, &height);
    vxl_frame_get_timestamp(frame, &timestamp);
    vxl_frame_get_format(frame, &format);

    STATS_LOCK();

    g_stats.frame_count++;
    g_stats.total_bytes += data_size;

    /* 根据格式统计帧类型 */
    switch (format) {
        case VXL_FORMAT_GRAY8:
            g_stats.ir_count++;
            break;
        case VXL_FORMAT_GRAY16:
        case VXL_FORMAT_Z16:
            g_stats.depth_count++;
            break;
        case VXL_FORMAT_MJPEG:
        case VXL_FORMAT_YUYV:
        case VXL_FORMAT_RGB:
        case VXL_FORMAT_BGR:
            g_stats.color_count++;
            break;
        default:
            g_stats.unknown_count++;
            break;
    }

    uint64_t count = g_stats.frame_count;
    bool should_save = g_stats.save_frames && (count % SAVE_FRAME_INTERVAL == 1);

    STATS_UNLOCK();

    /* 打印前几帧的详细信息 */
    if (count <= 5) {
        printf("  [Frame %llu] %s %ux%u, %zu bytes, ts=%llu\n",
               (unsigned long long)count,
               vxl_format_string(format),
               width, height, data_size,
               (unsigned long long)timestamp);
    }

    /* 保存帧 */
    if (should_save) {
        char prefix[32];
        snprintf(prefix, sizeof(prefix), "%s_frame", g_stats.sensor_name);
        if (save_frame_to_file(frame, prefix, count) == 0) {
            STATS_LOCK();
            g_stats.saved_count++;
            STATS_UNLOCK();
        }
    }
}

/*============================================================================
 * 测试单个传感器
 *============================================================================*/

static int test_sensor(vxl_device_t *device, vxl_sensor_type_t sensor_type,
                      int capture_time_sec, bool save_frames)
{
    vxl_error_t err;
    vxl_sensor_t *sensor = NULL;
    vxl_stream_t *stream = NULL;
    vxl_profile_t *profile = NULL;

    const char *type_name = sensor_type_string(sensor_type);
    printf("\n>>> Testing %s sensor <<<\n\n", type_name);

    /* 重置统计 */
    STATS_LOCK();
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.save_frames = save_frames;
    g_stats.sensor_name = type_name;
    STATS_UNLOCK();

    /* 获取传感器 */
    err = vxl_device_get_sensor(device, sensor_type, &sensor);
    if (err != VXL_SUCCESS) {
        printf("  Sensor not available: %s\n", vxl_error_string(err));
        return -1;
    }

    /* 获取 Profile
     * Y16 传感器使用 profile[1] (640x400)，COLOR 使用 profile[0]
     */
    int profile_idx = (sensor_type != VXL_SENSOR_COLOR) ? 1 : 0;
    err = vxl_sensor_get_profile(sensor, profile_idx, &profile);
    if (err != VXL_SUCCESS) {
        printf("  No profile[%d] available: %s\n", profile_idx, vxl_error_string(err));
        vxl_sensor_release(sensor);
        return -1;
    }

    vxl_format_t format;
    uint32_t width, height, fps;
    vxl_profile_get_format(profile, &format);
    vxl_profile_get_width(profile, &width);
    vxl_profile_get_height(profile, &height);
    vxl_profile_get_fps(profile, &fps);
    printf("  Profile[%d]: %s %ux%u @ %u fps\n",
           profile_idx, vxl_format_string(format), width, height, fps);

    /* 创建流 */
    err = vxl_sensor_create_stream(sensor, profile, &stream);
    if (err != VXL_SUCCESS) {
        printf("  Failed to create stream: %s\n", vxl_error_string(err));
        vxl_profile_release(profile);
        vxl_sensor_release(sensor);
        return -1;
    }

    /* 启动流 (回调模式) */
    printf("  Starting stream (callback mode)...\n");
    g_stats.start_time = get_time_sec();

    err = vxl_stream_start(stream, frame_callback, NULL);
    if (err != VXL_SUCCESS) {
        printf("  Failed to start stream: %s\n", vxl_error_string(err));
        vxl_stream_release(stream);
        vxl_profile_release(profile);
        vxl_sensor_release(sensor);
        return -1;
    }

    printf("  Capturing for %d seconds...\n\n", capture_time_sec);

    /* 等待采集完成 */
    double end_time = g_stats.start_time + capture_time_sec;
    double last_print = g_stats.start_time;

    while (get_time_sec() < end_time) {
        sleep_ms(100);

        double now = get_time_sec();
        if (now - last_print >= 1.0) {
            STATS_LOCK();
            double elapsed = now - g_stats.start_time;
            printf("  [%.1fs] Frames: %llu (IR:%llu, Depth:%llu, Color:%llu), "
                   "FPS: %.1f, Data: %.2f MB\n",
                   elapsed,
                   (unsigned long long)g_stats.frame_count,
                   (unsigned long long)g_stats.ir_count,
                   (unsigned long long)g_stats.depth_count,
                   (unsigned long long)g_stats.color_count,
                   g_stats.frame_count / elapsed,
                   g_stats.total_bytes / (1024.0 * 1024.0));
            STATS_UNLOCK();
            last_print = now;
        }
    }

    /* 停止流 */
    printf("\n  Stopping stream...\n");
    vxl_stream_stop(stream);

    /* 打印统计 */
    STATS_LOCK();
    double total_time = get_time_sec() - g_stats.start_time;
    printf("\n  --- %s Sensor Statistics ---\n", type_name);
    printf("  Duration:     %.2f seconds\n", total_time);
    printf("  Total Frames: %llu\n", (unsigned long long)g_stats.frame_count);
    printf("    IR:         %llu\n", (unsigned long long)g_stats.ir_count);
    printf("    Depth:      %llu\n", (unsigned long long)g_stats.depth_count);
    printf("    Color:      %llu\n", (unsigned long long)g_stats.color_count);
    printf("    Unknown:    %llu\n", (unsigned long long)g_stats.unknown_count);
    printf("  Average FPS:  %.2f\n", g_stats.frame_count / total_time);
    printf("  Total Data:   %.2f MB\n", g_stats.total_bytes / (1024.0 * 1024.0));
    if (save_frames) {
        printf("  Frames Saved: %llu\n", (unsigned long long)g_stats.saved_count);
    }
    STATS_UNLOCK();

    /* 清理 */
    vxl_stream_release(stream);
    vxl_profile_release(profile);
    vxl_sensor_release(sensor);

    return 0;
}

/*============================================================================
 * 主程序
 *============================================================================*/

int main(int argc, char *argv[])
{
    vxl_error_t err;
    vxl_context_t *ctx = NULL;
    vxl_device_t *device = NULL;

    /* 流模式: id=IR+Depth, rd=RGB+Depth, ir=IR+RGB */
    bool test_ir = true;
    bool test_depth = true;
    bool test_color = false;
    bool save_frames = false;
    int capture_time = DEFAULT_CAPTURE_TIME_SEC;
    const char *mode_str = "id";  /* 默认模式 */

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode_str = argv[++i];
            if (strcmp(mode_str, "id") == 0) {
                /* IR + Depth */
                test_ir = true;
                test_depth = true;
                test_color = false;
            } else if (strcmp(mode_str, "rd") == 0) {
                /* RGB + Depth */
                test_ir = false;
                test_depth = true;
                test_color = true;
            } else if (strcmp(mode_str, "ir") == 0) {
                /* IR + RGB */
                test_ir = true;
                test_depth = false;
                test_color = true;
            } else {
                fprintf(stderr, "Unknown mode: %s (use: id, rd, ir)\n", mode_str);
                return 1;
            }
        } else if (strcmp(argv[i], "--save") == 0) {
            save_frames = true;
        } else if (strcmp(argv[i], "--time") == 0 && i + 1 < argc) {
            capture_time = atoi(argv[++i]);
            if (capture_time <= 0) capture_time = DEFAULT_CAPTURE_TIME_SEC;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --mode M  Stream mode (default: id)\n");
            printf("            id = IR + Depth\n");
            printf("            rd = RGB + Depth\n");
            printf("            ir = IR + RGB\n");
            printf("  --save    Save frames to files\n");
            printf("  --time N  Capture time in seconds (default: %d)\n", DEFAULT_CAPTURE_TIME_SEC);
            printf("  --help    Show this help\n");
            return 0;
        }
    }

    STATS_INIT();

    printf("\n");
    printf("============================================================\n");
    printf("         VXL SDK Low Level API Test\n");
    printf("============================================================\n");
    printf("SDK Version: %s\n", vxl_get_version_string());
    printf("Test Config:\n");
    if (strcmp(mode_str, "id") == 0) {
        printf("  Mode:   id (IR + Depth)\n");
    } else if (strcmp(mode_str, "rd") == 0) {
        printf("  Mode:   rd (RGB + Depth)\n");
    } else if (strcmp(mode_str, "ir") == 0) {
        printf("  Mode:   ir (IR + RGB)\n");
    }
    printf("  Time:   %d seconds each\n", capture_time);
    printf("  Save:   %s\n", save_frames ? "YES" : "NO");

    /*========================================================================
     * Step 1: 创建上下文并枚举设备
     *========================================================================*/
    print_step(1, "Creating Context and Enumerating Devices");

    err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create context: %s\n", vxl_error_string(err));
        return 1;
    }

    size_t device_count = 0;
    vxl_context_get_device_count(ctx, &device_count);
    printf("Found %zu device(s)\n", device_count);

    if (device_count == 0) {
        printf("No devices found. Please connect a camera.\n");
        vxl_context_destroy(ctx);
        return 1;
    }

    /* 列出设备 */
    for (size_t i = 0; i < device_count; i++) {
        vxl_device_t *dev = NULL;
        vxl_context_get_device(ctx, i, &dev);
        if (dev) {
            vxl_device_info_t info;
            vxl_device_get_info(dev, &info);
            printf("  [%zu] %s (VID:%04X PID:%04X)\n",
                   i, info.name, info.vendor_id, info.product_id);
            if (strlen(info.serial_number) > 0) {
                printf("       Serial: %s\n", info.serial_number);
            }
            vxl_device_release(dev);
        }
    }

    /*========================================================================
     * Step 2: 打开第一个设备
     *========================================================================*/
    print_step(2, "Opening Device");

    err = vxl_context_get_device(ctx, 0, &device);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to get device: %s\n", vxl_error_string(err));
        vxl_context_destroy(ctx);
        return 1;
    }

    vxl_device_info_t dev_info;
    vxl_device_get_info(device, &dev_info);
    printf("Opening: %s\n", dev_info.name);

    err = vxl_device_open(device);
    if (err != VXL_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to open device: %s\n", vxl_error_string(err));
        vxl_device_release(device);
        vxl_context_destroy(ctx);
        return 1;
    }
    printf("Device opened successfully.\n");

    /*========================================================================
     * Step 3: 列出传感器
     *========================================================================*/
    print_step(3, "Available Sensors");

    size_t sensor_count = 0;
    vxl_device_get_sensor_count(device, &sensor_count);
    printf("Found %zu sensor(s):\n", sensor_count);

    for (size_t i = 0; i < sensor_count; i++) {
        vxl_sensor_t *s = NULL;
        vxl_device_get_sensor_by_index(device, i, &s);
        if (s) {
            vxl_sensor_type_t type;
            vxl_sensor_get_type(s, &type);

            size_t profile_count = 0;
            vxl_sensor_get_profile_count(s, &profile_count);

            printf("  [%zu] %s (%zu profiles)\n", i, sensor_type_string(type), profile_count);

            /* 列出前 3 个 profile */
            for (size_t j = 0; j < profile_count && j < 3; j++) {
                vxl_profile_t *p = NULL;
                vxl_sensor_get_profile(s, j, &p);
                if (p) {
                    vxl_format_t fmt;
                    uint32_t w, h, fps;
                    vxl_profile_get_format(p, &fmt);
                    vxl_profile_get_width(p, &w);
                    vxl_profile_get_height(p, &h);
                    vxl_profile_get_fps(p, &fps);
                    printf("       - %s %ux%u @ %u fps\n", vxl_format_string(fmt), w, h, fps);
                    vxl_profile_release(p);
                }
            }
            if (profile_count > 3) {
                printf("       ... and %zu more\n", profile_count - 3);
            }

            vxl_sensor_release(s);
        }
    }

    /*========================================================================
     * Step 4: 测试各传感器
     *========================================================================*/
    print_step(4, "Testing Sensors");

    if (test_ir) {
        test_sensor(device, VXL_SENSOR_IR, capture_time, save_frames);
    }

    if (test_depth) {
        test_sensor(device, VXL_SENSOR_DEPTH, capture_time, save_frames);
    }

    if (test_color) {
        test_sensor(device, VXL_SENSOR_COLOR, capture_time, save_frames);
    }

    /*========================================================================
     * Step 5: 清理
     *========================================================================*/
    print_step(5, "Cleanup");

    vxl_device_close(device);
    printf("Device closed.\n");

    vxl_device_release(device);
    printf("Device released.\n");

    vxl_context_destroy(ctx);
    printf("Context destroyed.\n");

    STATS_DEINIT();

    printf("\nDone.\n\n");
    return 0;
}
