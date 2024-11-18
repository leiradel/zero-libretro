#pragma once

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define RM_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define RM_LITTLE_ENDIAN 1
#elif defined(_WIN32) // Assume little-endian on Windows
    #define RM_LITTLE_ENDIAN 1
#else
    #error "Cannot determine endianness"
#endif
