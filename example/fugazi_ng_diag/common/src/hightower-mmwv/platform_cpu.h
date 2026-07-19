/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of Highrise CPU Library.
 *
 * Copyright (c) 2019 - by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define HIGHRISE_INFO_BUF_SIZE          256
#define HIGHRISE_CPU_INFO_FILE           "/highrise_cpuinfo.txt"

#define CPU_STRESS_LOG   "/tmp/cpu_core_log"
#define CPU_STRESS_RLT   "/tmp/cpu_core_rlt"
#define HIGHRISE_CPU_NUM  4

#define CPU_AP_REG_BASE         0xF0000000
#define CPU_ONDIE_TEMP_REG      0x6F808C

#define CPU_AP_ONDIE_TEMP_REG_CTL0  0xF06F8084
#define CPU_AP_ONDIE_TEMP_REG_CTL1  0xF06F8088
#define CPU_AP_ONDIE_TEMP_REG_STAT  0xF06F808C
#define CPU_CP_ONDIE_TEMP_REG_CTL0  0xF2400070
#define CPU_CP_ONDIE_TEMP_REG_CTL1  0xF2400074
#define CPU_CP_ONDIE_TEMP_REG_STAT  0xF2400078
#define CPU_ONDIE_TEMP_TSN_OSR_OFF       24
#define CPU_ONDIE_TEMP_TSN_OSR_WIDTH     2
#define CPU_ONDIE_TEMP_TSN_ENB_OFF       2
#define CPU_ONDIE_TEMP_TSN_ENB_WIDTH     1
#define CPU_ONDIE_TEMP_TSN_RST_OFF       1
#define CPU_ONDIE_TEMP_TSN_RST_WIDTH     1
#define CPU_ONDIE_TEMP_TSN_START_OFF     0
#define CPU_ONDIE_TEMP_TSN_START_WIDTH   1

#define CPU_ONDIE_TEMP_TSN_INTR_EN_OFF   25
#define CPU_ONDIE_TEMP_TSN_INTR_EN_WIDTH 1
#define CPU_ONDIE_TEMP_TSN_SEN_SEL_OFF   21
#define CPU_ONDIE_TEMP_TSN_SEN_SEL_WIDTH 3
#define CPU_ONDIE_TEMP_TSN_THRESH_OFF    3
#define CPU_ONDIE_TEMP_TSN_THRESH_WIDTH  16

#define CPU_ONDIE_TEMP_TSN_VALID_OFF     16
#define CPU_ONDIE_TEMP_TSN_VALID_WIDTH   1
#define CPU_ONDIE_TEMP_TSN_READOUT_OFF   0
#define CPU_ONDIE_TEMP_TSN_READOUT_WIDTH 10


#define CPU_SAR_REG             0x6F4400
#define CPU_SAR_RST2_FREQ_MASK  0x1F

#define CPU_2000_DDR_1200_RCLK_1200  0x0
#define CPU_2200_DDR_1200_RCLK_1200  0x2
#define CPU_1600_DDR_1200_RCLK_1200  0x4

#define CPU_THERM_TEMP_OFFSET   0
#define CPU_THERM_TEMP_MASK     (0x3FF << CPU_THERM_TEMP_OFFSET)

#define CPU_THERM_OUTPUT_MSB    512
#define CPU_THERM_OUTPUT_COMP   1024

#define CPU_THERM_GAIN          425
#define CPU_THERM_OFFSET        153400
#define CPU_THERM_DIV           1000



/*
 * Define ECC relative commands leveraged from TSN
 * pls refer to 88F7040 CPU Datasheet memory architecture
 */
#define ECC_ERR_LOG_CONFIG            "devmem 0xf0020360 32 0x1"
#define ECC_1BIT_ERR_COUNTER          "devmem 0xf0020364 32 0x0"
#define ECC_ERR_INFO_0           	  "devmem 0xf0020368 32 0x0"
#define ECC_ERR_INFO_1           	  "devmem 0xf002036c 32 0x0"
#define INTERRUPT_STATUS_REG          "devmem 0xf0020140 32 0x1000"
#define INTERRUPT_ENABLE_REG          "devmem 0xf0020144 32 0x1000"
#define PHY_REG_FILE_ACCESS_0         "devmem 0xf00116a0 32 0xc0030003"
#define PHY_REG_FILE_ACCESS_1         "devmem 0xf00116a0 32 0xd010001f"

/******** History ********
$Log: platform_cpu.h,v $
Revision 1.2  2021/06/02 02:56:21  alpeng
merge sears into trunk


$Endlog$
*/

