#include "misc.h"
#include "../driver/led.h"
#include "../board/led_port.h"


led_t g_led1,g_led2;

void init_model(void)
{
    led_port_init();

    led_init(&g_led1, (void*)0x1000, 0x0, 1);
    led_init(&g_led2, (void*)0x1000, 0x1, 1);
}

void open_led(void)
{
    led_on(&g_led1);
    led_on(&g_led2);
}

void close_led(void)
{
    led_off(&g_led1);
    led_off(&g_led2);
}
