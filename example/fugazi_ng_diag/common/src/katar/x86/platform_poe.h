/* $Id: platform_poe.h,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_poe.h,v $
 *------------------------------------------------------------------
 *
 * katar_poe.h - This file contains functions for katar POE
 *                    controller.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _WLCNG_POE_H_
#define _WLCNG_POE_H_

//katar remove poe module after P2
//#define ENABLE_POE_MODULE

/* Common definition */
typedef enum {
    POE_PWR_OFF = 0,
    POE_PWR_ON
} POE_PWR_OPT;

typedef enum {
    POE_PORT_1 = 1,
    POE_PORT_2,
    POE_PORT_3,
    POE_PORT_4
} POE_PORT_NUM;

typedef enum {
    POE_UNKNOWN = 0,
    POE_SHORT,
    POE_CPD_TOO_HIGH,
    POE_RSIG_TOO_LOW,
    POE_GOOD,
    POE_RSIG_TOO_HIGH,
    POE_OPEN_CIRCUIT,
    POE_RESERVED
} POE_DETECTION;

typedef enum {
    POE_C_UNKNOWN = 0,
    POE_C_CLASS_1,
    POE_C_CLASS_2,
    POE_C_CLASS_3,
    POE_C_CLASS_4,
    POE_C_RESERVED,
    POE_C_CLASS_0,
    POE_C_OVERCURRENT,
} POE_CLASS;

/* PoE controller register definition */
typedef struct poe_reg_info {
    char    *name;
    int     addr;
    int     type;
    uint8_t wr_mask;
    uint8_t reset_val;
} poe_reg_info_t;

#define RO_REG 0
#define RW_REG 1
#define COR_REG 2
#define RSRV_REG 3
#define SO_REG 4

/* PoE controller Registers map definition */
#define POE_INT_REG 0x00
#define POE_INTMASK_REG 0x01
#define POE_PWREVN_REG 0x02
#define POE_PWREVN_COR_REG 0x03
#define POE_DETEVN_REG 0x04
#define POE_DETEVN_COR_REG 0x05
#define POE_FLTEVN_REG 0x06
#define POE_FLTEVN_COR_REG 0x07
#define POE_TSEVN_REG 0x08
#define POE_TSEVN_COR_REG 0x09
#define POE_SUPEVN_REG 0x0A
#define POE_SUPEVN_COR_REG 0x0B
#define POE_STATP1_REG 0x0C
#define POE_STATP2_REG 0x0D
#define POE_STATP3_REG 0x0E
#define POE_STATP4_REG 0x0F
#define POE_STATPWR_REG 0x10
#define POE_STATPIN_REG 0x11
#define POE_OPMD_REG 0x12
#define POE_DISENA_REG 0x13
#define POE_DETENA_REG 0x14
#define POE_MIDSPAN_REG 0x15
#define POE_TCONF_REG 0x16
#define POE_MCONF_REG 0x17
#define POE_DETPB_REG 0x18
#define POE_PWRPB_REG 0x19
#define POE_RSTPB_REG 0x1A
#define POE_ID_REG 0x1B
#define POE_TLIM12_REG 0x1E
#define POE_TLIM34_REG 0x1F
#define POE_IP1LSB_REG 0x30
#define POE_IP1MSB_REG 0x31
#define POE_VP1LSB_REG 0x32
#define POE_VP1MSB_REG 0x33
#define POE_IP2LSB_REG 0x34
#define POE_IP2MSB_REG 0x35
#define POE_VP2LSB_REG 0x36
#define POE_VP2MSB_REG 0x37
#define POE_IP3LSB_REG 0x38
#define POE_IP3MSB_REG 0x39
#define POE_VP3LSB_REG 0x3A
#define POE_VP3MSB_REG 0x3B
#define POE_IP4LSB_REG 0x3C
#define POE_IP4MSB_REG 0x3D
#define POE_VP4LSB_REG 0x3E
#define POE_VP4MSB_REG 0x3F
#define POE_FIRMWARE_REG 0x41
#define POE_WDOG_REG 0x42
#define POE_DEVID_REG 0x43
#define POE_HPEN_REG 0x44
#define POE_HPMD1_REG 0x46
#define POE_CUT1_REG 0x47
#define POE_LIM1_REG 0x48
#define POE_HPSTAT1_REG 0x49
#define POE_HPMD2_REG 0x4B
#define POE_CUT2_REG 0x4C
#define POE_LIM2_REG 0x4D
#define POE_HPSTAT2_REG 0x4E
#define POE_HPMD3_REG 0x50
#define POE_CUT3_REG 0x51
#define POE_LIM3_REG 0x52
#define POE_HPSTAT3_REG 0x53
#define POE_HPMD4_REG 0x55
#define POE_CUT4_REG 0x56
#define POE_LIM4_REG 0x57
#define POE_HPSTAT4_REG 0x58

