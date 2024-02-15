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
#include "Platform_TypesDef.h"

#ifdef MULTIPLE_IMAGE
    #define RAM_DATA_INIT_ON_ALL_CORES
    /* If this is a secodary core, it shall wait for the MSCM clock to be initialized */
    #if defined(CORE1)||defined(CORE2)||defined(CORE3)
        #define NO_MSCM_CLOCK_INIT
    #endif
#endif

#ifdef MCAL_TESTING_ENVIRONMENT
void _core_loop(void) __attribute__((naked, section(".core_loop")));
void _core_loop(void) {
    for (;;) {
        __asm("nop \n");
        __asm("nop \n");
        __asm("nop \n");
        __asm("nop \n");
    }
}
#endif

extern const uint32_t __RAM_INTERRUPT_START[];
extern const uint32_t __INIT_INTERRUPT_START[];

void init_data_bss(void);
void init_data_bss_core2(void);
void SystemInit(void);
void Enable_FPU(void);
void startup_go_to_user_mode(void);
int main(void);

/************************************************************************/
/* Autosar startup code (See MCU Specification):                        */
/*                                                                      */
/*   Before the MCU driver can be initialized, a basic initialization   */
/*   of the MCU has to be executed. This MCU specific initialization is */
/*   typically executed in a start-up code. The start-up code of the    */
/*   MCU shall be executed after power up and any kind of micro-        */
/*   controller reset. It shall perform very basic and microcontroller  */
/*   specific start-up initialization and shall be kept short, because  */
/*   the MCU clock and PLL is not yet initialized. The start-up code    */
/*   shall cover MCU specific initialization, which is not part of      */
/*   other MCU services or other MCAL drivers. The following steps      */
/*   summarizes basic functionality which shall be included in the      */
/*   start-up code. They are listed for guidance, because some          */
/*   functionality might not be supported. No code will be found in     */
/*   case.                                                              */
/************************************************************************/

