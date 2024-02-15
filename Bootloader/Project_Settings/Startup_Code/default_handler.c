//---------------------------------------------------------------------------------------------------------------------
//
// WTMEC CORPORATION CONFIDENTIAL
// ________________________________
//
// [2021] Wtmec Corporation
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
#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "Mcal.h"

#define PLATFORM_START_SEC_CODE
#include "Platform_MemMap.h"

void Default_Handler(void) __attribute__((weak));

void Default_Handler(void) {
    register volatile IRQn_Type CurrentInterrupt =
        (int32_t)((S32_SCB->ICSR & S32_SCB_ICSR_VECTACTIVE_MASK) >> S32_SCB_ICSR_VECTACTIVE_SHIFT) -
        16;
    switch (CurrentInterrupt) {

    default:
        for (;;)
            ;
    }
}

#define PLATFORM_STOP_SEC_CODE
#include "Platform_MemMap.h"

#ifdef __cplusplus
}
#endif
