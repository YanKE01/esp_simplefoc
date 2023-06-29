#include "../../Arduino-FOC/src/common/foc_utils.h"
#include <math.h>
#include "soc/soc_caps.h"

/**
 * @description: Using fast math sin.
 * @param {float} a
 * @return {*}
 */
float _sin(float a)
{
    return sinf(a);
}

/**
 * @description: Using fast math cos.
 * @param {float} a
 * @return {*}
 */
float _cos(float a)
{
    return cosf(a);
}

/**
 * @description: Normalizing radian angle to [0,2PI].
 * @param {float} angle
 * @return {*}
 */
float _normalizeAngle(float angle)
{
    float a = fmod(angle, _2PI);
    return a >= 0 ? a : (a + _2PI);
}

/**
 * @description: Electrical angle calculation.
 * @param {float} number
 * @return {*}
 */
float _electricalAngle(float shaft_angle, int pole_pairs)
{
    return (shaft_angle * pole_pairs);
}

/**
 * @description: Using fast math sqrt
 * @param {float} number
 * @return {*}
 */
float _sqrtApprox(float number)
{
    return sqrtf(number);
}