void Reset_Handler(void) __attribute__((naked, __noreturn__, target("general-regs-only"), section(".startup")));
void Reset_Handler(void) {
    /*****************************************************/
    /* Skip normal entry point as nothing is initialized */
    /*****************************************************/
    /* Disable System Interrupts */
    SuspendAllInterrupts();

    /************************************************************************/
    /* Delay trap for debugger attachs before touching any peripherals      */
    /* This is workaround for debugger cannot handle halt process properly, */
    /* User can safely disable this delay trap using -DDISABLE_DEBUGGER_TRAP*/
    /************************************************************************/
#ifndef DISABLE_DEBUGGER_TRAP
    {
        uint32_t delay = 100;
        do {
            --delay;
        } while (delay > 0);
    }
#endif

    /******************************************************************************/
    /*                      MSCM initialization process                           */
    /* Only Master core can initialize clock for MSCM module in multicore testing */
    /******************************************************************************/
#ifndef NO_MSCM_CLOCK_INIT
    /* If the MSCM clock is enabled, skip this sequence */
    if (0 == (IP_MC_ME->PRTN1_COFB0_STAT & MC_ME_PRTN1_COFB0_STAT_BLOCK24_MASK)) {
        /* Step1: Enable clock in PRTN1 */
        IP_MC_ME->PRTN1_COFB0_CLKEN |= MC_ME_PRTN1_COFB0_CLKEN_REQ24_MASK;
        /* Step2: Set PUPD field */
        IP_MC_ME->PRTN1_PUPD |= MC_ME_PRTN1_PUPD_PCUD_MASK;

#define MCME_KEY     (0x5AF0)
#define MCME_INV_KEY (0xA50F)

        /* Step3: Trigger update by writing valid key */
        IP_MC_ME->CTL_KEY = MCME_KEY;
        IP_MC_ME->CTL_KEY = MCME_INV_KEY;

        /* Step4: Wait for process to complete */
        while (IP_MC_ME->PRTN1_PUPD & MC_ME_PRTN1_PUPD_PCUD_MASK)
            ;
#endif
#ifndef SIM_TYPE_VDK
        /* Step5: Check if the MSCM clock is enabled */
        while (0 == (IP_MC_ME->PRTN1_COFB0_STAT & MC_ME_PRTN1_COFB0_STAT_BLOCK24_MASK))
            ;
#endif
#ifndef NO_MSCM_CLOCK_INIT
    }
#endif

    /*******************************************************************/
    /* NXP Guidance  - Init registers to avoid lock-step issues        */
    /* N/A                                                             */
    /*******************************************************************/

    /*******************************************************************/
    /* NXP Guidance  - MMU Initialization for CPU                      */
    /*  TLB0 - PbridgeB                                                */
    /*  TLB1 - Internal Flash                                          */
    /*  TLB2 - External SRAM                                           */
    /*  TLB3 - Internal SRAM                                           */
    /*  TLB4 - PbridgeA                                                */
    /*******************************************************************/
    /******************************************************************/
    /* Autosar Guidance   - The start-up code shall ensure that the   */
    /* MCU internal watchdog shall not be serviced until the watchdog */
    /* is initialized from the MCAL watchdog driver. This can be      */
    /* done for example by increasing the watchdog service time.      */
    /*                                                                */
    /******************************************************************/

#define MAIN_CORE 0

#ifdef MULTIPLE_CORE
    /*GetCoreID*/
    if (MAIN_CORE == IP_MSCM->CPXNUM)
#endif
    {
        /* Disable wdg0 */
        IP_SWT_0->SR = 0xC520;
        IP_SWT_0->SR = 0xD928;
        IP_SWT_0->CR = SWT_CR_MAP0_MASK | SWT_CR_MAP1_MASK | SWT_CR_MAP2_MASK | SWT_CR_MAP3_MASK |
                       SWT_CR_MAP4_MASK | SWT_CR_MAP5_MASK | SWT_CR_MAP6_MASK | SWT_CR_MAP7_MASK |
                       SWT_CR_ITR_MASK; /* 0xFF000040 */
    }

    /******************************************************************/
    /* Autosar Guidance - The start-up code shall initialize the      */
    /* base addresses for interrupt and trap vector tables. These base*/
    /* addresses are provided as configuration parameters or          */
    /* linker/locator setting.                                        */
    /******************************************************************/

    /* Set VTOR to default vector table */
    S32_SCB->VTOR = (uint32_t)__INIT_INTERRUPT_START;

    /******************************************************************/
    /* Autosar Guidance  - The start-up code shall initialize the     */
    /* interrupt stack pointer, if an interrupt stack is              */
    /* supported by the MCU. The interrupt stack pointer base address */
    /* and the stack size are provided as configuration parameter or  */
    /* linker/locator setting.                                        */
    /*                                                                */
    /******************************************************************/

    /* Enable DTCM and Disable RETEN bit */
    S32_SCB->DTCMCR = (S32_SCB->DTCMCR & (~4U)) | 1U;
    /* Enable ITCM and Disable RETEN bit */
    S32_SCB->ITCMCR = (S32_SCB->ITCMCR & (~4U)) | 1U;

#if defined(MULTIPLE_CORE) && !defined(MULTIPLE_IMAGE)
    /*GetCoreID*/
    if (MAIN_CORE == IP_MSCM->CPXNUM)
#endif
    {
        /******************************************************************/
        /* Autosar Guidance   - If the MCU supports context save          */
        /* operation, the start-up code shall initialize the memory which */
        /* is used for context save operation. The maximum amount of      */
        /* consecutive context save operations is provided as             */
        /* configuration parameter or linker/locator setting.             */
        /*                                                                */
        /******************************************************************/

        /******************************************************************/
        /* Autosar Guidance    - The start-up code shall initialize a     */
        /* minimum amount of RAM in order to allow proper execution of    */
        /* the MCU driver services and the caller of these services.      */
        /******************************************************************/

        /* Initialize SRAM ECC */
        extern uint32_t __RAM_INIT[];
        /* Skip if __RAM_INIT is not set */
        if (0 != (uint32_t)__RAM_INIT) {
            extern uint64_t __INT_SRAM_START[];
            extern uint64_t __INT_SRAM_END[];
            uint64_t* pStart = __INT_SRAM_START;
            uint64_t* pEnd = __INT_SRAM_END;
            /* NO_INIT_STANDBY_REGION */
#if defined(EXTEND_LOWRAM_DERIVATIVES) && (defined(S32K310) || defined(S32K311) || defined(S32M276) || defined(S32M274))
            if (0 == (IP_MC_RGM->DES & MC_RGM_DES_F_POR_MASK)) {
                extern uint64_t __BSS_SRAM_NC_START[];
                gStart = __BSS_SRAM_NC_START;
            }
#endif

            /* ZERO_64B_RAM */
            DevAssert(0 == ((uint32_t)pStart & 7));
            DevAssert(0 == ((uint32_t)pEnd & 7));
            while (pStart < pEnd) {
                *(pStart++) = 0ULL;
            }
        }
    }

    /* Initialize DTCM ECC */
    extern uint32_t __DTCM_INIT[];
    /* Skip if __DTCM_INIT is not set */
    if (0 != (uint32_t)__DTCM_INIT) {
        extern uint64_t __INT_DTCM_START[];
        extern uint64_t __INT_DTCM_END[];
        uint64_t* pStart = __INT_DTCM_START;
        uint64_t* pEnd = __INT_DTCM_END;

        DevAssert(0 == ((uint32_t)pStart & 7));
        DevAssert(0 == ((uint32_t)pEnd & 7));
        while (pStart < pEnd) {
            *(pStart++) = 0ULL;
        }
    }

    /* Initialize ITCM ECC */
    extern uint32_t __ITCM_INIT[];
    /* Skip if __DTCM_INIT is not set */
    if (0 != (uint32_t)__ITCM_INIT) {
        extern uint64_t __INT_ITCM_START[];
        extern uint64_t __INT_ITCM_END[];
        uint64_t* pStart = __INT_ITCM_START;
        uint64_t* pEnd = __INT_ITCM_END;

        DevAssert(0 == ((uint32_t)pStart & 7));
        DevAssert(0 == ((uint32_t)pEnd & 7));
        while (pStart < pEnd) {
            *(pStart++) = 0ULL;
        }
    }

    #ifndef NDEBUG
    {
        extern uint32_t RESET_CATCH_CORE;
        while (0x5A5A5A5A == RESET_CATCH_CORE);
    }
    #endif

    /******************************************************************/
    /* Autosar Guidance   - The start-up code shall initialize the    */
    /* user stack pointer. The user stack pointer base address and    */
    /* the stack size are provided as configuration parameter or      */
    /* linker/locator setting.                                        */
    /******************************************************************/

    /* set up stack; r13 SP*/
    extern uint32_t __Stack_dtcm_start[];
    __asm (
		"msr msp, %[inputSP] \t\n"
    	:
    	: [inputSP] "r" (__Stack_dtcm_start)
    );

#if defined(MULTIPLE_CORE) && !defined(MULTIPLE_IMAGE)
    /*GetCoreID*/
    if (MAIN_CORE == IP_MSCM->CPXNUM) {

#if (CM7_1_ENABLE == 0)
        /* EnableCore1 */
        if (0 == (IP_MC_ME->PRTN0_CORE1_STAT & MC_ME_PRTN0_CORE1_STAT_CCS_MASK)) {
            IP_MC_ME->PRTN0_CORE1_ADDR = (uint32_t)__INIT_INTERRUPT_START;
            IP_MC_ME->PRTN0_CORE1_PCONF = MC_ME_PRTN0_CORE1_PCONF_CCE_MASK;
            IP_MC_ME->PRTN0_CORE1_PUPD = MC_ME_PRTN0_CORE1_PUPD_CCUPD_MASK;
            IP_MC_ME->CTL_KEY = MCME_KEY;
            IP_MC_ME->CTL_KEY = MCME_INV_KEY;
            while (0 == (IP_MC_ME->PRTN0_CORE1_STAT & MC_ME_PRTN0_CORE1_STAT_CCS_MASK));
        }
#endif
#if (CM7_2_ENABLE == 0)
        /* EnableCore2 */
        if (0 == (IP_MC_ME->PRTN0_CORE4_STAT & MC_ME_PRTN0_CORE4_STAT_CCS_MASK)) {
            IP_MC_ME->PRTN0_CORE4_ADDR = (uint32_t)__INIT_INTERRUPT_START;
            IP_MC_ME->PRTN0_CORE4_PCONF = MC_ME_PRTN0_CORE4_PCONF_CCE_MASK;
            IP_MC_ME->PRTN0_CORE4_PUPD = MC_ME_PRTN0_CORE4_PUPD_CCUPD_MASK;
            IP_MC_ME->CTL_KEY = MCME_KEY;
            IP_MC_ME->CTL_KEY = MCME_INV_KEY;
            while (0 == (IP_MC_ME->PRTN0_CORE4_STAT & MC_ME_PRTN0_CORE4_STAT_CCS_MASK));
        }
#endif
    }
#endif

    /************************/
    /* Erase ".bss Section" */
    /************************/
#ifndef RAM_DATA_INIT_ON_ALL_CORES
    /* If this is the primary core, initialize data and bss */
    /*GetCoreID*/
    if (MAIN_CORE == IP_MSCM->CPXNUM) {
        init_data_bss();
    } else {
        init_data_bss_core2();
    }
#endif

    /* Set VTOR to default vector table */
    S32_SCB->VTOR = (uint32_t)__RAM_INTERRUPT_START;

    /***************************************************************/
    /* FPU need to enable prior to data-initialization process,in  */
    /* case optimization level set to O3 compiler can utilize SIMD */
    /* instruction which required involvement of FPU co-processor  */
    /***************************************************************/

    Enable_FPU();

    /******************************************************************/
    /* Autosar Guidance   - If the MCU supports cache memory for data */
    /* and/or code, it shall be initialized and enabled in the        */
    /* start-up code.                                                 */
    /*                                                                */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance   - The start-up code shall initialize MCU    */
    /* specific features of internal memory like memory protection.   */
    /*                                                                */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance   - If external memory is used, the memory    */
    /* shall be initialized in the start-up code. The start-up code   */
    /* shall be prepared to support different memory configurations   */
    /* depending on code location. Different configuration options    */
    /* shall be taken into account for code execution from            */
    /* external/internal memory.                                      */
    /* N/A - external memory is not used                              */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance   - The settings of the different memories    */
    /* shall be provided to the start-up code as configuration        */
    /* parameters.                                                    */
    /* N/A - all memories are already configured                      */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance    - In the start-up code a default           */
    /* initialization of the MCU clock system shall be performed      */
    /* including global clock prescalers.                             */
    /******************************************************************/

    SystemInit();

    /******************************************************************/
    /* Autosar Guidance   - The start-up code shall ensure that the   */
    /* MCU internal watchdog shall not be serviced until the watchdog */
    /* is initialized from the MCAL watchdog driver. This can be      */
    /* done for example by increasing the watchdog service time.      */
    /*                                                                */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance    - The start-up code shall enable           */
    /* protection mechanisms for special function registers(SFRs),    */
    /* if supported by the MCU.                                       */
    /* N/A - will be handled by Autosar OS                            */
    /******************************************************************/

    /******************************************************************/
    /* Autosar Guidance    - The start-up code shall initialize all   */
    /* necessary write once registers or registers common to several  */
    /* drivers where one write, rather than repeated writes, to the   */
    /* register is required or highly desirable.                      */
    /******************************************************************/

    /*********************************/
    /* Set the small ro data pointer */
    /*********************************/


    /*********************************/
    /* Set the small rw data pointer */
    /*********************************/

    /******************************************************************/
    /* Call Main Routine                                              */
    /******************************************************************/
    ResumeAllInterrupts();
    startup_go_to_user_mode();
    main();
}

