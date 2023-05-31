/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ESP_HAL_MISC_H
#define ESP_HAL_MISC_H

/**
 * @description: get time in ms since boot.
 * @return {*}
 */
unsigned long micros();

/**
 * @description: get time in us since boot.
 * @return {*}
 */
unsigned long millis();
void delayMicroseconds(uint32_t us);

/**
 * @description: delay function using freertos
 * @param {uint32_t} ms
 * @return {*}
 */
void delay(uint32_t ms);

/**
 * @description: return minimum value
 * @param {float} a
 * @param {float} b
 * @return {*}
 */
float min(float a, float b);

#endif
