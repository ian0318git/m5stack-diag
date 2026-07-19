/* $Id: platform_cpu.h,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_cpu.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of Highrise CPU Library.
 *
 * Copyright (c) 2019 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

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

#define CPU_1200_DDR_800_RCLK_800  0x19
#define CPU_1400_DDR_800_RCLK_800  0x1a
#define CPU_600_DDR_800_RCLK_800   0x1b
#define CPU_800_DDR_800_RCLK_800   0x1c
#define CPU_1000_DDR_800_RCLK_800  0x1d

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

#endif
/*********************************************************************
 * $Log: platform_cpu.h,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.2  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 * Revision 1.1.4.1  2020/11/25 02:38:06  alpeng
 * update cvs id field
 *
 *
 *********************************************************************
 */

