/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           :
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 5.0.0
*   Build Version        : S32K3_RTD_5_0_0_D2408_ASR_REL_4_7_REV_0000_20241002
*
*   Copyright 2020 - 2024 NXP
*
*   NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms.  By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms.  If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/
/*
* Target device: This linker is demo and it is using for device S32K314 only. It is not part of the production code deliverables.
* Target core: ARM cortex M7
* Linker need to align with MPU default setup in system.c
*/

/*
* GCC Linker Command File:
* 0x00000000    0x00007FFF  32768   ITCM
* 0x20000000    0x2000FFFF  65536   DTCM
* 0x00400000    0x007FFFFF  4194304 Program Flash
* 0x10000000    0x1001FFFF  131072  Data Flash
* 0x20400000    0x20427FFF  163840  SRAM_0
* 0x20428000    0x20443FFF  114688  SRAM_1  Actually only initialize 112 kB instead of 160 kB in RM, 48kB is blocked by fimrware
* Last 176 KB of CODE_FLASH reserved by HSE Firmware
* standby_data section should contain only uninitialized data
*/

ENTRY(Reset_Handler)

MEMORY
{
    int_ivtcore             : ORIGIN = 0x10000000, LENGTH = 0x00010000    /* 64KB */
    int_itcm                : ORIGIN = 0x00000000, LENGTH = 0x00007F80    /* 32KB */
    int_basecode            : ORIGIN = 0x00007F80, LENGTH = 0x00000080    /* 128 */
    int_dtcm                : ORIGIN = 0x20000000, LENGTH = 0x0000FF00    /* 60KB */
    int_stack_dtcm          : ORIGIN = 0x2000FF00, LENGTH = 0x00000100    /* 4KB */
    int_sram_edata          : ORIGIN = 0x20443FF0, LENGTH = 0x00000010    /* Exchange data section */
}

SECTIONS
{
    .ivtcore :
    {
        KEEP(*(.boot_header))
        . = ALIGN(128);
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
        *(.mcal_text.*)
        . = ALIGN(4);
        *(.acmcu_code_rom)
        *(.acmcu_code_rom.*)
        . = ALIGN(4);
        __acfls_code_rom_start = .;
        *(.acfls_code_rom)
        *(.acfls_code_rom.*)
        . = ALIGN(4);
        __acfls_code_rom_end = .;
        __acmem_43_infls_code_rom_start = .;
        *(.acmem_43_infls_code_rom)
        *(.acmem_43_infls_code_rom.*)
        . = ALIGN(4);
        __acmem_43_infls_code_rom_end = .;
        KEEP(*(.init))
        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        *(.mcal_const_cfg)
        *(.mcal_const_cfg.*)
        . = ALIGN(4);
        *(.mcal_const)
        *(.mcal_const.*)
    } > int_ivtcore

    .copy.table :
    {
      . = ALIGN(4);
      __copy_table_start__ = .;

      LONG ((__copy_table_end__ - __copy_table_start__) / 12)

      LONG (__sram_data_rom)
      LONG (__sram_data_begin__)
      LONG ((__sram_data_end__ - __sram_data_begin__) / 8)

      LONG (__tcm_data_rom)
      LONG (__dtcm_data_start__)
      LONG ((__dtcm_data_end__ - __dtcm_data_start__) / 8)

      __copy_table_end__ = .;
    } > int_ivtcore

    PROVIDE(__copy_table = __copy_table_start__);

    .zero.table :
    {
      . = ALIGN(4);
      __zero_table_start__ = .;
      /* Add each additional bss section here */

      LONG ((__zero_table_end__ - __zero_table_start__) / 8)

      LONG (__Stack_dtcm_end)
      LONG (((__Stack_dtcm_start - __Stack_dtcm_end) / 8) - 2)

      LONG (__sram_bss_start)
      LONG ((__sram_bss_end - __sram_bss_start) / 8)

      LONG (__dtcm_bss_start__)
      LONG ((__dtcm_bss_end__ - __dtcm_bss_start__) / 8)

      __zero_table_end__ = .;
    } > int_ivtcore

    PROVIDE(__zero_table = __zero_table_start__);

    . = ALIGN(4);
    __text_end = .;
    __sram_data_rom = __text_end;

    .sram_bss (NOLOAD) :
    {
        . = ALIGN(8);
        __standby_ram_begin__ = .;
        *(.standby_data)
        *(.standby_data.*)
        . = ALIGN(8);
        __standby_ram_end__ = .;
        . = ALIGN(16);
        __sram_bss_start = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(16);
        *(.mcal_bss)
        *(.mcal_bss.*)
        *(.mcal_bss_no_cacheable)
        *(.mcal_bss_no_cacheable.*)
        . = ALIGN(8);
        __sram_bss_end = .;
    } > int_dtcm

    .sram_data : AT(__sram_data_rom)
    {
        . = ALIGN(8);
        __sram_data_begin__ = .;
        . = ALIGN(4);
        *(.ramcode)
        *(.ramcode.*)
        . = ALIGN(4);
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        *(.mcal_data)
        *(.mcal_data.*)
        . = ALIGN(8);
        __sram_data_end__ = .;
    } > int_dtcm

    __sram_data_rom_end = __sram_data_rom + (__sram_data_end__ - __sram_data_begin__);

    __tcm_data_rom = __sram_data_rom_end;

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

    __base_code_rom_start = __tcm_code_rom_end;

    .base_code : AT(__base_code_rom_start)
    {
        . = ALIGN(8);
        __base_code_start__ = .;
        KEEP(*(.basecodeentry*))
        KEEP(*(.basecode*))
        . = ALIGN(8);
        __base_code_end__ = .;
    } > int_basecode

    __Stack_dtcm_end        = ORIGIN(int_stack_dtcm);
    __Stack_dtcm_start      = ORIGIN(int_stack_dtcm) + LENGTH(int_stack_dtcm);

    CM7_START_ADDRESS        = __interrupts_init_start;

    PROVIDE(__edata = ORIGIN(int_sram_edata));
    PROVIDE(__edatasize = LENGTH(int_sram_edata) / 8);

}