/*  */
#define POE_VTEMP_REG 0x70
#define POE_VMAIN_LSB_REG 0x71
#define POE_VMAIN_MSB_REG 0x72
#define POE_PORT_SR12_REG 0x75
#define POE_PORT_SR34_REG 0x76
#define POE_INVD_CNT_REG 0x77
#define POE_PWRD_CNT_REG 0x78
#define POE_OVL_CNT_REG 0x79
#define POE_UDL_CNT_REG 0x7A
#define POE_SC_CNT_REG 0x7B
#define POE_CLS_CNT_REG 0x7C
#define POE_INTR_EN_REG 0x7D
#define POE_SYS_CFG_REG 0x7E
#define POE_SW_CFG_REG 0x7F
#define POE_PRIO_CR_REG 0x80
#define POE_PWR_CR1_REG 0x81
#define POE_PWR_CR2_REG 0x82
#define POE_PWR_CR3_REG 0x83
#define POE_PWR_CR4_REG 0x84
#define POE_TMP_PWR_CR1_REG 0x85
#define POE_TMP_PWR_CR2_REG 0x86
#define POE_TMP_PWR_CR3_REG 0x87
#define POE_TMP_PWR_CR4_REG 0x88
#define POE_BNK0_REG 0x89
#define POE_BNK1_REG 0x8A
#define POE_BNK2_REG 0x8B
#define POE_BNK3_REG 0x8C
#define POE_BNK4_REG 0x8D
#define POE_BNK5_REG 0x8E
#define POE_BNK6_REG 0x8F
#define POE_BNK7_REG 0x90
#define POE_PWRGD_REG 0x91
#define POE_PORT1_CONS_REG 0x92
#define POE_PORT2_CONS_REG 0x93
#define POE_PORT3_CONS_REG 0x94
#define POE_PORT4_CONS_REG 0x95
#define POE_TOTAL_PWR_CONS_REG 0x96
#define POE_TOTAL_PWR_CALC_REG 0x97
#define POE_CHIP_PWR_REQ_REG 0x98
#define POE_ICUT_AT_MAX_LSB_REG 0x99
#define POE_ICUT_AT_MAX_MSB_REG 0x9A
#define POE_POE_MAX_LED_GB_REG 0x9F
#define POE_VMAIN_LOW_TH_LSB_REG 0xCB
#define POE_VMAIN_LOW_TH_MSB_REG 0xCC

/* DETEVN(0x04) */
#define POE_DETEVN_P1_DET   0x01
#define POE_DETEVN_P2_DET   0x02
#define POE_DETEVN_P3_DET   0x04
#define POE_DETEVN_P4_DET   0x08
#define POE_DETEVN_P1_CLA   0x10
#define POE_DETEVN_P2_CLA   0x20
#define POE_DETEVN_P3_CLA   0x40
#define POE_DETEVN_P4_CLA   0x80

/* STAT(P1: 0x0C, P2:0x0D, P3:0x0E, P4:0x0F */
#define POE_STAT_DETECT     0x07
#define POE_STAT_CLASS      0x70
#define POE_STAT_CLASS_OFF     4

/* OPMD(0x12) */
#define POE_OPMD_P1_AUTO   0x03
#define POE_OPMD_P1_SEMI   0x02
#define POE_OPMD_P1_MANU   0x01
#define POE_OPMD_P2_AUTO   0x0C
#define POE_OPMD_P2_SEMI   0x08
#define POE_OPMD_P2_MANU   0x04
#define POE_OPMD_P3_AUTO   0x30
#define POE_OPMD_P3_SEMI   0x20
#define POE_OPMD_P3_MANU   0x10
#define POE_OPMD_P4_AUTO   0xC0
#define POE_OPMD_P4_SEMI   0x80
#define POE_OPMD_P4_MANU   0x40

/* DETENA(0x14) */
#define POE_DETENA_P1_DET   0x01
#define POE_DETENA_P2_DET   0x02
#define POE_DETENA_P3_DET   0x04
#define POE_DETENA_P4_DET   0x08
#define POE_DETENA_P1_CLA   0x10
#define POE_DETENA_P2_CLA   0x20
#define POE_DETENA_P3_CLA   0x40
#define POE_DETENA_P4_CLA   0x80

/* DEVID(0x43) */
#define POE_DEVID              0x44
#define POE_DEVID_REV_MSK      0x07
#define POE_DEVID_MSCCID_MSK   0xE0
#define POE_DEVID_MSCCID_OFF   5

/* INTR_EN(0x7D) */
#define POE_INTREN_INT_OUTPUT  0x01

/* SYS_CFG(0x7E) */
#define POE_SYSCFG_R_DET   0x01

/* Externs */
extern int katar_poe_test(char*);
extern int poe_utils(void);
extern int katar_poe_reg_rd(int, uint8_t *);
extern int katar_poe_reg_wr(int, uint8_t);
extern int katar_poe_reset_check(char *);
extern int katar_poe_pwrctrl(int, int);
extern int katar_poe_config(void);
extern int katar_gen_poe_intr(int);
extern int katar_poe_set_force_intr(void);
extern int katar_poe_clear_force_intr(void);
extern int poe_status_util (int bBrief);

#endif  /* _WLCNG_POE_H_ */

/*
 *------------------------------------------------------------------
 * $Log: platform_poe.h,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2019/02/12 08:06:30  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/01/29 08:02:05  mikech2
 * remove POE test for katar P2 build
 *
 * Revision 1.1.2.1  2018/10/22 08:02:31  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/07/17 11:39:00  benlu
 * For poe diag test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

