/*
 * datatype.h - basic integer and utility type definitions
 *
 */
#ifndef DATATYPE_H
#define DATATYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 整数
typedef uint8_t      u8;
typedef uint16_t     u16;
typedef uint32_t     u32;
typedef int8_t       i8;
typedef int16_t      i16;
typedef int32_t      i32;
#ifdef HAS_INT64
typedef uint64_t     u64;
typedef int64_t      i64;
#endif
// 浮点数
typedef float        f32;
typedef double       f64;
// size
typedef size_t          usize;

#endif   // !DATATYPE_H
