/* $Id: dev_1114.h,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e111x_marvell/dev_1114.h,v $
 ***********************************************************************
 * File Name: dev_1114.h
 *
 * Description: Contains definitions specific for the Marvell 88E1114 PHY
 *
 * Copyright (c)2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#ifndef _MRV88E1114_PHY_H_
#define _MRV88E1114_PHY_H_

#include "dev_mrvl_ge.h"

typedef struct dev_88e1114_p0_t_ {
    smi_t cp_ctrl;	/* 0 - Copper Control */
    smi_t cp_stat;	/* Copper Status */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t cp_adv;	/* Autonegotiation Copper Advertisement */
    smi_t cp_pt_ab;	/* Autonegotiation Copper Link Partner Ability Base
			 * Page */
    smi_t cp_exp;	/* Autonegotiation Copper Expansion */
    smi_t cp_nx_tx;	/* Autonegotiation Copper Next Page Transmit
			 * Register */
    smi_t cp_pt_nx;	/* 8 - Autonegotiation Copper Link Partner Next Page */
    smi_t ct_1000b;	/* 1000BASE-T Control */
    smi_t st_1000b;	/* 1000BASE-T Status */
    smi_t reserve1[4];	/* Reserved */
    smi_t ext_stat;	/* Extended Status */
    smi_t cp_sp_ct1;	/* 0x10 - Copper Specific Control Register 1 */
    smi_t cp_sp_st1;	/* Copper Specific Status Register 1 */
    smi_t cp_int_en;	/* Copper Interrupt Enable Register */
    smi_t cp_sp_st2;	/* Copper Specific Status Register 2 */
    smi_t reserve2;	/* Reserved */
    smi_t rcv_err_ct;	/* Receive Error Counter */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve3[3];	/* 0x17 - Reserved */
    smi_t cp_sp_ct2;	/* Copper Specific Control Register 2 */
    smi_t reserve4[2];	/* Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p0_t;

typedef struct dev_88e1114_p1_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[11];	/* Reserved */
    smi_t ext_stat;	/* Extended Status */
    smi_t reserve3[5];	/* 0x10 - Reserved */
    smi_t rcv_err_ct;	/* Receive Error Counter */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[6];	/* 0x17 - Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p1_t;

typedef struct dev_88e1114_p2_t_ {
    smi_t mac_ctrl;	/* 0 - MAC Control */
    smi_t reserve1;	/* Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[12];	/* Reserved */
    smi_t mac_sp_ct1;	/* 0x10 - MAC Specific Control Register 1 */
    smi_t mac_sp_st1;	/* MAC Specific Status Register 1 */
    smi_t mac_int_en;	/* MAC Interrupt Enable Register */
    smi_t mac_sp_st2;	/* MAC Specific Status Register 2 */
    smi_t reserve3[2];	/* Reserved */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[3];	/* 0x17 - Reserved */
    smi_t mac_sp_ct2;	/* MAC Specific Control Register 2 */
    smi_t reserve5[2];	/* Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p2_t;

typedef struct dev_88e1114_p3_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[12];	/* Reserved */
    smi_t los_fn_ctl;	/* 0x10 - LOS, INIT, STATUS[1:0] Function Control
			 * Register */
    smi_t los_po_ctl;	/* LOS, INIT, STATUS[1:0] Polarity Control Register */
    smi_t los_tm_ctl;	/* LOS, INIT, STATUS[1:0] Timer Control Register */
    smi_t reserve3[3];	/* Reserved */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[6];	/* 0x17 - Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p3_t;

typedef struct dev_88e1114_p4_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[12];	/* Reserved */
    smi_t nv_addr1;	/* 0x10 - Non-Volatile Memory Address */
    smi_t nv_rd;	/* Non-Volatile Memory Read Data and Status */
    smi_t nv_wr_1;	/* Non-Volatile Memory Write Data and Control */
    smi_t nv_wr_2;	/* Non-Volatile Memory Write Data and Control */
    smi_t nv_addr2;	/* Non-Volatile Memory Address */
    smi_t reserve3;	/* Reserved */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[6];	/* 0x17 - Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p4_t;

typedef struct dev_88e1114_p5_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[12];	/* Reserved */
    smi_t vct_stat0;	/* 0x10 - VCT Status MDI[0] */
    smi_t vct_stat1;	/* VCT Status MDI[1]*/
    smi_t vct_stat2;	/* VCT Status MDI[2]*/
    smi_t vct_stat3;	/* VCT Status MDI[3]*/
    smi_t vct_skew;	/* VCT Skew */
    smi_t vct_sw_pol;	/* VCT Pair Swap and Polarity */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve3[3];  /* 0x17 - Reserved */
    smi_t vct_dsp_d;	/* VCT DSP Distance */
    smi_t reserve4[2];	/* Reserved */
    smi_t fac_tst_m1;   /* Factory Test modes */
    smi_t fac_tst_m2;   /* Factory Test modes */
    smi_t fac_tst_m3;   /* Factory Test modes */
} dev_88e1114_p5_t;

