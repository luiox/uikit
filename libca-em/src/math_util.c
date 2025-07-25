#include "math_util.h"

int32_t pow32i(int32_t base, int32_t exp)
{
    // 计算思路是每次将指数右移一位，底数平方，直到指数为0
    // 这样速度会比直接循环快很多
    int32_t result = 1;
    while (exp > 0) {
        if (exp & 1) {
            // 奇数次方
            result *= base;
        }
        base *= base;
        exp >>= 1;
    }
    return result;
}

float clampf(float value, float min, float max)
{
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }
    return value;
}