/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp-hal-serial.h"

/**
 * ESP32-S-Devkitc default serial port: tx:1 rx:3
 * ESP32-S3-Devkitc default serial port: tx:43 rx:44
 */

#if CONFIG_IDF_TARGET_ESP32
HardwareSerial Serial(UART_NUM_0, 115200, GPIO_NUM_1, GPIO_NUM_3);
#elif CONFIG_IDF_TARGET_ESP32S3
HardwareSerial Serial(UART_NUM_0, 115200, GPIO_NUM_43, GPIO_NUM_44);
#endif


HardwareSerial::HardwareSerial(int uart_num, int baud_rate, gpio_num_t tx, gpio_num_t rx)
{
    this->_uart_num = uart_num;
    this->_rxBufferSize = 256;
    this->_txBufferSize = 0;

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(this->_uart_num, this->_rxBufferSize, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(this->_uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(this->_uart_num, tx, rx, -1, -1));
}

HardwareSerial::~HardwareSerial()
{
    ESP_ERROR_CHECK(uart_driver_delete(_uart_num));
}

/**
 * @description: set rx buffer size
 * @param {size_t} new_size
 * @return {*}
 */
size_t HardwareSerial::setRxBufferSize(size_t new_size)
{
    if (new_size <= SOC_UART_FIFO_LEN)
    {
        return 0; // ESP32, S2, S3 and C3 means higher than 128
    }
    _rxBufferSize = new_size;
    return _rxBufferSize;
}

/**
 * @description: set tx buffer size
 * @param {size_t} new_size
 * @return {*}
 */
size_t HardwareSerial::setTxBufferSize(size_t new_size)
{
    if (new_size <= SOC_UART_FIFO_LEN)
    {
        return 0; // ESP32, S2, S3 and C3 means higher than 128
    }
    _txBufferSize = new_size;
    return _txBufferSize;
}
/**
 * @description: serial write
 * @param {uint8_t} c
 * @return {*}
 */
size_t HardwareSerial::write(uint8_t c)
{
    uart_write_bytes(_uart_num, &c, 1);
    return 1;
}

/**
 * @description: serial write buffer
 * @param {uint8_t} *buffer
 * @param {size_t} size
 * @return {*}
 */
size_t HardwareSerial::write(const uint8_t *buffer, size_t size)
{
    uart_write_bytes(_uart_num, buffer, size);
    return size;
}

/**
 * @description: judge if serial is available
 * @return {*}
 */
int HardwareSerial::available(void)
{
    return uart_is_driver_installed(_uart_num);
}

/**
 * @description: serial peek function not defined
 * @return {*}
 */
int HardwareSerial::peek(void)
{
    return 1;
}

/**
 * @description: serial read one byte function
 * @return {*}
 */
int HardwareSerial::read(void)
{
    uint8_t c = 0;
    if (uart_read_bytes(_uart_num, &c, 1, 0) == 1)
    {
        return c;
    }
    else
    {
        return -1;
    }
}

/**
 * @description: serial read buffer function
 * @param {uint8_t} *buffer
 * @param {size_t} size
 * @return {*}
 */
size_t HardwareSerial::read(uint8_t *buffer, size_t size)
{
    return uart_read_bytes(_uart_num, buffer, size, 0);
}

/**
 * @description: serial read bytes function
 * @param {uint8_t} *buffer
 * @param {size_t} length
 * @return {*}
 */
size_t HardwareSerial::readBytes(uint8_t *buffer, size_t length)
{
    return uart_read_bytes(_uart_num, buffer, length, (uint32_t)getTimeout());
}