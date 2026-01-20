/**
 * @file led.h
 * @author canrad (1517807724@qq.com)
 * @brief LED driver. Port initialization is guaranteed by the board adapter to avoid null pointer risks.
 * @version 0.1
 * @date 2026-01-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef DRIVER_LED_H
#define DRIVER_LED_H

#include "../datatype.h"

// LED states
typedef enum led_state_enum
{
    led_state_off,
    led_state_on,
    led_state_unknown
} led_state;

typedef struct led
{
    void* gpio;
    u16 pin;
    u8 valid; // active level: 1 means active-high
    led_state state; // current LED state
} led_t;

// port
typedef struct led_port{
    void (*write_pin)(void* gpio, u16 pin, u8 value);
}led_port_t;

// bind port
void led_bind_port(const led_port_t* port);

// API

// Initialize LED
void led_init(led_t* led, void* gpio, u16 pin, u8 valid);
// Turn on
void led_on(led_t* led);
// Turn off
void led_off(led_t* led);
// Toggle LED state
void led_toggle(led_t* led);


#endif // !DRIVER_LED_H
