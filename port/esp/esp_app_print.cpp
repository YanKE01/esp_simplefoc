/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "esp_platform.h"
#include "esp_app_print.h"

/**
 * @description: Write buffer.
 * @param {uint8_t} *buffer
 * @param {size_t} size
 * @return {*}
 */
size_t Print::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--)
    {
        n += write(*buffer++);
    }
    return n;
}

/**
 * @description: Write buffer.
 * @param {char} *str
 * @return {*}
 */
size_t Print::write(const char *str)
{
    if (str == nullptr)
    {
        return 0;
    }
    return write((const uint8_t *)str, strlen(str));
}

/**
 * @description: Write buffer.
 * @param {char} *buffer
 * @param {size_t} size
 * @return {*}
 */
size_t Print::write(const char *buffer, size_t size)
{
    return write((const uint8_t *)buffer, size);
}

/**
 * @description: Print double.
 * @param {double} n
 * @param {int} digits
 * @return {*}
 */
size_t Print::print(double n, int digits)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(digits) << n;
    std::string str = oss.str();

    return write(str.c_str());
}

/**
 * @description: Print new line.
 * @return {*}
 */
size_t Print::println(void)
{
    return print("\r\n");
}

/**
 * @description: Print double and new line.
 * @param {double} num
 * @param {int} digits
 * @return {*}
 */
size_t Print::println(double num, int digits)
{
    size_t n = print(num, digits);
    n += println();
    return n;
}

/**
 * @description: Constructor function
 * @return {*}
 */
Stream::Stream()
{
    _startMillis = 0;
    _timeout = 0;
}

/**
 * @description: Character read (timeout limit).
 * @return {*}
 */
int Stream::timedRead()
{
    int c;
    _startMillis = millis();
    do
    {
        c = read();
        if (c >= 0)
        {
            return c;
        }
    } while (millis() - _startMillis < _timeout);
    return -1;
}

/**
 * @description: String reading.
 * @param {char} *buffer
 * @param {size_t} length
 * @return {*}
 */
size_t Stream::readBytes(char *buffer, size_t length)
{
    size_t count = 0;
    while (count < length)
    {
        int c = timedRead();
        if (c < 0)
        {
            break;
        }
        *buffer++ = (char)c;
        count++;
    }
    return count;
}

/**
 * @description: String reading.
 * @param {uint8_t} *buffer
 * @param {size_t} length
 * @return {*}
 */
size_t Stream::readBytes(uint8_t *buffer, size_t length)
{
    return readBytes((char *)buffer, length);
}

/**
 * @description: Set timeout.
 * @param {unsigned long} timeout
 * @return {*}
 */
void Stream::setTimeout(unsigned long timeout)
{
    _timeout = timeout;
}

/**
 * @description: Get setted timeout.
 * @return {*}
 */
unsigned long Stream::getTimeout()
{
    return _timeout;
}