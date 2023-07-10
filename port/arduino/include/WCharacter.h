#ifndef Character_h
#define Character_h

#include <ctype.h>

inline boolean isDigit(int c)
{
    return (isdigit(c) == 0 ? false : true);
}

#endif
