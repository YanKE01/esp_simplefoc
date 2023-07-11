#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_simplefoc.h"
#include "unity.h"

void angle_sensor_init()
{
    sensor_as5600_init(0, GPIO_NUM_1, GPIO_NUM_2);
}

float angle_sensor_get()
{
    return sensor_as5600_getAngle(0);
}

BLDCMotor motor = BLDCMotor(14);
BLDCDriver3PWM driver = BLDCDriver3PWM(17, 16, 15);
GenericSensor sensor = GenericSensor(angle_sensor_get, angle_sensor_init);
LowsideCurrentSense cs = LowsideCurrentSense(0.005f, 10, 4, 5, 6);

float target_value = 0.0f;
Commander command = Commander(Serial);
void doTarget(char *cmd) { command.scalar(&target_value, cmd); }
void onMotor(char *cmd) { command.motor(&motor, cmd); }

TEST_CASE("test esp_simplefoc openloop", "[single motor][openloop]")
{
    SimpleFOCDebug::enable(); // enbale debug
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init(); // enable 3pwm driver
    motor.linkDriver(&driver);

    motor.velocity_limit = 200.0; // 200 rad/s velocity limit
    motor.voltage_limit = 12.0;   // 12 Volt
    motor.controller = MotionControlType::velocity_openloop;

    Serial.begin(115200);
    motor.init();
    while (1)
    {
        motor.move(1.2f);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

TEST_CASE("test esp_simplefoc position control", "[single motor][position control]")
{
    SimpleFOCDebug::enable(); // enable debug
    sensor.init();            // enable as5600 angle sensor
    motor.linkSensor(&sensor);
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init(); // enable 3pwm driver
    motor.linkDriver(&driver);
    motor.controller = MotionControlType::angle; // set position control mode

    // set velocity pid
    motor.PID_velocity.P = 0.9f;
    motor.PID_velocity.I = 2.2f;
    motor.voltage_limit = 11;
    motor.voltage_sensor_align = 2;

    // set angle pid
    motor.LPF_velocity.Tf = 0.05;
    motor.P_angle.P = 15.5;
    motor.P_angle.D = 0.05;
    motor.velocity_limit = 200;

    Serial.begin(115200);

    motor.useMonitoring(Serial);
    motor.init();    // initialize motor
    motor.initFOC(); // align sensor and start FOC

    while (1)
    {
        motor.loopFOC();
        motor.move(2.5f);
    }
}

TEST_CASE("test esp_simplefoc velocity control", "[single motor][velocity control]")
{
    SimpleFOCDebug::enable(); // enable debug
    sensor.init();            // enable as5600 sensor angle
    motor.linkSensor(&sensor);
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init();
    motor.linkDriver(&driver);
    motor.controller = MotionControlType::velocity; // set velocity control mode

    // set velocity pid
    motor.PID_velocity.P = 0.9f;
    motor.PID_velocity.I = 2.2f;
    motor.voltage_limit = 11;
    motor.voltage_sensor_align = 2;

    motor.LPF_velocity.Tf = 0.05;
    motor.velocity_limit = 200;

    Serial.begin(115200);

    motor.useMonitoring(Serial);
    motor.init();    // initialize motor
    motor.initFOC(); // align sensor and start FOC

    while (1)
    {
        motor.loopFOC();
        motor.move(2.5f);
    }
}

TEST_CASE("test esp_simplefoc command velocity control", "[single motor][velocity control][command]")
{
    SimpleFOCDebug::enable(); // enable debug
    sensor.init();            // enable as5600 angle sensor
    motor.linkSensor(&sensor);
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init();
    motor.linkDriver(&driver);
    motor.controller = MotionControlType::velocity; // set velocity control mode

    // set velocity pid
    motor.PID_velocity.P = 0.9f;
    motor.PID_velocity.I = 2.2f;
    motor.voltage_limit = 11;
    motor.voltage_sensor_align = 2;

    motor.LPF_velocity.Tf = 0.05;
    motor.velocity_limit = 200;

    Serial.begin(115200);

    motor.useMonitoring(Serial);
    motor.init();    // initialize motor
    motor.initFOC(); // align sensor and start FOC

    // using "T1.2" to control velocity
    command.add('T', doTarget, const_cast<char *>("target angle")); // add serial command

    while (1)
    {
        motor.loopFOC();
        motor.move(target_value);
        command.run();
    }
}

TEST_CASE("test esp_simplefoc angle current control", "[single motor][angle control][current]")
{
    SimpleFOCDebug::enable();
    sensor.init();
    motor.linkSensor(&sensor);
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init();
    motor.linkDriver(&driver);
    cs.linkDriver(&driver);

    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

    motor.torque_controller = TorqueControlType::voltage;
    motor.controller = MotionControlType::angle;
    motor.motion_downsample = 0.0;

    // set velocity pid
    motor.PID_velocity.P = 1.2;
    motor.PID_velocity.I = 1.2;
    motor.LPF_velocity.Tf = 0.05;

    // set angle PID
    motor.P_angle.P = 10.0;
    motor.P_angle.D = 0.05;
    motor.LPF_angle.Tf = 0.0;

    // set current pid
    motor.PID_current_q.P = 3.0;
    motor.PID_current_q.I = 100.0;
    motor.LPF_current_q.Tf = 0.05;
    motor.PID_current_d.P = 3.0;
    motor.PID_current_d.I = 100.0;
    motor.LPF_current_d.Tf = 0.05;

    // limits
    motor.velocity_limit = 200.0; // 100 rad/s velocity limit
    motor.voltage_limit = 12.0;   // 12 Volt limit
    motor.current_limit = 3.0;    // 3 Amp current limit
    motor.monitor_downsample = 100;

    Serial.begin(115200);

    motor.useMonitoring(Serial);
    motor.init();

    cs.init();
    cs.gain_a *= -1;
    cs.gain_b *= -1;
    cs.gain_c *= -1;
    motor.linkCurrentSense(&cs);
    motor.initFOC();

    command.add('T', doTarget, const_cast<char *>("target angle"));
    command.add('M', onMotor, const_cast<char *>(("my motor")));

    while (1)
    {
        motor.loopFOC();
        motor.move(5.5f);
        motor.monitor();
        command.run();
    }
}

extern "C" void app_main(void)
{
    printf("  _____ ____  ____      ____ ___ __  __ ____  _     _____ _____ ___   ____ \n");
    printf(" | ____/ ___||  _ \\    / ___|_ _|  \\/  |  _ \\| |   | ____|  ___/ _ \\ / ___|\n");
    printf(" |  _| \\___ \\| |_) |___\\___ \\| || |\\/| | |_) | |   |  _| | |_ | | | | |    \n");
    printf(" | |___ ___) |  __/_____|__) | || |  | |  __/| |___| |___|  _|| |_| | |___ \n");
    printf(" |_____|____/|_|       |____/___|_|  |_|_|   |_____|_____|_|   \\___/ \\____|\n");
    unity_run_menu();
}