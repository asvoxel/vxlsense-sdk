/**
 * @file rgbd_fusion.c
 * @brief VXL SDK RGB-D 融合示例
 *
 * 演示如何使用 VXL SDK 进行 RGB-D 融合:
 * 1. 初始化设备
 * 2. 同时打开多传感器流
 * 3. 深度图伪彩色渲染
 * 4. 生成彩色点云
 * 5. 保存点云到 PLY 文件
 *
 * 用法:
 *   ./rgbd_fusion [选项]
 *
 * 选项:
 *   --mode M      流组合模式:
 *                 id = IR + Depth
 *                 rd = RGB + Depth (默认)
 *                 ir = IR + RGB
 *   --time N      采集时间 (秒，默认 5)
 *   --colormap    保存深度伪彩色图像
 *   --pointcloud  生成并保存点云
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
#include "vxl_dip.h"
#include "vxl615_types.h"  /* VXL615 specific options */

/*============================================================================
 * 常量定义
 *============================================================================*/

#define DEFAULT_CAPTURE_TIME_SEC    5       /* 默认采集时长 (秒) */
#define DEPTH_SCALE                 16.0f   /* VXL615 深度比例因子 */

/*============================================================================
 * 全局数据
 *============================================================================*/

typedef struct {
    volatile uint64_t depth_count;
    volatile uint64_t rgb_count;
    volatile uint64_t ir_count;
    vxl_frame_t *last_depth_frame;
    vxl_frame_t *last_rgb_frame;
    vxl_frame_t *last_ir_frame;
    bool save_colormap;
    bool save_pointcloud;
    bool running;
    /* 流模式配置 */
    bool use_ir;
    bool use_depth;
    bool use_color;
    const char *mode_str;
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} capture_ctx_t;

static capture_ctx_t g_ctx = {0};

#ifdef _WIN32
#define CTX_LOCK()   EnterCriticalSection(&g_ctx.lock)
#define CTX_UNLOCK() LeaveCriticalSection(&g_ctx.lock)
#define CTX_INIT()   InitializeCriticalSection(&g_ctx.lock)
#define CTX_DEINIT() DeleteCriticalSection(&g_ctx.lock)
#else
#define CTX_LOCK()   pthread_mutex_lock(&g_ctx.lock)
#define CTX_UNLOCK() pthread_mutex_unlock(&g_ctx.lock)
#define CTX_INIT()   pthread_mutex_init(&g_ctx.lock, NULL)
#define CTX_DEINIT() pthread_mutex_destroy(&g_ctx.lock)
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

/*============================================================================
 * 文件保存函数
 *============================================================================*/

/* 保存 RGB 图像为 PPM */
static int save_rgb_ppm(const char *filename, const uint8_t *data,
                        uint32_t width, uint32_t height)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }
    fprintf(fp, "P6\n%u %u\n255\n", width, height);
    fwrite(data, 1, width * height * 3, fp);
    fclose(fp);
    return 0;
}

/* 保存点云为 PLY 格式 */
static int save_pointcloud_ply(const char *filename, const vxl_pointcloud_t *pc)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }

    /* PLY 头部 */
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %zu\n", pc->count);
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "property uchar red\n");
    fprintf(fp, "property uchar green\n");
    fprintf(fp, "property uchar blue\n");
    fprintf(fp, "end_header\n");

    /* 写入点数据 */
    for (size_t i = 0; i < pc->count; i++) {
        const vxl_point3d_rgb_t *pt = &pc->points[i];
        fprintf(fp, "%.3f %.3f %.3f %u %u %u\n",
                pt->x, pt->y, pt->z, pt->r, pt->g, pt->b);
    }

    fclose(fp);
    return 0;
}

/*============================================================================
 * 帧回调函数
 *============================================================================*/

static void depth_frame_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;

    uint64_t timestamp = 0;
    uint32_t sequence = 0;
    uint16_t frame_count = 0;
    vxl_frame_get_timestamp(frame, &timestamp);
    vxl_frame_get_sequence(frame, &sequence);
    vxl_frame_get_frame_count(frame, &frame_count);

    CTX_LOCK();
    g_ctx.depth_count++;
    uint64_t count = g_ctx.depth_count;

    /* 保存最新的深度帧 */
    if (g_ctx.last_depth_frame) {
        vxl_frame_release(g_ctx.last_depth_frame);
    }
    vxl_frame_add_ref(frame);
    g_ctx.last_depth_frame = frame;

    CTX_UNLOCK();

    /* 打印前10帧的帧计数信息 */
    if (count <= 10) {
        printf("  [DEPTH] fc=%u seq=%u ts=%llu us\n", frame_count, sequence, (unsigned long long)timestamp);
    }
}

