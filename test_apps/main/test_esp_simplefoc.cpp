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

BLDCMotor motor = BLDCMotor(14);
BLDCDriver3PWM driver = BLDCDriver3PWM(17, 16, 15);

TEST_CASE("test esp_simplefoc", "[single motor][openloop]")
{
    SimpleFOCDebug::enable(); // enbale debug
    driver.voltage_power_supply = 12;
    driver.voltage_limit = 11;
    driver.init();
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

TEST_CASE("test esp_simplefoc", "[single motor][position control]")
{
    
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