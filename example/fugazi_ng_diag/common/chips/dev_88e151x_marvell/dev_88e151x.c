/* $Id: dev_88e151x.c,v 1.3 2019/10/17 02:16:14 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e151x_marvell/dev_88e151x.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_88e151x.c
 *
 * Description:	Marvell 88E151x Device Driver
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "defs.h"
#include "common.h"
#include "dev_88e151x.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

static uint32 dev_88e151x_attach(dev_object_t *);
static uint32 dev_88e151x_detach(dev_object_t *);
static uint32 dev_88e151x_restart(dev_object_t *);
static int dev_88e151x_show_reg(dev_object_t *);
static int dev_88e151x_display_reg(dev_object_t *);
static void dev_88e151x_destroy(dev_object_t **);
static int dev_88e151x_alter_reg(dev_object_t *);
static int dev_88e151x_test_reg(dev_object_t *dev);
static int dev_88e151x_set_spd(dev_object_t *, int);
static int dev_88e151x_set_lpbk(dev_object_t *, int, int);
static int dev_88e151x_set_testmode(dev_object_t *);
static int dev_88e151x_set_eee(dev_object_t *);
static int dev_88e151x_enable_force_interrupt(dev_object_t *);
static int dev_88e151x_enable_stub(dev_object_t *, int);
static int dev_88e151x_disable_interrupt(dev_object_t *);
static int dev_88e151x_control_led2_intr(dev_object_t *, int);
static int dev_88e151x_smi_rd(ulong, int, ulong *, void *);
static int dev_88e151x_smi_wr(ulong, int, ulong, void *);
static int phy_smi_wr_with_page(dev_object_t *, ulong , ulong , ulong);
static int dev_88e151x_led_on(dev_object_t *);
static int dev_88e151x_led_single_on(dev_object_t *);
static int dev_88e151x_led_off(dev_object_t *);
static int dev_88e151x_init(dev_object_t *);
static int dev_88e151x_set_sgmii_amp(dev_object_t *, int );

void mrv_88e151x_dev_create(dev_object_t *, dev_error_report_t);

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
static char mrv_88e151x_err_buf[MRV88E151X_ERR_BUF_SIZE];

static reg_info_t_ext reg_ext = {
	sizeof(ushort), dev_88e151x_smi_rd, dev_88e151x_smi_wr, 0};

/* Registers test table */
/* Page 0 - Copper */
static reg_info_t ge_151x_p0_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) and  Bit 9 (Restart Auto-Negotiation)
     * are SC (self clear)
     * Bit 14 (Loopback) - Loopback speed is determined by the mode the device
     * Bit 8 (Duplex Mode), Bit 12 (Auto-Negotiation Enable), and Bits 13, 6
     * (Speed Selection) - "A write to this register bit does not take effect
     * until any one of the following also occurs: Software reset is asserted,
     * Restart Auto-Negotiation is asserted, Power down transition from power
     * down to normal operation.
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
    {"Control Register", MRV88E151XN_CONTROL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x3940, 0x1940},
    {"Status Register", MRV88E151XN_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0x7949},
    {"PHY Identifier 1", MRV88E151X_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(ulong)&reg_ext}, 0xFFFF, MRV88E151X_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E151X_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(ulong)&reg_ext}, 0xFFFF, 0},	
    {"Auto-Negotiation Advertisement Register", MRV88E151XN_AUTONEG_ADVR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xBFFF, 0x01FF},	
    {"Link Partner Ability Register - Base Page", MRV88E151XN_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Auto-Negotiation Expansion Register", MRV88E151XN_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x001F, 0x0004},	
    {"Next Page Transmit Register", MRV88E151XN_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E151XN_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"1000BASE-T Control Register", MRV88E151XN_1000B_CNTL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x1F00, 0x0300},
    {"1000BASE-T Status Register", MRV88E151XN_1000B_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFCFF, 0x0000},
    {"XMDIO MMD Control Register", MRV88E151XN_XMDIO_MMD_CNTL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xC01F, 0x0000},
    {"Extended Status Register", MRV88E151XN_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xF000, 0x3000},	
    {"Specific Control Register 1", MRV88E151XN_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7C9F, 0x3060},
    {"Specific Status Register 1", MRV88E151XN_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFF7F, 0x2040},
    {"Interrupt Enable Register", MRV88E151XN_INT_ENABLE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Interrupt Status Register", MRV88E151XN_SPECIFIC_INT_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E151XN_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x00FF, 0x0020},	
    {"Receive Error Counter Register", MRV88E151XN_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Global Interrupt Status Register", MRV88E151XN_GL_INT_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0001, 0x0000},
    {"Specific Control Register 3", MRV88E151XN_SPECIFIC_CONTROL3_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x63FF, 0x0040},	
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Page 1 - Fiber */
static reg_info_t ge_151x_p1_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) and  Bit 9 (Restart Auto-Negotiation) 
     * are SC (self clear)
     * Bit 13 (speed Select) - "To configure the SGMII media side interface
     * speed when 0_1.12 disables Auto-Negotiation, use 0_2:6, 0_2.13"
     * Bit 12 (Auto-Negotiation Enable), Bit 8 (Duplex Mode) and Bit 6
     * (Speed Selection (MSB)) - "A write to this register bit does not take
     * effect until any one of the following also occurs: Software reset is
     * asserted, Restart Auto-Negotiation is asserted, Power down transition
     * from power down to normal operation.
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
	 
    {"Control Register", MRV88E151XF_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x1100, 0x1140},	
    {"Status Register", MRV88E151XF_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFD, 0x0141},
    {"Phy Identifier 1", MRV88E151XF_PHY_ID_1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0141},
    {"Phy Identifier 2", MRV88E151XF_PHY_ID_2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0DD0},	
	{"Auto-Negotiation Expansion Register", MRV88E151XF_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x000F, 0x0004},		
    {"Next Page Transmit Register", MRV88E151XF_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xB7FF, 0x0001},	
    {"Link Partner Next Page Register", MRV88E151XF_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Extended Status Register", MRV88E151XF_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xF000, 0x0000},	
    {"Specific Control Register 1", MRV88E151XF_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFC8C, 0x8084},		
    {"Specific Status Register", MRV88E151XF_SPECIFIC_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFCFC, 0x0010},	
    {"Interrupt Enable Register", MRV88E151XF_INT_ENABLE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7FB0, 0x0000},	
    {"Interrupt Status Register", MRV88E151XF_INT_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7FB0, 0x0000},	
    {"Receive Error Counter Register", MRV88E151XF_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"PRBS Control Register", MRV88E151XF_PRBS_CONTROL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x00EF, 0x0000},
    {"PRBS Error Counter LSB Register", MRV88E151XF_PRBS_ERR_COUNTER_LSB_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"PRBS Error Counter MSB Register", MRV88E151XF_PRBS_ERR_COUNTER_MSB_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Specific Control Register 2", MRV88E151XF_SPECIFIC_CONTROL2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xC26F, 0x0042},
    {"End of Page 1 registers", 0, 0, {0}, 0, 0},
};

/* Page 2 - MAC */
static reg_info_t ge_151x_p2_reg_tbl[] = {
    {"Specific Control Register 1", MRV88E151XM_SPECIFIC_CONTROL1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFF84, 0x4048},
    {"Interrupt Enable Register", MRV88E151XM_INT_ENABLE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x008C, 0x0000},
    {"Specific Status Register", MRV88E151XM_SPECIFIC_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x008C, 0x0000},
    {"RX_ER Byte Capture Register", MRV88E151XM_RX_ER_BYTE_CAP_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xB3FF, 0x0000},	
    {"Specific Control Register 2", MRV88E151XM_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x4008, 0x1046},
    {"RGMII O/P Impedance Target Register", MRV88E151XM_OP_IMP_TARGET_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0007, 0x0003},	
    {"End of Page 2 registers", 0, 0, {0}, 0, 0},
};

/* Page 3 : LED*/
static reg_info_t ge_151x_p3_reg_tbl[] = {
    {"Function Control Register", MRV88E151XL_FUNC_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x1777},
    {"Polarity Control Register", MRV88E151XL_POL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x8800},
    {"Timer Control Register", MRV88E151XL_TMR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xF70F, 0x4905},	
    {"PTP LED Function Control Register", MRV88E151XL_PTP_LED_FN_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x2000, 0x0073},
    {"End of Page 3 registers", 0, 0, {0}, 0, 0},
};

/* Page 4 - RGMII Rx Error Capture */
static reg_info_t ge_151x_p4_reg_tbl[] = {
    {"RGMII RX_ER Byte Capture Register", MRV88E151XR_RGMII_RX_ER_BYTE_CAP_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xB3FF, 0x0000},
    {"End of Page 4 registers", 0, 0, {0}, 0, 0},
};

/* Page 5 - Virtual Cable Tester */
static reg_info_t ge_151x_p5_reg_tbl[] = {
    {"1000 BASE-T Pair Skew Register", MRV88E151XN_VCT_PAIR_SKEW_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"1000 BASE-T Pair Swap and Polarity", MRV88E151XN_VCT_PAIR_SWP_POL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x007F, 0x0000},
    {"VCT Control", MRV88E151XN_VCT_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x3FFF, 0x0000},
    {"VCT Sample Point Distance", MRV88E151XN_VCT_SAMPLE_PT_DIST_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0x0000},
    {"VCT Cross Pair Positive Threshold", MRV88E151XN_VCT_CROSS_PAIR_POS_TH_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0104},
    {"VCT Same Pair Impedance Positive Threshold 0 & 1", MRV88E151XN_VCT_SP_IM_POS_TH_0_1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0F12},
    {"VCT Same Pair Impedance Positive Threshold 2 & 3", MRV88E151XN_VCT_SP_IM_POS_TH_2_3_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0A0C},	
    {"VCT Same Pair Impedance Positive Threshold 4 & Tx Pulse", MRV88E151XN_VCT_SP_IM_POS_TH_4_TX_PULSE,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x3FFF, 0x0006},	
    {"End of Page 5 registers", 0, 0, {0}, 0, 0},
};

/* Page 6 - Miscellaneous */
static reg_info_t ge_151x_p6_reg_tbl[] = {
    {"Port Packet Generation Register", MRV88E151XN_PRT_PACKET_GEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x005F, 0x0000},	
    {"Port CRC Counters Register", MRV88E151XN_PRT_CRC_CHKR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Checker Control Register", MRV88E151XN_PRT_CHKR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0007, 0x0000},	
    {"Port Packet Generation 2 Register", MRV88E151XN_PRT_PACKET_GEN_REG2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x00FF, 0x0012},	
    {"General Control Register", MRV88E151XN_GEN_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03C0, 0x0200},	
    {"Late Collision Counter 1 & 2 Register", MRV88E151XN_COLL_CNT_1_2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Late Collision Counter 3 & 4 Register", MRV88E151XN_COLL_CNT_3_4_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Late Collision Window Adjust/Link Disconnect", MRV88E151XN_COLL_AD_LNK_DC_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x1F00, 0x0000},
    {"End of Page 6 registers", 0, 0, {0}, 0, 0},
};


/* Page 7 - Copper and VCT  */
static reg_info_t ge_151x_p7_reg_tbl[] = {
    {"Phy cable Diagnostics Pair0 Length", MRV88E151XN_DIAG_PAIR0_LEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Phy cable Diagnostics Pair1 Length", MRV88E151XN_DIAG_PAIR1_LEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Phy cable Diagnostics Pair2 Length", MRV88E151XN_DIAG_PAIR2_LEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Phy cable Diagnostics Pair3 Length", MRV88E151XN_DIAG_PAIR3_LEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Phy cable Diagnostics Results", MRV88E151XN_DIAG_RESULT_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"Phy cable Diagnostics Control", MRV88E151XN_DIAG_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x6400, 0x4000},	
    {"Advanced VCT Cross Pair Negative Threshold", MRV88E151XN_VCT_CROSS_PAIR_NEG_TH_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0104},	
    {"VCT Same Pair Impedance Negative Threshold 0 & 1", MRV88E151XN_VCT_SP_IM_NEG_TH_0_1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0F12},
    {"VCT Same Pair Impedance Negative Threshold 2 & 3", MRV88E151XN_VCT_SP_IM_NEG_TH_2_3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x7F7F, 0x0A0C},	
    {"VCT Same Pair Impedance Negative Threshold 4", MRV88E151XN_VCT_SP_IM_NEG_TH_4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x007F, 0x0006},	
    {"End of Page 7 registers", 0, 0, {0}, 0, 0},
};


/* Page 8 - Precise Timing  Protocol Arrival*/
static reg_info_t ge_151x_p8_reg_tbl[] = {
    {"PTP Port Configuration Register 0", MRV88E151XP_PTP_ARV_PRT_CONFG0_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0003, 0x4000},	
    {"PTP Port Configuration Register 1", MRV88E151XP_PTP_ARV_PRT_CONFG1_REG,
	READ_ONLY| SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0x020C},	
    {"PTP Port Configuration Register 2", MRV88E151XP_PTP_ARV_PRT_CONFG2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0002, 0x0000},	
    {"PTP Arrival 0 Time Port Status Register", MRV88E151XP_PTP_ARV0_TIME_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0007, 0x0000},	
    {"PTP Arrival 0 Time Register Bytes 1 & 0", MRV88E151XP_PTP_ARV0_TIME_BYTE_1_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Arrival 0 Time Register Bytes 3 & 2", MRV88E151XP_PTP_ARV0_TIME_BYTE_3_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Arrival 0 sequence Identifier Register", MRV88E151XP_PTP_ARV0_SEQ_IDN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Arrival 1 Time Port Status Register", MRV88E151XP_PTP_ARV1_TIME_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0007, 0x0000},	
    {"PTP Arrival 1 Time Register Bytes 1 & 0", MRV88E151XP_PTP_ARV1_TIME_BYTE_1_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Arrival 1 Time Register Bytes 3 & 2", MRV88E151XP_PTP_ARV1_TIME_BYTE_3_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Arrival 1 sequence Identifier Register", MRV88E151XP_PTP_ARV1_SEQ_IDN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"End of Page 8 registers", 0, 0, {0}, 0, 0},
};


/* Page 9 - Precise Timing  Protocol Departure*/
static reg_info_t ge_151x_p9_reg_tbl[] = {
    {"PTP Departure Time Port Status Register", MRV88E151XP_PTP_DEP_TIME_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0007, 0x0000},	
    {"PTP Departure Time Register Bytes 1 & 0", MRV88E151XP_PTP_DEP_TIME_BYTE_1_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Departure Time Register Bytes 3 & 2", MRV88E151XP_PTP_DEP_TIME_BYTE_3_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Departure sequence Identifier Register", MRV88E151XP_PTP_DEP_SEQ_IDN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"PTP Port Discard Counter Register", MRV88E151XP_PTP_DISCARD_CNTR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"End of Page 9 registers", 0, 0, {0}, 0, 0},
};

/* Page 12 - Global Configuration */
static reg_info_t ge_151x_p12_reg_tbl[] = {
    {"TAI Global Config Register 0", MRV88E151XT_TAI_GL_CONFIG0_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xC30F, 0x0000},	
    {"TAI Global Config Register 1", MRV88E151XT_TAI_GL_CONFIG1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x1F40},	
    {"TAI Global Config Register 2", MRV88E151XT_TAI_GL_CONFIG2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"TAI Global Config Register 3", MRV88E151XT_TAI_GL_CONFIG3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"TAI Global Config Register 4", MRV88E151XT_TAI_GL_CONFIG4_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},	
    {"TAI Global Config Register 5", MRV88E151XT_TAI_GL_CONFIG5_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0xF000}, 
    {"TAI Global Config Register 8", MRV88E151XT_TAI_GL_CONFIG8_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x8000, 0x0000},  	
    {"TAI Global Config Register 9", MRV88E151XT_TAI_GL_CONFIG9_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x83FF, 0x0000},  	
    {"Event Capture register Byte 1 & 0", MRV88E151XT_EVNT_CPTR_BYTE_1_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"Event Capture register Byte 3 & 2", MRV88E151XT_EVNT_CPTR_BYTE_3_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"PTP Global Time register Byte 1 & 0", MRV88E151XT_PTP_GL_TIME_BYTE_1_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"PTP Global Time register Byte 3 & 2", MRV88E151XT_PTP_GL_TIME_BYTE_3_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"End of Page 12 registers", 0, 0, {0}, 0, 0},
};


/* Page 14 - Global Configuration */
static reg_info_t ge_151x_p14_reg_tbl[] = {
    {"PTP Global Config Register0 ", MRV88E151XP_PTP_GL_CONFIG0_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x88F7},  	
    {"PTP Global Config Register 1", MRV88E151XP_PTP_GL_CONFIG1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"PTP Global Config Register 2", MRV88E151XP_PTP_GL_CONFIG2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"PTP Global Config Register 3", MRV88E151XP_PTP_GL_CONFIG3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0001, 0x0001},  	
    {"PTP Global Status Register", MRV88E151XP_PTP_GL_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x000F, 0x0000},  	
    {"Read Plus Command Register", MRV88E151XP_RD_PLUS_CMND_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x8F1F, 0x0000},  	
    {"Read Plus Data Register", MRV88E151XP_RD_PLUS_DATA_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"End of Page 14 registers", 0, 0, {0}, 0, 0},
};

/* Page 17 */
static reg_info_t ge_151x_p17_reg_tbl[] = {
    {"WOL Control", MRV88E151XW_WOL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xE1FF, 0x0000},  	
    {"WOL Status", MRV88E151XW_WOL_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xE0FF, 0x0000},  	
    {"SRAM PAcket 7/6 Length", MRV88E151XW_SRAM_PCKT_7_6_LEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0x0CFF},  	
    {"SRAM PAcket 5/4 Length", MRV88E151XW_SRAM_PCKT_5_4_LEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0x0CFF},  	
    {"SRAM PAcket 3/2 Length", MRV88E151XW_SRAM_PCKT_3_2_LEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0x0CFF},  	
    {"SRAM PAcket 1/0 Length", MRV88E151XW_SRAM_PCKT_1_0_LEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0x0CFF},  	
    {"Magic Packet Destination Address Word 2", MRV88E151XW_MAGIC_PKT_DA_WRD2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"Magic Packet Destination Address Word 1", MRV88E151XW_MAGIC_PKT_DA_WRD1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"Magic Packet Destination Address Word 0", MRV88E151XW_MAGIC_PKT_DA_WRD0_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"SRAM Byte Adress Control", MRV88E151XW_SRAM_BYTE_ADRS_CNTRL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0BFF, 0x0000},  	
    {"SRAM Byte Data Control", MRV88E151XW_SRAM_BYTE_DATA_CNTRL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x01FF, 0x0000},  	
    {"End of Page 17 registers", 0, 0, {0}, 0, 0},
};


/* Page 18 */
static reg_info_t ge_151x_p18_reg_tbl[] = {
    {"EEE Buffer Control Register 1", MRV88E151XB_EEE_BUFFR_CNTRL1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFF01, 0x0C00},  	
    {"EEE Buffer Control Register 2", MRV88E151XB_EEE_BUFFR_CNTRL2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x111E},  	
    {"EEE Buffer Control Register 3", MRV88E151XB_EEE_BUFFR_CNTRL3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x111E},  	
    {"Packet Generation", MRV88E151XB_PACKET_GEN_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFF0F, 0x0000},  	
    {"CRC Counters", MRV88E151XB_CRC_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},  	
    {"Checker Control", MRV88E151XB_CHCKR_CONTROL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x000F, 0x0000},  	
    {"Packet Generation 2", MRV88E151XB_PACKET_GEN2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x00FF, 0x000C},  	
    {"General Control Register 1", MRV88E151XB_GN_CONTROL1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x027B, 0x0200},  	
    {"Link Disconnect Count", MRV88E151XB_LNK_DISCONNECT_CNT_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x00FF, 0x0000},  	
    {"SRDES RX_ER Byte Capture", MRV88E151XB_SRDES_RX_ER_BYT_CAPTURE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xB3FF, 0x0000},  	
    {"End of Page 18 registers", 0, 0, {0}, 0, 0},
};

/* 88E151X */
static dev_mrvl_reg_info_t ge_151x_reg_tbl[] = {
    {MRV88E151X_REG_PAGE_0,  &ge_151x_p0_reg_tbl[0]},	/* Page 0 */
    {MRV88E151X_REG_PAGE_1,  &ge_151x_p1_reg_tbl[0]},	/* Page 1 */
    {MRV88E151X_REG_PAGE_2,  &ge_151x_p2_reg_tbl[0]},	/* Page 2 */
    {MRV88E151X_REG_PAGE_3,  &ge_151x_p3_reg_tbl[0]},	/* Page 3 */
    {MRV88E151X_REG_PAGE_4,  &ge_151x_p4_reg_tbl[0]},	/* Page 4 */
    {MRV88E151X_REG_PAGE_5,  &ge_151x_p5_reg_tbl[0]},	/* Page 5 */
    {MRV88E151X_REG_PAGE_6,  &ge_151x_p6_reg_tbl[0]},	/* Page 6 */
    {MRV88E151X_REG_PAGE_7,  &ge_151x_p7_reg_tbl[0]},	/* Page 7 */
    {MRV88E151X_REG_PAGE_8,  &ge_151x_p8_reg_tbl[0]},	/* Page 8 */
    {MRV88E151X_REG_PAGE_9,  &ge_151x_p9_reg_tbl[0]},	/* Page 9 */
    {MRV88E151X_REG_PAGE_12, &ge_151x_p12_reg_tbl[0]},	/* Page 12 */
    {MRV88E151X_REG_PAGE_14, &ge_151x_p14_reg_tbl[0]},	/* Page 14 */
    {MRV88E151X_REG_PAGE_17, &ge_151x_p17_reg_tbl[0]},	/* Page 17 */
    {MRV88E151X_REG_PAGE_18, &ge_151x_p18_reg_tbl[0]},	/* Page 18 */
    {0, 0},						/* End */
};


/* ======== 88E151x Test Mode Setting ========== */

/* Based on Marvell FAE,
 * Steps to enter 151x PHY to Test Mode 1, 2 or 4 are:
 * 1. Write Page 0, Reg  9 = 0x1F00 (Set PHY to Master mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_phy_setup_t phy_testmode124_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_1000B_CNTL_REG,  0x1F00, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0x9140, 0x7B40},
    {MRV88E151X_REG_PAGE_4, MRV88E151XR_PAGE4_REG27, 0x3E80, 0xFFFF},
    {MRV88E151X_REG_PAGE_6, MRV88E151XN_MISC_TEST_REG, 0x8000, 0xFFA0}
};


/* Based on Marvell FAE,
 * Steps to enter 151x PHY to Test Mode 3 are:
 * 1. Write Page 0, Reg  9 = 0x1700 (Set PHY to Slave mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_phy_setup_t phy_testmode3_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_1000B_CNTL_REG,  0x1700, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0x9140, 0x7B40},
    {MRV88E151X_REG_PAGE_4, MRV88E151XR_PAGE4_REG27, 0x3E80, 0xFFFF},
    {MRV88E151X_REG_PAGE_6, MRV88E151XN_MISC_TEST_REG, 0x8000, 0xFFA0}
};

/* 
 * Steps to enter 151x PHY to 10M Pseudo-Random Test are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 6, Reg 16 = 0x0008 (Enable Packet Generator)
 */
static mrvl_phy_setup_t phy_testmode5_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_SPECIFIC_CONTROL1_REG,  0x0400, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0x8100, 0x7B40},
    {MRV88E151X_REG_PAGE_6, MRV88E151XN_PRT_PACKET_GEN_REG, 0x0008, 0xFFFF}
};


/* 
 * Steps to enter 151x PHY to 10M data 0/1 Test Mode are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 2, Reg 21 = 0x5044 (Enable Loopback of MDI to MDI)
 */
static mrvl_phy_setup_t phy_testmode6_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_SPECIFIC_CONTROL1_REG,  0x0400, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0x8100, 0x7B40},
    {MRV88E151X_REG_PAGE_12, MRV88E151XT_PAGE12_REG16, 0x40, 0xFFFF}
};


/* 
 * Steps to enter 151x PHY to 100M Test Mode are:
 * 1. Write Page 0, Reg 16 = 0x0000 (Disable Auto-MDIX )
 * 2. Write Page 0, Reg  0 = 0xA100 (Disable Auto-Neg. & Force speed to 100M)
 */
static mrvl_phy_setup_t phy_testmode7_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_SPECIFIC_CONTROL1_REG,  0x0000, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0xA100, 0x7B40},
};


