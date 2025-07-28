#include "math_util.h"
#include <stdbool.h>

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
    }
    else if (value > max) {
        return max;
    }
    return value;
}

static s8 endian_flag = -1;
// 是否是小端序
bool is_little_endian()
{
    if (endian_flag != -1) {
        return endian_flag == 1;
    }
    union
    {
        u8  bytes[2];
        u16 value;
    } u;
    u.value = 0x0102;
    if (u.bytes[0] == 0x01) {
        endian_flag = 1;
        return true;
    }
    endian_flag = 0;
    return false;
}

// 是否是大端序
bool is_big_endian()
{
    return !is_little_endian();
}
