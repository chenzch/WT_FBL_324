/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "platform.h"

#define SBAF_BOOT_MARKER            (0x5AA55AA5)
#define CM7_0_ENABLE_SHIFT          (0)
#define CM7_1_ENABLE_SHIFT          (1)
#define CM7_2_ENABLE_SHIFT          (2)

extern const uint32_t __CORE0_VTOR[];
extern const uint32_t __CORE1_VTOR[];
extern const uint32_t __CORE2_VTOR[];

#define CM7_0_VTOR_ADDR (__CORE0_VTOR)
#define CM7_1_VTOR_ADDR (__CORE1_VTOR)
#define CM7_2_VTOR_ADDR (__CORE2_VTOR)

/******************************************************************************
 * Boot header
 ******************************************************************************/
typedef const struct
{
  const uint32_t  Header;                   /* Header of boot header structure */
  const uint32_t  BootConfig;               /* Boot Configuration Word */
  const uint32_t  Reserved3;                /* Reserved */
  const uint32_t* CM7_0_StartAddress;       /* Start address of application on CM7_0 core */
  const uint32_t  Reserved4;                /* Reserved */
  const uint32_t* CM7_1_StartAddress;       /* Start address of application on CM7_1 core */
  const uint32_t  Reserved5;                /* Reserved */
  const uint32_t* CM7_2_StartAddress;       /* Start address of application on CM7_2 core */
  const uint32_t* XRDCConfig_StartAddress;  /* Address of XRDC configuration data */
  const uint32_t* LCConfig;                 /* Address of LC configuration */
  const uint32_t  Reserved1;                /* Reserved */
  const uint32_t* HseFwHeader_StartAddress; /* Start address of HSE-FW image */
  const uint8_t   Reserved[192];            /* Reserved for future use */
  const uint8_t   CMAC[16];                 /* CMAC */
} boot_header_t;

/******************************************************************************
 * XRDC Configuration
 ******************************************************************************/
typedef struct
{
  uint32_t  Header;
  uint32_t  MDAConfig_ProcessorCore0;
  uint32_t  MDAConfig_eDMA_AHB;
  uint32_t  MDAConfig_TestPort_AHB;
  uint32_t  MDAConfig_ProcessorCore1;
  uint32_t  MDAConfig_ENET_AHB;
  uint32_t  Reserved0[10];
  uint32_t  PDAC[14][2];
  uint32_t  Reserved2[20];
  uint8_t   CMAC[16];
} xrdc_config_t;

const boot_header_t boot_header;

/******************************************************************************
 * LC Configuration
 ******************************************************************************/
typedef uint32_t lc_config_t;

/******************************************************************************
 * LC Configuration
 ******************************************************************************/
static const lc_config_t lc_config;

/******************************************************************************
 * XRDC Configuration
 ******************************************************************************/
static const xrdc_config_t xrdc_config;

const boot_header_t
#if defined (__ghs__)
#pragma ghs section rodata =".boot_header"
#elif defined (__GNUC__) || defined (__DCC__)
__attribute__((section (".boot_header")))
#elif defined (__ICCARM__)
#pragma location = ".boot_header"
#endif
boot_header = {
  .Header = SBAF_BOOT_MARKER,
  .BootConfig = (CM7_0_ENABLE << CM7_0_ENABLE_SHIFT) | (CM7_1_ENABLE << CM7_1_ENABLE_SHIFT) | (CM7_2_ENABLE << CM7_2_ENABLE_SHIFT), /* Boot configuration word */
  .CM7_0_StartAddress = (const uint32_t*)CM7_0_VTOR_ADDR,
#if 0 != CM7_1_ENABLE
  .CM7_1_StartAddress = (const uint32_t*)CM7_1_VTOR_ADDR,
#endif
#if 0 != CM7_2_ENABLE
  .CM7_2_StartAddress = (const uint32_t*)CM7_2_VTOR_ADDR,
#endif
  .XRDCConfig_StartAddress  = (const uint32_t*)&xrdc_config,
  .LCConfig                 = (const uint32_t*)&lc_config,
  .HseFwHeader_StartAddress = (const uint32_t*)0U,
};

/******************************************************************************
 * LC Configuration
 ******************************************************************************/
static const lc_config_t
#if defined (__ghs__)
#pragma ghs section rodata =".boot_header"
#elif defined (__GNUC__) || defined (__DCC__)
__attribute__((section (".boot_header")))
#elif defined (__ICCARM__)
#pragma location = ".boot_header"
#endif
lc_config = 0xffffffff;

/******************************************************************************
 * XRDC Configuration
 ******************************************************************************/
static const xrdc_config_t
#if defined (__ghs__)
#pragma ghs section rodata =".boot_header"
#elif defined (__GNUC__) || defined (__DCC__)
__attribute__((section (".boot_header")))
#elif defined (__ICCARM__)
#pragma location = ".boot_header"
#endif
xrdc_config = {
  .Header = 0xffffffff
};

#if defined (__ghs__)
#pragma ghs section rodata =default
#endif