/* 
 * Steps to enter 151x PHY to 10M Link Pulse Test are:
 * 1. Write Page 0, Reg  16 = 0x0000 (Disable Auto-MDIX)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 */
static mrvl_phy_setup_t phy_testmode8_steps[] = {
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_SPECIFIC_CONTROL1_REG,  0x0000, 0xFFFF},
    {MRV88E151X_REG_PAGE_0, MRV88E151XN_CONTROL_REG,  0x8100, 0x7B40},
};



/******************************************************************************
 *
 * Name:	mrv88e151x_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the 88E151X device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void mrv88e151x_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_88e151x_object_t *obj_88e151x= (dev_88e151x_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 88e151x_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_88e151x->base.dev_object_fvt->dev_attach	= dev_88e151x_attach;
    obj_88e151x->base.dev_object_fvt->dev_detach	= dev_88e151x_detach;
    obj_88e151x->base.dev_object_fvt->dev_restart	= dev_88e151x_restart;
    obj_88e151x->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_88e151x->base.dev_object_fvt->dev_destroy	= dev_88e151x_destroy;
    obj_88e151x->base.dev_object_fvt->dev_name	= "88E151X Marvell PHY";

    obj_88e151x->callin_fvt = (dev_88e151x_callin_fvt_t *)
                               malloc(sizeof(dev_88e151x_callin_fvt_t));
    obj_88e151x->callout_fvt = (dev_88e151x_callout_fvt_t *)
                                malloc(sizeof(dev_88e151x_callout_fvt_t));

    obj_88e151x->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_88e151x_attach()
 *
 * Description:	Attach the 88E151X device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the 88E151X device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e151x_attach (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;

    if (obj_88e151x->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e151x_attach() callin malloc", DEV_88E151X_ATTACH);
        return (FAILED);
    }

    if (obj_88e151x->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e151x_attach() callout malloc", DEV_88E151X_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    obj_88e151x->callin_fvt->register_test  = dev_88e151x_test_reg;
    obj_88e151x->callin_fvt->dump_register  = dev_88e151x_show_reg;
    obj_88e151x->callin_fvt->display_register  = dev_88e151x_display_reg;
    obj_88e151x->callin_fvt->alter_register = dev_88e151x_alter_reg;
    obj_88e151x->callin_fvt->set_spd = dev_88e151x_set_spd;
    obj_88e151x->callin_fvt->set_lpbk = dev_88e151x_set_lpbk;
    obj_88e151x->callin_fvt->set_testmode = dev_88e151x_set_testmode;
    obj_88e151x->callin_fvt->set_eee = dev_88e151x_set_eee;	
    obj_88e151x->callin_fvt->enable_force_interrupt = dev_88e151x_enable_force_interrupt;
    obj_88e151x->callin_fvt->disable_interrupt = dev_88e151x_disable_interrupt;
    obj_88e151x->callin_fvt->gephy_set_led_on = dev_88e151x_led_on;
    obj_88e151x->callin_fvt->gephy_set_led_single_on = dev_88e151x_led_single_on;
    obj_88e151x->callin_fvt->gephy_set_led_off = dev_88e151x_led_off;
    obj_88e151x->callin_fvt->init_phy = dev_88e151x_init;
    obj_88e151x->callin_fvt->set_sgmii_output_amp = dev_88e151x_set_sgmii_amp;


    obj_88e151x->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_88e151x_detach()
 *
 * Description:	detach the device specific functions from the caller.
 *	        	All of the device specific function are connected to the
 *        		dev_do_nothing() function, except for the dev_attach()
 *        		function. Also, the dev_state must be assigned the value
 *        		of DEV_STATE_DETACH.
 *
 *        		Since, some platforms may want to detach the device, but not
 *        		release the memory resources (via a free () in the
 *        		dev_destroy()), this function can be executed to accomplish
 *        		this task. However, before a detached device can be used again,
 *        		it must be re-attached (via the dev_attach()).
 *
 * Input:	Pointer to the 88E151X device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e151x_detach (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_88e151x->base.dev_object_fvt);

    obj_88e151x->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_88e151x_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the 88E151X device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_88e151x_restart (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x= (dev_88e151x_object_t *) dev;

    obj_88e151x->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_88e151x_show_reg
 *
 * Description:	Dump GE PHY registers
 *
 * Input:	dev_object_t pointer to the 88E151x device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_88e151x_show_reg (dev_object_t *dev)
{
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_88e151x_display_reg
 *
 * Description:	Utility to display GE PHY register
 *
 * Input:	dev_object_t pointer to the 88E151x device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_88e151x_display_reg (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ",
                              0x0, 0x0, 0xFF);

    if (obj_88e151x->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_ALTER);
        return (FAILED);
    }

    printf("Offset %#x=%#x\n", reg_addr, reg_data);

    return (PASSED);
}


/******************************************************************************
 * Name:	dev_88e151x_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the 88E151Xdevice
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_88e151x_destroy (dev_object_t **dev)
{
    dev_88e151x_object_t *obj_88e151x;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_88e151x = (dev_88e151x_object_t *)*dev;

    if (obj_88e151x->callout_fvt) {
        free(obj_88e151x->callout_fvt);	/* Free callout struct */
    }

    if (obj_88e151x->callin_fvt) {
        free(obj_88e151x->callin_fvt);		/* Free callin struct */
    }

    free(obj_88e151x->base.dev_object_fvt);	/* Free dev_object_t */
}


