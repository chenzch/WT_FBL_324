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
==================================================================================================*/
/*
* Target device: This linker is demo and it is using for device S32K324 only. It is not part of the production code deliverables.
* Target core: ARM cortex M7
* Linker need to align with MPU default setup in system.c
*/

/*
* GCC Linker Command File:
* 0x00000000    0x00007FFF  32768       ITCM
* 0x20000000    0x2001FFFF  65536       DTCM
* 0x00400000    0x007FFFFF  4194304     Program Flash
* 0x10000000    0x1001FFFF  131072      Data Flash
* 0x20400000    0x20427FFF  163840      SRAM_0
* 0x20428000    0x20443FFF  114688      SRAM_1  Actually only initialize 112 kB instead of 160 kB in RM, 48kB is blocked by fimrware
* Last 176 KB of CODE_FLASH reserved by HSE Firmware
* standby_data section should contain only uninitialized data
*/
__STANDBY_RAM_LIMIT_END  = 0x20407FFF;  /* 32Kbyte for standby ram */

ENTRY(Reset_Handler)

MEMORY
{
    int_pfblflash           : ORIGIN = 0x00400000, LENGTH = 0x00100000    /* 1024KB */
    int_pappflash           : ORIGIN = 0x00500000, LENGTH = 0x002D4000    /* 3072KB - 176KB (sBAF + HSE)*/
    int_dbootheader         : ORIGIN = 0x10000000, LENGTH = 0x00000218    /* 536B */
    int_dflash              : ORIGIN = 0x10000218, LENGTH = 0x0001FDE8    /* 128KB - 536B */
    int_itcm                : ORIGIN = 0x00000000, LENGTH = 0x00008000    /* 32KB */
    int_dtcm                : ORIGIN = 0x20000000, LENGTH = 0x0000F000    /* 60KB */
    int_stack_dtcm          : ORIGIN = 0x2000F000, LENGTH = 0x00001000    /* 4KB */
    int_sram                : ORIGIN = 0x20400000, LENGTH = 0x0002FF00    /* 192KB, needs to include int_sram_fls_rsv */
    int_sram_fls_rsv        : ORIGIN = 0x2042FF00, LENGTH = 0x00000100
    int_sram_no_cacheable   : ORIGIN = 0x20430000, LENGTH = 0x0000FF00    /* 64KB, needs to include int_sram_results  */
    int_sram_results        : ORIGIN = 0x2043FF00, LENGTH = 0x00000100
    int_sram_shareable      : ORIGIN = 0x20440000, LENGTH = 0x00003FF0    /* 16KB - 16B */
    ram_rsvd2               : ORIGIN = 0x20443FF0, LENGTH = 0x00000000    /* End of SRAM */
    int_exchange            : ORIGIN = 0x20443FF0, LENGTH = 0x00000010    /* 16B, exchange area */
}

TARGET(binary) /* specify the file format of binary file */
INPUT (..\..\Application\Release_FLASH\Application.bin)
OUTPUT_FORMAT(default) /* restore the out file format */

