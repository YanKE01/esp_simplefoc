#ifndef ESP_HAL_MISC_H
#define ESP_HAL_MISC_H

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned long micros();
    unsigned long millis();
    void delayMicroseconds(uint32_t us);
    void delay(uint32_t ms);
    float min(float a, float b);

#ifdef __cplusplus
}
#endif

#endif
