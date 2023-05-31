/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ESP_HAL_MCPWM_H
#define ESP_HAL_MCPWM_H

#include "driver/mcpwm_prelude.h"

// default pwm frequency and max pwm frequency
#define _PWM_FREQUENCY 25000                           // default
#define _PWM_FREQUENCY_MAX 50000                       // max
#define _PWM_TIMEBASE_RESOLUTION_HZ (10 * 1000 * 1000) // 10MHZ 0.1us per tick

typedef struct ESP32MCPWMDriverParams
{
    long pwm_frequency;
    uint32_t pwm_timeperiod;
    mcpwm_cmpr_handle_t comparator[6]; // A maximum of six comparators can be set
} ESP32MCPWMDriverParams;

#endif
