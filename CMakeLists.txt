set(SRC_FILES   "Arduino-FOC/src/common/foc_utils.cpp"
                "Arduino-FOC/src/common/pid.cpp"
                "Arduino-FOC/src/common/time_utils.cpp"
                "Arduino-FOC/src/common/lowpass_filter.cpp"
                "Arduino-FOC/src/common/base_classes/CurrentSense.cpp"
                "Arduino-FOC/src/common/base_classes/Sensor.cpp"
                "Arduino-FOC/src/common/base_classes/FOCMotor.cpp"
                "Arduino-FOC/src/communication/SimpleFOCDebug.cpp"
                "Arduino-FOC/src/drivers/BLDCDriver3PWM.cpp"
                "Arduino-FOC/src/drivers/BLDCDriver6PWM.cpp"
                "Arduino-FOC/src/sensors/GenericSensor.cpp"
                "Arduino-FOC/src/sensors/Encoder.cpp"
                "Arduino-FOC/src/BLDCMotor.cpp"
                "port/esp/esp-hal-misc.cpp"
                "port/esp/esp-hal-serial.cpp"
                "port/esp/esp-hal-gpio.cpp"
                "port/esp/esp-hal-mcpwm.cpp"
                "port/arduino/Print.cpp"
                "port/arduino/Stream.cpp"                
)


set(INC_FILES   "port/arduino/include/"
                "port/esp/include"
                "Arduino-FOC/src/common/"
                "Arduino-FOC/src/common/base_classes"
                "Arduino-FOC/src/"
) 


idf_component_register(SRCS ${SRC_FILES}
                    INCLUDE_DIRS ${INC_FILES}
                    REQUIRES esp_timer driver esp_rom
)

#Cancel the compilation warning caused by changes to the voliate variable in simplefoc
set_source_files_properties( "Arduino-FOC/src/sensors/Encoder.cpp"
    PROPERTIES COMPILE_FLAGS
    -Wno-volatile
)