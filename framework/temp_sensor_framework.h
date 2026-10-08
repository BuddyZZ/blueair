#ifndef _TEMP_SENSOR_FRAMEWORK_H_
#define _TEMP_SENSOR_FRAMEWORK_H_

#include "temp_sensor_driver.h"
#include <stdbool.h>

#define MAX_SUBSCRIBERS 4

/* ---------------------------------------------------------
 * 1. 灵活的滤波策略接口 (策略模式)
 * ---------------------------------------------------------*/
typedef struct {
    void* (*init)(void *config);                      /* 初始化滤波器，返回滤波器上下文 */
    float (*process)(void *ctx, float new_val);       /* 执行滤波算法，输入新值，输出滤波后的值 */
    void  (*deinit)(void *ctx);                       /* 销毁滤波器 */
} sensor_filter_ops_t;

/* ---------------------------------------------------------
 * 2. 跨平台OS抽象 (针对多进程/多任务保护)
 * ---------------------------------------------------------*/
typedef void* os_mutex_t;

/* ---------------------------------------------------------
 * 3. App层回调函数定义 (观察者模式)
 * ---------------------------------------------------------*/
typedef void (*sensor_data_cb_t)(float filtered_temp, void *user_data);

/* ---------------------------------------------------------
 * 4. 框架层 Sensor 设备对象抽象
 * ---------------------------------------------------------*/
typedef struct {
    const char              *name;              /* 传感器名称，如 "SHT30_TEMP" */
    
    /* 底层驱动挂载 */
    temp_sensor_drv_ops_t   *drv_ops;           
    
    /* 滤波策略挂载 */
    sensor_filter_ops_t     *filter_ops;        
    void                    *filter_ctx;        /* 滤波器上下文状态 */
    
    /* 跨任务/多进程访问保护 */
    os_mutex_t              lock;               
    
    /* 订阅者列表 (解耦App) */
    sensor_data_cb_t        subscribers[MAX_SUBSCRIBERS]; 
    void                    *sub_ctx[MAX_SUBSCRIBERS];    
    uint8_t                 sub_count;
} temp_sensor_dev_t;

/* ---------------------------------------------------------
 * 5. 框架层对外暴露的 API 接口
 * ---------------------------------------------------------*/

/* 系统启动时，将底层驱动和设备对象注册到框架中 */
int sensor_fw_register_device(temp_sensor_dev_t *dev);

/* 初始化传感器框架对象 (内部打包调用 driver init) */
int sensor_fw_init(temp_sensor_dev_t *dev);

/* 配置滤波方式 (如传入滑动平均滤波、卡尔曼滤波的ops) */
int sensor_fw_set_filter(temp_sensor_dev_t *dev, sensor_filter_ops_t *filter, void *filter_cfg);

/* App层主动读取数据的接口 (内部包含加锁、读取、转换、滤波、解锁) */
int sensor_fw_read_temp(temp_sensor_dev_t *dev, float *out_temp);

/* App层订阅传感器数据 (注册回调，业务分离) */
int sensor_fw_subscribe(temp_sensor_dev_t *dev, sensor_data_cb_t cb, void *user_data);

/* 框架层主线程或定时器中调用的轮询分发函数，将新数据推送给所有App订阅者 */
void sensor_fw_poll_and_notify(temp_sensor_dev_t *dev);

#endif