/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           :
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 4.0.0
*   Build Version        : S32K3_RTD_4_0_0_HF01_D2401_ASR_REL_4_7_REV_0000_20240116
*
*   Copyright 2020 - 2024 NXP
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
*/
/*================================================================================================
*   @file    system.c
*   @version 4.0.0
*
*   @brief   AUTOSAR Platform - SYSTEM
*   @details SYSTEM
*            This file contains sample code only. It is not part of the production code deliverables.
*   @addtogroup PLATFORM
*   @{
*
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "platform.h"
#include "Mcal.h"
#include "core_specific.h"

/*==================================================================================================
*                                      FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL CONSTANTS
==================================================================================================*/

extern uint32 __INT_ITCM_START[];
extern uint32 __ROM_CODE_START[];
extern uint32 __ROM_DATA_START[];
extern uint32 __INT_DTCM_START[];
extern uint32 __INT_SRAM_START[];
extern uint32 __RAM_NO_CACHEABLE_START[];
extern uint32 __RAM_SHAREABLE_START[];
extern uint32 __RAM_CACHEABLE_SIZE[];
extern uint32 __RAM_NO_CACHEABLE_SIZE[];
extern uint32 __RAM_SHAREABLE_SIZE[];
/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
#define CM7_0 (0UL)
#define CM7_1 (1UL)
#define CM7_2 (2UL)
#define CM7_3 (3UL)

#define SVC_GoToSupervisor() ASM_KEYWORD("svc 0x0")
#define SVC_GoToUser()       ASM_KEYWORD("svc 0x1")

#define SCB_CCR_DC_MASK (1UL << 16U)
#define SCB_CCR_IC_MASK (1UL << 17U)
#define SCB_CCSIDR_NUMSETS_SHIFT 13U
#define SCB_CCSIDR_NUMSETS_MASK (0x7FFFUL << 13U)
#define SCB_CCSIDR_ASSOCIATIVITY_SHIFT 3U
#define SCB_CCSIDR_ASSOCIATIVITY_MASK  (0x3FFUL << 3U)
#define SCB_DCISW_SET_SHIFT 5U
#define SCB_DCISW_SET_MASK  (0x1FFUL << 5U)
#define SCB_DCISW_WAY_SHIFT 30U
#define SCB_DCISW_WAY_MASK (3UL << 30U)

/* Cache Size ID Register Macros */
#define CCSIDR_WAYS(x) (((x) & SCB_CCSIDR_ASSOCIATIVITY_MASK) >> SCB_CCSIDR_ASSOCIATIVITY_SHIFT)
#define CCSIDR_SETS(x) (((x) & SCB_CCSIDR_NUMSETS_MASK) >> SCB_CCSIDR_NUMSETS_SHIFT)

#define S32_SCB_CPACR_CPx_MASK(CpNum)  (0x3U << S32_SCB_CPACR_CPx_SHIFT(CpNum))
#define S32_SCB_CPACR_CPx_SHIFT(CpNum) (2U * ((uint32)CpNum))
#define S32_SCB_CPACR_CPx(CpNum, x)                                                                \
    (((uint32)(((uint32)(x)) << S32_SCB_CPACR_CPx_SHIFT((CpNum)))) &                               \
     S32_SCB_CPACR_CPx_MASK((CpNum)))

/* MPU setting */
#define CPU_MPU_MEMORY_COUNT (16U)

/*==================================================================================================
*                                       LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================-
*                                       GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL VARIABLES
==================================================================================================*/
#define PLATFORM_START_SEC_VAR_CLEARED_32
#include "Platform_MemMap.h"

#define PLATFORM_STOP_SEC_VAR_CLEARED_32
#include "Platform_MemMap.h"
/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

#define PLATFORM_START_SEC_CODE
#include "Platform_MemMap.h"

#define PLATFORM_STOP_SEC_CODE
#include "Platform_MemMap.h"

