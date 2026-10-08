#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// 滤波接口抽象
typedef struct filter_instance filter_t;

typedef struct {
    float (*update)(filter_t *self, float raw_val);
    void  (*reset)(filter_t *self);
} filter_ops_t;

struct filter_instance {
    const filter_ops_t *ops;
};

// --- 1. 滑动平均滤波 (Moving Average) ---
typedef struct {
    filter_t base;
    float   *buffer;
    uint8_t  window_size;
    uint8_t  index;
    uint8_t  count;
    float    sum;
} filter_moving_avg_t;

filter_t *filter_moving_avg_create(uint8_t window_size);

// --- 2. 一阶低通滤波 (First-Order IIR) ---
typedef struct {
    filter_t base;
    float    alpha;     // 滤波系数 (0.0 < alpha <= 1.0)
    float    prev_val;
    bool     initialized;
} filter_lowpass_t;

filter_t *filter_lowpass_create(float alpha);

// --- 3. 一维卡尔曼滤波 (1D Kalman) ---
typedef struct {
    filter_t base;
    float q; // 过程噪声协方差
    float r; // 测量噪声协方差
    float p; // 估计误差协方差
    float k; // 卡尔曼增益
    float x; // 估计值
} filter_kalman_t;

filter_t *filter_kalman_create(float q, float r, float p, float initial_value);

void filter_destroy(filter_t *filter);

#endif // SENSOR_FILTER_H