SECTIONS
{

    .pfblflash :
    {
        . = ALIGN(8);
        __text_start = .;
        __interrupts_init_start = .;
        KEEP(*(.intc_vector))
        . = ALIGN(8);
        __interrupts_init_end = .;
        KEEP(*(.core_loop))
        . = ALIGN(4);
        *(.startup)
        . = ALIGN(4);
        *(.systeminit)
        . = ALIGN(4);
        *(.text.startup)
        . = ALIGN(4);
        *(.text)
        *(.text*)
        . = ALIGN(4);
        *(.mcal_text)
        . = ALIGN(4);
        *(.acmcu_code_rom)
        . = ALIGN(8);
        __acfls_code_rom_start = .;
        *(.acfls_code_rom)
        . = ALIGN(8);
        __acfls_code_rom_end = .;
        __acmem_43_infls_code_rom_start = .;
        *(.acmem_43_infls_code_rom)
        . = ALIGN(8);
        __acmem_43_infls_code_rom_end = .;
        KEEP(*(.init))
        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        *(.mcal_const_cfg)
        . = ALIGN(4);
        *(.mcal_const)
        . = ALIGN(8);
    } > int_pfblflash

    . = ALIGN(8);
    __text_end = .;
    __sram_data_rom = __text_end;

    .pappflash :
    {
        __applEntrance = .;
        ..\..\Application\Release_FLASH\Application.bin
    } > int_pappflash

    .bootheader :
    {
        KEEP(*(.boot_header))
        /* . = ALIGN(8192); */ /* The minimum erase size of C40 Flash IP is 8kb */
    } > int_dbootheader

    .sram_bss (NOLOAD) :
    {
        . = ALIGN(8);
        __standby_ram_begin__ = .;
        *(.standby_data)
        . = ALIGN(8);
        __standby_ram_end__ = .;
        . = ALIGN(16);
        __sram_bss_start = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(16);
        *(.mcal_bss)
        . = ALIGN(4);
        __sram_bss_end = .;
    } > int_sram

    .sram_data : AT(__sram_data_rom)
    {
        . = ALIGN(8);
        __sram_data_begin__ = .;
        . = ALIGN(4);
        *(.ramcode)
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
        *(.mcal_data)
        . = ALIGN(8);
        __sram_data_end__ = .;
    } > int_sram

    __sram_data_rom_end = __sram_data_rom + (__sram_data_end__ - __sram_data_begin__);

    .acfls_code_ram :
    {
        . = ALIGN(8);
        __acfls_code_ram_start  = .;
        *(.acfls_code_ram)
        . = ALIGN(8);
        __acfls_code_ram_end   = .;
        __acmem_43_infls_code_ram_start = .;
        *(.acmem_43_infls_code_ram)
        . = ALIGN(8);
        __acmem_43_infls_code_ram_end = .;
    } > int_sram_fls_rsv

    __non_cacheable_data_rom = __sram_data_rom_end;

    .non_cacheable_data : AT(__non_cacheable_data_rom)
    {
        . = ALIGN(8);
        __non_cacheable_data_start__ = .;
        *(.mcal_data_no_cacheable)
        . = ALIGN(4);
        *(.mcal_const_no_cacheable)
        . = ALIGN(4);
        HSE_LOOP_ADDR = .;
        /* LONG(0x0); */
        . = ALIGN(8);
        __non_cacheable_data_end__ = .;
    } > int_sram_no_cacheable

    __non_cacheable_data_rom_end = __non_cacheable_data_rom + (__non_cacheable_data_end__ - __non_cacheable_data_start__);

    .non_cacheable_bss (NOLOAD) :
    {
        . = ALIGN(16);
        __non_cacheable_bss_start = .;
        *(.mcal_bss_no_cacheable)
        . = ALIGN(4);
        __non_cacheable_bss_end = .;
    } > int_sram_no_cacheable

    .int_results (NOLOAD):
    {
        . = ALIGN(4);
        KEEP(*(.int_results))
        /* . += 0x100; */
    } > int_sram_results

    __shareable_data_rom = __non_cacheable_data_rom_end;

    .shareable_data : AT(__shareable_data_rom)
    {
        . = ALIGN(8);
        __shareable_data_start__ = .;
        KEEP(*(.mcal_shared_data))
        . = ALIGN(8);
        __shareable_data_end__ = .;
    } > int_sram_shareable

    __shareable_data_rom_end = __shareable_data_rom + (__shareable_data_end__ - __shareable_data_start__);

    .int_vector :
    {
        . = ALIGN(2048);
        __interrupts_ram_start = .;
        . += __interrupts_init_end - __interrupts_init_start;
        . = ALIGN(8);
        __interrupts_ram_end = .;
    } > int_dtcm

    __tcm_data_rom = __shareable_data_rom_end;

    .data_tcm_data : AT(__tcm_data_rom)
    {
        . = ALIGN(8);
        __dtcm_data_start__ = .;
        *(.dtcm_data*)
        . = ALIGN(8);
        __dtcm_data_end__ = .;
    } > int_dtcm

    __tcm_data_rom_end = __tcm_data_rom + (__dtcm_data_end__ - __dtcm_data_start__);

    .bss_tcm_data (NOLOAD) :
    {
        . = ALIGN(8);
        __dtcm_bss_start__ = .;
        *(.dtcm_bss*)
        . = ALIGN(8);
        __dtcm_bss_end__ = .;
    } > int_dtcm

    __tcm_code_rom_start = __tcm_data_rom_end;

   .itcm_text : AT(__tcm_code_rom_start)
    {
        . = ALIGN(8);
        __itcm_start__ = .;
        *(.itcm_text*)
        . = ALIGN(8);
        __itcm_end__ = .;
    } > int_itcm

    __tcm_code_rom_end = __tcm_code_rom_start + (__itcm_end__ - __itcm_start__);

    .shareable_bss (NOLOAD) :
    {
        . = ALIGN(16);
        __shareable_bss_start = .;
        *(.mcal_shared_bss)
        . = ALIGN(4);
        __shareable_bss_end = .;
    } > int_sram_shareable

    __Stack_dtcm_end         = ORIGIN(int_stack_dtcm);
    __Stack_dtcm_start       = ORIGIN(int_stack_dtcm) + LENGTH(int_stack_dtcm);

    __INT_SRAM_START         = ORIGIN(int_sram);
    __INT_SRAM_END           = ORIGIN(ram_rsvd2);

    __INT_ITCM_START         = ORIGIN(int_itcm);
    __INT_ITCM_END           = ORIGIN(int_itcm) + LENGTH(int_itcm);

    __INT_DTCM_START         = ORIGIN(int_dtcm);
    __INT_DTCM_END           = ORIGIN(int_dtcm) + LENGTH(int_dtcm);

    __RAM_SHAREABLE_START    = __shareable_data_start__;
    __RAM_SHAREABLE_SIZE     = 0xE;  /* 16 kbyte in power of 2 */
    __ROM_SHAREABLE_START    = __shareable_data_rom;
    __ROM_SHAREABLE_END      = __shareable_data_rom_end;
    __RAM_NO_CACHEABLE_START = __non_cacheable_data_start__;
    __RAM_NO_CACHEABLE_SIZE  = 0x10;  /* 64kbyte in power of 2 */
    __ROM_NO_CACHEABLE_START = __non_cacheable_data_rom;
    __ROM_NO_CACHEABLE_END   = __non_cacheable_data_rom_end;
    __RAM_CACHEABLE_START    = __sram_data_begin__;
    __RAM_CACHEABLE_SIZE     = 0x12;  /* 256kbyte in power of 2 */
    __ROM_CACHEABLE_START    = __sram_data_rom;
    __ROM_CACHEABLE_END      = __sram_data_rom_end;
    __ROM_CODE_START         = ORIGIN(int_pfblflash);
    __ROM_DATA_START         = ORIGIN(int_dflash);

    __BSS_SRAM_START         = __sram_bss_start;
    __BSS_SRAM_END           = __sram_bss_end;
    __BSS_SRAM_SIZE          = __sram_bss_end - __sram_bss_start;

    __BSS_SRAM_NC_START      = __non_cacheable_bss_start;
    __BSS_SRAM_NC_SIZE       = __non_cacheable_bss_end - __non_cacheable_bss_start;
    __BSS_SRAM_NC_END        = __non_cacheable_bss_end;

    __BSS_SRAM_SH_START      = __shareable_bss_start;
    __BSS_SRAM_SH_SIZE       = __shareable_bss_end - __shareable_bss_start;
    __BSS_SRAM_SH_END        = __shareable_bss_end;

    __RAM_INTERRUPT_START    = __interrupts_ram_start;
    __INIT_INTERRUPT_START   = __interrupts_init_start;
    __INIT_INTERRUPT_END     = __interrupts_init_end;

    __RAM_ITCM_START         = __itcm_start__;
    __ROM_ITCM_START         = __tcm_code_rom_start;
    __ROM_ITCM_END           = __tcm_code_rom_end;

    __RAM_DTCM_DATA_START    = __dtcm_data_start__;
    __ROM_DTCM_DATA_START    = __tcm_data_rom;
    __ROM_DTCM_END           = __tcm_data_rom_end;

    __RAM_INIT               = 0;
    __ITCM_INIT              = 0;
    __DTCM_INIT              = 0;

    __EXCHANGE_START         = ORIGIN(int_exchange);
    __EXCHANGE_END           = ORIGIN(int_exchange) + LENGTH(int_exchange);

    Fls_ACEraseRomStart      = __acfls_code_rom_start;
    Fls_ACEraseRomEnd        = __acfls_code_rom_end;
    Fls_ACEraseSize          = (__acfls_code_rom_end - __acfls_code_rom_start) / 8; /* Copy 8 bytes at a time*/

    Fls_ACWriteRomStart      = __acfls_code_rom_start;
    Fls_ACWriteRomEnd        = __acfls_code_rom_end;
    Fls_ACWriteSize          = (__acfls_code_rom_end - __acfls_code_rom_start) / 8; /* Copy 8 bytes at a time*/

    _ERASE_FUNC_ADDRESS_     = __acfls_code_ram_start;
    _WRITE_FUNC_ADDRESS_     = __acfls_code_ram_start;

    Mem_43_INFLS_ACEraseRomStart         = __acmem_43_infls_code_rom_start;
    Mem_43_INFLS_ACEraseRomEnd           = __acmem_43_infls_code_rom_end;
    Mem_43_INFLS_ACEraseSize             = (__acmem_43_infls_code_rom_end - __acmem_43_infls_code_rom_start) / 8; /* Copy 8 bytes at a time*/

    Mem_43_INFLS_ACWriteRomStart         = __acmem_43_infls_code_rom_start;
    Mem_43_INFLS_ACWriteRomEnd           = __acmem_43_infls_code_rom_end;
    Mem_43_INFLS_ACWriteSize             = (__acmem_43_infls_code_rom_end - __acmem_43_infls_code_rom_start) / 8; /* Copy 8 bytes at a time*/

    _MEM_43_INFLS_ERASE_FUNC_ADDRESS_    = __acmem_43_infls_code_ram_start;
    _MEM_43_INFLS_WRITE_FUNC_ADDRESS_    = __acmem_43_infls_code_ram_start;

    __ENTRY_VTABLE           = __INIT_INTERRUPT_START;

    __CORE0_VTOR             = __INIT_INTERRUPT_START;
    __CORE1_VTOR             = __INIT_INTERRUPT_START;
    __CORE2_VTOR             = __INIT_INTERRUPT_START;

    __INDEX_COPY_CORE2       = 3;    /* This symbol is used to initialize data of ITCM/DTCM for CORE2 */

    ASSERT(__tcm_code_rom_end <= ORIGIN(int_pfblflash) + LENGTH(int_pfblflash), "Flash for code overflow")

    ASSERT(__standby_ram_end__ <= __STANDBY_RAM_LIMIT_END, "Memory for standby ram overflow")
}
