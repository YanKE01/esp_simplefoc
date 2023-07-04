/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP_SIMPLE_FOC_H
#define ESP_SIMPLE_FOC_H


#include "Arduino.h"
#include "../../../Arduino-FOC/src/BLDCMotor.h"
#include "../../../Arduino-FOC/src/drivers/BLDCDriver3PWM.h"
#include "../../../Arduino-FOC/src/communication/SimpleFOCDebug.h"
#include "../../../Arduino-FOC/src/communication/Commander.h"
#include "../../../Arduino-FOC/src/sensors/GenericSensor.h"
#include "../../../Arduino-FOC/src/current_sense/GenericCurrentSense.h"
#include "../../../Arduino-FOC/src/current_sense/LowsideCurrentSense.h"
#include "../../angle_sensor/sensor_as5600.h"

#endif