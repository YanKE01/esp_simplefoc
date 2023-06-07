/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ESP_HAL_MISC_H
#define ESP_HAL_MISC_H

unsigned long micros();
unsigned long millis();
void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);
float min(float a, float b);

#endif