/*================================================================================================*/
/*
 * @brief Initializes the caches on the platform based on build options. This requires the MPU areas to be configured and enabled before calling this routine.
 * @param: None
 *
 * @return: None
 */
static INLINE void sys_m7_cache_init(void);
/*
 * @brief Disables any previously configured and initialized cache, please make sure MPU is enabled before calling these apis
 * @param: None
 *
 * @return: None
 */
static INLINE void sys_m7_cache_disable(void);
/*
 * @brief Performs a cache clean operation over the configured caches.
 * @param: None
 *
 * @return: None
 */
static INLINE void sys_m7_cache_clean(void);

#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
LOCAL_INLINE void Direct_GoToUser(void);
#endif
/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
LOCAL_INLINE void Direct_GoToUser(void) {
    ASM_KEYWORD("push {r0}");
    ASM_KEYWORD("ldr r0, =0x1");
    ASM_KEYWORD("msr CONTROL, r0");
    ASM_KEYWORD("pop {r0}");
}
#endif
/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
extern uint32 startup_getControlRegisterValue(void);
extern uint32 startup_getAipsRegisterValue(void);
extern void   Suspend_Interrupts(void);
extern void   Resume_Interrupts(void);
#endif /*MCAL_ENABLE_USER_MODE_SUPPORT*/

/*================================================================================================*/
/**
* @brief    startup_go_to_user_mode
* @details  Function called from startup.s to switch to user mode if MCAL_ENABLE_USER_MODE_SUPPORT
*           is defined
*/
/*================================================================================================*/
void startup_go_to_user_mode(void);
void startup_go_to_user_mode(void) {
#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
    ASM_KEYWORD("svc 0x1");
#endif
}

/*================================================================================================*/
/**
* @brief Sys_GoToSupervisor
* @details function used to enter to supervisor mode.
*           check if it's needed to switch to supervisor mode and make the switch.
*           Return 1 if switch was done
*/
/*================================================================================================*/

#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
uint32 Sys_GoToSupervisor(void) {
    uint32 u32ControlRegValue;
    uint32 u32AipsRegValue;
    uint32 u32SwitchToSupervisor;

    /* if it's 0 then Thread mode is already in supervisor mode */
    u32ControlRegValue = startup_getControlRegisterValue();
    /* if it's 0 the core is in Thread mode, otherwise in Handler mode */
    u32AipsRegValue = startup_getAipsRegisterValue();

    /* if core is already in supervisor mode for Thread mode, or running form Handler mode, there is no need to make the switch */
    if ((0U == (u32ControlRegValue & 1u)) || (0u < (u32AipsRegValue & 0xFFu))) {
        u32SwitchToSupervisor = 0U;
    } else {
        u32SwitchToSupervisor = 1U;
        SVC_GoToSupervisor();
    }

    return u32SwitchToSupervisor;
}

/*================================================================================================*/
/**
* @brief Sys_GoToUser_Return
* @details function used to switch back to user mode for Thread mode, return a uint32 value passed as parameter
*/
/*================================================================================================*/
uint32 Sys_GoToUser_Return(uint32 u32SwitchToSupervisor, uint32 u32returnValue) {
    if (1UL == u32SwitchToSupervisor) {
        Direct_GoToUser();
    }

    return u32returnValue;
}

