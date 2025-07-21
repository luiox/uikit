#ifndef DATATYPE_H
#define DATATYPE_H

#include <stdint.h>
#include <stdlib.h>

// 整数
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
#ifdef HAS_INT64
typedef uint64_t u64;
typedef int64_t s64;
typedef volatile u64 vu64;
typedef volatile s64 vs64;
#endif
// 浮点数
typedef float f32;
typedef double f64;
typedef volatile f32 vf32;
typedef volatile f64 vf64;
// size
typedef size_t usize;
typedef volatile size_t vsize;

// 对于仅需要存位级别的数据，但是不需要很精准控制高低位的情况下可以使用下面这个
// 8个位
typedef union {
    struct {
        u8 b0 : 1; // 位0
        u8 b1 : 1; // 位1
        u8 b2 : 1; // 位2
        u8 b3 : 1; // 位3
        u8 b4 : 1; // 位4
        u8 b5 : 1; // 位5
        u8 b6 : 1; // 位6
        u8 b7 : 1; // 位7
    } u;
    u8 val; // 整体值
}bits8_t;

// 16个位
typedef union {
    struct {
        u8 b0 : 1; // 位0
        u8 b1 : 1; // 位1
        u8 b2 : 1; // 位2
        u8 b3 : 1; // 位3
        u8 b4 : 1; // 位4
        u8 b5 : 1; // 位5
        u8 b6 : 1; // 位6
        u8 b7 : 1; // 位7
        u8 b8 : 1; // 位8
        u8 b9 : 1; // 位9
        u8 b10: 1; // 位10
        u8 b11: 1; // 位11
        u8 b12: 1; // 位12
        u8 b13: 1; // 位13
        u8 b14: 1; // 位14
        u8 b15: 1; // 位15
    } u;
    u16 val; // 整体值
}bits16_t;

// 对于需要精准控制高低位的情况下使用下面的宏
// 其中bits是一个整数类型的变量，一般是u8、u16、u32等
// n是位的索引，从0开始
// val是要设置的值，0或1
// 取一个位上的值
#define bits_get(bits, n) (((bits) >> (n)) & 0x01)
// 设置一个位上的值
#define bits_set(bits, n, val) ((bits) = ((bits) & ~(1 << (n))) | ((val) << (n)))
// 反转一个位上的值
#define bits_toggle(bits, n) ((bits) ^= (1 << (n)))

#endif // !DATATYPE_H
