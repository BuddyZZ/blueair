// temp_sensor_bsp.c
#include "sensor_core.h"
#include <stdio.h>

// 模拟特定 MCU (如 STM32 HAL / ESP32 IDF) 的硬件私有结构体
typedef struct {
    uint8_t  i2c_bus_id;
    uint8_t  dev_addr;
} stm32_i2c_temp_hw_t;

static bool mock_temp_init(void *hw_config) {
    stm32_i2c_temp_hw_t *hw = (stm32_i2c_temp_hw_t *)hw_config;
    printf("[BSP] Initializing I2C Bus %d, Dev Addr: 0x%02X\n", hw->i2c_bus_id, hw->dev_addr);
    return true;
}

static bool mock_temp_read_raw(void *hw_config, float *raw_val) {
    (void)hw_config;
    // 模拟读取原始温度并注入波动噪声
    static float base_temp = 25.0f;
    float noise = ((float)(rand() % 100) / 50.0f) - 1.0f; // -1.0C ~ +1.0C 随机噪声
    *raw_val = base_temp + noise;
    return true;
}

// 导出传感器 Ops
const sensor_ops_t g_temp_sensor_ops = {
    .init = mock_temp_init,
    .read_raw = mock_temp_read_raw
};