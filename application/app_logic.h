#ifndef _APP_LOGIC_H_
#define _APP_LOGIC_H_

#include "temp_sensor_framework.h"

/* 
 * App业务不需要改变框架，只需把以下函数通过 sensor_fw_subscribe 注册进去。
 * 数据更新时，Framework会自动调用它们。
 */

/* 1. GUI 业务模块接口 */
void app_gui_update_cb(float temp, void *user_data);

/* 2. MQTT 数据上报模块接口 */
void app_mqtt_publish_cb(float temp, void *user_data);

/* 3. 业务计算/报警模块接口 */
void app_business_calc_cb(float temp, void *user_data);

/* App层初始化逻辑 (用于统一订阅和配置) */
void app_layer_init(temp_sensor_dev_t *target_sensor);

#endif