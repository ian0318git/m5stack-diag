/* $Id: plug_host_fpga_lib.h,v 1.2 2019/01/10 06:36:29 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/plug_host_fpga_lib.h,v $
 *------------------------------------------------------------------
 * 
 * plug_host_fpga_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "diag_sirius_fpga_lib.h"
#include "diag_i2c_lib.h"
#include "ethernet.h"
#include "platform_stub.h"

typedef enum {
    PLUG_SLOT_1 = 1,
    PLUG_SLOT_2
} plug_slot_no;

typedef enum {
    PLUG_SLOT_PWR_OFF,
    PLUG_SLOT_PWR_ON
} plug_power_ctrl;

#define ACT2_RESET_UNRESET_DELAY    (500)
#define ACT2_UNRESET_DELAY          (5000)
#define FIRST_SLOT                      (1)
#define MAX_PLUG_SLOT_C1101             (1)
#define MAX_PLUG_SLOT_NUMBER            (2)
#define MAX_PLUG_USB_MASS_STORE_ON_SYS  (8)

#define FPGA_EXTER_DEV_RST_REG       0x1004

/* FPGA External Device Reset Reg(0x1004) */
#define FPGA_USB_HUB_RESET_BIT      (27)
#define FPGA_USB_HUB_RESET          (1 << 27)
#define FPGA_EMMC_RESET             (1 << 23)
#define FPGA_GEWAN1_RESET           (1 << 22)
#define FPGA_GEWAN0_RESET           (1 << 21)
#define EXT_DSL_CHIP_RESET          (1 << 20)
#define EXT_ROMMON_FLASH_RESET      (1 << 19)
#define EXT_PRI_POE_DC_RESET        (1 << 11)
#define EXT_PRI_LTE_RESET           (1 << 5)
#define EXT_WLAN_RESET              (1 << 4)
#define EXT_CPU_SYS_RESET           (1 << 3)
#define EXT_ESW_RESET               (1 << 1)

/* Star GPS define diff with Curie */
#define PLUG_TESTCARD_GPS_SLOT1              (0x100)
#define PLUG_TESTCARD_GPS_SLOT2              (0x200)

#define PLUG_LTE_GPS_SYNC_STATUS_0        8
#define PLUG_LTE_GPS_SYNC_STATUS_1        9

#define BIT_VAL(a,b)                    ((a & BIT(b)) >> b)
#define BIT(x)                          (1 << (x))
#define PLUG_UART_CONTROL_OFFSET_BY_SLOT(offset, slot) (offset + ((slot - 1) * 0x100))

extern void plug_i2c_act2_reset(sc_context *);
extern int diag_plug_pwr_on_util(void); 
extern int diag_plug_pwr_off_util(void); 

/*-------------------------------------------------
 * $Log: plug_host_fpga_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
