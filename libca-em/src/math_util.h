/**
 * @file math_util.h
 * @author canrad (1517807724@qq.com)
 * @brief 数学相关的工具函数，补充标准库的不足
 * @version 0.1
 * @date 2025-07-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include "datatype.h"
#include <stdbool.h>

typedef struct {
  float x;
  float y;
} vec2f;

typedef struct {
  float x;
  float y;
  float z;
} vec3f_t;

// 最小值
#define math_min(a, b) ((a) < (b) ? (a) : (b))
// 最大值
#define math_max(a, b) ((a) > (b) ? (a) : (b))
// 绝对值
#define math_abs(x) ((x) < 0 ? -(x) : (x))
// 判断是否是2的幂
#define math_is_power_of_two(x) ((x) && !((x) & ((x) - 1)))
// 向上对齐
#define math_align_up(x, align) (((x) + (align) - 1) & ~((align) - 1))
// 向下对齐
#define math_align_down(x, align) ((x) & ~((align) - 1))

/**
 * @brief 计算整数的幂
 * 
 * @param base 底数
 * @param exp 指数
 * @return int32_t 底数的指数次幂
 */
i32 pow32i(i32 base, i32 exp);
/**
 * @brief 限幅函数，将输入值限制在min和max之间
 * 
 * @param value 输入值
 * @param min 最大值
 * @param max 最小值
 * @return 限幅后的值 
 */
float clampf(float value, float min, float max);

// 是否是小端序
bool is_little_endian();
// 是否是大端序
bool is_big_endian();

#endif // !MATH_UTIL_H
