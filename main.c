#include <stdio.h>
#include <unistd.h>
#include "temp_sensor_driver.h"
#include "temp_sensor_framework.h"
#include "app_logic.h"

/* ============================================================================
 * 1. Driver 层：具体硬件驱动实现示例 (以 SHT30 传感器为例)
 * ============================================================================ */
static int sht30_hardware_init(void) {
    printf("[Driver] SHT30 I2C Hardware Init OK.\n");
    return 0;
}

static int sht30_read_raw_data(int32_t *raw_temp) {
    // 模拟从硬件 I2C 寄存器读取到的原始数据 (例如代表 26.5 ℃ 的原始刻度)
    *raw_temp = 2650; 
    return 0;
}

// 绑定底层驱动的操作函数句柄
static temp_sensor_drv_ops_t g_sht30_drv_ops = {
    .init = sht30_hardware_init,
    .config = NULL,
    .read_raw = sht30_read_raw_data,
    .write_reg = NULL,
    .deinit = NULL
};

/* ============================================================================
 * 2. Framework 层扩展：具体滤波器策略实现示例 (以 滑动平均滤波 为例)
 * ============================================================================ */
typedef struct {
    float buffer[4];
    uint8_t index;
} moving_avg_ctx_t;

static void* moving_avg_init(void *config) {
    static moving_avg_ctx_t ctx = {0};
    return &ctx;
}

static float moving_avg_process(void *ctx, float new_val) {
    moving_avg_ctx_t *m_ctx = (moving_avg_ctx_t*)ctx;
    m_ctx->buffer[m_ctx->index] = new_val;
    m_ctx->index = (m_ctx->index + 1) % 4;

    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += m_ctx->buffer[i];
    }
    return sum / 4.0f; // 返回均值
}

// 绑定滤波器算法接口
static sensor_filter_ops_t g_moving_avg_filter_ops = {
    .init = moving_avg_init,
    .process = moving_avg_process,
    .deinit = NULL
};

/* ============================================================================
 * 3. App 层：各独立业务的回调实现 (解耦，互不干扰)
 * ============================================================================ */
void app_gui_update_cb(float temp, void *user_data) {
    printf("[App - GUI]  刷新UI界面显示温度: %.2f ℃\n", temp);
}

void app_mqtt_publish_cb(float temp, void *user_data) {
    printf("[App - MQTT] 推送 JSON 数据包至 MQTT Broker, Temp: %.2f\n", temp);
}

void app_business_calc_cb(float temp, void *user_data) {
    if (temp > 30.0f) {
        printf("[App - CALC] 警告：温度过高! 触发风扇启动逻辑...\n");
    } else {
        printf("[App - CALC] 温度正常，维持运行.\n");
    }
}

/* ============================================================================
 * 4. 系统初始化与主运行入口 (系统组装)
 * ============================================================================ */
int main(void) {
    // A. 创建传感器设备实例并绑定驱动
    temp_sensor_dev_t sht30_dev = {
        .name = "SHT30_Room_Sensor",
        .drv_ops = &g_sht30_drv_ops
    };

    // B. 注册并初始化传感器
    sensor_fw_register_device(&sht30_dev);
    sensor_fw_init(&sht30_dev);

    // C. 动态挂载滤波算法 (此处挂载滑动平均滤波，也可随时替换为卡尔曼滤波)
    sensor_fw_set_filter(&sht30_dev, &g_moving_avg_filter_ops, NULL);

    // D. App 层模块各自向框架订阅数据关注 (观察者模式解耦)
    sensor_fw_subscribe(&sht30_dev, app_gui_update_cb, NULL);
    sensor_fw_subscribe(&sht30_dev, app_mqtt_publish_cb, NULL);
    sensor_fw_subscribe(&sht30_dev, app_business_calc_cb, NULL);

    printf("\n=== 传感器系统启动完成，开始采集轮询 ===\n\n");

    // E. 业务主循环/定时器任务
    for (int i = 0; i < 3; i++) {
        // 框架统一触发采样、滤波并通知所有订阅的 App 业务
        sensor_fw_poll_and_notify(&sht30_dev);
        sleep(1);
    }

    return 0;
}