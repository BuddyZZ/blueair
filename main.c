// main.c
#include <stdio.h>
#include <unistd.h>
#include "sensor_core.h"
#include "sensor_filter.h"

extern const sensor_ops_t g_temp_sensor_ops;
extern void gui_on_temp_update(const sensor_event_t *event, void *user_data);
extern void mqtt_on_temp_update(const sensor_event_t *event, void *user_data);
extern void logic_on_temp_update(const sensor_event_t *event, void *user_data);

int main(void) {
    printf("=== IoT Cross-Platform Sensor Framework Demo ===\n\n");

    // 1. 准备 MCU 相关的硬件配置结构体
    struct { uint8_t bus; uint8_t addr; } stm32_hw = { .bus = 1, .addr = 0x48 };

    // 2. 实例化并注册温度传感器
    sensor_dev_t temp_sensor;
    sensor_register(&temp_sensor, "Room_Temp", SENSOR_TYPE_TEMP, &g_temp_sensor_ops, &stm32_hw);

    // 3. 灵活挂载滤波模式 (三选一，可随时切换)
    // 方案 A: 卡尔曼滤波
    filter_t *kalman = filter_kalman_create(0.01f, 0.25f, 1.0f, 25.0f);
    sensor_set_filter(&temp_sensor, kalman);

    /* 方案 B: 一阶低通 (若改用低通，解开注释即可)
    filter_t *lowpass = filter_lowpass_create(0.2f);
    sensor_set_filter(&temp_sensor, lowpass);
    */

    /* 方案 C: 滑动平均 (5点滑动)
    filter_t *moving_avg = filter_moving_avg_create(5);
    sensor_set_filter(&temp_sensor, moving_avg);
    */

    // 4. 绑定应用层订阅（完成 GUI、MQTT 和业务计算的解耦）
    alarm_ctx_t alarm_cfg = { .threshold = 25.4f };

    sensor_subscribe(&temp_sensor, gui_on_temp_update, NULL);
    sensor_subscribe(&temp_sensor, mqtt_on_temp_update, (void *)"Client_ESP32");
    sensor_subscribe(&temp_sensor, logic_on_temp_update, &alarm_cfg);

    // 5. 模拟 MCU 主循环调度采样 (Poll)
    printf("\n--- Starting Sensor Sampling Engine Loop ---\n");
    for (int i = 0; i < 5; i++) {
        printf("\n--- Tick %d ---\n", i + 1);
        sensor_poll(&temp_sensor, i * 1000);
        usleep(500000); // 延时 500ms
    }

    // 资源释放
    filter_destroy(kalman);
    return 0;
}