# WT_FBL_324
WT's sample bootloader based on S32K324
Requirements:
 - S32DS 3.6.0
 - SW32K3_S32M27x_RTD_R21-11_5.0.0_D2410_DesignStudio_updatesite.zip
 - S32K3xx development package	3.6.0.202411272154

PFLASH 0x0040_0000 + 0x0001_0000 Bootloader
PFLASH 0x0041_0000 + 0x003C_4000 Application
DFLASH 0x1000_0000 + 0x0000_2000 IVTCore