uint32 Sys_GoToUser(void) {
    Direct_GoToUser();
    return 0UL;
}
/*================================================================================================*/
/**
* @brief Sys_SuspendInterrupts
* @details Suspend Interrupts
*/
/*================================================================================================*/
void Sys_SuspendInterrupts(void) {
    uint32 u32ControlRegValue;
    uint32 u32AipsRegValue;

    /* if it's 0 then Thread mode is already in supervisor mode */
    u32ControlRegValue = startup_getControlRegisterValue();
    /* if it's 0 the core is in Thread mode, otherwise in Handler mode */
    u32AipsRegValue = startup_getAipsRegisterValue();

    if ((0U == (u32ControlRegValue & 1u)) || (0u < (u32AipsRegValue & 0xFFu))) {
        Suspend_Interrupts();
    } else {
        ASM_KEYWORD(" svc 0x3");
    }
}
/*================================================================================================*/
/**
* @brief Sys_ResumeInterrupts
* @details Resume Interrupts
*/
/*================================================================================================*/
void Sys_ResumeInterrupts(void) {
    uint32 u32ControlRegValue;
    uint32 u32AipsRegValue;

    /* if it's 0 then Thread mode is already in supervisor mode */
    u32ControlRegValue = startup_getControlRegisterValue();
    /* if it's 0 the core is in Thread mode, otherwise in Handler mode */
    u32AipsRegValue = startup_getAipsRegisterValue();

    if ((0U == (u32ControlRegValue & 1u)) || (0u < (u32AipsRegValue & 0xFFu))) {
        Resume_Interrupts();
    } else {
        ASM_KEYWORD(" svc 0x2");
    }
}
#endif
/*================================================================================================*/
/**
* @brief Sys_GetCoreID
* @details Function used to get the ID of the currently executing thread
*/
/*================================================================================================*/
#if !defined(USING_OS_AUTOSAROS)
uint8 Sys_GetCoreID(void) {
    return (IP_MSCM->CPXNUM & MSCM_CPXNUM_CPN_MASK);
}
#endif

/*================================================================================================*/
/*
 * system initialization : system clock, interrupt router ...
 */

