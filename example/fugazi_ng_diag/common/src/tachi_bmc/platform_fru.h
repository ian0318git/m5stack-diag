/* $Id: platform_fru.h,v 1.5 2018/06/12 01:41:49 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for Tachi BMC FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2015, Times Huang
 * Copyright (c) 2013 - 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

/* define fru offset for Tachi BMC platform */
typedef enum {
    MB = 0,
    MEM_DIMM0,
    MEM_DIMM1,
/*  Tachi-L do not need these definitions
    MB_SFP0,
    MB_SFP1,
    PSU0,
    PSU1,
    RPS,
    BACKPLANE,
*/
    MB_MEMORY,
    MB_SPI_FLASH,
    MB_BIOS_FLASH,
    MB_NAND,
    MB_FPGA,
    MB_MCU,
    MB_GEPHY,
    MB_NCSI_GEPHY,
    MB_GESWITCH,
    MB_I2C,
    MB_TEMP_SENSOR,
    MB_FAN,
    MB_RTC,
    MB_PECI,
    MB_SGPIO,
    GESW_98DX,
    GESW_98DX_I2C,
    INTEL_I2C,
    INTEL_CPU,
    INTEL_MEM,
    INTEL_HDD,
    INTEL_USB,
    INTEL_SSD,
    INTEL_EMMC,
    INTEL_BMCUSB0,
    INTEL_BMCUSB1,
    INTEL_I350,
    INTEL_X710,
    INTEL_I210,
    INTEL_CORE,
    INTEL_PCIE,
    INTEL_TPM20,
    ISP_TEST,
    ISP_TEST_UART,
    ISP_TEST_SGMII,
    ISP_RAID,
    ISP_CRYPTO,
    ISP_RAID_PCA9557,
    ISP_RAID_5M570,
    ISP_RAID_SGPIO,
    ISP_RAID_VDD,
    ISP_RAID_SBR,
    WIC0, 
    WIC1, 
    WIC2,
    SM0_WIC,
    SM0_WIC0_DC,
} fru_offset_t;

extern uchar gesw_98DX[];
extern uchar gesw_98DX_i2c[];
extern uchar intel_i2c[];
extern uchar intel_cpu[];
extern uchar intel_mem[];
extern uchar intel_hdd[];
extern uchar intel_usb[];
extern uchar intel_ssd[];
extern uchar intel_emmc[];
extern uchar intel_bmcusb0[];
extern uchar intel_bmcusb1[];
extern uchar intel_i350[];
extern uchar intel_x710[];
extern uchar intel_i210[];
extern uchar intel_core[];
extern uchar intel_pcie[];
extern uchar intel_tpm20[];
extern uchar isp_test[];
extern uchar isp_test_uart[];
extern uchar isp_test_sgmii[];
extern uchar isp_raid[];
extern uchar isp_crypto[];
extern uchar nim_10gkr[]; 

extern uchar mb_loc[];
extern uchar dimm0_loc[];
extern uchar dimm1_loc[];
extern uchar mb_memory_loc[];
extern uchar mb_spi_flash_loc[];
extern uchar mb_bios_flash_loc[];
extern uchar mb_nand_loc[];
extern uchar mb_fpga_loc[];
extern uchar mb_mcu_loc[];
extern uchar mb_gephy_loc[];
extern uchar mb_ncsi_gephy_loc[];
extern uchar mb_geswitch_loc[];
extern uchar mb_i2c_loc[];
extern uchar mb_temp_sensor_loc[];
extern uchar mb_fan_loc[];
extern uchar mb_rtc_loc[];
extern uchar mb_peci_loc[];
extern uchar gesw_98DX_loc[];
extern uchar gesw_98DX_i2c_loc[];
extern uchar intel_i2c_loc[];
extern uchar intel_cpu_loc[];
extern uchar intel_mem_loc[];
extern uchar intel_hdd_loc[];
extern uchar intel_usb_loc[];
extern uchar intel_ssd_loc[];
extern uchar intel_emmc_loc[];
extern uchar intel_bmcusb0_loc[];
extern uchar intel_bmcusb1_loc[];
extern uchar intel_i350_loc[];
extern uchar intel_x710_loc[];
extern uchar intel_i210_loc[];
extern uchar intel_core_loc[];
extern uchar intel_pcie_loc[];
extern uchar intel_tpm20_loc[];
extern uchar isp_test_loc[];
extern uchar isp_test_uart_loc[];
extern uchar isp_test_sgmii_loc[];
extern uchar isp_raid_loc[];
extern uchar isp_crypto_loc[];
extern uchar nim_10gkr_loc[]; 

extern uchar isp_raid_pca9557[];
extern uchar isp_raid_5m570[];
extern uchar isp_raid_sgpio[];
extern uchar isp_raid_vdd[];
extern uchar isp_raid_sbr[];
extern uchar isp_raid_pca9557_loc[];
extern uchar isp_raid_5m570_loc[];
extern uchar isp_raid_sgpio_loc[];
extern uchar isp_raid_vdd_loc[];
extern uchar isp_raid_sbr_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

/******** History ******** 
$Log: platform_fru.h,v $
Revision 1.5  2018/06/12 01:41:49  haohsu
Add REVA NIM for TACHI platform

Revision 1.4  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.3.10.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.3  2016/08/09 07:44:47  hondwang
Add RAID SGPIO testing

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.11  2016/04/11 14:18:33  hondwang
Add TPM20 testing function

Revision 1.1.2.10  2016/03/08 08:28:19  benchen2
add raid card enhance error message

Revision 1.1.2.9  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.8  2016/03/07 07:10:06  benchen2
sgpio test

Revision 1.1.2.7  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.6  2016/03/04 06:27:16  jimmyya
Remove some useless definitions in fru_table

Revision 1.1.2.5  2016/03/03 09:46:37  jimmyya
add GESW I2C test

Revision 1.1.2.4  2016/02/26 09:00:23  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.3  2016/02/16 23:41:15  jskow
Add enhanced error messaging to Lewis GESW

Revision 1.1.2.2  2016/02/02 07:25:25  benchen2
add enhanced error message

Revision 1.1.2.1  2015/06/11 02:01:11  tirawan
Add files for Tachi BMC project


$Endlog$
*/

