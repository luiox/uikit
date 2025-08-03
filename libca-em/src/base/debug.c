#include <mylib/utility/debug.h>
#include <mylib/driver/uart.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// 定义一个缓冲区大小
#define DEBUG_BUFFER_SIZE 256

static uint32_t g_uart;

static inline void usart_send_string(const char* str)
{
    uart_send(g_uart, (void*)str, strlen(str));
}

void debug_init(uint32_t uart)
{
    g_uart = uart;
}

// 平台无关，无需修改
void debug_print(const char* fmt, ...)
{
    va_list args;                     // 定义一个 va_list 类型的变量，用来存放参数
    char buffer[DEBUG_BUFFER_SIZE];   // 定义一个字符数组，用来存放格式化后的字符串

    va_start(args, fmt);   // 初始化 args，fmt 是最后一个固定参数

    // 使用 vsprintf 将参数格式化到 buffer 中
    vsprintf(buffer, fmt, args);

    va_end(args);   // 清理 args

    // 输出格式化后的字符串
    // 这里是发送到串口
    usart_send_string(buffer);
}