void SystemInit(void) __attribute__((section(".systeminit")));
void SystemInit(void) {
    uint32 i;
    uint32 coreMask = 0UL;
    uint8  coreId   = OsIf_GetCoreID();
#ifdef MPU_ENABLE
    uint8 regionNum = 0U;
#endif /* MPU_ENABLE */
    switch (coreId) {
    case CM7_0:
        coreMask = (1UL << MSCM_IRSPRC_M7_0_SHIFT);
        break;
    case CM7_1:
#if defined(S32K324) || defined(S32K358) || defined(S32K328) || defined(S32K338) || defined(S32K348)
        coreMask = (1UL << MSCM_IRSPRC_M7_1_SHIFT);
#endif
        break;
    case CM7_2:
#if defined(S32K396) || defined(S32K394) || defined(S32K376) || defined(S32K374)
        coreMask = (1UL << MSCM_IRSPRC_M7_2_SHIFT);
#endif
        break;
    case CM7_3:
#ifdef S32K388
        coreMask = (1UL << MSCM_IRSPRC_M7_3_SHIFT);
        break;
#endif
    default:
        coreMask = 0UL;
        break;
    }

    /* Configure MSCM to enable/disable interrupts routing to Core processor */
    for (i = 0; i < MSCM_IRSPRC_COUNT; i++) {
        IP_MSCM->IRSPRC[i] |= coreMask;
    }

    /**************************************************************************/
    /* MPU ENABLE                                                             */
    /**************************************************************************/
#ifdef MPU_ENABLE
    /**************************************************************************/
    /* DEFAULT MEMORY ENABLE*/
    /**************************************************************************/

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();

    /*Checking if cache is enable before*/
    if (((((uint32)1U << (uint32)17U) & S32_SCB->CCR) != (uint32)0) ||
        ((((uint32)1U << (uint32)16U) & S32_SCB->CCR) != (uint32)0)) {
        /*synchronize cache before update mpu */
        sys_m7_cache_clean();
        sys_m7_cache_disable();
    }

    enum {
        MPU_EXECUTE    = 0,
        MPU_NO_EXECUTE = 1,
    };

    enum {
        MPU_AP_NOACCESS    = 0,
        MPU_AP_RW_NOACCESS = 1,
        MPU_AP_RW_RO       = 2,
        MPU_AP_FULLACCESS  = 3,
        MPU_AP_RESERVED    = 4,
        MPU_AP_RO_NOACCESS = 5,
        MPU_AP_READONLY_0  = 6,
        MPU_AP_READONLY_1  = 7,
    };

    enum {
        MPU_SIZE_32B   = 4,
        MPU_SIZE_64B   = 5,
        MPU_SIZE_128B  = 6,
        MPU_SIZE_256B  = 7,
        MPU_SIZE_512B  = 8,
        MPU_SIZE_1KB   = 9,
        MPU_SIZE_2KB   = 10,
        MPU_SIZE_4KB   = 11,
        MPU_SIZE_8KB   = 12,
        MPU_SIZE_16KB  = 13,
        MPU_SIZE_32KB  = 14,
        MPU_SIZE_64KB  = 15,
        MPU_SIZE_128KB = 16,
        MPU_SIZE_256KB = 17,
        MPU_SIZE_512KB = 18,
        MPU_SIZE_1MB   = 19,
        MPU_SIZE_2MB   = 20,
        MPU_SIZE_4MB   = 21,
        MPU_SIZE_8MB   = 22,
        MPU_SIZE_16MB  = 23,
        MPU_SIZE_32MB  = 24,
        MPU_SIZE_64MB  = 25,
        MPU_SIZE_128MB = 26,
        MPU_SIZE_256MB = 27,
        MPU_SIZE_512MB = 28,
        MPU_SIZE_1GB   = 29,
        MPU_SIZE_2GB   = 30,
        MPU_SIZE_4GB   = 31,
    };

    /*********************************************************************************************
     * TEX S C B Memory type      Description                         Shareable
     * --- - - - ---------------- ----------------------------------- ---------
     *  0  1 0 0 Strongly Ordered Strongly ordered                    Yes
     *  0  1 0 1 Device           Shared device                       Yes
     *  0  S 1 0 Normal           Write through, no write allocate    S bit
     *  0  S 1 1 Normal           Write-back, no write allocate       S bit
     *  1  S 0 0 Normal           Non-cacheable                       S bit
     *  1  0 0 1 Reserved         Reserved                            Reserved
     *  1  0 1 0 Undefined        Undefined                           Undefined
     *  1  S 1 1 Normal           Write-back, write and read allocate S bit
     *  2  0 0 0 Device           Non-shareable device                No
     *  2  0 0 1 Reserved         Reserved                            Reserved
     ********************************************************************************************/
    enum {
        MPU_TYPE_WriteThrough         = 2,  // 0b00010
        MPU_TYPE_WriteBack            = 3,  // 0b00011
        MPU_TYPE_StrongOrder          = 4,  // 0b00100
        MPU_TYPE_SharedDevice         = 5,  // 0b00101
        MPU_TYPE_SharedWriteThrough   = 6,  // 0b00110
        MPU_TYPE_SharedWriteBack      = 7,  // 0b00111
        MPU_TYPE_NoCache              = 8,  // 0b01000
        MPU_TYPE_Reserved             = 9,  // 0b01001
        MPU_TYPE_Undefined            = 10, // 0b01010
        MPU_TYPE_WriteBackAlloc       = 11, // 0b01011
        MPU_TYPE_SharedNoCache        = 12, // 0b01100
        MPU_TYPE_SharedWriteBackAlloc = 15, // 0b01111
        MPU_TYPE_Device               = 16, // 0b10000
        MPU_TYPE_Reserved1            = 17, // 0b10001
    };

#define BIT(X) ((1UL) << (uint32_t)(X))
#define MPU_REGION(Addr, Access, Size, XN, TYPE, SRD)                                              \
    do {                                                                                           \
        ASSERT(0 == ((uint32_t)(Addr) & (BIT(Size) - 1)));                                         \
        S32_MPU->RNR  = regionNum++;                                                               \
        S32_MPU->RBAR = (uint32)(Addr);                                                            \
        S32_MPU->RASR = S32_MPU_RASR_XN(XN) | S32_MPU_RASR_AP(Access) |                            \
                        S32_MPU_RASR_TEX((TYPE) >> 3) | S32_MPU_RASR_S(((TYPE) >> 2) & 1) |        \
                        S32_MPU_RASR_C(((TYPE) >> 1) & 1) | S32_MPU_RASR_B((TYPE) & 1) |           \
                        S32_MPU_RASR_SRD(SRD) | S32_MPU_RASR_SIZE(Size) | S32_MPU_RASR_ENABLE(1);  \
    } while (0)
    /* Init MPU table for memory layout*/

    /**********************************************************************************************
     *  Check symbol and linker information in linker file of each derivative
     *  Region 15: Only S32K388
     *********************************************************************************************/
    /**********************************************************************************************
     *
     * Region Description      Start               End                         Size[KB]  Type              Inner Cache Policy    Outer Cache Policy    Shareable    Executable    Privileged Access    Unprivileged Access
     * ------ ---------------- ----------          ----------                ----------  ----------------  --------------------  --------------------  -----------  ------------  -------------------  ---------------------
     *   0    Whole memory map 0x00000000          0xFFFFFFFF                   4194304  Strongly Ordered  None                  None                  Yes          No            No Access            No Access
     *   1    ITCM             0x00000000          0x0000FFFF                        64  Normal            None                  None                  No           Yes           Read/Write           Read/Write
     *   2    Program Flash 1  0x40000000          PFLASH SIZE              PFLASH SIZE  Normal            Write-Back/Allocate   Write-Back/Allocate   No           Yes           Read-Only            Read-Only
     *   3    Data Flash       0x10000000          0x1003FFFF                       256  Normal            Write-Back/Allocate   Write-Back/Allocate   No           No            Read-Only            Read-Only
     *   4    UTEST            0x1B000000          0x1B001FFF                         8  Normal            Write-Back/Allocate   Write-Back/Allocate   No           No            Read-Only            Read-Only
     *   5    DTCM             0x20000000          0x2001FFFF                       128  Normal            None                  None                  No           Yes           Read/Write           Read/Write
     *   6    SRAM CACHE       _CACHEABLE_START    _CACHEABLE_END       _CACHEABLE_SIZE  Normal            Write-Back/Allocate   Write-Back/Allocate   No           Yes           Read/Write           Read/Write
     *   7    SRAM N-CACHE     _NO_CACHEABLE_START _NO_CACHEABLE_END _NO_CACHEABLE_SIZE  Normal            None                  None                  Yes          No            Read/Write           Read/Write
     *   8    SRAM SHARED      _SHAREABLE_START    _SHAREABLE_END       _SHAREABLE_SIZE  Normal            None                  None                  Yes          No            Read/Write           Read/Write
     *   9    AIPS_0/1/2       0x40000000          0x405FFFFF                      6144  Strongly ordered  None                  None                  Yes          No            Read/Write           Read/Write
     *   10   AIPS_3           0x40600000          0x407FFFFF                      2048  Strongly ordered  None                  None                  Yes          No            Read/Write           Read/Write
     *   11   QSPI Rx          0x67000000          0x670003FF                         1  Strongly ordered  None                  None                  Yes          No            Read/Write           Read/Write
     *   12   QSPI AHB         0x68000000          0x6FFFFFFF                    131072  Normal            Write-Back/Allocate   Write-Back/Allocate   No           Yes           Read/Write           Read/Write
     *   13   PPB              0xE0000000          0xE00FFFFF                      1024  Strongly Ordered  None                  None                  Yes          No            Read/Write           Read/Write
     *   14   Program Flash 2  0x00800000          PFLASH SIZE              PFLASH SIZE  Normal            Write-Back/Allocate   Write-Back/Allocate   No           Yes           Read-Only            Read-Only
     *   15   ACE              0x44000000          0x440003FF                         1  Strongly-ordered  None                  None                  Yes          No            Read/Write           Read/Write
     *********************************************************************************************/

    /* Cover all memory on device as background set all memory as strong-order and no access*/
    MPU_REGION(0, MPU_AP_NOACCESS, MPU_SIZE_4GB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder, 0);

    /* Note: For code portability to other Arm processors or systems, Arm recommends that TCM regions are always defined as Normal, Non-shared memory in the MPU. */
    /* This is consistent with the default ARMv7E-M memory map attributes that apply when the MPU is either disabled or not implemented.*/

    /* ITCM for cortex M7 if no set it as zero */
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__INT_ITCM_START, MPU_AP_FULLACCESS, MPU_SIZE_64KB, MPU_EXECUTE, MPU_TYPE_NoCache,
               0);

