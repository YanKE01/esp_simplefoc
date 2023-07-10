#ifndef String_class_h
#define String_class_h
#ifdef __cplusplus

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

class __FlashStringHelper;
#define FPSTR(str_pointer) (reinterpret_cast<const __FlashStringHelper *>(str_pointer))
#define F(string_literal) (FPSTR((string_literal)))

#endif
#endif
