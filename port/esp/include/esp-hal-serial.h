/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ESP_HAL_SERIAL_H
#define ESP_HAL_SERIAL_H

#include "Stream.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/soc_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class HardwareSerial : public Stream
{
public:
    HardwareSerial(){};
    HardwareSerial(int uart_num, int baud_rate, gpio_num_t tx, gpio_num_t rx);
    ~HardwareSerial();
    size_t setRxBufferSize(size_t new_size);
    size_t setTxBufferSize(size_t new_size);
    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(const char *buffer, size_t size)
    {
        return write((uint8_t *)buffer, size);
    }
    inline size_t write(const char *s)
    {
        return write((uint8_t *)s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t)n);
    }

    int peek(void);
    int available(void);
    int read(void);
    size_t read(uint8_t *buffer, size_t size);
    size_t readBytes(uint8_t *buffer, size_t length);
    
private:
    int _uart_num;
    size_t _rxBufferSize;
    size_t _txBufferSize;
};

extern HardwareSerial Serial;

#endif