#if defined(S32K311) || defined(S32K341) || defined(S32M276) || defined(S32K310) || defined(S32M274)
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: No, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(__ROM_CODE_START, MPU_AP_READONLY_1, MPU_SIZE_1MB, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);
#elif defined(S32K342) || defined(S32K312) || defined(S32K322)
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: No, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(__ROM_CODE_START, MPU_AP_READONLY_1, MPU_SIZE_2MB, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);
#else
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: No, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(__ROM_CODE_START, MPU_AP_READONLY_1, MPU_SIZE_4MB, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);
#endif

    /*Data flash which would extract from linker symbol*/
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: Yes, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(__ROM_DATA_START, MPU_AP_READONLY_0, MPU_SIZE_256KB, MPU_NO_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);

    /*UTEST*/
    /* Size: 8KB, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: Yes, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(0x1B000000UL, MPU_AP_READONLY_0, MPU_SIZE_8KB, MPU_NO_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);

    /*DTCM for cortex m7 if no set it as zero*/
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__INT_DTCM_START, MPU_AP_FULLACCESS, MPU_SIZE_128KB, MPU_EXECUTE, MPU_TYPE_NoCache,
               0);

    /*Ram unified section*/
    /* Limitation : TCM is not cacheable memory, the purpose is to expand the RAM size for low RAM derivatives. Used for cases like ccov testing,... */
