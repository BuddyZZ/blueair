// sensor_core.h
#ifndef SENSOR_CORE_H
#define SENSOR_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_filter.h"

#define MAX_SUBSCRIBERS_PER_SENSOR 5

typedef enum {
    SENSOR_TYPE_TEMP,
    SENSOR_TYPE_HUMI,
    SENSOR_TYPE_PRESSURE
} sensor_type_t;

typedef struct {
    sensor_type_t type;
    float         value;      // 滤波后的最终值
    float         raw_value;  // 滤波前的原始数据
    uint32_t      timestamp;  // 系统时间戳 (ms)
} sensor_event_t;

// 事件回调定义
typedef void (*sensor_event_cb_t)(const sensor_event_t *event, void *user_data);

typedef struct {
    sensor_event_cb_t cb;
    void             *user_data;
} subscriber_t;

// 传感器硬件操作接口抽象 (实现跨 MCU 移植)
typedef struct {
    bool (*init)(void *hw_config);
    bool (*read_raw)(void *hw_config, float *raw_val);
} sensor_ops_t;

// 传感器控制块 (Sensor Object)
typedef struct {
    const char          *name;
    sensor_type_t        type;
    const sensor_ops_t  *ops;
    void                *hw_config; // MCU 硬件相关句柄 (如 I2C 句柄/GPIO 引脚)
    filter_t            *filter;    // 可动态替换的滤波策略
    
    // 标定参数 (Y = raw * scale + offset)
    float                scale;
    float                offset;

    // 观察者列表
    subscriber_t         subscribers[MAX_SUBSCRIBERS_PER_SENSOR];
    uint8_t              sub_count;
} sensor_dev_t;

// API
bool sensor_register(sensor_dev_t *dev, const char *name, sensor_type_t type, 
                     const sensor_ops_t *ops, void *hw_config);
bool sensor_set_filter(sensor_dev_t *dev, filter_t *filter);
bool sensor_subscribe(sensor_dev_t *dev, sensor_event_cb_t cb, void *user_data);
void sensor_poll(sensor_dev_t *dev, uint32_t current_timestamp_ms);

#endif // SENSOR_CORE_H