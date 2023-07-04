#ifndef _SENSOR_AS5600_H
#define _SENSOR_AS5600_H

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @description: sensor as5600 init.
     * @return {*}
     */
    void sensor_as5600_init();

    /**
     * @description: sensor as5600 get raw angle.
     * @return {*}
     */
    float sensor_as5600_getAngle(void);
#ifdef __cplusplus
}
#endif

#endif