#if defined(EXTEND_LOWRAM_DERIVATIVES) &&                                                          \
    (defined(S32K310) || defined(S32K311) || defined(S32M274) || defined(S32M276))
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back, write and read allocate, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__INT_DTCM_START, MPU_AP_FULLACCESS, (uint32)__RAM_CACHEABLE_SIZE - 1, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);
#elif defined(S32K396) || defined(S32K394) || defined(S32K344) || defined(S32K324) ||              \
    defined(S32K314) || defined(S32K374) || defined(S32K376)
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back, write and read allocate, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    /* Disable subregion 7 & 8*/
    MPU_REGION(__INT_SRAM_START, MPU_AP_FULLACCESS, (uint32)__RAM_CACHEABLE_SIZE - 1, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, BIT(7) | BIT(6));
#else
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back, write and read allocate, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__INT_SRAM_START, MPU_AP_FULLACCESS, (uint32)__RAM_CACHEABLE_SIZE - 1, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);
#endif

#if defined(EXTEND_LOWRAM_DERIVATIVES) &&                                                          \
    (defined(S32K310) || defined(S32K311) || defined(S32M274) || defined(S32M276))
    /*Ram non-cache section plus int result which is using for test report*/
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__INT_SRAM_START, MPU_AP_FULLACCESS, (uint32)__RAM_NO_CACHEABLE_SIZE - 1,
               MPU_NO_EXECUTE, MPU_TYPE_SharedNoCache, 0);
#else
    /*Ram non-cache section plus int result which is using for test report*/
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__RAM_NO_CACHEABLE_START, MPU_AP_FULLACCESS, (uint32)__RAM_NO_CACHEABLE_SIZE - 1,
               MPU_NO_EXECUTE, MPU_TYPE_SharedNoCache, 0);
