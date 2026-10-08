#ifndef _TEMP_SENSOR_DRIVER_H_
#define _TEMP_SENSOR_DRIVER_H_

#include <stdint.h>

/* 定义传感器底层的操作接口 */
typedef struct {
    int (*init)(void);                                  /* 硬件初始化 (如I2C/SPI初始化) */
    int (*config)(uint32_t cmd, void *arg);             /* 硬件参数配置 (如分辨率、采样率) */
    int (*read_raw)(int32_t *raw_temp);                 /* 读取传感器原始ADC值或寄存器值 */
    int (*write_reg)(uint8_t reg, uint8_t data);        /* 直接写寄存器 (供特殊配置使用) */
    int (*deinit)(void);                                /* 硬件反初始化，进入低功耗等 */
} temp_sensor_drv_ops_t;

#endif