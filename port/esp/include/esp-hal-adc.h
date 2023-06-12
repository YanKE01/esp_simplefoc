/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP_HAL_ADC_H
#define ESP_HAL_ADC_H

#include "driver/mcpwm_prelude.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

typedef struct ESP32MCPWMCurrentSenseParams
{
    int pins[3];
    int adc_raw[3];            // original adc value
    int adc_voltage_conv[3]; // calibrated adc value
    mcpwm_timer_handle_t timer;
    adc_oneshot_unit_handle_t adc_handle; // adc handler
    adc_cali_handle_t adc_cli_handle[3];  // adc handlers after calibration
    bool adc_calibrated[3];               // adc calibration
    bool is_config = false;
} ESP32MCPWMCurrentSenseParams;

extern int adc_raw;
extern int voltage_test;
extern ESP32MCPWMCurrentSenseParams senseParams;
extern const mcpwm_timer_event_callbacks_t mcpwm_timer_cb;

/**
 * @description: Configure ADC and register the MCPWM interrupt function.
 * @param {void} *driver_params
 * @param {int} pinA
 * @param {int} pinB
 * @param {int} pinC
 * @return {*}
 */
void *_configureADCLowSide(const void *driver_params, const int pinA, const int pinB, const int pinC);
float halAdcTestFunction(const int pinA);
#endif