#endif

    /*Ram shareable section*/
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(__RAM_SHAREABLE_START, MPU_AP_FULLACCESS, (uint32)__RAM_SHAREABLE_SIZE - 1,
               MPU_NO_EXECUTE, MPU_TYPE_SharedNoCache, 0);
    /* Additional configuration for peripheral device*/

    /*AIPS_0, AIPS_1, AIPS_2*/
    /* Size: 6MB, Type: Strongly-ordered, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    /* Disable subregion 7 & 8, 8MB * 3/4 */
    MPU_REGION(0x40000000UL, MPU_AP_FULLACCESS, MPU_SIZE_8MB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder,
               BIT(7) | BIT(6));

    /*AIPS_3*/
#if defined(S32K396) || defined(S32K394) || defined(S32K374) || defined(S32K376)
    /* Size: 2MB, Type: Strongly-ordered, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(0x40600000UL, MPU_AP_FULLACCESS, MPU_SIZE_2MB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder,
               0);
#endif /* S32K39x */

    /*QSPI Rx*/
    /* Size: 1KB, Type: Strongly-ordered, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(0x67000000UL, MPU_AP_FULLACCESS, MPU_SIZE_1KB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder,
               0);

    /*QSPI AHB*/
    /* Size: 128MB, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back, write and read allocate, Shareable: No, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(0x68000000UL, MPU_AP_FULLACCESS, MPU_SIZE_128MB, MPU_EXECUTE,
               MPU_TYPE_WriteBackAlloc, 0);

    /*Private Peripheral Bus*/
    /* Size: Normal, Type: Strongly-ordered, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(0xE0000000UL, MPU_AP_FULLACCESS, MPU_SIZE_1MB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder,
               0);

    /* Program flash */
    /* Note: Do not merge with MPU region 2 because of alignment with the size */
#if defined(S32K396) || defined(S32K376)
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: No, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(((uint32)__ROM_CODE_START + 0x400000UL), MPU_AP_READONLY_1, MPU_SIZE_2MB,
               MPU_EXECUTE, MPU_TYPE_WriteBackAlloc, 0);
#elif defined(S32K358) || defined(S32K388) || defined(S32K328) || defined(S32K338) ||              \
    defined(S32K348)
    /* Size: import information from linker symbol, Type: Normal, Inner Cache Policy: Inner write-back, write and read allocate, Outer Cache Policy: Outer write-back. write and read allocate, Shareable: No, Privileged Access: Read-Only, Unprivileged Access: Read-Only */
    MPU_REGION(((uint32)__ROM_CODE_START + 0x400000UL), MPU_AP_READONLY_1, MPU_SIZE_4MB,
               MPU_EXECUTE, MPU_TYPE_WriteBackAlloc, 0);
#endif

    /*ACE region*/
#if defined(S32K388)
    /* Size: 1KB, Type: Strongly-ordered, Inner Cache Policy: None, Outer Cache Policy: None, Shareable: Yes, Privileged Access:RW, Unprivileged Access:RW */
    MPU_REGION(0x44000000UL, MPU_AP_FULLACCESS, MPU_SIZE_1KB, MPU_NO_EXECUTE, MPU_TYPE_StrongOrder,
               0);
#endif

    /* Enable MPU, enables the MPU during the HardFault handler */
    S32_MPU->CTRL |= (S32_MPU_CTRL_ENABLE_MASK | S32_MPU_CTRL_HFNMIENA_MASK);

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();

#endif /* MPU_ENABLE */
    /**************************************************************************/
    /* ENABLE CACHE */
/**************************************************************************/
#if defined(D_CACHE_ENABLE) || defined(I_CACHE_ENABLE)
    sys_m7_cache_init();
#endif /*defined(D_CACHE_ENABLE) || defined(I_CACHE_ENABLE)*/
}

