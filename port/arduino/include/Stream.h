/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <inttypes.h>
#include "Print.h"

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