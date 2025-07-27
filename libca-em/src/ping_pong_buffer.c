#include "ping_pong_buffer.h"

/**
 * @brief 初始化乒乓缓冲区
 * 
 * @param ping_pong_buf 乒乓缓冲区结构体指针
 * @param buffer1 缓冲区1指针
 * @param buffer2 缓冲区2指针
 * @param buffer_size 缓冲区大小
 */
void ping_pong_buffer_init(ping_pong_buffer_t* ping_pong_buf, u8* buffer1, u8* buffer2, u16 buffer_size)
{
    ping_pong_buf->read_buffer = buffer1;
    ping_pong_buf->write_buffer = buffer2;
    ping_pong_buf->buffer_size = buffer_size;
}

/**
 * @brief 切换读写缓冲区
 * 
 * @param ping_pong_buf 乒乓缓冲区结构体指针
 */
void ping_pong_buffer_switch(ping_pong_buffer_t* ping_pong_buf)
{
    u8* temp = ping_pong_buf->read_buffer;
    ping_pong_buf->read_buffer = ping_pong_buf->write_buffer;
    ping_pong_buf->write_buffer = temp;
}

/**
 * @brief 获取读缓冲区指针
 * 
 * @param ping_pong_buf 乒乓缓冲区结构体指针
 * @return u8* 读缓冲区指针
 */
u8* ping_pong_buffer_get_read_buffer(ping_pong_buffer_t* ping_pong_buf)
{
    return ping_pong_buf->read_buffer;
}

/**
 * @brief 获取写缓冲区指针
 * 
 * @param ping_pong_buf 乒乓缓冲区结构体指针
 * @return u8* 写缓冲区指针
 */
u8* ping_pong_buffer_get_write_buffer(ping_pong_buffer_t* ping_pong_buf)
{
    return ping_pong_buf->write_buffer;
}

/**
 * @brief 获取缓冲区大小
 * 
 * @param ping_pong_buf 乒乓缓冲区结构体指针
 * @return u16 缓冲区大小
 */
u16 ping_pong_buffer_get_size(ping_pong_buffer_t* ping_pong_buf)
{
    return ping_pong_buf->buffer_size;
}

/**
 * @brief 清空指定缓冲区
 * 
 * @param buffer 要清空的缓冲区指针
 * @param size 缓冲区大小
 */
void ping_pong_buffer_clear(u8* buffer, u16 size)
{
    for (u16 i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}