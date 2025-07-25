#include "string_util.h"

char* str_cpy(char* dest, const char* src, usize size)
{
    if (size == 0)
        return dest;

    char* ret = dest;
    usize i;

    for (i = 0; i < size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';   // 确保终止符

    return ret;
}

bool hex_str_to_uint(const char* str, u32* out_value)
{
    if (!str || !out_value)
        return false;

    u32 value = 0;
    while (*str) {
        char c = *str;
        u32  digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        }
        else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        }
        else {
            return false;   // 非法字符
        }

        if (value > (UINT32_MAX - digit) / 16) {
            return false;   // 溢出
        }

        value = value * 16 + digit;
        str++;
    }

    *out_value = value;
    return true;
}


void uint_to_hex_str(u32 value, char* out_str, size_t out_size)
{
    if (!out_str || out_size == 0)
        return;

    const char* digits = "0123456789ABCDEF";
    usize       pos    = out_size - 1;
    out_str[pos]       = '\0';   // 终止符

    do {
        pos--;
        out_str[pos] = digits[value & 0xF];
        value >>= 4;
    } while (pos > 0 && value != 0);

    // 如果还有空间，左移字符串
    if (pos > 0) {
        memmove(out_str, &out_str[pos], out_size - pos);
    }
}
