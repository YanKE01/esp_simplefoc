/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ESP_HAL_GPIO_H
#define ESP_HAL_GPIO_H

#include <stdint.h>
#include "Arduino.h"

typedef struct _IoRecord
{
    gpio_config_t conf;
    gpio_num_t number;
} IoRecord;

/**
 * @description: set gpio mode
 * @param {uint8_t} pin
 * @param {uint8_t} mode
 * @return {*}
 */
void pinMode(uint8_t pin, uint8_t mode);

/**
 * @description: gpio write
 * @param {uint8_t} pin
 * @param {uint8_t} val
 * @return {*}
 */
void digitalWrite(uint8_t pin, uint8_t val);

/**
 * @description: gpio read
 * @param {uint8_t} pin
 * @return {*}
 */
int digitalRead(uint8_t pin);

/**
 * @description: only return pin number
 * @param {uint8_t} pin
 * @return {*}
 */
uint8_t digitalPinToInterrupt(uint8_t pin);

/**
 * @description: bind gpio interrupt
 * @param {uint8_t} pin
 * @param {int} mode
 * @return {*}
 */
void attachInterrupt(uint8_t pin, void (*handler)(void), int mode);

#endif