#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "esp-hal-misc.h"
#include "esp-hal-serial.h"
#include "esp-hal-gpio.h"
#include "esp-hal-mcpwm.h"



#define NOP() asm volatile("nop")
#define PI 3.14159265358979f

typedef bool boolean;
typedef uint8_t byte;
typedef unsigned int word;

#include "WString.h"
#include "Stream.h"
#include "Printable.h"
#include "Print.h"

enum PINMODE
{
    INPUT = 0,
    OUTPUT,
    INPUT_PULLUP,
    INPUT_PULLDOWN,
};

enum PINLEVEL
{
    LOW = 0,
    HIGH,
};

enum PININTERRUPT
{
    RISING = 0, // low->high
    FALLING,    // high->low
    CHANGE,     // high->low or low->high
};

#endif
