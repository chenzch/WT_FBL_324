/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           :
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 3.0.0
*   Build Version        : S32K3_RTD_3_0_0_D2303_ASR_REL_4_7_REV_0000_20230331
*
*   Copyright 2020 - 2023 NXP Semiconductors
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/
/**
*   @implements startup.c_Artifact
*/

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 *******************************************************************************/
/*!
 * @brief Defines the init table layout
 */
typedef struct {
    uint64_t       *ram_start; /*!< Start address of section in RAM */
    const uint64_t *rom_start; /*!< Start address of section in ROM */
    const uint64_t *rom_end;   /*!< End address of section in ROM */
} Sys_CopyLayoutType;

/*!
 * @brief Defines the zero table layout
 */
typedef struct {
    uint64_t *ram_start; /*!< Start address of section in RAM */
    uint64_t *ram_end;   /*!< End address of section in RAM */
} Sys_ZeroLayoutType;

extern uint32 __INIT_TABLE[];
extern uint32 __ZERO_TABLE[];
extern uint32 __INDEX_COPY_CORE2[];

/*******************************************************************************
 * Static Variables
 ******************************************************************************/

extern uint64_t       __RAM_CACHEABLE_START[];
extern const uint64_t __ROM_CACHEABLE_START[];
extern const uint64_t __ROM_CACHEABLE_END[];
extern uint64_t       __RAM_NO_CACHEABLE_START[];
extern const uint64_t __ROM_NO_CACHEABLE_START[];
extern const uint64_t __ROM_NO_CACHEABLE_END[];
extern uint64_t       __RAM_SHAREABLE_START[];
extern const uint64_t __ROM_SHAREABLE_START[];
extern const uint64_t __ROM_SHAREABLE_END[];
extern uint64_t       __RAM_INTERRUPT_START[];
extern const uint64_t __INIT_INTERRUPT_START[];
extern const uint64_t __INIT_INTERRUPT_END[];
extern uint64_t       __RAM_ITCM_START[];
extern const uint64_t __ROM_ITCM_START[];
extern const uint64_t __ROM_ITCM_END[];
extern uint64_t       __RAM_DTCM_DATA_START[];
extern const uint64_t __ROM_DTCM_DATA_START[];
extern const uint64_t __ROM_DTCM_END[];

static const struct {
    uint32_t                 size;
    const Sys_CopyLayoutType table[6];
} InitTable = {
    6,
    {
        {
            __RAM_CACHEABLE_START,
            __ROM_CACHEABLE_START,
            __ROM_CACHEABLE_END,
        },
        {
            __RAM_NO_CACHEABLE_START,
            __ROM_NO_CACHEABLE_START,
            __ROM_NO_CACHEABLE_END,
        },
        {
            __RAM_SHAREABLE_START,
            __ROM_SHAREABLE_START,
            __ROM_SHAREABLE_END,
        },
        {
            __RAM_INTERRUPT_START,
            __INIT_INTERRUPT_START,
            __INIT_INTERRUPT_END,
        },
        {
            __RAM_ITCM_START,
            __ROM_ITCM_START,
            __ROM_ITCM_END,
        },
        {
            __RAM_DTCM_DATA_START,
            __ROM_DTCM_DATA_START,
            __ROM_DTCM_END,
        },
    },
};

extern uint64_t __BSS_SRAM_SH_START[];
extern uint64_t __BSS_SRAM_SH_END[];
extern uint64_t __BSS_SRAM_NC_START[];
extern uint64_t __BSS_SRAM_NC_END[];
extern uint64_t __BSS_SRAM_START[];
extern uint64_t __BSS_SRAM_END[];

static const struct {
    uint32_t                 size;
    const Sys_ZeroLayoutType table[3];
} ZeroTable = {
    3,
    {
        {
            __BSS_SRAM_SH_START,
            __BSS_SRAM_SH_END,
        },
        {
            __BSS_SRAM_NC_START,
            __BSS_SRAM_NC_END,
        },
        {
            __BSS_SRAM_START,
            __BSS_SRAM_END,
        },
    },
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : init_data_bss
 * Description   : Make necessary initializations for RAM.
 * - Copy the vector table from ROM to RAM.
 * - Copy initialized data from ROM to RAM.
 * - Copy code that should reside in RAM from ROM
 * - Clear the zero-initialized data section.
 *
 * Tool Chains:
 *   __GNUC__           : GNU Compiler Collection
 *   __ghs__            : Green Hills ARM Compiler
 *   __ICCARM__         : IAR ARM Compiler
 *   __DCC__            : Wind River Diab Compiler
 *   __ARMCC_VERSION    : ARMC Compiler
 *
 * Implements    : init_data_bss_Activity
 *END**************************************************************************/
#define PLATFORM_START_SEC_CODE
#include "Platform_MemMap.h"

void init_data_bss(void);
void init_data_bss_core2(void);

void init_data_bss(void) {
    const Sys_CopyLayoutType *copy_layout;
    const Sys_ZeroLayoutType *zero_layout;

    const uint64_t *rom;
    const uint64_t *romEnd;
    uint64_t       *ram;

    uint32_t len;
    uint32_t i;

    /* Copy initialized table */
    len         = InitTable.size;
    copy_layout = &InitTable.table[0];
    for (i = 0; i < len; ++i, ++copy_layout) {
        ram    = copy_layout->ram_start;
        rom    = copy_layout->rom_start;
        romEnd = copy_layout->rom_end;
        while (rom < romEnd) {
            *(ram++) = *(rom++);
        }
    }

    /* Clear zero table */
    len         = ZeroTable.size;
    zero_layout = &ZeroTable.table[0];
    for (i = 0; i < len; ++i, ++zero_layout) {
        ram    = zero_layout->ram_start;
        romEnd = zero_layout->ram_end;
        while (ram < romEnd) {
            *(ram++) = 0;
        }
    }
}

void init_data_bss_core2(void)
{
    const Sys_CopyLayoutType * copy_layout;

    const uint64_t *rom;
    const uint64_t *romEnd;
    uint64_t       *ram;

    uint32_t len;
    uint32_t i;

    /* Copy initialized table */
    len         = InitTable.size;
    copy_layout = &InitTable.table[i = (uint32)__INDEX_COPY_CORE2];

    for (; i < len; ++i, ++copy_layout) {
        ram    = copy_layout->ram_start;
        rom    = copy_layout->rom_start;
        romEnd = copy_layout->rom_end;
        while (rom < romEnd) {
            *(ram++) = *(rom++);
        }
    }
}
#define PLATFORM_STOP_SEC_CODE
#include "Platform_MemMap.h"

/*******************************************************************************
 * EOF
 ******************************************************************************/
