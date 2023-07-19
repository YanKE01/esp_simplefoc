/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Arduino.h"
#include "Stream.h"

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