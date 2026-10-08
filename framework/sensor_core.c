// sensor_core.c
#include "sensor_core.h"
#include <string.h>

bool sensor_register(sensor_dev_t *dev, const char *name, sensor_type_t type, 
                     const sensor_ops_t *ops, void *hw_config) {
    if (!dev || !ops) return false;
    memset(dev, 0, sizeof(sensor_dev_t));
    dev->name = name;
    dev->type = type;
    dev->ops = ops;
    dev->hw_config = hw_config;
    dev->scale = 1.0f;
    dev->offset = 0.0f;
    
    if (dev->ops->init) {
        return dev->ops->init(dev->hw_config);
    }
    return true;
}

bool sensor_set_filter(sensor_dev_t *dev, filter_t *filter) {
    if (!dev) return false;
    dev->filter = filter;
    return true;
}

bool sensor_subscribe(sensor_dev_t *dev, sensor_event_cb_t cb, void *user_data) {
    if (!dev || !cb || dev->sub_count >= MAX_SUBSCRIBERS_PER_SENSOR) return false;
    dev->subscribers[dev->sub_count].cb = cb;
    dev->subscribers[dev->sub_count].user_data = user_data;
    dev->sub_count++;
    return true;
}

void sensor_poll(sensor_dev_t *dev, uint32_t current_timestamp_ms) {
    if (!dev || !dev->ops || !dev->ops->read_raw) return;

    float raw_val = 0.0f;
    if (!dev->ops->read_raw(dev->hw_config, &raw_val)) return;

    // 1. 物理标定纠偏
    float calibrated_val = raw_val * dev->scale + dev->offset;

    // 2. 策略滤波
    float filtered_val = calibrated_val;
    if (dev->filter && dev->filter->ops && dev->filter->ops->update) {
        filtered_val = dev->filter->ops->update(dev->filter, calibrated_val);
    }

    // 3. 构建发布事件
    sensor_event_t event = {
        .type = dev->type,
        .raw_value = raw_val,
        .value = filtered_val,
        .timestamp = current_timestamp_ms
    };

    // 4. 分发通知给所有订阅者 (GUI/MQTT/Business)
    for (uint8_t i = 0; i < dev->sub_count; i++) {
        if (dev->subscribers[i].cb) {
            dev->subscribers[i].cb(&event, dev->subscribers[i].user_data);
        }
    }
}