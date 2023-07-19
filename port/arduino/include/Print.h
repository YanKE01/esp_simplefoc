/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#include "WString.h"

#define DEC 10

class Print
{
public:
    Print() {}
    virtual ~Print() {}
    virtual size_t write(uint8_t) = 0;
    size_t write(const char *str);
    virtual size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size);

    size_t print(const __FlashStringHelper *ifsh) { return print(reinterpret_cast<const char *>(ifsh)); }
    size_t print(const char[]);
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
    size_t print(long long, int = DEC);
    size_t print(unsigned long long, int = DEC);
    size_t print(double, int = 2);

    size_t println(const __FlashStringHelper *ifsh) { return println(reinterpret_cast<const char *>(ifsh)); }
    size_t println(const char[]);
    size_t println(char);
    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(unsigned int, int = DEC);
    size_t println(long, int = DEC);
    size_t println(unsigned long, int = DEC);
    size_t println(long long, int = DEC);
    size_t println(unsigned long long, int = DEC);
    size_t println(double, int = 2);
    size_t println(void);
};
