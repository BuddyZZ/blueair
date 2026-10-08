#include "sensor_filter.h"

// --- 滑动平均实现 ---
static float moving_avg_update(filter_t *self, float raw) {
    filter_moving_avg_t *f = (filter_moving_avg_t *)self;
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = raw;
    f->sum += raw;
    f->index = (f->index + 1) % f->window_size;
    if (f->count < f->window_size) f->count++;
    return f->sum / f->count;
}

static void moving_avg_reset(filter_t *self) {
    filter_moving_avg_t *f = (filter_moving_avg_t *)self;
    for (uint8_t i = 0; i < f->window_size; i++) f->buffer[i] = 0.0f;
    f->index = 0; f->count = 0; f->sum = 0.0f;
}

static const filter_ops_t moving_avg_ops = { .update = moving_avg_update, .reset = moving_avg_reset };

filter_t *filter_moving_avg_create(uint8_t window_size) {
    filter_moving_avg_t *f = (filter_moving_avg_t *)malloc(sizeof(filter_moving_avg_t));
    if (!f) return NULL;
    f->base.ops = &moving_avg_ops;
    f->window_size = window_size;
    f->buffer = (float *)calloc(window_size, sizeof(float));
    f->index = 0; f->count = 0; f->sum = 0.0f;
    return (filter_t *)f;
}

// --- 一阶低通实现 ---
static float lowpass_update(filter_t *self, float raw) {
    filter_lowpass_t *f = (filter_lowpass_t *)self;
    if (!f->initialized) {
        f->prev_val = raw;
        f->initialized = true;
    }
    f->prev_val = f->alpha * raw + (1.0f - f->alpha) * f->prev_val;
    return f->prev_val;
}

static void lowpass_reset(filter_t *self) {
    filter_lowpass_t *f = (filter_lowpass_t *)self;
    f->initialized = false;
}

static const filter_ops_t lowpass_ops = { .update = lowpass_update, .reset = lowpass_reset };

filter_t *filter_lowpass_create(float alpha) {
    filter_lowpass_t *f = (filter_lowpass_t *)malloc(sizeof(filter_lowpass_t));
    if (!f) return NULL;
    f->base.ops = &lowpass_ops;
    f->alpha = alpha;
    f->initialized = false;
    return (filter_t *)f;
}

// --- 卡尔曼滤波实现 ---
static float kalman_update(filter_t *self, float raw) {
    filter_kalman_t *f = (filter_kalman_t *)self;
    f->p = f->p + f->q;
    f->k = f->p / (f->p + f->r);
    f->x = f->x + f->k * (raw - f->x);
    f->p = (1.0f - f->k) * f->p;
    return f->x;
}

static void kalman_reset(filter_t *self) { (void)self; }

static const filter_ops_t kalman_ops = { .update = kalman_update, .reset = kalman_reset };

filter_t *filter_kalman_create(float q, float r, float p, float initial_value) {
    filter_kalman_t *f = (filter_kalman_t *)malloc(sizeof(filter_kalman_t));
    if (!f) return NULL;
    f->base.ops = &kalman_ops;
    f->q = q; f->r = r; f->p = p; f->x = initial_value;
    return (filter_t *)f;
}

void filter_destroy(filter_t *filter) {
    if (!filter) return;
    if (filter->ops == &moving_avg_ops) {
        free(((filter_moving_avg_t *)filter)->buffer);
    }
    free(filter);
}