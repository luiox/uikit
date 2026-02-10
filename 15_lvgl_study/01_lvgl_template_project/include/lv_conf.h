#ifndef LV_CONF_H
#define LV_CONF_H

/* Minimal LVGL configuration for 800x480 RGB888/RGB565 display */

#define LV_COLOR_DEPTH 32
#define LV_HOR_RES_MAX 800
#define LV_VER_RES_MAX 480

/* Use simple include: our lv_conf.h is in include/ and lvgl headers will find it */
#define LV_CONF_INCLUDE_SIMPLE 1

/* Memory options */
#define LV_MEM_CUSTOM 0

/* Tick: we'll use lv_tick_inc in our driver */
#define LV_TICK_CUSTOM 0

/* Enable required features */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_USER_DATA 1

/* Floating point is fine for simple demos */
#define LV_USE_FLOAT 1

#endif /*LV_CONF_H*/
