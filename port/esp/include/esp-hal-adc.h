/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP_HAL_ADC_H
#define ESP_HAL_ADC_H

#include "driver/mcpwm_prelude.h"

extern const mcpwm_timer_event_callbacks_t mcpwm_timer_cb;
extern int adc_count;
void *_configureADCLowSide(const void *driver_params, const int pinA, const int pinB, const int pinC);

#endif
