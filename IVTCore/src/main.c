/*==================================================================================================
* Project : RTD AUTOSAR 4.7
* Platform : CORTEXM
* Peripheral : S32K3XX
* Dependencies : none
*
* Autosar Version : 4.7.0
* Autosar Revision : ASR_REL_4_7_REV_0000
* Autosar Conf.Variant :
* SW Version : 5.0.0
* Build Version : S32K3_RTD_5_0_0_D2408_ASR_REL_4_7_REV_0000_20241002
*
* Copyright 2020 - 2024 NXP
*
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms.  By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms.  If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/**
*   @file main.c
*
*   @addtogroup main_module main module documentation
*   @{
*/

/* Including necessary configuration files. */
#include "Mcal.h"

/* User includes */
#include "Siul2_Port_Ip.h"
#include "Siul2_Port_Ip_Cfg.h"
#include "Siul2_Dio_Ip.h"

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void) {
    /* Write your code here */

    // Test for PTB14 Blinking
	// Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals, g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

	for (;;) {
		uint32_t count = 10000;
		Siul2_Dio_Ip_TogglePins(LED_PORT, (1 << LED_PIN));
		while (--count > 0) {
			__asm volatile (" nop ");
		}
	}

    return 0;
}

uint8 Sys_GetCoreID(void) {
    return 0;
}

static void __attribute__((section(".basecode"))) quadcopy(uint64_t *dest, const uint64_t *src,
                                                          uint32_t wlen) {
    for (uint32_t i = 0u; i < wlen; ++i) {
        *(dest++) = *(src++);
    }
}

static void __attribute__((section(".basecode"))) quadzero(uint64_t *dest, uint32_t wlen) {
    for (uint32_t i = 0u; i < wlen; ++i) {
        *(dest++) = 0u;
    }
}

typedef struct {
    void (*pfCopy)(uint64_t *dest, const uint64_t *src, uint32_t wlen);
    void (*pfZero)(uint64_t *dest, uint32_t wlen);
} RAMCodeEntry;

static RAMCodeEntry __attribute__((section(".basecodeentry"))) g_ramcode = {quadcopy, quadzero};

#if defined(__ICCARM__)
#pragma diag_suppress = Pe1305
#endif
#if defined(__GNUC__)
void __attribute__((naked, __noreturn__, target("general-regs-only"))) Reset_Handler(void)
#else
void __NAKED __NO_RETURN Reset_Handler(void)
#endif
{
    /* SP initialization is required for S32Debugger when program loaded into RAM by debugger*/
    {
        extern uint32_t __Stack_dtcm_start;

        uint64_t *pSP = (uint64_t *)&__Stack_dtcm_start - 2;
        pSP[0]        = 0;
        pSP[1]        = 0;
        __asm volatile("MSR msp, %0" : : "r"((uint32_t)(pSP + 2)) :);
    }

    /* Move base code quadcopy & quadzero to ram */
    {
        extern uint64_t __base_code_rom_start[1];
        extern uint64_t __base_code_start__[1];
        extern uint64_t __base_code_end__[1];

        for (uint64_t *pSrc = __base_code_rom_start, *pDest = __base_code_start__;
             pDest < __base_code_end__; ++pSrc, ++pDest) {
            *pDest = *pSrc;
        }
    }

    {
        typedef struct {
            uint32_t item_count;
            struct {
                uint64_t *pDest;
                uint32_t  wlen;
            } items[1];
        } copy_table_t;
        extern const copy_table_t __zero_table[1];
        for (uint32_t i = 0; i < __zero_table->item_count; ++i) {
            quadzero(__zero_table->items[i].pDest, __zero_table->items[i].wlen);
        }
    }

    {
        typedef struct {
            uint32_t item_count;
            struct {
                const uint64_t *pSrc;
                uint64_t       *pDest;
                uint32_t        wlen;
            } items[1];
        } copy_table_t;
        extern const copy_table_t __copy_table[1];
        for (uint32_t i = 0; i < __copy_table->item_count; ++i) {
            quadcopy(__copy_table->items[i].pDest, __copy_table->items[i].pSrc,
                     __copy_table->items[i].wlen);
        }
    }

    {
        extern const uint64_t __edata[1];
        extern const uint32_t __edatasize[1];
#define POR (1)
        if (POR) {
            quadzero((uint64_t *)__edata, ((uint32_t)__edatasize));
        }
    }

    SuspendAllInterrupts();
    main();
}
#if defined(__ICCARM__)
#pragma diag_default = Pe1305
#endif

/** @} */
