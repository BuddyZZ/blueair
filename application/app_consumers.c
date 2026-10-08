// app_consumers.c
#include <stdio.h>
#include "sensor_core.h"

// --- 模块 A: GUI 渲染任务 (例如 LVGL UI) ---
void gui_on_temp_update(const sensor_event_t *event, void *user_data) {
    (void)user_data;
    printf("[UI Layer] Update Label -> Temp: %.2f °C\n", event->value);
}

// --- 模块 B: MQTT 上报任务 ---
void mqtt_on_temp_update(const sensor_event_t *event, void *user_data) {
    const char *client_id = (const char *)user_data;
    // 在 RTOS 环境下，通常在此处将数据写入 Message Queue 避免阻塞 Sensor 采样主频
    printf("[MQTT Layer][%s] Publish Topic: /sys/sensor/temp, Payload: {\"val\": %.2f}\n", 
           client_id, event->value);
}

// --- 模块 C: 业务控制逻辑 (超温报警控制) ---
typedef struct {
    float threshold;
} alarm_ctx_t;

void logic_on_temp_update(const sensor_event_t *event, void *user_data) {
    alarm_ctx_t *ctx = (alarm_ctx_t *)user_data;
    if (event->value > ctx->threshold) {
        printf("[Logic Layer] ALARM! Temp (%.2f °C) exceeds limit (%.1f °C)! Turn on Fan.\n", 
               event->value, ctx->threshold);
    } else {
        printf("[Logic Layer] Temp normal (%.2f °C).\n", event->value);
    }
}