/* Cache apis which are using for cache initilization, please make sure MPU is enable before calling these apis. Due to limitation of speculative access on cortex m7, MPU need to be initialized before enable cache. So if user specify -DDISABLE_MPUSTARTUP, cache will be disable in startup as well. If user want to enable cache again please call cache api after RM_init() or MPU_init() */

static INLINE void sys_m7_cache_init(void) {
#ifdef D_CACHE_ENABLE
    uint32 ccsidr = 0U;
    uint32 sets   = 0U;
    uint32 ways   = 0U;

    /*init Data caches*/
    S32_SCB->CSSELR = 0U; /* select Level 1 data cache */
    MCAL_DATA_SYNC_BARRIER();
    ccsidr = S32_SCB->CCSIDR;
    sets   = (uint32)(CCSIDR_SETS(ccsidr));
    do {
        ways = (uint32)(CCSIDR_WAYS(ccsidr));
        do {
            S32_SCB->DCISW = (((sets << SCB_DCISW_SET_SHIFT) & SCB_DCISW_SET_MASK) |
                              ((ways << SCB_DCISW_WAY_SHIFT) & SCB_DCISW_WAY_MASK));
            MCAL_DATA_SYNC_BARRIER();
        } while (ways-- != 0U);
    } while (sets-- != 0U);
    MCAL_DATA_SYNC_BARRIER();
    S32_SCB->CCR |= (uint32)SCB_CCR_DC_MASK; /* enable D-Cache */
    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
#endif /*D_CACHE_ENABLE*/

#ifdef I_CACHE_ENABLE
    /*init Code caches*/
    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();

    S32_SCB->ICIALLU = 0UL; /* invalidate I-Cache */

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();

    S32_SCB->CCR |= (uint32)SCB_CCR_IC_MASK; /* enable I-Cache */

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
#endif /*I_CACHE_ENABLE*/
}

static INLINE void sys_m7_cache_disable(void) {
    sys_m7_cache_clean();
    S32_SCB->CCR &= ~((uint32)1U << 17U);

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();

    S32_SCB->CCR &= ~((uint32)1U << 16U);
    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
}

static INLINE void sys_m7_cache_clean(void) {
#ifdef D_CACHE_ENABLE
    uint32 ccsidr = 0U;
    uint32 sets   = 0U;
    uint32 ways   = 0U;

    S32_SCB->CSSELR = 0U; /* select Level 1 data cache */
    MCAL_DATA_SYNC_BARRIER();
    ccsidr = S32_SCB->CCSIDR;
    sets   = (uint32)(CCSIDR_SETS(ccsidr));
    do {
        ways = (uint32)(CCSIDR_WAYS(ccsidr));
        do {
            S32_SCB->DCCISW =
                (((sets << 5) & (uint32)0x3FE0U) | ((ways << 30) & (uint32)0xC0000000U));
            MCAL_DATA_SYNC_BARRIER();
        } while (ways-- != 0U);
    } while (sets-- != 0U);

    S32_SCB->CSSELR = (uint32)((S32_SCB->CSSELR) | 1U);
#endif /*D_CACHE_ENABLE*/

#ifdef I_CACHE_ENABLE
    S32_SCB->ICIALLU = 0UL;
#endif /*I_CACHE_ENABLE*/
    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
    ;
}
/**************************************************************************/
/* FPU ENABLE*/
/**************************************************************************/
void Enable_FPU(void) __attribute__((section(".systeminit")));
void Enable_FPU(void) {
#ifdef ENABLE_FPU
    /* Enable CP10 and CP11 coprocessors */
    S32_SCB->CPACR |= (S32_SCB_CPACR_CPx(10U, 3U) | S32_SCB_CPACR_CPx(11U, 3U));

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
#endif /* ENABLE_FPU */
}

#ifdef __cplusplus
}
#endif
