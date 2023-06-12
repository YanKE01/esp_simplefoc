/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp-hal-adc.h"

#include "esp_log.h"
#include "soc/adc_periph.h"
#include "esp_private/adc_private.h"
#include <map>
#include "../../../Arduino-FOC/src/current_sense/hardware_api.h"

int adc_raw = 0;
int voltage_test = 0; // mv

ESP32MCPWMCurrentSenseParams senseParams;

// convert pin to adc_unit and channel. only support ADC_UINIT_1
#if CONFIG_IDF_TARGET_ESP32
std::map<int, std::pair<adc_unit_t, adc_channel_t>> pinToAdc = {
    {GPIO_NUM_32, {ADC_UNIT_1, ADC_CHANNEL_4}},
    {GPIO_NUM_33, {ADC_UNIT_1, ADC_CHANNEL_5}},
    {GPIO_NUM_34, {ADC_UNIT_1, ADC_CHANNEL_6}},
    {GPIO_NUM_35, {ADC_UNIT_1, ADC_CHANNEL_7}},
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
    printf("calibration scheme version is %s", "Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (ret == ESP_OK)
    {
        calibrated = true;
    }
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
    mcpwm_timer_handle_t timer = ((ESP32MCPWMDriverParams *)driver_params)->timer;
    senseParams.timer = timer; // copy timer from driver params
    int pins[3] = {pinA, pinB, pinC};

    // ADC_UNIT_1 Init
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &senseParams.adc_handle));

    // ADC_UNIT_1 Config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i = 0; i < 3; i++)
    {
        senseParams.pins[i] = pins[i];                                                                          // copy pins to adc params
        ESP_ERROR_CHECK(adc_oneshot_config_channel(senseParams.adc_handle, pinToAdc[pins[i]].second, &config)); // config channel

        if (_adcCalibrationInit(pinToAdc[pins[i]].first, pinToAdc[pins[i]].second, ADC_ATTEN_DB_11, &senseParams.adc_cli_handle[i]))
        {
            printf("GPIO[%d] Channel[%d] calibrates successfully\n", pins[i], pinToAdc[pins[i]].second);
        }
    }

    senseParams.is_config = true; // Wait for the struct to be assigned a value.

    ESP32MCPWMCurrentSenseParams *params = &senseParams;
    return params;
}

/**
 * @description: read lowside voltage. It's just taking data from the buffer, not actually reading adc.
 * @param {int} pinA
 * @param {void} *cs_params
 * @return {*}
 */
float _readADCVoltageLowSide(const int pinA, const void *cs_params)
{
    for (int i = 0; i < 3; i++)
    {
        if (pinA == senseParams.pins[i])
        {
            return (senseParams.adc_voltage_conv[i] / 1000.0f);
        }
    }
    return 0.0f;
}

/**
 * @description: read adc voltage. It's just taking data from the buffer, not actually reading adc.
 * @param {int} pinA
 * @return {*}
 */
float halAdcTestFunction(const int pinA)
{
    for (int i = 0; i < 3; i++)
    {
        if (pinA == senseParams.pins[i])
        {
            return senseParams.adc_voltage_conv[i] / 1000.0f;
        }
    }

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
    // empty function
}

/**
 * @description: mcpwm callback function
 * @param {mcpwm_timer_handle_t} timer
 * @param {mcpwm_timer_event_data_t} *edata
 * @param {void} *user_ctx
 * @return {*}
 */
bool mcpwm_isr_handler(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx)
{
    static int index = 0;
    /**
     *  In order to avoid interrupting function running, adc_handle is a null pointer,
     *  so the current collection starts only after is_config is set.
     */
    if (senseParams.is_config)
    {
        adc_oneshot_read_isr(senseParams.adc_handle, pinToAdc[senseParams.pins[index]].second, &senseParams.adc_raw[index]);
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(senseParams.adc_cli_handle[index], senseParams.adc_raw[index], &senseParams.adc_voltage_conv[index]));
        index++;
        index %= 3; // the number of adc pins is three
    }
    return true;
}