typedef struct dev_88e1114_p6_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[12];	/* Reserved */
    smi_t pkt_gen;	/* 0x10 - Packet Generation/Stub loopback */
    smi_t crc_check;	/* CRC Checker */
    smi_t reserve3[4];	/* Reserved */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[6];	/* 0x17 - Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p6_t;

typedef struct dev_88e1114_p_rsv_t_ {
    smi_t reserve1[2];	/* 0 - Reserved */
    smi_t phy_id1;	/* PHY Identifier 1 */
    smi_t phy_id2;	/* PHY Identifier 2 */
    smi_t reserve2[18];	/* Reserved */
    smi_t pg_addr;	/* Page Address */
    smi_t reserve4[6];	/* 0x17 - Reserved */
    smi_t fac_tst_m1;	/* Factory Test modes */
    smi_t fac_tst_m2;	/* Factory Test modes */
    smi_t fac_tst_m3;	/* Factory Test modes */
} dev_88e1114_p_rsv_t;


#define MRV88E1114_PHY_ID_1_VALUE               0x0141
#define MRV88E1114_PHY_ID_2_VALUE               0x0C90
#define MRV88E1114_PHY_ID                       0x01430C90

typedef struct mrv_88e1114_reg_t_ {
    dev_88e1114_p0_t	pg0;	/* Page 0 */
    dev_88e1114_p1_t	pg1;	/* Page 1 */
    dev_88e1114_p2_t	pg2;	/* Page 2 */
    dev_88e1114_p3_t	pg3;	/* Page 3 */
    dev_88e1114_p4_t	pg4;	/* Page 4 */
    dev_88e1114_p5_t	pg5;	/* Page 5 */
    dev_88e1114_p6_t	pg6;	/* Page 6 */
    dev_88e1114_p_rsv_t	rs[249]; /* Reserved Pages */
} mrv_88e1114_reg_t;

#if 0
/* reg_info_t extension for SMI access */
static reg_info_t_ext reg_ext = {
        sizeof(smi_t), mrvl_ge_smi_rd, mrvl_ge_smi_wr, 0};

/* Common registers for all pages */
static reg_info_t ge_1114_cmn_reg_tbl[] = {
    {"PHY Identifier 1", MRV88E111N_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(uint)&reg_ext}, 0xFFFF, MRV88E1114_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E111N_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(uint)&reg_ext}, 0xFFFF, MRV88E1114_PHY_ID_2_VALUE},
    {"Page Address", MRV88E111N_PAGE_ADDRESS_REG, READ_WRITE | SAVE_RESTORE |
	REG_ACCESS, {(uint)&reg_ext}, 0x80FF, 0x8000},
};

