/**
 * @file ping_pong_buffer.h
 * @author canrad (1517807724@qq.com)
 * @brief 乒乓缓冲区，即双缓冲区，主要是配合DMA使用
 *        乒乓缓冲区的原理是有两个缓冲区，交替使用，
 *        当一个缓冲区正在被处理时，另一个缓冲区可以被填充数据。
 *        这样可以减少数据处理的延迟，提高数据传输效率。
 * @version 0.1
 * @date 2025-07-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef PING_PONG_BUFFER_H
#define PING_PONG_BUFFER_H

#include "datatype.h"

typedef struct
{
    u8* read_buffer;
    u8* write_buffer;
    u16 buffer_size;
}ping_pong_buffer_t;

// 函数声明
void ping_pong_buffer_init(ping_pong_buffer_t* ping_pong_buf, u8* buffer1, u8* buffer2, u16 buffer_size);
void ping_pong_buffer_switch(ping_pong_buffer_t* ping_pong_buf);
u8* ping_pong_buffer_get_read_buffer(ping_pong_buffer_t* ping_pong_buf);
u8* ping_pong_buffer_get_write_buffer(ping_pong_buffer_t* ping_pong_buf);
u16 ping_pong_buffer_get_size(ping_pong_buffer_t* ping_pong_buf);
void ping_pong_buffer_clear(u8* buffer, u16 size);

#endif // PING_PONG_BUFFER_H