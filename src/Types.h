#pragma once

#include "Config.h"

#include <stdint.h>
#include <string>

typedef uint8_t  byte;
typedef uint16_t ushort;
typedef uint32_t uint;

typedef int8_t  sbyte;
static_assert(sizeof(short) == 2);
static_assert(sizeof(int)   == 4);

typedef intptr_t IntPtr;

typedef std::string string;

