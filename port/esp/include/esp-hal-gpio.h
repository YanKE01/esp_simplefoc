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

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
uint8_t digitalPinToInterrupt(uint8_t pin);
void attachInterrupt(uint8_t pin, void (*handler)(void), int mode);

#endif