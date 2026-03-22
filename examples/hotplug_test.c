/**
 * @file hotplug_test.c
 * @brief VXL 热插拔功能测试
 */

#include <vxl_context.h>
#include <vxl_types.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile int g_running = 1;

static void signal_handler(int sig)
{
    (void)sig;
    printf("\nReceived signal, stopping...\n");
    g_running = 0;
}

static void device_event_callback(
    const vxl_device_info_t *info,
    bool added,
    void *user_data)
{
    (void)user_data;

    printf("\n=== Device %s ===\n", added ? "CONNECTED" : "DISCONNECTED");
    printf("  Name: %s\n", info->name);
    printf("  VID:  0x%04X\n", info->vendor_id);
    printf("  PID:  0x%04X\n", info->product_id);
    if (info->serial_number[0]) {
        printf("  S/N:  %s\n", info->serial_number);
    }
    printf("==================\n\n");
}

int main(void)
{
    printf("============================================================\n");
    printf("  VXL Hotplug Test\n");
    printf("============================================================\n\n");

    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 创建 context */
    vxl_context_t *ctx = NULL;
    vxl_error_t err = vxl_context_create(&ctx);
    if (err != VXL_SUCCESS) {
        printf("Failed to create context: %d\n", err);
        return 1;
    }

    /* 获取当前设备数量 */
    size_t count = 0;
    vxl_context_get_device_count(ctx, &count);
    printf("Initial device count: %zu\n\n", count);

    /* 注册热插拔回调 */
    printf("Registering hotplug callback...\n");
    err = vxl_context_set_device_event_callback(ctx, device_event_callback, NULL);
    if (err != VXL_SUCCESS) {
        printf("Failed to set callback: %d\n", err);
    } else {
        printf("Callback registered.\n");
    }

    printf("\nWaiting for device events (Ctrl+C to exit)...\n\n");

    /* 等待事件 */
    while (g_running) {
        sleep(1);
    }

    /* 取消回调 */
    printf("Unregistering callback...\n");
    vxl_context_set_device_event_callback(ctx, NULL, NULL);

    /* 销毁 context */
    vxl_context_destroy(ctx);

    printf("Test completed.\n");
    return 0;
}
