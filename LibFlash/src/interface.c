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
#include "platform.h"
#include "libflash.h"

extern uint64_t __start_of_itcm[];
extern uint32_t __size_of_itcm[];
extern uint64_t __start_of_dtcm[];
extern uint32_t __size_of_dtcm[];

boolean Flash_Unlock(uint32_t Start, uint32_t Size);
boolean Flash_Erase(uint32_t Start, uint32_t Size);
boolean Flash_Program(uint32_t Start, uint32_t Size, uint32_t *Data);

const IfFlash
#if defined(__ghs__)
#pragma ghs section rodata = ".vectors"
#elif defined(__GNUC__) || defined(__DCC__)
    __attribute__((section(".vectors")))
#elif defined(__ICCARM__)
#pragma location = ".vectors"
#endif
gFlash = {
    .ITCM_Pos  = __start_of_itcm,
    .ITCM_Size = (uint32_t)__size_of_itcm,
    .DTCM_Pos  = __start_of_dtcm,
    .DTCM_Size = (uint32_t)__size_of_dtcm,
    .Unlock    = Flash_Unlock,
    .Erase     = Flash_Erase,
    .Program   = Flash_Program,
};
