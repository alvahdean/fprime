/*
 * STM32/newlib compatibility shims for missing 64-bit PRI* format macros.
 * Applied only for the STM32 platform via compiler forced-include flags.
 */
#ifndef STM32_INTTYPES_COMPAT_H
#define STM32_INTTYPES_COMPAT_H

#ifndef PRId64
#define PRId64 "lld"
#endif
#ifndef PRIi64
#define PRIi64 "lli"
#endif
#ifndef PRIu64
#define PRIu64 "llu"
#endif
#ifndef PRIx64
#define PRIx64 "llx"
#endif
#ifndef PRIX64
#define PRIX64 "llX"
#endif
#ifndef PRIo64
#define PRIo64 "llo"
#endif

#endif
