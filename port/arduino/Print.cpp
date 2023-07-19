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
#include "Arduino.h"
#include "Print.h"

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
 * @description: Print str.
 * @param {char} str
 * @return {*}
 */
size_t Print::print(const char str[])
{
    return write(str);
}

/**
 * @description: Print char.
 * @param {char} c
 * @return {*}
 */
size_t Print::print(char c)
{
    return write(c);
}

/**
 * @description: Print char.
 * @param {unsigned char} b
 * @param {int} base
 * @return {*}
 */
size_t Print::print(unsigned char b, int base)
{
    return write(b);
}

/**
 * @description: Print int.
 * @param {int} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(int n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
}

/**
 * @description: Print unsigned int.
 * @param {unsigned int} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(unsigned int n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
}

/**
 * @description: Print long.
 * @param {long} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(long n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
}

/**
 * @description: Print unsigned long.
 * @param {unsigned long} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(unsigned long n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
}

/**
 * @description: Print long long.
 * @param {long long} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(long long n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
}

/**
 * @description: Print unsigned long long.
 * @param {unsigned long long} n
 * @param {int} base
 * @return {*}
 */
size_t Print::print(unsigned long long n, int base)
{
    std::string str = std::to_string(n);
    return write(str.c_str());
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
 * @description: Print char* and new line.
 * @param {char} c
 * @return {*}
 */
size_t Print::println(const char c[])
{
    return print(c) + println();
}

/**
 * @description: Print char and new line.
 * @param {char}
 * @return {*}
 */
size_t Print::println(char c)
{
    return print(c) + println();
}

/**
 * @description: Print unsigned char and new line.
 * @param {unsigned char} b
 * @param {int} base
 * @return {*}
 */
size_t Print::println(unsigned char b, int base)
{
    return print(b, base) + println();
}

/**
 * @description: Print num and new line.
 * @param {int} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(int num, int base)
{
    return print(num, base) + println();
}

/**
 * @description: Print unsigned int and new line.
 * @param {unsigned int} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(unsigned int num, int base)
{
    return print(num, base) + println();
}

/**
 * @description: Print long and new line.
 * @param {long} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(long num, int base)
{
    return print(num, base) + println();
}

/**
 * @description: Print unsigned long and new line.
 * @param {unsigned long} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(unsigned long num, int base)
{
    return print(num, base) + println();
}

/**
 * @description: Print long long ane new line.
 * @param {long long} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(long long num, int base)
{
    return print(num, base) + println();
}

/**
 * @description: Print unsigned long long and new line.
 * @param {unsigned long long} num
 * @param {int} base
 * @return {*}
 */
size_t Print::println(unsigned long long num, int base)
{
    return print(num, base) + println();
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
