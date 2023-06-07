/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp-hal-adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "soc/adc_periph.h"
#include <map>
#include "../../../Arduino-FOC/src/current_sense/hardware_api.h"

int adc_count = 0;

typedef struct ESP32MCPWMCurrentSenseParams
{
    int pins[3];
    int adc_raw[3]; // original adc value
    float adc_voltage_conv;
    mcpwm_timer_handle_t timer;
    adc_oneshot_unit_handle_t adc_handle; // adc handler
    adc_cali_handle_t adc_cli_handle[3];  // adc handlers after calibration
    bool adc_calibrated[3];               // adc calibration
} ESP32MCPWMCurrentSenseParams;

// convert pin to adc_unit and channel. only support ADC_UINIT_1
#if CONFIG_IDF_TARGET_ESP32
std::map<int, std::pair<adc_unit_t, adc_channel_t>> pinToAdc = {
    {GPIO_NUM_4, {ADC_UNIT_1, ADC_CHANNEL_3}},
};
#elif CONFIG_IDF_TARGET_ESP32S3
std::map<int, std::pair<adc_unit_t, adc_channel_t>> pinToAdc = {
    {GPIO_NUM_1, {ADC_UNIT_1, ADC_CHANNEL_0}},
    {GPIO_NUM_2, {ADC_UNIT_1, ADC_CHANNEL_1}},
    {GPIO_NUM_3, {ADC_UNIT_1, ADC_CHANNEL_2}},
    {GPIO_NUM_4, {ADC_UNIT_1, ADC_CHANNEL_3}},
    {GPIO_NUM_5, {ADC_UNIT_1, ADC_CHANNEL_4}},
    {GPIO_NUM_6, {ADC_UNIT_1, ADC_CHANNEL_5}},
    {GPIO_NUM_7, {ADC_UNIT_1, ADC_CHANNEL_6}},
    {GPIO_NUM_8, {ADC_UNIT_1, ADC_CHANNEL_7}},
};
#endif

bool mcpwm_isr_handler(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx);

const mcpwm_timer_event_callbacks_t mcpwm_timer_cb = {
    .on_full = mcpwm_isr_handler,
    .on_empty = NULL,
    .on_stop = NULL,
};

/**
 * @description: empty function
 * @return {*}
 */
void _startADC3PinConversionLowSide()
{
}

/**
 * @description: adc calibration
 * @param {adc_unit_t} unit
 * @param {adc_channel_t} channel
 * @param {adc_atten_t} atten
 * @param {adc_cali_handle_t} *out_handle
 * @return {*}
 */
bool _adcCalibrationInit(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    // The ESP32-S3 supports curve fitting and the ESP32-S supports straight line fitting.
#if CONFIG_IDF_TARGET_ESP32

#elif CONFIG_IDF_TARGET_ESP32S3
    printf("calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);

    if (ret == ESP_OK)
    {
        calibrated = true;
    }
#endif

    *out_handle = handle;

    return calibrated;
}

/**
 * @description: config adc. Only three-resistor sampling is supported.
 * @param {void} *driver_params
 * @param {int} pinA
 * @param {int} pinB
 * @param {int} pinC
 * @return {*}
 */
void *_configureADCLowSide(const void *driver_params, const int pinA, const int pinB, const int pinC)
{
    if (!pinToAdc.count(pinA) || !pinToAdc.count(pinB) || !pinToAdc.count(pinC))
    {
        printf("GPIO[%d] or GPIO[%d] or GPIO[%d] don't support ADC\n", pinA, pinB, pinC);
        return NULL;
    }
    // get driver's timer
    mcpwm_timer_handle_t timer = NULL; // ((ESP32MCPWMDriverParams *)driver_params)->timer;
    ESP32MCPWMCurrentSenseParams *params = new ESP32MCPWMCurrentSenseParams;
    params->timer = timer; // copy timer from driver params
    int pins[3] = {pinA, pinB, pinC};

    // ADC_UNIT_1 Init
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &params->adc_handle));

    // ADC_UNIT_1 Config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i = 0; i < 3; i++)
    {
        params->pins[i] = pins[i];                                                                          // copy pins to adc params
        ESP_ERROR_CHECK(adc_oneshot_config_channel(params->adc_handle, pinToAdc[pins[i]].second, &config)); // config channel

        if (_adcCalibrationInit(pinToAdc[pins[i]].first, pinToAdc[pins[i]].second, ADC_ATTEN_DB_11, &params->adc_cli_handle[i]))
        {
            printf("GPIO[%d] Channel[%d] calibrates successfully\n", pins[i], pinToAdc[pins[i]].second);
        }
    }

    return params;
}

/**
 * @description: read lowside voltage. It's just taking data from the buffer, not actually reading it.
 * @param {int} pinA
 * @param {void} *cs_params
 * @return {*}
 */
float _readADCVoltageLowSide(const int pinA, const void *cs_params)
{
    return 0.0f;
}

/**
 * @description: Register callback functions
 * @param {void} *driver_params
 * @param {void} *cs_params
 * @return {*}
 */
void _driverSyncLowSide(void *driver_params, void *cs_params)
{
}

bool mcpwm_isr_handler(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx)
{
    adc_count++;
    if (adc_count == 100000)
    {
        adc_count = 0;
    }
    return true;
}