/**
 * \brief PlatformTypes.h C-compatible type definitions for STM32 + FreeRTOS
 */
#ifndef STM32_FREERTOS_PLATFORM_TYPES_H_
#define STM32_FREERTOS_PLATFORM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdint.h>

#ifndef __SIZEOF_POINTER__
#error "Compiler does not support __SIZEOF_POINTER__, cannot determine PlatformPointerCastType"
#endif

#if __SIZEOF_POINTER__ == 8
typedef uint64_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx64
#elif __SIZEOF_POINTER__ == 4
typedef uint32_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx32
#elif __SIZEOF_POINTER__ == 2
typedef uint16_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx16
#elif __SIZEOF_POINTER__ == 1
typedef uint8_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx8
#else
#error "Expected __SIZEOF_POINTER__ to be one of 8, 4, 2, or 1"
#endif

#ifdef __cplusplus
}
#endif

#endif  // STM32_FREERTOS_PLATFORM_TYPES_H_
