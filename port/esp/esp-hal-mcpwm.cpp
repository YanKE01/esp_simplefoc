/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp-hal-mcpwm.h"
#include "esp_log.h"
#include "esp-hal-adc.h"
#include "../../../Arduino-FOC/src/drivers/hardware_api.h"

/**
 * @description: initialize three pwm drivers
 * @param {long} pwm_frequency
 * @param {int} pinA
 * @param {int} pinB
 * @param {int} pinC
 * @return {*}
 */
void *_configure3PWM(long pwm_frequency, const int pinA, const int pinB, const int pinC)
{
    if (!pwm_frequency || !_isset(pwm_frequency))
    {
        pwm_frequency = _PWM_FREQUENCY; // default frequency 25KHZ
    }
    else
    {
        pwm_frequency = _constrain(pwm_frequency, 0, _PWM_FREQUENCY_MAX); // constrain to 40kHz max
    }

    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config;

    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = _PWM_TIMEBASE_RESOLUTION_HZ;
    timer_config.period_ticks = (uint32_t)(1.0 * _PWM_TIMEBASE_RESOLUTION_HZ / pwm_frequency);
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_oper_handle_t oper[3];
    mcpwm_operator_config_t operator_config;
    operator_config.group_id = 0; // operator must be in the same group to the timer

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper[i]));
    }

    // Connect timer and operator
    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper[i], timer));
    }

    // Create comparator and generator from the operator
    mcpwm_cmpr_handle_t comparator[3];
    mcpwm_comparator_config_t comparator_config;
    comparator_config.flags.update_cmp_on_tep = true;

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_comparator(oper[i], &comparator_config, &comparator[i]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator[i], (0))); // set vlaue 0 for comparator
    }

    // Bind gpio number
    mcpwm_gen_handle_t generator[3] = {};
    mcpwm_generator_config_t generator_config = {};
    const int gen_gpios[3] = {pinA, pinB, pinC}; // set three pins
    for (int i = 0; i < 3; i++)
    {
        generator_config.gen_gpio_num = gen_gpios[i];
        ESP_ERROR_CHECK(mcpwm_new_generator(oper[i], &generator_config, &generator[i]));
    }

    // Set generator action on timer and compare event

    for (int i = 0; i < 3; i++)
    {
        // go high on counter empty
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator[i],
                                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
        // go low on compare threshold
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator[i],
                                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator[i], MCPWM_GEN_ACTION_LOW)));
    }

    // Enable and start timer
    mcpwm_timer_register_event_callbacks(timer, &mcpwm_timer_cb, NULL); // bind isr handler
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    ESP32MCPWMDriverParams *params = new ESP32MCPWMDriverParams{};
    params->pwm_frequency = pwm_frequency;
    params->pwm_timeperiod = timer_config.period_ticks;
    params->timer = timer;
    for (int i = 0; i < 3; i++)
    {
        params->comparator[i] = comparator[i];
    }

    return params;
}

/**
 * @description:
 * @param {float} dc_a [0,1]
 * @param {float} dc_b [0,1]
 * @param {float} dc_c [0,1]
 * @param {void} *params
 * @return {*}
 */
void _writeDutyCycle3PWM(float dc_a, float dc_b, float dc_c, void *params)
{
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(((ESP32MCPWMDriverParams *)params)->comparator[0], 1.0 * (((ESP32MCPWMDriverParams *)params)->pwm_timeperiod) * dc_a));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(((ESP32MCPWMDriverParams *)params)->comparator[1], 1.0 * (((ESP32MCPWMDriverParams *)params)->pwm_timeperiod) * dc_b));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(((ESP32MCPWMDriverParams *)params)->comparator[2], 1.0 * (((ESP32MCPWMDriverParams *)params)->pwm_timeperiod) * dc_c));
}