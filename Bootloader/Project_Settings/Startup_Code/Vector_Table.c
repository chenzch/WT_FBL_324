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
#include "platform.h"

typedef void (*pFunc)(void);

extern uint32_t __Stack_dtcm_start[];

void Reset_Handler(void);
void _core_loop(void);
void NMI_Handler(void);        /* NMI Handler */
void HardFault_Handler(void);  /* Hard Fault Handler */
void MemManage_Handler(void);  /* Reserved */
void BusFault_Handler(void);   /* Bus Fault Handler */
void UsageFault_Handler(void); /* Usage Fault Handler */
void SVC_Handler(void);        /* SVC Handler */
void DebugMon_Handler(void);   /* Debug Monitor Handler */
void PendSV_Handler(void);     /* PendSV Handler */
void SysTick_Handler(void);    /* SysTick Handler */

typedef struct {
    uint32_t sp;
    pFunc    ISR[15];
} IntVector;
#if defined(__ghs__)
#pragma ghs section rodata = ".intc_vector"
const IntVector
#elif defined(__GNUC__) || defined(__DCC__)
const IntVector __attribute__((section(".intc_vector")))
#elif defined(__ICCARM__)
#pragma location = ".intc_vector"
const IntVector
#endif
    __VECTOR_TABLE = {
        (uint32_t)__Stack_dtcm_start,
        {
#ifdef MCAL_TESTING_ENVIRONMENT
            _core_loop, /* Set an infinte loop as entry point which will be changed by the debugger */
#else
        Reset_Handler, /* Initial Program Counter: Reset Handler */
#endif

            NMI_Handler,        /* -14 NMI Handler */
            HardFault_Handler,  /* -13 Hard Fault Handler */
            MemManage_Handler,  /* -12 MPU Fault Handler */
            BusFault_Handler,   /* -11 Bus Fault Handler */
            UsageFault_Handler, /* -10 Usage Fault Handler */
            0,                  /*     Reserved */
            0,                  /*     Reserved */
            0,                  /*     Reserved */
            0,                  /*     Reserved */
            SVC_Handler,        /*  -5 SVCall Handler */
            DebugMon_Handler,   /*  -4 Debug Monitor Handler */
            0,                  /*     Reserved */
            PendSV_Handler,     /*  -2 PendSV Handler */
            SysTick_Handler,    /*  -1 SysTick Handler */
        },
};