static void rgb_frame_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;

    uint64_t timestamp = 0;
    uint32_t sequence = 0;
    uint16_t frame_count = 0;
    vxl_frame_get_timestamp(frame, &timestamp);
    vxl_frame_get_sequence(frame, &sequence);
    vxl_frame_get_frame_count(frame, &frame_count);

    CTX_LOCK();
    g_ctx.rgb_count++;
    uint64_t count = g_ctx.rgb_count;

    /* 保存最新的 RGB 帧 */
    if (g_ctx.last_rgb_frame) {
        vxl_frame_release(g_ctx.last_rgb_frame);
    }
    vxl_frame_add_ref(frame);
    g_ctx.last_rgb_frame = frame;

    CTX_UNLOCK();

    /* 打印前10帧的帧计数信息 */
    if (count <= 10) {
        printf("  [RGB  ] fc=%u seq=%u\n", frame_count, sequence);
    }
}

static void ir_frame_callback(vxl_frame_t *frame, void *user_data)
{
    (void)user_data;

    CTX_LOCK();
    g_ctx.ir_count++;

    /* 保存最新的 IR 帧 */
    if (g_ctx.last_ir_frame) {
        vxl_frame_release(g_ctx.last_ir_frame);
    }
    vxl_frame_add_ref(frame);
    g_ctx.last_ir_frame = frame;

    CTX_UNLOCK();
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main(int argc, char **argv)
{
    vxl_error_t err;
    int capture_time = DEFAULT_CAPTURE_TIME_SEC;
    bool do_colormap = false;
    bool do_pointcloud = false;

    /* 默认模式: rd (RGB + Depth) */
    g_ctx.use_ir = false;
    g_ctx.use_depth = true;
    g_ctx.use_color = true;
    g_ctx.mode_str = "rd";

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_ctx.mode_str = argv[++i];
            if (strcmp(g_ctx.mode_str, "id") == 0) {
                /* IR + Depth */
                g_ctx.use_ir = true;
                g_ctx.use_depth = true;
                g_ctx.use_color = false;
            } else if (strcmp(g_ctx.mode_str, "rd") == 0) {
                /* RGB + Depth */
                g_ctx.use_ir = false;
                g_ctx.use_depth = true;
                g_ctx.use_color = true;
            } else if (strcmp(g_ctx.mode_str, "ir") == 0) {
                /* IR + RGB */
                g_ctx.use_ir = true;
                g_ctx.use_depth = false;
                g_ctx.use_color = true;
            } else if (strcmp(g_ctx.mode_str, "ird") == 0) {
                /* IR + RGB + Depth (三流模式) */
                g_ctx.use_ir = true;
                g_ctx.use_depth = true;
                g_ctx.use_color = true;
            } else {
                fprintf(stderr, "Unknown mode: %s (use: id, rd, ir, ird)\n", g_ctx.mode_str);
                return 1;
            }
        } else if (strcmp(argv[i], "--time") == 0 && i + 1 < argc) {
            capture_time = atoi(argv[++i]);
            if (capture_time <= 0) {
                capture_time = DEFAULT_CAPTURE_TIME_SEC;
            }
        } else if (strcmp(argv[i], "--colormap") == 0) {
            do_colormap = true;
        } else if (strcmp(argv[i], "--pointcloud") == 0) {
            do_pointcloud = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --mode M      Stream mode (default: rd)\n");
            printf("                id  = IR + Depth (15fps each)\n");
            printf("                rd  = RGB + Depth (30fps + 15fps)\n");
            printf("                ir  = IR + RGB (15fps + 30fps)\n");
            printf("                ird = IR + RGB + Depth (15fps + 30fps + 15fps)\n");
            printf("  --time N      Capture time in seconds (default: %d)\n", DEFAULT_CAPTURE_TIME_SEC);
            printf("  --colormap    Save depth colormap image\n");
            printf("  --pointcloud  Generate and save point cloud\n");
            printf("  --help        Show this help\n");
            return 0;
        }
    }

    g_ctx.save_colormap = do_colormap;
    g_ctx.save_pointcloud = do_pointcloud;

    printf("\n");
    printf("========================================\n");
    printf("  VXL SDK RGB-D Fusion Example\n");
    printf("========================================\n");
    if (strcmp(g_ctx.mode_str, "id") == 0) {
        printf("Mode: id (IR + Depth)\n");
    } else if (strcmp(g_ctx.mode_str, "rd") == 0) {
        printf("Mode: rd (RGB + Depth)\n");
    } else if (strcmp(g_ctx.mode_str, "ir") == 0) {
        printf("Mode: ir (IR + RGB)\n");
    } else if (strcmp(g_ctx.mode_str, "ird") == 0) {
        printf("Mode: ird (IR + RGB + Depth)\n");
    }
    printf("Capture time: %d seconds\n", capture_time);
    printf("Save colormap: %s\n", do_colormap ? "Yes" : "No");
    printf("Save pointcloud: %s\n", do_pointcloud ? "Yes" : "No");

    CTX_INIT();

    /*========================================================================
     * Step 1: 初始化 VXL Context
     *========================================================================*/
    print_step(1, "Initialize VXL Context");

    vxl_context_t *ctx = NULL;
    err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS || !ctx) {
        printf("ERROR: Failed to create context: %s\n", vxl_error_string(err));
        return 1;
    }
    printf("Context created successfully\n");

    /*========================================================================
     * Step 2: 枚举设备
     *========================================================================*/
    print_step(2, "Enumerate Devices");

    size_t device_count = 0;
    err = vxl_context_get_device_count(ctx, &device_count);
    if (err != VXL_SUCCESS) {
        printf("ERROR: Failed to get device count: %s\n", vxl_error_string(err));
        vxl_context_destroy(ctx);
        return 1;
    }
    printf("Found %zu device(s)\n", device_count);

    if (device_count == 0) {
        printf("No devices found!\n");
        vxl_context_destroy(ctx);
        return 1;
    }

    /*========================================================================
     * Step 3: 打开设备
     *========================================================================*/
    print_step(3, "Open Device");

    vxl_device_t *device = NULL;
    err = vxl_context_get_device(ctx, 0, &device);
    if (err != VXL_SUCCESS || !device) {
        printf("ERROR: Failed to get device: %s\n", vxl_error_string(err));
        vxl_context_destroy(ctx);
        return 1;
    }

    err = vxl_device_open(device);
    if (err != VXL_SUCCESS) {
        printf("ERROR: Failed to open device: %s\n", vxl_error_string(err));
        vxl_device_release(device);
        vxl_context_destroy(ctx);
        return 1;
    }
    printf("Device opened successfully\n");

    /*========================================================================
     * Step 4: 获取传感器
     *========================================================================*/
    print_step(4, "Get Sensors");

    vxl_sensor_t *depth_sensor = NULL;
    vxl_sensor_t *color_sensor = NULL;
    vxl_sensor_t *ir_sensor = NULL;

    /* 获取深度传感器 (如果需要) */
    if (g_ctx.use_depth) {
        err = vxl_device_get_sensor(device, VXL_SENSOR_DEPTH, &depth_sensor);
        if (err == VXL_SUCCESS && depth_sensor) {
            printf("Found DEPTH sensor\n");

            /* 尝试设置 DEPTH_ONLY 模式 (VXL615 特有，30fps 纯深度) */
            err = vxl_sensor_set_option(depth_sensor,
                                        (vxl_option_t)VXL_OPTION_VXL615_STREAM_MODE,
                                        (float)VXL615_STREAM_MODE_DEPTH_ONLY);
            if (err == VXL_SUCCESS) {
                printf("Stream mode set to DEPTH_ONLY (30fps)\n");
            } else {
                printf("Stream mode set failed (err=%s), using default NIR+DEPTH\n",
                       vxl_error_string(err));
            }

        } else {
            printf("ERROR: No depth sensor found: %s\n", vxl_error_string(err));
        }
    }

    /* 获取颜色传感器 (如果需要) */
    if (g_ctx.use_color) {
        err = vxl_device_get_sensor(device, VXL_SENSOR_COLOR, &color_sensor);
        if (err == VXL_SUCCESS && color_sensor) {
            printf("Found COLOR sensor\n");
        } else {
            printf("No color sensor found\n");
        }
    }

    /* 获取 IR 传感器 (如果需要) */
    if (g_ctx.use_ir) {
        err = vxl_device_get_sensor(device, VXL_SENSOR_IR, &ir_sensor);
        if (err == VXL_SUCCESS && ir_sensor) {
            printf("Found IR sensor\n");
        } else {
            printf("No IR sensor found\n");
        }
    }

    /*========================================================================
     * Step 5: 选择 Profile 并创建流
     *========================================================================*/
    print_step(5, "Create Streams");

    vxl_stream_t *depth_stream = NULL;
    vxl_stream_t *color_stream = NULL;
    vxl_stream_t *ir_stream = NULL;
    vxl_profile_t *depth_profile = NULL;
    vxl_profile_t *color_profile = NULL;
    vxl_profile_t *ir_profile = NULL;
    int stream_count = 0;

    /* 双流模式检测：ir 或 rd 模式需要使用较低分辨率的 Y16 profile */
    bool is_dual_stream = (strcmp(g_ctx.mode_str, "ir") == 0 ||
                           strcmp(g_ctx.mode_str, "rd") == 0);
    int y16_profile_idx = is_dual_stream ? 1 : 0;  /* 640x400 for dual stream */

    /* 创建深度流 */
    if (depth_sensor) {
        err = vxl_sensor_get_profile(depth_sensor, y16_profile_idx, &depth_profile);
        if (err == VXL_SUCCESS && depth_profile) {
            uint32_t w, h;
            vxl_profile_get_width(depth_profile, &w);
            vxl_profile_get_height(depth_profile, &h);
            printf("Depth profile[%d]: %ux%u%s\n", y16_profile_idx, w, h,
                   is_dual_stream ? " (dual-stream mode)" : "");

            err = vxl_sensor_create_stream(depth_sensor, depth_profile, &depth_stream);
            if (err != VXL_SUCCESS) {
                printf("ERROR: Failed to create depth stream: %s\n", vxl_error_string(err));
            } else {
                printf("Depth stream created\n");
                stream_count++;
            }
        }
    }

    /* 创建 RGB 流 */
    if (color_sensor) {
        err = vxl_sensor_get_profile(color_sensor, 0, &color_profile);
        if (err == VXL_SUCCESS && color_profile) {
            uint32_t w, h;
            vxl_profile_get_width(color_profile, &w);
            vxl_profile_get_height(color_profile, &h);
            printf("Color profile: %ux%u\n", w, h);

            err = vxl_sensor_create_stream(color_sensor, color_profile, &color_stream);
            if (err != VXL_SUCCESS) {
                printf("ERROR: Failed to create color stream: %s\n", vxl_error_string(err));
            } else {
                printf("Color stream created\n");
                stream_count++;
            }
        }
    }

    /* 创建 IR 流 */
    if (ir_sensor) {
        err = vxl_sensor_get_profile(ir_sensor, y16_profile_idx, &ir_profile);
        if (err == VXL_SUCCESS && ir_profile) {
            uint32_t w, h;
            vxl_profile_get_width(ir_profile, &w);
            vxl_profile_get_height(ir_profile, &h);
            printf("IR profile[%d]: %ux%u%s\n", y16_profile_idx, w, h,
                   is_dual_stream ? " (dual-stream mode)" : "");

            err = vxl_sensor_create_stream(ir_sensor, ir_profile, &ir_stream);
            if (err != VXL_SUCCESS) {
                printf("ERROR: Failed to create IR stream: %s\n", vxl_error_string(err));
            } else {
                printf("IR stream created\n");
                stream_count++;
            }
        }
    }

    if (stream_count == 0) {
        printf("ERROR: No streams created!\n");
        if (depth_profile) vxl_profile_release(depth_profile);
        if (color_profile) vxl_profile_release(color_profile);
        if (ir_profile) vxl_profile_release(ir_profile);
        if (depth_sensor) vxl_sensor_release(depth_sensor);
        if (color_sensor) vxl_sensor_release(color_sensor);
        if (ir_sensor) vxl_sensor_release(ir_sensor);
        vxl_device_close(device);
        vxl_device_release(device);
        vxl_context_destroy(ctx);
        return 1;
    }

    /*========================================================================
     * Step 6: 启动流并采集
     *========================================================================*/
    print_step(6, "Start Streaming");

    /* 启动流 (先 Y16 后 RGB，确保双流模式正常工作) */

    /* 第一步：启动 Y16 流 (depth/ir) */
    if (depth_stream) {
        printf("Starting depth stream (Y16)...\n");
        err = vxl_stream_start(depth_stream, depth_frame_callback, NULL);
        if (err != VXL_SUCCESS) {
            printf("ERROR: Failed to start depth stream: %s\n", vxl_error_string(err));
        } else {
            printf("Depth stream started\n");
        }
    }

    if (ir_stream) {
        printf("Starting IR stream (Y16)...\n");
        err = vxl_stream_start(ir_stream, ir_frame_callback, NULL);
        if (err != VXL_SUCCESS) {
            printf("ERROR: Failed to start IR stream: %s\n", vxl_error_string(err));
        } else {
            printf("IR stream started\n");
        }
    }

    /* 第二步：启动 RGB 流 */
    if (color_stream) {
        printf("Starting color stream (RGB)...\n");
        err = vxl_stream_start(color_stream, rgb_frame_callback, NULL);
        if (err != VXL_SUCCESS) {
            printf("ERROR: Failed to start color stream: %s\n", vxl_error_string(err));
        } else {
            printf("Color stream started\n");
        }
    }

    g_ctx.running = true;
    double start_time = get_time_sec();
    double last_print = start_time;

    printf("Capturing for %d seconds...\n", capture_time);

    while (get_time_sec() - start_time < capture_time) {
        sleep_ms(100);

        /* 定期打印统计 */
        double now = get_time_sec();
        if (now - last_print >= 1.0) {
            CTX_LOCK();
            printf("  Frames:");
            if (g_ctx.use_depth) printf(" Depth=%llu", (unsigned long long)g_ctx.depth_count);
            if (g_ctx.use_color) printf(" RGB=%llu", (unsigned long long)g_ctx.rgb_count);
            if (g_ctx.use_ir) printf(" IR=%llu", (unsigned long long)g_ctx.ir_count);
            printf("\n");
            CTX_UNLOCK();
            last_print = now;
        }
    }

    g_ctx.running = false;
    printf("Capture complete!\n");

    /*========================================================================
     * Step 7: 停止流
     *========================================================================*/
    print_step(7, "Stop Streams");

    if (depth_stream) {
        vxl_stream_stop(depth_stream);
        printf("Depth stream stopped\n");
    }
    if (color_stream) {
        vxl_stream_stop(color_stream);
        printf("Color stream stopped\n");
    }
    if (ir_stream) {
        vxl_stream_stop(ir_stream);
        printf("IR stream stopped\n");
    }

    /*========================================================================
     * Step 8: 处理结果
     *========================================================================*/
    print_step(8, "Processing Results");

    /* 流已停止，安全获取最后一帧 */
    CTX_LOCK();
    vxl_frame_t *depth_frame = g_ctx.last_depth_frame;
    vxl_frame_t *rgb_frame = g_ctx.last_rgb_frame;
    vxl_frame_t *ir_frame = g_ctx.last_ir_frame;
    g_ctx.last_depth_frame = NULL;  /* 转移所有权 */
    g_ctx.last_rgb_frame = NULL;
    g_ctx.last_ir_frame = NULL;
    CTX_UNLOCK();

    if (depth_frame && do_colormap) {
        printf("Generating depth colormap...\n");
        vxl_frame_t *colormap_frame = NULL;
        err = vxl_dip_depth_colormap(depth_frame, &colormap_frame, 0, 8000);
        if (err == VXL_SUCCESS && colormap_frame) {
            uint32_t w, h;
            vxl_frame_get_width(colormap_frame, &w);
            vxl_frame_get_height(colormap_frame, &h);

            const void *data;
            vxl_frame_get_data(colormap_frame, &data);
            save_rgb_ppm("depth_colormap.ppm", (const uint8_t *)data, w, h);
            printf("Saved: depth_colormap.ppm (%ux%u)\n", w, h);

            vxl_frame_release(colormap_frame);
        } else {
            printf("ERROR: Failed to generate colormap: %s\n", vxl_error_string(err));
        }
    }

    if (depth_frame && do_pointcloud) {
        printf("Generating point cloud...\n");

        /* 使用示例内参 (实际应用需从设备获取标定数据) */
        vxl_intrinsics_t depth_intrin = {
            .fx = 580.0f,
            .fy = 580.0f,
            .cx = 320.0f,
            .cy = 200.0f,
            .width = 640,
            .height = 400
        };

        /* 获取实际深度帧尺寸 */
        vxl_frame_get_width(depth_frame, &depth_intrin.width);
        vxl_frame_get_height(depth_frame, &depth_intrin.height);
        depth_intrin.cx = depth_intrin.width / 2.0f;
        depth_intrin.cy = depth_intrin.height / 2.0f;

        /* 创建点云 */
        vxl_pointcloud_t *pc = vxl_pointcloud_create(
            depth_intrin.width * depth_intrin.height);
        if (pc) {
            if (rgb_frame) {
                /* 有 RGB，生成彩色点云 */
                vxl_intrinsics_t rgb_intrin = depth_intrin;
                vxl_frame_get_width(rgb_frame, &rgb_intrin.width);
                vxl_frame_get_height(rgb_frame, &rgb_intrin.height);
                rgb_intrin.cx = rgb_intrin.width / 2.0f;
                rgb_intrin.cy = rgb_intrin.height / 2.0f;

                /* 解码 MJPEG 到 RGB */
                vxl_frame_t *rgb_decoded = NULL;
                vxl_format_t fmt;
                vxl_frame_get_format(rgb_frame, &fmt);
                if (fmt == VXL_FORMAT_MJPEG) {
                    vxl_dip_mjpeg_to_rgb(rgb_frame, &rgb_decoded);
                } else if (fmt == VXL_FORMAT_RGB) {
                    rgb_decoded = rgb_frame;
                    vxl_frame_add_ref(rgb_decoded);
                }

                if (rgb_decoded) {
                    err = vxl_dip_rgbd_to_pointcloud(depth_frame, rgb_decoded,
                                                     &depth_intrin, &rgb_intrin,
                                                     NULL, DEPTH_SCALE, pc);
                    vxl_frame_release(rgb_decoded);
                } else {
                    err = vxl_dip_depth_to_pointcloud(depth_frame, &depth_intrin,
                                                      DEPTH_SCALE, pc);
                }
            } else {
                /* 无 RGB，生成灰度点云 */
                err = vxl_dip_depth_to_pointcloud(depth_frame, &depth_intrin,
                                                  DEPTH_SCALE, pc);
            }

            if (err == VXL_SUCCESS && pc->count > 0) {
                save_pointcloud_ply("pointcloud.ply", pc);
                printf("Saved: pointcloud.ply (%zu points)\n", pc->count);
            } else {
                printf("ERROR: Failed to generate pointcloud: %s (count=%zu)\n",
                       vxl_error_string(err), pc->count);
            }

            vxl_pointcloud_destroy(pc);
        }
    }

    if (depth_frame) vxl_frame_release(depth_frame);
    if (rgb_frame) vxl_frame_release(rgb_frame);
    if (ir_frame) vxl_frame_release(ir_frame);

    /*========================================================================
     * Step 9: 释放资源
     *========================================================================*/
    print_step(9, "Cleanup");

    /* 释放资源 (流已在 Step 7 停止) */
    if (depth_stream) vxl_stream_release(depth_stream);
    if (color_stream) vxl_stream_release(color_stream);
    if (ir_stream) vxl_stream_release(ir_stream);
    if (depth_profile) vxl_profile_release(depth_profile);
    if (color_profile) vxl_profile_release(color_profile);
    if (ir_profile) vxl_profile_release(ir_profile);
    if (depth_sensor) vxl_sensor_release(depth_sensor);
    if (color_sensor) vxl_sensor_release(color_sensor);
    if (ir_sensor) vxl_sensor_release(ir_sensor);

    vxl_device_close(device);
    vxl_device_release(device);
    vxl_context_destroy(ctx);

    printf("\n");
    printf("========================================\n");
    printf("  Fusion Example Complete!\n");
    printf("========================================\n");
    if (g_ctx.use_depth) printf("Total depth frames: %llu\n", (unsigned long long)g_ctx.depth_count);
    if (g_ctx.use_color) printf("Total RGB frames: %llu\n", (unsigned long long)g_ctx.rgb_count);
    if (g_ctx.use_ir) printf("Total IR frames: %llu\n", (unsigned long long)g_ctx.ir_count);

    CTX_DEINIT();
    return 0;
}
