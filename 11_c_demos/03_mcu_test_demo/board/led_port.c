#include "led_port.h"
#include "../driver/led.h"
#include <stdio.h>

static led_port_t g_led_port = {0};

static void led_write_pin(void* gpio, u16 pin, u8 value)
{
    printf("write pin, gpio:%p, pin:%u, value:%u\n", gpio, (unsigned)pin, (unsigned)value);
}

void led_port_init(void)
{
    g_led_port.write_pin = led_write_pin;
    led_bind_port(&g_led_port);
}