/* Page 0 - Copper */
static reg_info_t ge_1114_p0_reg_tbl[] = {
    {"Control Register", MRV88E111N_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFB40, 0x1140},
    {"Status Register", MRV88E111N_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFF7F, 0x0149},
    {"Auto-Negotiation Advertisement Register", MRV88E111N_AUTONEG_ADVR_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xAFFF, 0x0001},
    {"Link Partner Ability Register - Base Page", MRV88E111N_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111N_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x001F, 0x0004},
    {"Next Page Transmit Register", MRV88E111N_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111N_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
    {"1000BASE-T Control Register", MRV88E111N_1000B_CNTL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFF00, 0x0F00},
    {"1000BASE-T Status Register", MRV88E111N_1000B_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFCFF, 0x0000},
    {"Extended Status Register", MRV88E111N_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x3000, 0x0000},
    {"Specific Control Register 1", MRV88E111N_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFEB, 0x6060},
    {"Specific Status Register 1", MRV88E111N_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFF7F, 0x0010},
    {"Interrupt Enable Register", MRV88E111N_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFF77, 0x0000},
    {"Specific Status Register 2", MRV88E111N_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFF77, 0x0000},
    {"Receive Error Counter Register", MRV88E111N_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111N_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x81FE, 0x0040},
};

/* Page 1 - Not Used */
static reg_info_t ge_1114_p1_reg_tbl[] = {
    {"Receive Error Counter Register", MRV88E111F_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
};

/* Page 2 - MAC */
static reg_info_t ge_1114_p2_reg_tbl[] = {
    {"Control Register", MRV88E111M_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xF840, 0x1000},
    {"Specific Control Register 1", MRV88E111M_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xF38C, 0x0288},
    {"Specific Status Register 1", MRV88E111M_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x0460, 0x0000},
    {"Interrupt Enable Register", MRV88E111M_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x0080, 0x0000},
    {"Specific Status Register 2", MRV88E111M_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x0080, 0x0000},
    {"Specific Control Register 2", MRV88E111M_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xE017, 0x0000},
};

/* Page 3 - LOS, INIT, STATUS[1:0] */
static reg_info_t ge_1114_p3_reg_tbl[] = {
    {"Function Control Register", MRV88E111L_FUNC_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x021E},
    {"Polarity Control Register 1", MRV88E111L_POL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x44A0},
    {"Timer Control Register 2", MRV88E111L_TMR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x770F, 0x4105},
};

/* Page 4 - Non-Volatile Memory */
static reg_info_t ge_1114_p4_reg_tbl[] = {
    {"I2C Address Register", MRV88E111NV_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0100},
    {"Read Data and Status Register", MRV88E111NV_READ_DATA_STATUS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xE7FF, 0x0000},
    {"Write Data and Control Register", MRV88E111NV_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFEFF, 0xA000},
    {"RAM Write Data and Control Register", MRV88E11NV_RAM_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x00FF, 0x0000},
    {"RAM Address Register", MRV88E111NV_RAM_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x00FF, 0x0000},
};

/* Page 5 - Virtual Cable Tester */
static reg_info_t ge_1114_p5_reg_tbl[] = {
    {"MDO[0] VCT Status Register", MRV88E111N_VCT_STATUS_MDI0_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x8000, 0x0000},
    {"MDI[1] VCT Status Register", MRV88E111N_VCT_STATUS_MDI1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x8000, 0x0000},
    {"MDI[2] VCT Status Register", MRV88E111N_VCT_STATUS_MDI2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x7FFF, 0x0000},
    {"MDI[3] VCT Status Register", MRV88E111N_VCT_STATUS_MDI3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x7FFF, 0x0000},
    {"1000 BASE-T Pair Skew Register", MRV88E111N_VCT_PAIR_SKEW_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
    {"1000 BASE-T Pair Swap and Polarity", MRV88E111N_VCT_PAIR_SWP_POL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x007F, 0x0000},
    {"VCT DSP Distance", MRV88E111N_VCT_DSP_DISTANCE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x0007, 0x0000},
};

/* Page 6 - Miscellaneous */
static reg_info_t ge_1114_p6_reg_tbl[] = {
    {"Packet Generation", MRV88E111N_PACKET_GEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0x003F, 0x0000},
    {"CRC Counters", MRV88E111N_CRC_CHKR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)&reg_ext},
	0xFFFF, 0x0000},
}
#endif /* 0 */

/* Prototypes */
void mrv88e1114_phy_cleanup(void);
int  mrv88e1114_setup_loopback(int speed);
int  mrv88e1114_setup_ext_loopback(void);
int  mrv88e1114_setup_loopback_param(int loopback, int speed);
int  mrv88e1114_read_phy_id(void);
int  mrv88e1114_setup_phy_loopback(int speed);
int  mrv88e1114_check_phy_link(void);
int  show_mrv88e1114_reg(int menu_option);
int  alter_mrv88e1114_reg(int menu_option);
int  dump_mrv88e1114_registers(int menu_option);
int  mrv88e1114_register_test_wrapper(int menu_option);

#endif /* _MRV88E1114_PHY_H_ */

/*
 *------------------------------------------------------------------
 * $Log: dev_1114.h,v $
 * Revision 1.2  2013/10/08 08:48:25  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:58:01  tirawan
 * First Woodlawn linux integration
 *
 * Revision 1.1  2013/03/13 06:42:07  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/08/28 08:22:45  leslie
 * *** empty log message ***
 *
 * Revision 1.3  2012/08/03 10:16:50  leslie
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.1.1  2009/10/17 02:05:27  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.1.32.1  2009/07/06 17:32:03  mcharon
 * change sleep to msleep....getline to get_line
 *
 * Revision 1.1  2007/12/26 22:16:55  siyen
 * Initial check-in.
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */

