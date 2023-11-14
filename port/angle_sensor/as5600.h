/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AS5600_H
#define AS5600_H

#include "common/base_classes/Sensor.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

class AS5600 : public Sensor {
public:
    AS5600(i2c_port_t i2c_port, gpio_num_t scl_io, gpio_num_t sda_io);
    ~AS5600();
    void init();
    void deinit();
    float getSensorAngle();

private:
    i2c_port_t _i2c_port;
    gpio_num_t _scl_io;
    gpio_num_t _sda_io;
    bool is_installed;
    uint8_t device_addr = 0x36;
    uint8_t raw_angle_addr = 0x0C;
    uint8_t raw_angle_buf[2]={0};
    uint16_t raw_angle;
    float angle;
};

#endif
