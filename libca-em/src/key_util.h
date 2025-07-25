/**
 * @file key_util.h
 * @author canrad (1517807724@qq.com)
 * @brief 硬件按键的常用代码，包括按键去抖动，按键长按、短按、双击检测等
 * @version 0.1
 * @date 2025-04-12
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef KEY_UTIL_H
#define KEY_UTIL_H

#include "datatype.h"

typedef struct
{
    s32 keyState;
    s32 judgeState;
    s32 singleFlag;
    s32 longFlag;
    s32 time;
} key_t;

void key_scan();

#endif   // !KEY_UTIL_H
