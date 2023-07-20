/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <sstream>
#include <string.h>

class Print
{
public:
    Print() {}
    virtual ~Print() {}
    virtual size_t write(uint8_t) = 0;
    size_t write(const char *str);
    virtual size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size);

    template <typename T>
    size_t print(T value)
    {
        std::stringstream ss;
        ss << value;
        return write(ss.str().c_str());
    }
    size_t print(double, int = 2);

    template <typename T>
    size_t println(T value)
    {
        std::stringstream ss;
        ss << value << "\r\n";
        return write(ss.str().c_str());
    }
    size_t println(double, int = 2);
    size_t println(void);
};

class Stream : public Print
{
protected:
    unsigned long _timeout;
    unsigned long _startMillis;
    int timedRead();

public:
    virtual int available() = 0;
    virtual int read() = 0;

    Stream();
    virtual ~Stream() {}
    void setTimeout(unsigned long timeout);
    unsigned long getTimeout(void);
    virtual size_t readBytes(char *buffer, size_t length);
    virtual size_t readBytes(uint8_t *buffer, size_t length);
};