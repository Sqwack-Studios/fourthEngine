#ifndef _MACROS_HLSLI
#define _MACROS_HLSLI

#define JOIN(a, b) a##b
#define TARGET(slot)  JOIN(SV_TARGET, slot)

#endif //_MACROS_HLSLI