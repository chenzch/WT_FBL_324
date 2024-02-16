//---------------------------------------------------------------------------------------------------------------------
//
// WTMEC CORPORATION CONFIDENTIAL
// ________________________________
//
// [2024] Wtmec Corporation
// All Rights Reserved.
//
// NOTICE: This is an unpublished work of authorship, which contains trade secrets.
// Wtmec Corporation owns all rights to this work and intends to maintain it in confidence to
// preserve its trade secret status. Wtmec Corporation reserves the right, under the copyright
// laws of the United States or those of any other country that may have jurisdiction, to protect
// this work as an unpublished work, in the event of an inadvertent or deliberate unauthorized
// publication. Wtmec Corporation also reserves its rights under all copyright laws to protect
// this work as a published work, when appropriate. Those having access to this work may not copy
// it, use it, modify it, or disclose the information contained in it without the written
// authorization of Wtmec Corporation.
//
//---------------------------------------------------------------------------------------------------------------------

#include "typedefs.h"

#ifdef S32K344
#define MCUTYPE S32K344_
#endif
#ifdef S32K342
#define MCUTYPE S32K342_
#endif
#ifdef S32K341
#define MCUTYPE S32K341_
#endif
#ifdef S32K324
#define MCUTYPE S32K324_
#endif
#ifdef S32K314
#define MCUTYPE S32K314_
#endif
#ifdef S32K312
#define MCUTYPE S32K312_
#endif
#ifdef S32K322
#define MCUTYPE S32K322_
#endif
#if defined(S32K396) || defined(S32K394)
#define MCUTYPE S32K39_
#endif
#if defined(S32K374) || defined(S32K376)
#define MCUTYPE S32K37_
#endif
#if defined(S32K358) || defined(S32K328) || defined(S32K338) || defined(S32K348)
#define MCUTYPE S32K358_
#endif
#ifdef S32K388
#define MCUTYPE S32K388_
#endif
#if defined(S32K311) || defined(S32K310)
#define MCUTYPE S32K311_
#endif
#if defined(S32M276) || defined(S32M274)
#define MCUTYPE S32M27x_
#endif

#if !defined(MCUTYPE)
#error MCUTYPE is not defined
#endif

#define T(X)       #X
#define MOD(X)     MOD2(MCUTYPE, X)
#define MOD2(X, Y) MOD3(X, Y)
#define MOD3(X, Y) T(X##Y)

//#include MOD(MC_RGM.h)

#if defined(NDEBUG)
#define BREAKPOINT()
#define ASSERT(X)
#else
#define BREAKPOINT()                                                                               \
    do {                                                                                           \
        __asm("BKPT #0");                                                                          \
    } while (0)
#define ASSERT(X)                                                                                  \
    do {                                                                                           \
        if (X) {                                                                                   \
        } else {                                                                                   \
            BREAKPOINT();                                                                          \
        }                                                                                          \
    } while (0)
#endif
