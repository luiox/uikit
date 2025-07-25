#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <stdint.h>

typedef struct {
  float x;
  float y;
} vec2f;

typedef struct {
  float x;
  float y;
  float z;
} vec3f_t;

int32_t pow32i(int32_t base, int32_t exp);
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

#endif // !MATH_UTIL_H