/******************************************************************************
 *
 * Function:	dev_88e151x_alter_reg
 *
 * Description:	Alter 88E151X register.
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *		    A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_alter_reg (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ",
                              0x0, 0x0, 0xFF);

    if (obj_88e151x->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_ALTER);
        return (FAILED);
    }

    printf("Original value: reg %#.2x, data %#.2x\n", reg_addr, reg_data);

    /* alter register with new msb value */
    reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0, 0xFFFF);

    if (obj_88e151x->callout_fvt->wr(reg_addr, reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_ALTER);
        return (FAILED);
    }

    if (obj_88e151x->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#.2x, data %#.2x\n", reg_addr, reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_88e151x_test_reg
 *
 * Description: Tests the 88E151X registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the 88E151X device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_test_reg (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort phy_id;
    ushort original_page_reg, tested_page;
    int rc;
    dev_mrvl_reg_info_t *reg_test_p;
    reg_info_t *tested_reg_ptr;

    /* Save device object to register test parameter */
    reg_ext.param = (void *)dev;

    /* Save the page register */
    rc = (*callout_p->rd)(MRV88E151X_PAGE_ADDRESS_REG, &original_page_reg);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Page Reg Read Failed", __func__);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }

    /* Change page to 0 to read PHY ID */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, 0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Change Page to 0 Failed", 
                                      __func__);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }


    /* Read PHY ID */
    rc = (*callout_p->rd)(MRV88E151X_PHYID_1_REG, &phy_id);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read PHY ID Failed", __func__);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }

    if (phy_id != MRV88E151X_PHY_ID_1_VALUE) {
        sprintf(mrv_88e151x_err_buf, "%s: PHY ID Read %#x Expect %#x", 
                __func__, phy_id, MRV88E151X_PHY_ID_1_VALUE);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST); 
        return (FAILED);
    }
    
    reg_test_p = ge_151x_reg_tbl;
    tested_page    = reg_test_p->page;
    tested_reg_ptr = reg_test_p->reg_p;

    /* Change page */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, tested_page);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Change Page to %d Failed", 
                                      __func__, tested_page);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }

    rc = register_tests(0, tested_reg_ptr);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Page %d register test failed",
                                      __func__, tested_page);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }

    /* Restore Page register */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, original_page_reg);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Restore Page to %d Failed", 
                                      __func__, original_page_reg);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_REG_TEST);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_88e151x_set_testmode
 *
 * Description:	Set test mode
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Outputs:	PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_set_testmode (dev_object_t *dev)
{
    int test_mode = 0;;
    int ctr = 0, total_steps = 0;
    uint16_t testmode_val = 0;
    mrvl_phy_setup_t *step_ptr;

    printf("PHY(Marvell 151x) Supported TestMode:\n");
    printf("[0] Normal Mode.\n");
    printf("[1] Transmit Waveform Test.\n");
    printf("[2] Transmit Jitter Test (Master).\n");
    printf("[3] Transmit Jitter Test (Slave).\n");
    printf("[4] Transmit Distortion Test.\n");
    printf("[5] 10M Pseudo-Random Test.\n");
    printf("[6] 10M Data 0/1 Test.\n");
    printf("[7] 100M Waveform Test. .\n");
    printf("[8] 10M Link Pulse Test. .\n");
    test_mode = gethex_answer("Enter Test mode (0-8)", 0, 0, 8);

   if ((test_mode == PHY_TESTMODE_1) || (test_mode == PHY_TESTMODE_2) ||
        (test_mode == PHY_TESTMODE_4)) {
        step_ptr = &phy_testmode124_steps[0];
        total_steps = sizeof(phy_testmode124_steps) / sizeof(mrvl_phy_setup_t);

        /* 1. Enable Test mode 1: 0x3F00
         * 2. Enable Test mode 2: 0x5F00
         * 3. Enable Test mode 4: 0x9F00
         */
        if (test_mode == PHY_TESTMODE_1) {
            testmode_val = PHY_TESTMODE_1_REG_VAL;
        } else if (test_mode == PHY_TESTMODE_2) {
            testmode_val = PHY_TESTMODE_2_REG_VAL;
        } else if (test_mode == PHY_TESTMODE_4) {
            testmode_val = PHY_TESTMODE_4_REG_VAL;
        }
    } else if (test_mode == PHY_TESTMODE_3) {
        step_ptr = &phy_testmode3_steps[0];
        total_steps = sizeof(phy_testmode3_steps) / sizeof(mrvl_phy_setup_t);

        /* Enable Test mode 3: 0x7700 */
        testmode_val = PHY_TESTMODE_3_REG_VAL;
    } else if (test_mode == PHY_TESTMODE_5) {
        step_ptr = &phy_testmode5_steps[0];
        total_steps = sizeof(phy_testmode5_steps) / sizeof(mrvl_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_6) {
        step_ptr = &phy_testmode6_steps[0];
        total_steps = sizeof(phy_testmode6_steps) / sizeof(mrvl_phy_setup_t);
    }  else if (test_mode == PHY_TESTMODE_7) {
        step_ptr = &phy_testmode7_steps[0];
        total_steps = sizeof(phy_testmode7_steps) / sizeof(mrvl_phy_setup_t);
    }  else if (test_mode == PHY_TESTMODE_8) {
        step_ptr = &phy_testmode8_steps[0];
        total_steps = sizeof(phy_testmode8_steps) / sizeof(mrvl_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_NORMAL) {
        phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_0, 
                             MRV88E151XN_CONTROL_REG, 0x9140);
        return (PASSED);
    } else {
        printf("%s: Not support TestMode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    }

    for (ctr = 0; ctr < total_steps; ctr++, step_ptr++) {
        /* Set register */
        prpass(testpass, "Set TestMode%d: Set page%d Reg%.2d to 0x%04X",
               test_mode, step_ptr->reg_page, step_ptr->reg_off, step_ptr->val);
        printf("\n");
        if (phy_smi_wr_with_page(dev, step_ptr->reg_page, step_ptr->reg_off, step_ptr->val)
                                  != PASSED) {
            printf("\n%s: Failed to set PHY(151x) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, step_ptr->reg_page,
                   step_ptr->reg_off, step_ptr->val);
            return (FAILED);
        }

    }

    /* Test mode 5, 6 and 7, donot need to set */
    if ((test_mode == PHY_TESTMODE_5) || (test_mode == PHY_TESTMODE_6) ||
        (test_mode == PHY_TESTMODE_7) || (test_mode == PHY_TESTMODE_8)) {

    } else {
        /* Set Test mode by write page0 Reg 9 */
        /* Set register */
        prpass(testpass, "Set TestMode%d: Set page%d Reg%.2d to 0x%04X",
               test_mode, MRV88E151X_REG_PAGE_0, MRV88E151XN_1000B_CNTL_REG, testmode_val);
        printf("\n");
        if (phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_0, MRV88E151XN_1000B_CNTL_REG, 
                                 testmode_val) != PASSED) {
            printf("\n%s: Failed to set PHY(151x) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, MRV88E151X_REG_PAGE_0, MRV88E151XN_1000B_CNTL_REG, 
                   testmode_val);
            return (FAILED);
        }
    }


    printf("\nNow PHY(151x) enter TestMode%d, and press \'q\' to exit: ",
            test_mode);

    while (1) {
        if(getchar() == 'q') {
            phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_0, 
                                 MRV88E151XN_CONTROL_REG, 0x9140);
            break;
        }
    }
    return (PASSED);

}

/******************************************************************************
 *
 * Function:	dev_88e151x_set_sgmii_amp
 *
 * Description:	SGMII amplitude setting
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *              sgmii_amp_val amplitude setting
 *
 * Outputs:	PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_set_sgmii_amp (dev_object_t *dev, int sgmii_amp_val)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort read_data;
    int rc = FAILED;

    /* Change to Page 1 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_1);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed",
                                      __func__, MRV88E151X_REG_PAGE_1);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_SET_SGMII_AMP);
        return (FAILED);
    }

    rc = (*callout_p->rd)(MRV88E151XF_SPECIFIC_CONTROL2_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed",
                                      __func__, MRV88E151XF_SPECIFIC_CONTROL2_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_SET_SGMII_AMP);
        return (FAILED);
    }   

    read_data &= ~(P1_R26_AMP_MASK);
    read_data |= sgmii_amp_val;

    rc = (*callout_p->wr)(MRV88E151XF_SPECIFIC_CONTROL2_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed",
                                      __func__, MRV88E151XF_SPECIFIC_CONTROL2_REG,
                                      read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_SET_SGMII_AMP);
        return (FAILED);
    } 

    /* Change to Page 0 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed",
                                      __func__, MRV88E151X_REG_PAGE_0);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_SET_SGMII_AMP);
        return (FAILED);
    }

    return (PASSED);

}

/******************************************************************************
 *
 * Function:	dev_88e151x_init
 *
 * Description:	Initial 88e151x
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Outputs:	PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_init (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort read_data;
    int rc = FAILED;

    /* refer to errata (Release Notes - Alaska
     * 88E1510/88E1518/88E1512/88E1514 Rev A0) 
     */
    phy_smi_wr_with_page(dev, 0xFF, 17, 0x214B); 
    phy_smi_wr_with_page(dev, 0xFF, 16, 0x2144); 
    phy_smi_wr_with_page(dev, 0xFF, 17, 0x0C28); 
    phy_smi_wr_with_page(dev, 0xFF, 16, 0x2146); 
    phy_smi_wr_with_page(dev, 0xFF, 17, 0xB233); 
    phy_smi_wr_with_page(dev, 0xFF, 16, 0x214D); 
    phy_smi_wr_with_page(dev, 0xFF, 17, 0xCC0C); 
    phy_smi_wr_with_page(dev, 0xFF, 16, 0x2159); 

    /* Change to Page 18 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_18);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed",
                                      __func__, MRV88E151X_REG_PAGE_18);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    }

    rc = (*callout_p->rd)(MRV88E151XB_GN_CONTROL1_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed",
                                      __func__, MRV88E151XB_GN_CONTROL1_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    }   

    read_data &= ~(P20_R18_MODE_MASK);
    read_data |= P20_R18_MODE_RGMII_COPPER;

    rc = (*callout_p->wr)(MRV88E151XB_GN_CONTROL1_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed",
                                      __func__, MRV88E151XB_GN_CONTROL1_REG,
                                      read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    } 

    rc = (*callout_p->rd)(MRV88E151XB_GN_CONTROL1_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed",
                                      __func__, MRV88E151XB_GN_CONTROL1_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    }   

    read_data |= P20_R18_RESET;

    rc = (*callout_p->wr)(MRV88E151XB_GN_CONTROL1_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed",
                                      __func__, MRV88E151XB_GN_CONTROL1_REG,
                                      read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    } 

    /* Change to Page 0 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed",
                                      __func__, MRV88E151X_REG_PAGE_0);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INIT);
        return (FAILED);
    }

    return (PASSED);

}

/******************************************************************************
 *
 * Function:	dev_88e151x_set_eee
 *
 * Description:	Set EEE mode
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Outputs:	PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_set_eee (dev_object_t *dev)
{
    int test_mode = 0;;
    int speed;

    printf("PHY(Marvell 151x) EEE Mode:\n");
    printf("[0] EEE Mode Disable.\n");
    printf("[1] EEE Mode Enable - 1000M.\n");
    printf("[2] EEE Mode Enable - 100M.\n");
    test_mode = gethex_answer("Enter (0-2)", 0, 0, 2);


    if (test_mode == PHY_TESTMODE_NORMAL) {
        phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_0, 
                             MRV88E151XN_CONTROL_REG, 0x9140);
        return (PASSED);
    } else if (test_mode == PHY_TESTMODE_1) {
        speed = DEV_88E151X_SPD_1000;
    } else if (test_mode == PHY_TESTMODE_2) {
        speed = DEV_88E151X_SPD_100;
    } else {
        printf("%s: Not support EEE Mode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    }

    dev_88e151x_set_spd(dev,speed);

    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,17, 0x214B);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,16, 0x2144);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,17, 0x0C28);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,16, 0x2146);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,17, 0xB233);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,16, 0x214D);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,17, 0xCC0C);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_255,16, 0x2159);
    phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_251,07, 0xC00D);

    printf("\nNow PHY(151x) enter EEE Mode%d, and press \'q\' to exit: ",
            test_mode);

    while (1) {
        if(getchar() == 'q') {
            phy_smi_wr_with_page(dev, MRV88E151X_REG_PAGE_0, 
                                 MRV88E151XN_CONTROL_REG, 0x9140);
            break;
        }
    }
	
    return (PASSED);    

}


/******************************************************************************
 *
 * Function:	dev_88e151x_set_lpbk
 *
 * Description:	Set Loopback mode
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *		    lpbk_mode - Internal/External/Line Loopback
 *		    speed - 10/100/1G
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_set_lpbk (dev_object_t *dev, int loopback, int speed)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort read_data;
    int ix, rc;

    /* Change to Page 0 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, MRV88E151X_REG_PAGE_0);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    if (loopback == DEV_88E151X_INT_LPBK) {
        /* Enable Stub if speed is 1000 */
        if (speed == DEV_88E151X_SPD_1000) {
            dev_88e151x_enable_stub(dev, TRUE);
        }

        /* Disable Link Pulse */
        rc = (*callout_p->rd)(MRV88E151XN_SPECIFIC_CONTROL1_REG, &read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                          __func__, MRV88E151XN_SPECIFIC_CONTROL1_REG);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }
    
        read_data |= PHY_SPCR1_DIS_LINK_PULSE;

        rc = (*callout_p->wr)(MRV88E151XN_SPECIFIC_CONTROL1_REG, read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                          __func__, MRV88E151XN_SPECIFIC_CONTROL1_REG, 
                                          read_data);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }
        
        rc = (*callout_p->rd)(MRV88E151XN_CONTROL_REG, &read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                          __func__, MRV88E151XN_CONTROL_REG);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

        read_data |= MRV88E151X_LPBK_ENABLED;

        rc = (*callout_p->wr)(MRV88E151XN_CONTROL_REG, read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                          __func__, MRV88E151XN_CONTROL_REG, 
                                          read_data);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

    } else if (loopback == DEV_88E151X_EXT_LPBK) {
        /* Need to disable Auto MDI and Energy detect for Copper Loopback */
        rc = (*callout_p->rd)(MRV88E151XN_SPECIFIC_CONTROL1_REG, &read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                          __func__, MRV88E151XN_SPECIFIC_CONTROL1_REG);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }
    
        read_data &= ~(PHY_SPCR1_EN_DETECT | PHY_SPCR1_MDIX_MASK);

        rc = (*callout_p->wr)(MRV88E151XN_SPECIFIC_CONTROL1_REG, read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                          __func__, MRV88E151XN_SPECIFIC_CONTROL1_REG, 
                                          read_data);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }
        /* Copper Loopback */

        /* Enable Stub if speed is 1000 */
        if (speed == DEV_88E151X_SPD_1000) {
            dev_88e151x_enable_stub(dev, TRUE);
        }

        /* Check linkup */
        /* Change to Page 0 */
        rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                          __func__, MRV88E151X_REG_PAGE_0);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

        for (ix = 0; ix < MRV88E151X_LINK_UP_TOUT; ix++) {
            rc = (*callout_p->rd)(MRV88E151XN_SPECIFIC_STATUS1_REG, &read_data);
            if (rc != PASSED) {
                sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                              __func__, MRV88E151XN_SPECIFIC_STATUS1_REG);
                DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
                return (FAILED);
            }

            if ((read_data & PHY_COP_SPSTS_COP_LINK_UP)) {
                break;
            }

            msleep(10);
        }

        if (ix == MRV88E151X_LINK_UP_TOUT || !(read_data & PHY_COP_SPSTS_COP_LINK_UP)) {
            sprintf(mrv_88e151x_err_buf, "%s: Link up timeout (%#x)", 
                                          __func__, read_data);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }
    } else if (loopback == DEV_88E151X_DIS_LPBK) {
        rc = (*callout_p->rd)(MRV88E151XN_CONTROL_REG, &read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                          __func__, MRV88E151XN_CONTROL_REG);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

        read_data &= ~MRV88E151X_LPBK_ENABLED;

        rc = (*callout_p->wr)(MRV88E151XN_CONTROL_REG, read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                          __func__, MRV88E151XN_CONTROL_REG, 
                                          read_data);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

        dev_88e151x_enable_stub(dev, FALSE);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function:	dev_88e151x_set_spd
 *
 * Description:	Set PHY speed
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *		    spd - 10/100/1000
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_set_spd (dev_object_t *dev, int speed)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort read_data;
    int ix, rc;

    /*
     * Set Page 2, register 21: MAC speed 
     */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_2);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, MRV88E151X_REG_PAGE_2);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    rc = (*callout_p->rd)(MRV88E151XM_SPECIFIC_CONTROL2_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from %#x Failed", 
                                      __func__, MRV88E151XM_SPECIFIC_CONTROL2_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /* Set the desired speed */
    read_data &= ~(MRV88E151X_SPD_MASK);
    
    switch (speed) {
        case DEV_88E151X_SPD_10:
            read_data |= MRV88E151X_SPD_10;
            break;
        case DEV_88E151X_SPD_100:
            read_data |= MRV88E151X_SPD_100;
            break;
        case DEV_88E151X_SPD_1000:
            read_data |= MRV88E151X_SPD_1000;
            break;
        default:
            sprintf(mrv_88e151x_err_buf, "%s: Invalid speed (%d)", 
                                          __func__, speed);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
            break;
    }

    rc = (*callout_p->wr)(MRV88E151XM_SPECIFIC_CONTROL2_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                      __func__, MRV88E151XM_SPECIFIC_CONTROL2_REG, 
                                      read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /* Change back to Page 0 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, MRV88E151X_REG_PAGE_0);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /* 
     * Set Page 0, register 0: PHY Speed and disable Auto Negotiation 
     * Also assert software reset to take effect
     */
    
    rc = (*callout_p->rd)(MRV88E151XN_CONTROL_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from %#x Failed", 
                                      __func__, MRV88E151XN_CONTROL_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /* Disable Auto-Negotiation and Loopback */
    read_data &= ~(MRV88E151X_AN_ENABLED | MRV88E151X_LPBK_ENABLED);

    /* Set the desired speed */
    read_data &= ~(MRV88E151X_SPD_MASK);
    
    switch (speed) {
        case DEV_88E151X_SPD_10:
            read_data |= MRV88E151X_SPD_10;
            break;
        case DEV_88E151X_SPD_100:
            read_data |= MRV88E151X_SPD_100;
            break;
        case DEV_88E151X_SPD_1000:
            read_data |= MRV88E151X_SPD_1000;
            break;
        default:
            sprintf(mrv_88e151x_err_buf, "%s: Invalid speed (%d)", 
                                          __func__, speed);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
            break;
    }

    /* Assert S/W reset */
    read_data |= MRV88E151X_SW_RESET;

    rc = (*callout_p->wr)(MRV88E151XN_CONTROL_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x (%#x) Failed", 
                                      __func__, MRV88E151XN_CONTROL_REG, 
                                      read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /*
     * Polling until S/W reset is cleared
     */
    for (ix = 0; ix < MRV88E151X_RESET_CLR_TOUT; ix++) {
        rc = (*callout_p->rd)(MRV88E151XN_CONTROL_REG, &read_data);
        if (rc != PASSED) {
            sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                          __func__, MRV88E151XN_CONTROL_REG);
            DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
            return (FAILED);
        }

        if (!(read_data & MRV88E151X_SW_RESET)) {
            break;
        }

        msleep(10);
    }

    if (ix == MRV88E151X_RESET_CLR_TOUT || read_data & MRV88E151X_SW_RESET) {
        sprintf(mrv_88e151x_err_buf, "%s: SW Reset is not cleared (%#x)", 
                                      __func__, read_data);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_88e151x_control_led2_intr
 *
 * Description:	 Enable/Disable Interrupt on LED 2 pin
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *          enable - Enable/Disable Interrupt
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_control_led2_intr (dev_object_t *dev, int enable)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *) dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    ushort read_data;
    int rc;

    /* Change to Page 3 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_3);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, MRV88E151X_REG_PAGE_6);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INT_TEST);
        return (FAILED);
    }
  
    rc = (*callout_p->rd)(MRV88E151XL_TMR_CONTROL_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                      __func__, MRV88E151XL_TMR_CONTROL_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INT_TEST);
        return (FAILED);
    }

    if (enable == TRUE) {
        /* Enable Interrupt on LED[2] and Force Interrupt */
        read_data |= PHY_TIMER_CNTRL_FORCE_INT | PHY_TIMER_CNTRL_INTR_EN;
    } else {
        /* Disable Interrupt on LED[2]. In this device driver code, this pin
         * is always setting as Interrupt pin. If we let this pin have LED 
         * feature, the LED behavior will affect our experiment.
         */
        read_data &= ~PHY_TIMER_CNTRL_FORCE_INT;
    }

    rc = (*callout_p->wr)(MRV88E151XL_TMR_CONTROL_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write to %#x Failed", 
                                      __func__, MRV88E151XL_TMR_CONTROL_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_INT_TEST);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function:	dev_88e151x_enable_force_interrupt
 *
 * Description:	Enable Interrupt and find a way to generate interrupt to 
 *              host CPU
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *          enable - Enable/Disable Interrupt
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_enable_force_interrupt (dev_object_t *dev)
{
    return (dev_88e151x_control_led2_intr(dev, TRUE));
}


/******************************************************************************
 *
 * Function:	dev_88e151x_disable_interrupt
 *
 * Description:	Disable Interrupt 
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_disable_interrupt (dev_object_t *dev)
{
    return (dev_88e151x_control_led2_intr(dev, FALSE));
}


/******************************************************************************
 *
 * Function:   dev_88e151x_enable_stub	
 *
 * Description:	Enable/Disable Stub
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *          enable - Enable/Disable Stub
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e151x_enable_stub (dev_object_t *dev, int enable)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc;
    ushort read_data, original_page;

    /* Save original page */
    rc = (*callout_p->rd)(MRV88E151X_PAGE_ADDRESS_REG, &original_page);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                      __func__, MRV88E151X_PAGE_ADDRESS_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    /* Section 2.5.4 of 1512 data sheet
     * For 10BASE-T and 100BASE-TX modes, the loopback test requires no
     * register writes. For 1000BASE-T mode, register 18_6.3 must be set
     * to 1 to enable the external loopback. ...
     */
    /* Change to Page 6 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_6);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, MRV88E151X_REG_PAGE_6);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }
  
    rc = (*callout_p->rd)(MRV88E151XN_PRT_CHKR_CONTROL_REG, &read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Read data from  %#x Failed", 
                                      __func__, MRV88E151XN_PRT_CHKR_CONTROL_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    if (enable) { 
        read_data |= PRT_CHKR_PKT_STUB_EN;
    } else {
        read_data &= ~(PRT_CHKR_PKT_STUB_EN);    
    }

    rc = (*callout_p->wr)(MRV88E151XN_PRT_CHKR_CONTROL_REG, read_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, MRV88E151XN_PRT_CHKR_CONTROL_REG);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, original_page);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Set Page to %d Failed", 
                                      __func__, original_page);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LPBK);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: phy_smi_wr_with_page
 *
 * This function: writes Marvell GE registers with write page
 *
 * Input : addr - offset of register to be written.
 *	   size - which page
 *         data  - write data. Note that SMI interface uses 2 bytes (smi_t).
 *		   similar to the read, we can either set the smi_if_t.buf
 *		   to point at the position of the lower 2 bytes of ulong, or
 *		   use a working smi_t area, and transfer the ulong data to the
 *		   smi_t work area.
 *         param - Pointer to the Marvell GE device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int phy_smi_wr_with_page (dev_object_t *dev, ulong page, ulong addr, 
                               ulong data)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc ;

    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, (ushort)page);

    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Failed (%#x)", __func__, rc);
        DEV_ERROR_REPORT((dev_object_t *)dev, mrv_88e151x_err_buf,
                         DEV_88E151X_WRITE);  
        return (FAILED);
    }

    rc = (*callout_p->wr)(addr, (ushort)data);

    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Failed (%#x)", __func__, rc);
        DEV_ERROR_REPORT((dev_object_t *)dev, mrv_88e151x_err_buf,
                         DEV_88E151X_WRITE);  
        return (FAILED);
    }



    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e151x_smi_rd
 *
 * This function: reads Marvell GE registers
 *
 * Input : addr - offset of register to be read.
 *	   size - ignored. For SMI, it is always 2.
 *	   buf  - points to the data buffer to be read. Note that this is a
 *		  pointer to ulong. SMI data is always 2 bytes long. Either
 *		  set SMI interface buf field to position at the lower 2 bytes
 *		  of the ulong, or use a working smi_t (2 bytes), then transfer
 *		  the resulting data to the ulong.
 *	   param - Pointer to the Marvell GE device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e151x_smi_rd (ulong addr, int size, ulong *buf, void *param)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)param;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc;

    rc = (*callout_p->rd)(addr, (ushort *)buf);

    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Failed (%#x)", __func__, rc);
        DEV_ERROR_REPORT((dev_object_t *)param, mrv_88e151x_err_buf,
                         DEV_88E151X_READ);  
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e151x_smi_wr
 *
 * This function: writes Marvell GE registers
 *
 * Input : addr - offset of register to be written.
 *	   size - Ignored. For SMI, it is always 2.
 *         data  - write data. Note that SMI interface uses 2 bytes (smi_t).
 *		   similar to the read, we can either set the smi_if_t.buf
 *		   to point at the position of the lower 2 bytes of ulong, or
 *		   use a working smi_t area, and transfer the ulong data to the
 *		   smi_t work area.
 *         param - Pointer to the Marvell GE device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e151x_smi_wr (ulong addr, int size, ulong data, void *param)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)param;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc ;

    rc = (*callout_p->wr)(addr, (ushort)data);

    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Failed (%#x)", __func__, rc);
        DEV_ERROR_REPORT((dev_object_t *)param, mrv_88e151x_err_buf,
                         DEV_88E151X_WRITE);  
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e151x_led_on
 *
 * This function: Turn on 88e151x led.
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e151x_led_on (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc;
    /* change to page 3 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_3);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x16);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_ON);
        return (FAILED);
    }
    /* write (16_3) to turn on led */ 
    rc = (*callout_p->wr)(MRV88E151XL_FUNC_CONTROL_REG, MRV88E151XL_FORCE_ON_LED);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x10);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_ON);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e151x_led_single_on
 *
 * This function: Turn on single 88e151x led.
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e151x_led_single_on (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc, wr_data, led_num;

    led_num  = gethex_answer("Enter which LED(0.LED0, 1.LED1):",
                              0x0, 0x0, 0x1);
    /* change to page 3 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_3);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x16);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_ON);
        return (FAILED);
    }
    /* write (16_3) to turn on led */
    if (led_num == MRV88E151XL_LED_0) {
        wr_data = MRV88E151XL_FORCE_ON_LED_0;
    } else {
        wr_data = MRV88E151XL_FORCE_ON_LED_1;
    }
    rc = (*callout_p->wr)(MRV88E151XL_FUNC_CONTROL_REG, wr_data);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x10);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_ON);
        return (FAILED);
    }

    /* change to page 0 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x16);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_ON);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e151x_led_off
 *
 * This function: Turn off 88e151x led.
 *
 * Inputs:	dev_object_t pointer to the 88E151X device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e151x_led_off (dev_object_t *dev)
{
    dev_88e151x_object_t *obj_88e151x = (dev_88e151x_object_t *)dev;
    dev_88e151x_callout_fvt_t *callout_p = obj_88e151x->callout_fvt;
    int rc;
    /* change to page 3 */
    rc = (*callout_p->wr)(MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_3);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x16);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_OFF);
        return (FAILED);
    }
    /* write (16_3) to turn off led */
    rc = (*callout_p->wr)(MRV88E151XL_FUNC_CONTROL_REG, MRV88E151XL_DEFAULT_FUNC_CONTROL);
    if (rc != PASSED) {
        sprintf(mrv_88e151x_err_buf, "%s: Write data to %#x Failed", 
                                      __func__, 0x10);
        DEV_ERROR_REPORT(dev, mrv_88e151x_err_buf, DEV_88E151X_LED_OFF);
        return (FAILED);
    }

    return (PASSED);
}
/*------------------------------------------------------------------
$Log: dev_88e151x.c,v $
Revision 1.3  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.6  2018/07/03 05:39:26  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.5  2018/04/26 08:14:14  lucywang
Added utility to set 88E1514 EEE

Revision 1.1.2.4  2018/03/16 02:01:02  olin2
Support GE PHY testmode util

Revision 1.1.2.3  2018/03/15 08:31:00  harrchan
Add new function for led test

Revision 1.1.2.2  2018/02/27 09:10:31  harrchan
Initial viper application code base


$Endlog$
*/
