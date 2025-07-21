#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <stdint.h>

typedef struct {
  float x;
  float y;
}vec2f;

typedef struct {
	float x;
	float y;
	float z;
}vec3f_t;

int32_t pow32i(int32_t base, int32_t exp);

#endif // !MATH_UTIL_H