/******************************************************************/
/* Init runtime check data space                                  */
/******************************************************************/
void MCAL_LTB_TRACE_OFF(void) __attribute__((naked, __noreturn__, target("general-regs-only"), section(".startup")));
void MCAL_LTB_TRACE_OFF(void) {
    __asm("nop");
#ifdef CCOV_ENABLE
    void ccov_main(void);
    /* code coverage is requested */
    ccov_main()
#endif
    for(;;);
}

/*BKPT #1 - removed to avoid debug fault being escalated to hardfault when debugger is not attached or on VDK*/ /* last instruction for the debugger to dump results data */
void _end_of_eunit_test(void) __attribute__((naked, __noreturn__, target("general-regs-only"), section(".startup")));
void _end_of_eunit_test(void) {
    for(;;);
}

#ifdef MCAL_ENABLE_USER_MODE_SUPPORT

void startup_getControlRegisterValue(void) __attribute__((naked, __noreturn__, target("general-regs-only"), section(".startup")));
void startup_getControlRegisterValue(void) {
    __asm("mrs r0, CONTROL \n");
    __asm("bx r14 \n");
}

void startup_getAipsRegisterValue(void) __attribute__((naked, __noreturn__, target("general-regs-only"), section(".startup")));
void startup_getAipsRegisterValue(void) {
    __asm("mrs r0, IPSR \n");
    __asm("bx r14 \n");
}

#endif
