/* $Id: dev_mrvl_ge.c,v 1.3 2021/09/24 01:22:45 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_n88e111x_marvell/dev_mrvl_ge.c,v $
 *------------------------------------------------------------------
 * 
 * dev_mrvl_ge.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "strings.h"
#include "dev_mrvl_ge.h"
#include "nvmonvars.h"
#include <sys/time.h>

/*===================================================================*
 *                    Basic Function                                 *
 *===================================================================*/
static uint32_t mvl_ge_dev_attach(dev_object_t *);
static uint32_t	mvl_ge_dev_detach(dev_object_t *);
static uint32_t	mvl_ge_dev_reconfig(dev_object_t *, void *, boolean *);
static uint32_t	mvl_ge_dev_restart(dev_object_t *);
static uint32_t	mvl_ge_dev_init(dev_object_t *);
static uint32_t	mvl_ge_dev_oper_en(dev_object_t *);
static uint32_t	mvl_ge_dev_oper_dis(dev_object_t *);
static uint32_t mvl_ge_dev_intr_en(dev_object_t *);
static uint32_t mvl_ge_dev_intr_dis(dev_object_t *);
static uint32_t	mvl_ge_dev_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32_t	mvl_ge_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	mvl_ge_destroy(dev_object_t **);

/*===================================================================*
 *                    Test Function                                  *
 *===================================================================*/
static int mvl_ge_reg_test(dev_object_t *);
static int mvl_ge_phy_mac_lpbk_test(dev_object_t *);
static int mvl_ge_ext_lpbk_test(dev_object_t *);
static int mvl_ge_phy_sfp_ext_lpbk_test(dev_object_t *);
static int mvl_ge_intr_test(dev_object_t *);
static int mvl_ge_phy_mac_lpbk_test_1gbps(dev_object_t *);
static int mvl_ge_ext_lpbk_test_1gbps(dev_object_t *);

/*===================================================================*
 *                    Read/ Write Function                           *
 *===================================================================*/
static int mvl_ge_smi_rd(ulong addr, int size, ulong *buf, void *param);
static int mvl_ge_smi_wr(ulong addr, int size, ulong data, void *param);
static int mvl_ge_rd_wr_reg(dev_object_t *, uint, smi_t, uchar, smi_t *);
static int mvl_ge_polling_reg(dev_object_t *, int, smi_t, uchar, smi_t, int);

/*===================================================================*
 *                    Utility Function                               *
 *===================================================================*/
static int util_mvl_ge_rd_reg(dev_object_t *);
static int util_mvl_ge_wr_reg(dev_object_t *);
static int util_mvl_ge_set_test_mode(dev_object_t *);
static int util_mvl_ge_set_tx_type(dev_object_t *);
static int util_mvl_ge_set_vod(dev_object_t *);

/*===================================================================*
 *                    Interface Function                             *
 *===================================================================*/
int if_mvl_ge_rd_reg(dev_object_t *, smi_t, uchar, smi_t *);
int if_mvl_ge_wr_reg(dev_object_t *, smi_t, uchar, smi_t *);
static int if_mvl_ge_set_tx_type(dev_object_t *, int);

/*===================================================================*
 *                    Advanced Function
 *
 * Comments: These function are used to set specific configs 
 *           to specific page and register via basic  
 *           GE register read/write function: mvl_ge_rd_wr_reg()     *
 *===================================================================*/
static int mvl_ge_mscr1_get_phy_media_mode(dev_object_t *, smi_t *);
static int mvl_ge_mscr1_set_phy_media_mode(dev_object_t *, smi_t);
static int mvl_ge_cscr1_disable_linkgood (dev_object_t *);
static int mvl_ge_mcr_set_media_spd(dev_object_t *, int);
static int mvl_ge_mcr_set_default (dev_object_t *);
static int mvl_ge_ccr_set_default (dev_object_t *);
static int mvl_ge_ccr_ext_lpbk_spd_config(dev_object_t *, int);
static int mvl_ge_xcr_soft_reset(dev_object_t *, dev_mrvl_page_type_t);
static int mvl_ge_xcr_enable_lpbk(dev_object_t *, dev_mrvl_page_type_t);
static int mvl_ge_xcr_power_control(dev_object_t *, dev_mrvl_page_type_t, dev_mrvl_pwr_type_t);
static int mvl_ge_xssr_check_linkup(dev_object_t *, dev_mrvl_page_type_t);
static int mvl_ena_stub_test(dev_object_t *, dev_mrvl_option_type_t);
static int mvl_ge_env_init(dev_object_t *);
static int mvl_ge_chk_intr_assert(dev_object_t *);
static int mvl_ge_chk_intr_deassert(dev_object_t *);

/*===================================================================*
 *                    Other Function                                 *
 *===================================================================*/
static dev_mrvl_reg_info_t *mvl_ge_get_master_table(dev_object_t *dev);
static int mvl_sfp_setup(dev_object_t *);
static int mvl_sfp_get_media_mode(dev_object_t *, smi_t *);
static int mvl_sfp_tx_disable(dev_object_t *);
static int mvl_sfp_tx_enable(dev_object_t *);
static int mvl_cpu_mac_setup(dev_object_t *, int);
static int mvl_cpu_phy_mac_check_linkup(dev_object_t *);
static int mvl_cpu_ext_lpbk_init(dev_object_t *);
static int mvl_ge_phy_tx_rx_test(dev_object_t *);

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
/* Speed table for loopback testing */

static int mvl_ge_copper_speed_t[] = {MRV88E1112_PHY_COP_SPD_10MBPS, 
                                      MRV88E1112_PHY_COP_SPD_100MBPS, 
                                      MRV88E1112_PHY_COP_SPD_1000MBPS};
/* reg_info_t extension for SMI access */
static reg_info_t_ext reg_ext = {sizeof(smi_t), mvl_ge_smi_rd, mvl_ge_smi_wr, 0};

/* Common registers for all pages */
static reg_info_t ge_1112_cmn_reg_tbl[] = {
    {"PHY Identifier 1", MRV88E111N_PA_R2_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1112_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E111N_PA_R3_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1112_PHY_ID_2_VALUE},
    /* Page Address bit 15 - Enable automatic medium register selection */
    /* The note in 88E1112C (Section 3 Registers Description)
     * - "Note that in order for the paging mechanism to work correctly
     * register 22 bit[15] must be set to 0 to disable the automatic medium
     * register selection"
     */
    {"Page Address", MRV88E111N_PA_R22_PAGE_ADDRESS_REG, READ_WRITE | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0x00FF, 0x0000},
    {"End of Common registers page", 0, 0, {0}, 0, 0},
};

/* Page 0 - Copper */
static reg_info_t ge_1112_p0_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) and  Bit 9 (Restart Auto-Negotiation)
     * are SC (self clear)
     * Bit 14 (Loopback) - Loopback speed is determined by the mode the device
     * is in Registers 0_2:13 and 0_2:6. After writing 0x7140, it reads back
     * 0x3140.
     * Bit 8 (Duplex Mode), Bit 12 (Auto-Negotiation Enable), and Bits 13, 6
     * (Speed Selection) - "A write to this register bit does not take effect
     * until any one of the following also occurs: Software reset is asserted,
     * Restart Auto-Negotiation is asserted, Power down transition from power
     * down to normal operation.
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
    {"Control Register", MRV88E111N_P0_R0_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1140},
    {"Status Register", MRV88E111N_P0_R1_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7F, 0x0149},
    {"Auto-Negotiation Advertisement Register", MRV88E111N_P0_R4_AUTONEG_ADVR_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xAFFF, 0x0001},
    {"Link Partner Ability Register - Base Page", MRV88E111N_P0_R5_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111N_P0_R6_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0004},
    {"Next Page Transmit Register", MRV88E111N_P0_R7_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111N_P0_R8_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000BASE-T Control Register", MRV88E111N_P0_R9_1000B_CNTL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x1F00, 0x0F00},
    {"1000BASE-T Status Register", MRV88E111N_P0_R10_1000B_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFCFF, 0x0000},
    {"Extended Status Register", MRV88E111N_P0_R15_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF000, 0x0000},
    {"Specific Control Register 1", MRV88E111N_P0_R16_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFE3, 0x6060},
    {"Specific Status Register 1", MRV88E111N_P0_R17_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0010},
    {"Interrupt Enable Register", MRV88E111N_P0_R18_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Specific Status Register 2", MRV88E111N_P0_R19_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Receive Error Counter Register", MRV88E111N_P0_R21_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111N_P0_R26_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x81F2, 0x0040},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Page 1 - Fiber */
static reg_info_t ge_1112_p1_reg_tbl[] = {
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
    {"Control Register", MRV88E111F_P1_R0_CONTROL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x4000, 0x0100},
    {"Status Register", MRV88E111F_P1_R1_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7D, 0x0141},
    {"Auto-Negotiation Advertisement Register", MRV88E111F_P1_R4_AUTONEG_ADVR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB1E0, 0x0060},
    {"Link Partner Ability Register - Base Page", MRV88E111F_P1_R5_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFDE0, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111F_P1_R6_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x000F, 0x0004},
    {"Next Page Transmit Register", MRV88E111F_P1_R7_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111F_P1_R8_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Extended Status Register", MRV88E111F_P1_R15_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF000, 0x0000},
    {"Specific Control Register 1", MRV88E111F_P1_R16_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0E00, 0x0000},
    {"Specific Status Register 1", MRV88E111F_P1_R17_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFF8, 0x0010},
    {"Interrupt Enable Register", MRV88E111F_P1_R18_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x3F10, 0x0000},
    {"Specific Status Register 2", MRV88E111F_P1_R19_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x3F10, 0x0000},
    {"Receive Error Counter Register", MRV88E111F_P1_R21_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111F_P1_R26_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE080, 0x8103},
    {"End of Page 1 registers", 0, 0, {0}, 0, 0},
};

/* Page 2 - MAC */
static reg_info_t ge_1112_p2_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) is SC (self clear)
     * Bit 14 (Loopback) is not working. After writing 0x7040, it returns
     * 0x3040.
     * Bits 13 and 6 (Default MAC Interface and SGMII Media Interface Speed),
     * Bit 12 (SGMII MAC Interface or GBIC Auto-Negotiation Enable) - "Change
     * to these bits are disruptive to the normal operation; therefore, any
     * changes to these registers must be followed by software reset to take
     * effect."
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
    {"Control Register", MRV88E111M_P2_R0_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1000},
    {"Specific Control Register 1", MRV88E111M_P2_R16_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF84, 0x0008},
    {"Specific Status Register 1", MRV88E111M_P2_R17_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0460, 0x0000},
    {"Interrupt Enable Register", MRV88E111M_P2_R18_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Status Register 2", MRV88E111M_P2_R19_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Control Register 2", MRV88E111M_P2_R26_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE090, 0x0000},
    {"End of Page 2 registers", 0, 0, {0}, 0, 0},
};

/* Page 3 - LOS, INIT, STATUS[1:0] */
static reg_info_t ge_1112_p3_reg_tbl[] = {
    {"Function Control Register", MRV88E111L_P3_R16_FUNC_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x021E},
    {"Polarity Control Register 1", MRV88E111L_P3_R17_POL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x44A0},
    {"Timer Control Register 2", MRV88E111L_P3_R18_TMR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x770F, 0x4105},
    {"End of Page 3 registers", 0, 0, {0}, 0, 0},
};

/* Page 4 - Non-Volatile Memory */
static reg_info_t ge_1112_p4_reg_tbl[] = {
    {"I2C Address Register", MRV88E111NV_P4_R16_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0100},
    {"Read Data and Status Register", MRV88E111NV_P4_R17_READ_DATA_STATUS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE7FF, 0x0000},
    {"Write Data and Control Register", MRV88E111NV_P4_R18_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF2FF, 0xA000},
    {"RAM Write Data and Control Register", MRV88E111NV_P4_R19_RAM_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"RAM Address Register", MRV88E111NV_P4_R20_RAM_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"End of Page 4 registers", 0, 0, {0}, 0, 0},
};

/* Page 5 - Virtual Cable Tester */
static reg_info_t ge_1112_p5_reg_tbl[] = {
    /* MDI[0] Virtual Cable Rester Status Register - Bit 15 (Run VCT Test)
     * is SC (self clear).
     */
    {"MDI[0] VCT Status Register", MRV88E111N_P5_R16_VCT_STATUS_MDI0_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x0000},
    {"MDI[1] VCT Status Register", MRV88E111N_P5_R17_VCT_STATUS_MDI1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x8000, 0x0000},
    {"MDI[2] VCT Status Register", MRV88E111N_P5_R18_VCT_STATUS_MDI2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"MDI[3] VCT Status Register", MRV88E111N_P5_R19_VCT_STATUS_MDI3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"1000 BASE-T Pair Skew Register", MRV88E111N_P5_R20_VCT_PAIR_SKEW_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000 BASE-T Pair Swap and Polarity", MRV88E111N_P5_R21_VCT_PAIR_SWP_POL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x007F, 0x0000},
    {"VCT DSP Distance", MRV88E111N_P5_R26_VCT_DSP_DISTANCE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0007, 0x0000},
    {"End of Page 5 registers", 0, 0, {0}, 0, 0},
};

/* Page 6 - Miscellaneous */
static reg_info_t ge_1112_p6_reg_tbl[] = {
    {"Packet Generation", MRV88E111N_P6_R16_PACKET_GEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0000},
    {"CRC Counters", MRV88E111N_P6_R17_CRC_CHKR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"End of Page 6 registers", 0, 0, {0}, 0, 0},
};

/* Master page tables */
/* 	88E1112 */
static dev_mrvl_reg_info_t ge_1112_reg_tbl[] = {
    {MRV88E111N_REG_PAGE_0, &ge_1112_cmn_reg_tbl[0]},	/* Common registers */
    {MRV88E111N_REG_PAGE_0, &ge_1112_p0_reg_tbl[0]},	/* Page 0 */
    {MRV88E111N_REG_PAGE_1, &ge_1112_p1_reg_tbl[0]},	/* Page 1 */
    {MRV88E111N_REG_PAGE_2, &ge_1112_p2_reg_tbl[0]},	/* Page 2 */
    {MRV88E111N_REG_PAGE_3, &ge_1112_p3_reg_tbl[0]},	/* Page 3 */
    {MRV88E111N_REG_PAGE_4, &ge_1112_p4_reg_tbl[0]},	/* Page 4 */
    {MRV88E111N_REG_PAGE_5, &ge_1112_p5_reg_tbl[0]},	/* Page 5 */
    {MRV88E111N_REG_PAGE_6, &ge_1112_p6_reg_tbl[0]},	/* Page 6 */
    {0, 0},						/* End */
};

/*******************************************************************************
 * Function:    mrvl_n88e111x_dev_create
 *
 * Description: Create object with various device function point to "do nothing"
 * Input:       dev             - dev_object_t pointer to the Marvell device.
 *	        error_report_fn - error reporting function pointer.
 * Returns:     PASSED/FAILED
 * Assumptions:	Since this device driver is shared amongst different Marvell
 *		chips, dev_object_fvt->dev_name needs to be filled after this
 *		call.
 *******************************************************************************/
int mrvl_n88e111x_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "obj malloc failure in mrvl_n88e111x_dev_create()",
			0);
	return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("\ndev obj @ %p\n", dev_fvt);
        }
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    mrvl_p->base.dev_object_fvt->dev_attach	= mvl_ge_dev_attach;
    mrvl_p->base.dev_object_fvt->dev_detach	= mvl_ge_dev_detach;
    mrvl_p->base.dev_object_fvt->dev_reconfig_needed = mvl_ge_dev_reconfig;
    mrvl_p->base.dev_object_fvt->dev_restart	= mvl_ge_dev_restart;
    mrvl_p->base.dev_object_fvt->dev_init	= mvl_ge_dev_init;
    mrvl_p->base.dev_object_fvt->dev_oper_enable  = mvl_ge_dev_oper_en;
    mrvl_p->base.dev_object_fvt->dev_oper_disable = mvl_ge_dev_oper_dis;
    mrvl_p->base.dev_object_fvt->dev_intr_enable  = mvl_ge_dev_intr_en;
    mrvl_p->base.dev_object_fvt->dev_intr_disable = mvl_ge_dev_intr_dis;
    /* unsupported fvt for dev_isr */
    /* mrvl_p->base.dev_object_fvt->dev_isr */
    mrvl_p->base.dev_object_fvt->dev_show	= mvl_ge_dev_show;
    mrvl_p->base.dev_object_fvt->dev_error_report = error_report_fn;
    mrvl_p->base.dev_object_fvt->dev_collect_crashinfo = mvl_ge_crsh;
    mrvl_p->base.dev_object_fvt->dev_destroy	= mvl_ge_destroy;
    mrvl_p->base.dev_object_fvt->dev_name = "Marvell GE PHY";

    if ((mrvl_p->callin_fvt = (mrvl_ge_callin_fvt_t *)
			       malloc(sizeof(mrvl_ge_callin_fvt_t))) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("freeing dev_fvt @ %p after malloc callin\n", dev_fvt);
        }
	free(dev_fvt);
	error_report_fn(dev, "callin malloc failure in mrvl_n88e111x_dev_create()",
			0);
	return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("callin_fvt @ %p\n", mrvl_p->callin_fvt);
        }
    }

    if ((mrvl_p->callout_fvt = (mrvl_ge_callout_fvt_t *)
			       malloc(sizeof(mrvl_ge_callout_fvt_t))) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("freeing callout @ %p after malloc callout \n", mrvl_p->callout_fvt);
        }
	free(mrvl_p->callin_fvt);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("freeing dev_fvt @ %p\n", dev_fvt);
        }
	free(dev_fvt);
	error_report_fn(dev, "callout malloc failure in mrvl_n88e111x_dev_create()",
			0);
	return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("callout_fvt @ %p\n", mrvl_p->callout_fvt);
        }
    }

    mrvl_p->base.dev_state = DEV_STATE_CREATE;
    return (PASSED);
}

/**************************************************************
 * Function:    mvl_ge_dev_attach()
 *
 * Description: Attach the Marvell GE device for use. This 
 *              function will initialize and setup all necessary 
 *              pointers and bring the chip to operation.
 * Input:       Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ***************************************************************/
static uint32_t mvl_ge_dev_attach (dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    char err_buf[ERR_BUF_SIZE];

    if (mrvl_p->base.dev_state != DEV_STATE_CREATE) {
	sprintf(err_buf, "Invalid device state %#x for attach",
			  mrvl_p->base.dev_state);
	DEV_ERROR_REPORT(dev, err_buf, (uint32_t)MRVL_GE_ATTACH);
	return (FAILED);
    }

    /* init the call in function */
    mrvl_p->callin_fvt->ge_register_test         = mvl_ge_reg_test;
    mrvl_p->callin_fvt->ge_phy_mac_lpbk_test     = mvl_ge_phy_mac_lpbk_test;
    mrvl_p->callin_fvt->ge_ext_lpbk_test         = mvl_ge_ext_lpbk_test;
    mrvl_p->callin_fvt->ge_phy_sfp_ext_lpbk_test = mvl_ge_phy_sfp_ext_lpbk_test;
    mrvl_p->callin_fvt->ge_phy_intr_test         = mvl_ge_intr_test;
    mrvl_p->callin_fvt->ge_phy_mac_lpbk_test_1gbps = mvl_ge_phy_mac_lpbk_test_1gbps;
    mrvl_p->callin_fvt->ge_ext_lpbk_test_1gbps     = mvl_ge_ext_lpbk_test_1gbps;

    mrvl_p->callin_fvt->util_ge_rd_reg           = util_mvl_ge_rd_reg;
    mrvl_p->callin_fvt->util_ge_wr_reg           = util_mvl_ge_wr_reg;
    mrvl_p->callin_fvt->util_ge_set_test_mode    = util_mvl_ge_set_test_mode;
    mrvl_p->callin_fvt->util_ge_set_tx_type      = util_mvl_ge_set_tx_type;
    mrvl_p->callin_fvt->util_ge_set_vod          = util_mvl_ge_set_vod;

    mrvl_p->callin_fvt->if_ge_rd_reg             = if_mvl_ge_rd_reg;
    mrvl_p->callin_fvt->if_ge_wr_reg             = if_mvl_ge_wr_reg;
    mrvl_p->callin_fvt->if_ge_set_tx_type        = if_mvl_ge_set_tx_type;
    /* Lock the SMI device */
    if ((rc = mrvl_p->callout_fvt->open(mrvl_p->smi_p)) != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_attach() SMI open failed with rc = %#x",
							rc);
        DEV_ERROR_REPORT(dev, err_buf, (uint32_t)MRVL_GE_ATTACH);
        return (FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************************
 * Function:    mrvl_ge_dev_detach()
 *
 * Description: detach the device specific functions from the caller.
 *		All of the device specific function are connected to the
 *		dev_do_nothing() function, except for the dev_attach()
 *		function. Also, the dev_state must be assigned the value
 *		of DEV_STATE_DETACH.
 *		Since, some platforms may want to detach the device, but not
 *		release the memory resources (via a free () in the
 *		dev_destroy()), this function can be executed to accomplish
 *		this task. However, before a detached device can be used again,
 *		it must be re-attached (via the dev_attach()).
 * Input:       Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ******************************************************************************/
static uint32_t mvl_ge_dev_detach (dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the SMI device */
    if ((rc = mrvl_p->callout_fvt->close(mrvl_p->smi_p)) != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_detach() SMI close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_DETACH);
	return (FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, mrvl_p->base.dev_object_fvt);

    mrvl_p->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*******************************************************************************
 * Function:    mvl_ge_dev_reconfig
 *
 * Description: To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 * Input:       dev_object_t pointer to the Marvell GE device
 *	        void * - a device/platform specific context handle
 *	        boolean * - a pointer to a boolean
 * Returns:     PASSED/FAILED, context information and a boolean value.
 *	        The boolean value shall be set to TRUE if the device must be
 *	        reconfigured from scratch and it shall be set to FALSE otherwise.
 * Assumptions: The dev_attach() function has been called and successfully
*************** *****************************************************************/
static uint32_t mvl_ge_dev_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    printf("%s:%d:No need to re-config\n", __FUNCTION__, __LINE__);
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return (PASSED);
}

/****************************************************************************
 * Function:    mvl_ge_dev_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 * Input:       dev_object_t pointer to the Marvell GE DEVICE
 * Returns:     PASSED/FAILED
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 ****************************************************************************/
static uint32_t mvl_ge_dev_restart(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *) dev;

    mrvl_p->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}

/*****************************************************************************
 * Function:    mvl_ge_dev_init()
 *
 * Description: Initializes the Marvell GE chip 
 * Input:       dev_object_t pointer to the Marvell GE device.
 *	        Caller has to setup the smi_p parameters with init values.
 * Returns:     PASSED/FAILED
 * Note:        Make sure base.dev_addr has been initialized to chip_base_addr
 *              before calling this function.
 *****************************************************************************/
static uint32_t mvl_ge_dev_init (dev_object_t *dev)
{
    uint32_t rc;
    smi_if_t smi_if;
    reg_info_t *init_p;
    dev_mrvl_reg_info_t *master_reg_p;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t smi_reg;
    char err_buf[ERR_BUF_SIZE];

    /* Setup the SMI API interface struct */
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;
    smi_if.buf = &smi_reg;

    /* Get the register info table */
    master_reg_p = mvl_ge_get_master_table(dev);
    if (master_reg_p == NULL_REG_TABLE) {
	DEV_ERROR_REPORT(dev, "Unable to get the table",
					MRVL_GE_INIT);
	return (FAILED);
    }

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup the page */
	smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
	smi_reg = master_reg_p->page;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "Page %#x write failed rc = %#x",
			smi_reg, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_INIT);
	    return (FAILED);
	}

	init_p = master_reg_p->reg_p;

	/* Call registers table init. */
	while(init_p->size.size != 0) {
	    smi_if.offset = init_p->offset;
	    smi_reg = init_p->reset_val; /* Use reset_val as init valure */

	    /* Write to the Marvell GE with platform default value */
	    rc = (*callout_p->wr)(&smi_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "Write %#x @ %#x pg %#x"
				 " rc = %#x", smi_reg, smi_if.offset,
				 master_reg_p->page, rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_INIT);
		return (FAILED);
	    }
	    init_p++;
	} /* endof while init_p */
	master_reg_p++;
    } /* endof master_reg_p */

    /* Reset to page 0 */
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
    smi_reg = MRV88E111N_REG_PAGE_0;

    rc = (*callout_p->wr)(&smi_if);
    if (rc != PASSED) {
	DEV_ERROR_REPORT(dev, "Unable to set to page 0",
				MRVL_GE_INIT);
	return (FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_INIT;

    return (PASSED);

}

/******************************************************************************
 * Function:    mvl_ge_dev_oper_en
 *
 * Description:	Enable device operation.
 *		Change the state of the device from its current state to an
 *		enabled state (which implies that the device is in an
 *		operational state at the end of this function execution). Also,
 *		the dev_state must be assigned the value of DEV_STATE_ENABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to enable all or only part of the total device port's
 *		or channel's.
 * Input:       dev_object_t pointer to the Marvell GE device
 * Returns:     PASSED/FAILED
 * Assumptions: The dev_init() function has been called and successfully
 ******************************************************************************/
static uint32_t mvl_ge_dev_oper_en(dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Take the GE device out of reset. The port (enum) to be enabled is in the
     * SMI interface struct */
    /* Enable the device */
    rc = (*callout_p->dev_reset)(ENABLE);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_oper_en() reset return code = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_RESET);
	return (FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_ENABLE_OP;

    return (PASSED);

}

/****************************************************************************
 * Name: mvl_ge_dev_oper_dis
 *
 * Description:	Disable device operation.
 *		Change the state of the device from its current state to an
 *		disabled state (which implies that the device is in a
 *		non-operational state at the end of this function execution).
 *		Also, the dev_state must be assigned the value of
 *		DEV_STATE_DISABLE_OP
 *		For devices such as port asic's and framers, this function
 *		be used to disable all the ports or channels of a specific
 *		device
 * Input:       dev_object_t pointer to the Marvell GE device
 * Returns:     PASSED/FAILED
 * Assumptions: The dev_init() function has been called and successfully
 ****************************************************************************/
static uint32_t mvl_ge_dev_oper_dis(dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Put the GE device into reset. The port (num) to be disabled is in the
     * SMI interface struct */
    /* Disable the device */
    rc = (*callout_p->dev_reset)(DISABLE);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_oper_dis() reset return code = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_RESET);
	return (FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_DISABLE_OP;

    return (PASSED);

}

/****************************************************************************
 * Name: mvl_ge_dev_intr_en
 *
 * Description: Enable the device interrupt(s).
 *		This function is expected to enable the interrupt(s) for a
 *		device with the expectation that all portions of the device
 *		are active, it is possible that some portions of a device are
 *		disabled.
 * Input:       dev_object_t pointer to the Marvell GE device
 * Returns:     PASSED/FAILED
 * Assumptions: The dev_init() function has been called and successfully
 ****************************************************************************/
static uint32_t mvl_ge_dev_intr_en (dev_object_t *dev)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page = MRV88E111N_REG_PAGE_3;
    uchar offset = MRV88E111L_P3_R16_FUNC_CONTROL_REG;
    smi_t smi_d;		/* Data bytes for SMI */
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Setup SMI API interface struct */
    smi_if.buf = &smi_d;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    /*==============================================================*/
    /*== setting GE PHY R3_P16 with data:0x0e1e, enable interrupt ==*/
    /*==============================================================*/
    printf("Catching GE PHY interrupt signal ...");
    /* write data */
    wr_buf = ENABLE_INTR; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* polling data */
    test_pattern = ENABLE_INTR;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: mvl_ge_dev_intr_dis
 *
 * Description: Disable the device interrupt(s).
 *		This function is expected to disable ALL of the device
 *		interrupt(s).
 * Input:       dev_object_t pointer to the Marvell GE device
 * Returns:     PASSED/FAILED
 * Assumptions: The dev_init() function has been called and successfully
 ***********************************************************************/
static uint32_t mvl_ge_dev_intr_dis (dev_object_t *dev)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page = MRV88E111N_REG_PAGE_3;
    uchar offset = MRV88E111L_P3_R16_FUNC_CONTROL_REG;

    smi_t smi_d;	/* Data bytes for SMI */
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Setup SMI API interface struct */
    smi_d = 0;		/* Disable all interrupts */
    smi_if.buf = &smi_d;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    /*===============================================================*/
    /*== setting GE PHY R3_P16 with data:0x021e, disable interrupt ==*/
    /*===============================================================*/
    printf("Setting GE PHY INT Pin(Pin.61) as de-asserted(low) ...");
    /* write data */
    wr_buf = DISABLE_INTR; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* polling data */
    test_pattern = DISABLE_INTR;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Name:        mvl_ge_dev_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 * Input:       dev_object_t pointer to the Marvell GE device
 *	        A device print function vector
 *	        A dev_show_cmd_e command
 * Returns:     PASSED/FAILED
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 ******************************************************************************/
static uint32_t mvl_ge_dev_show (dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32_t ix, rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d, original_page_reg;
    reg_info_t *reg_p;
    dev_mrvl_reg_info_t  *master_reg_p;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */

    /* Get the original Page register */
    smi_if.buf = &original_page_reg;
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_show() Page reg read failed. rc = %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	return (FAILED);
    }

    smi_if.buf = &reg_d;	/* Buffer address */

    /* Get the register info table */
    master_reg_p = mvl_ge_get_master_table(dev);
    if (master_reg_p == NULL_REG_TABLE) {
	DEV_ERROR_REPORT(dev, "mvl_ge_dev_show() unable to get the table",
					MRVL_GE_SHOW);
	return (FAILED);
    }

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup the page */
	smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
	reg_d = master_reg_p->page;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    /* If we cannot write to the page register, then we cannot recover
	     * the page register.
	     */
	    sprintf(err_buf, "mvl_ge_dev_show() Page %#x write failed rc %#x",
					reg_d, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	    return (FAILED);
	}

	/* Page set. Ready to show all registers in the page */
	reg_p = master_reg_p->reg_p;
	dev_print("\n Page %d:\n", master_reg_p->page);

	switch (cmd) {
	case DEV_SHOW_ALL:
	case DEV_SHOW_CONFIG:
	case DEV_SHOW_REGISTERS:
	    while (reg_p->size.size != 0) {
		/* Read the data bytes of Marvell GE */
		smi_if.offset = reg_p->offset;
		rc = (*callout_p->rd)(&smi_if);

		if (rc != PASSED) {
		    sprintf(err_buf, "mvl_ge_dev_show() %s register read failed"
				     " rc = %#x", reg_p->name, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
		    return (FAILED);
		}

		dev_print("%s @ %#.2x = %#.4x\n",
		       reg_p->name, reg_p->offset, reg_d);
		reg_p++;
	    } /* endof while */
	    break;
	case DEV_SHOW_BRIEF:
	    for (ix = 0; ix < MAX_SMI_REGS; ix++) {
		/* Read the data bytes of Marvell GE */
		smi_if.offset = ix;
		rc = (*callout_p->rd)(&smi_if);

		if (rc != PASSED) {
		    sprintf(err_buf, "mvl_ge_dev_show() brief read @ %x failed"
				     " rc = %#x", ix, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
		    return (FAILED);
		}
		dev_print("%#.4x ", reg_d);
		if ((ix & SMI_SHOW_MASK) == SMI_SHOW_MASK) {
		    dev_print("\n");
		}
	    } /* endof for */
	    break;
	default:
	    assert(!"mvl_ge_dev_show");
	    return (FAILED);
	    break;
	} /* endof switch */
	master_reg_p++;		/* Point to the next page */
    } /* endof while */

    /* Restore the page register */
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_show() Page reg restore failed rc %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Name:        mvl_ge_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 * Input:       dev_object_t pointer to the Marvell GE device
 *              A crash print function vector.
 *              A verbosity level.
 * Returns:     PASSED/FAILED
 * Assumptions: A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 ******************************************************************************/
static uint32_t mvl_ge_crsh (dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("mvl_ge_crsh(): No Crash info available for Marvell GE\n");
    return (PASSED);
}

/***************************************************************************
 * Name: mvl_ge_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 * Input:       dev_object_t pointer to the Marvell GE device
 * Returns:     none
 * Assumptions: The dev_attch() function has been called and successfully
 ***************************************************************************/
static void mvl_ge_destroy(dev_object_t **dev)
{
    dev_mrvl_ge_object_t *mrvl_p;
    uint32_t rc;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    mrvl_p = (dev_mrvl_ge_object_t *)*dev;

    /* Unlock the SMI device */
    if ((rc = mrvl_p->callout_fvt->close(mrvl_p->smi_p)) != PASSED) {
	sprintf(err_buf, "mvl_ge_destroy() SMI close return code = %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, MRVL_GE_DESTROY);
    }

    /* Free callout struct */
    if (mrvl_p->callout_fvt) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("freeing callout @ %p\n", mrvl_p->callout_fvt);
        }
	free(mrvl_p->callout_fvt);
    }

    /* Free callin struct */
    if (mrvl_p->callin_fvt) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("freeing callin @ %p\n", mrvl_p->callin_fvt);
        }
	free(mrvl_p->callin_fvt);
    }

    /* Free dev_object_t */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("freeing object @ %p\n", mrvl_p->base.dev_object_fvt);
    }
    free(mrvl_p->base.dev_object_fvt);

}


/************************************************************************************
 * Function:    mvl_ge_smi_rd
 *
 * Description: reads Marvell GE registers
 * Input:       addr  - offset of register to be read.
 *	        size  - ignored. For SMI, it is always 2.
 *	        buf   - points to the data buffer to be read. Note that this is a
 *                      pointer to ulong. SMI data is always 2 bytes long. Either
 *                      set SMI interface buf field to position at the lower 2 bytes
 *                      of the ulong, or use a working smi_t (2 bytes), then transfer
 *                      the resulting data to the ulong.
 *              param - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ************************************************************************************/
static int mvl_ge_smi_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32_t rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)param;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t smi_buf;
    char err_buf[ERR_BUF_SIZE];

    /* Setup the interface struct for SMI API read */
    smi_if.offset = (uint8_t)addr;
    smi_if.buf = &smi_buf;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    /* Call the SMI Read API */
    rc = (*callout_p->rd)(&smi_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "mvl_ge_smi_rd() read return code = %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, MRVL_GE_READ);
	return (FAILED);
    } else {
        *buf = smi_buf;
	    return (PASSED);
    }
}

/*********************************************************************************
 * Function: mvl_ge_smi_wr
 *
 * Description: writes Marvell GE registers
 * Input:       addr - offset of register to be written.
 *              size - Ignored. For SMI, it is always 2.
 *              data - write data. Note that SMI interface uses 2 bytes (smi_t).
 *                     similar to the read, we can either set the smi_if_t.buf
 *                     to point at the position of the lower 2 bytes of ulong, or
 *                     use a working smi_t area, and transfer the ulong data to the
 *                     smi_t work area.
 *              param - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 **********************************************************************************/
static int mvl_ge_smi_wr(ulong addr, int size, ulong data, void *param)
{
    uint32_t rc;
    smi_t smi_buf;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)param;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Setup the interface struct for SMI API write */
    smi_if.offset = (uint8_t)addr;
    smi_if.buf = &smi_buf;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    smi_buf = (smi_t)data;	/* Get 2 bytes */

    /* Call the SMI Write API */
    rc = (*callout_p->wr)(&smi_if);
    if (rc != PASSED) {
	/* Write failed */
	sprintf(err_buf, "mvl_ge_smi_wr() write return code = %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, MRVL_GE_WRITE);
	return (FAILED);
    } else {
	/* Data written */
	return (PASSED);
    }
}

/****************************************************************************
 * Function:	mvl_ge_get_master_table
 *
 * Description: returns the pointer to the beginning of register table array.
 * Input:	dev_object_t pointer to the Marvell GE device
 * Returns:	Pointer to the beginning of the register table array.
 ****************************************************************************/
static dev_mrvl_reg_info_t *mvl_ge_get_master_table(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Check if the user provided the table */
    if (mrvl_p->reg_info_p) {
	/* User provided the table */
	return (mrvl_p->reg_info_p);
    } else {
	/* Use default table provided in this file */
	switch(mrvl_p->type) {
	case MRVL_GE_PHY_1112:
	    /* 88E1112 Copper and Fiber */
	    return (&ge_1112_reg_tbl[0]);
	    break;
	default:
	    /* Unknown type */
	    assert(!"mvl_ge_get_master_table");
	    return (NULL_REG_TABLE);
	    break;
	} /* endof switch */
    }
}

/*****************************************************
 * Function:    mvl_sfp_setup
 *
 * Description: it will call a callout funciton to 
 *              setup SFP module
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_sfp_setup(dev_object_t *dev)
{
    int rsv = 0;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->sfp_setup)(rsv));
}

/*****************************************************
 * Function:    mvl_sfp_get_media_mode
 *
 * Description: it will call a callout function to 
 *              get the exact media mode
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_sfp_get_media_mode(dev_object_t *dev, smi_t *wanted_media_mode)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->sfp_get_media_mode)(wanted_media_mode));
}
/*****************************************************
 * Function:    mvl_sfp_tx_disable
 *
 * Description: it will call a callout function to 
 *              disable SFP TX transmitter 
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_sfp_tx_disable(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->sfp_tx_disable)());
}

/************************************************
 * Function:    mvl_sfp_tx_enable
 *
 * Description: it will call a callout function to 
 *              enable SFP TX transmitter 
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 ************************************************/
static int mvl_sfp_tx_enable(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->sfp_tx_enable)());
}

/**********************************************************
 * Function:    mvl_cpu_mac_setup
 *
 * Description: setup CPU side MAC for MAC loopback test
 * Input:       dev      - pointer to the Marvell GE device 
 *              test_spd - test speed
 * Returns: PASSED/FAILED
 **********************************************************/
static int mvl_cpu_mac_setup(dev_object_t *dev, int test_spd)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->cpu_phy_mac_setup)(test_spd));
}

/*****************************************************************
 * Function:    mvl_cpu_phy_mac_check_linkup
 *
 * Description: checking CPU side MAC is linkup to GE PHY side MAC
 * Input:       dev      - pointer to the Marvell GE device 
 * Returns: PASSED/FAILED
 *****************************************************************/
static int mvl_cpu_phy_mac_check_linkup(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->cpu_phy_mac_check_linkup)());
}

/**********************************************************
 * Function:    mvl_cpu_ext_lpbk_init
 *
 * Description: init CPU side for external loopback test
 * Input:       dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 **********************************************************/
static int mvl_cpu_ext_lpbk_init(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->cpu_phy_mac_autoneg_setup)());
}

/**********************************************************
 * Function:    mvl_ge_phy_tx_rx_test
 *
 * Description: run loopback test
 * Input:       dev      - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 **********************************************************/
static int mvl_ge_phy_tx_rx_test(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    return ((*callout_p->ge_phy_tx_rx_test)());
}

/************************************************************
 * Function:    mvl_ge_rd_wr_reg
 *
 * Description: reads or writes a register of specified page.
 * Input:	dev    - pointer to the Marvell GE device.
 *		op     - PHY_READ or PHY_WRITE
 *		page   - Page number.
 *		offset - register offset.
 *		data   - Points to data of read/write
 * Returns:     PASSED/FAILED
*************************************************************/
static int mvl_ge_rd_wr_reg (dev_object_t *dev, uint op, smi_t page, uchar offset, smi_t *data)
{
    uint32_t rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */

    reg_d = page;
    smi_if.buf = &reg_d;
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
    /* 
     * Before reading or writing a specific register, there're two steps as below:
     * (1) Writing the page number into "Reg. 22, Page Address Register" by callout function first.
     * (2) Then read/write data by callout function.
     */
    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_rd_wr_reg() set page %#x failed. rc = %#x",
				page, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_PAGE);
	return (FAILED);
    }

    smi_if.buf = data;
    smi_if.offset = offset;

    if (op == PHY_READ) {
	rc = (*callout_p->rd)(&smi_if);
    } else {
	rc = (*callout_p->wr)(&smi_if);
    }

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_rd_wr_reg() op = %#x reg @ %#x failed. "
			 " rc = %#x", op, offset, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_PAGE);
    }

    return (rc);
}

/*******************************************************************
 * Function:    mvl_ge_polling_reg
 *
 * Description: Polling a specific register.
 * Input:	dev              - pointer to the Marvell GE device.
 *              compare_op       - COMPARE_AND/COMPARE_EQL
 *		page             - Page number.
 *		offset           - register offset.
 *		pattern          - a test pattern for data checking
 *		max_polling_time - max poling time
 * Returns:     PASSED/FAILED
 *******************************************************************/
static int mvl_ge_polling_reg (dev_object_t *dev, int compare_op, smi_t page, uchar offset, smi_t pattern, int max_polling_time)
{
    int rc = FAILED, polling_rc = FAILED, ix;
    smi_t rd_buf;
    
    for (ix = 0; ix < MAX_POLLING_ROUND; ix++)
    {
        rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
        if (rc != PASSED) {
            printf("%s:%d: Polling: PHY PAGE:%d REG:%d  fail\n", 
                   __FUNCTION__, __LINE__, page, offset);
            return (FAILED);
        }

        /* compare data with pattern */
        if (compare_op == COMPARE_AND) {
            if ((rd_buf & pattern) == 0) {
                polling_rc = PASSED;
                break;
            }
        } else if(compare_op == COMPARE_EQL) {
            if (rd_buf == pattern) {
                polling_rc = PASSED;
                break;
            }
        } else {
            if ((rd_buf & pattern) == pattern) {
                polling_rc = PASSED;
                break;
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] The inner PHY SMI bus is busy\n", __FUNCTION__, __LINE__);
        }

        /* put a delay for hardware preparation */
        msleep(POLLING_PERIOD);
    }
    
    /* checking polling result */ 
    if (polling_rc != PASSED) {
            printf("%s:%d: Polling: PHY PAGE:%d REG:%d  fail, pattern:0x%04x, read data:0x%04x\n", 
                   __FUNCTION__, __LINE__, page, offset, pattern, rd_buf);
            return (FAILED);
    }

    return (polling_rc);
}

/*****************************************************************
 * Function:    mvl_ge_mscr1_get_phy_media_mode
 * Note:        mscr1:MAC Specific Control Register 1
 *
 * Description: Get the original media mode from 
 *              Page 2 Reg 16 - MAC Specific Control Register 1 
 * Input:	dev            - pointer to the Marvell GE device.
 *              ori_media_mode - original media mode
 * Returns:     PASSED/FAILED
 *****************************************************************/
static int mvl_ge_mscr1_get_phy_media_mode (dev_object_t *dev, smi_t *current_media_mode)
{
    uint32_t rc;
    smi_t page   = MRV88E111N_REG_PAGE_2;
    uchar offset = MRV88E111M_P2_R16_SPECIFIC_CONTROL1_REG;

    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, current_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:2 REG:16, Read and store current GEWAN PHY media mode fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/**************************************************************
 * Function:    mvl_ge_mscr1_set_phy_media_mode
 * Note:        mscr1:MAC Specific Control Register 1
 *
 * Description: Set the media to  
 *              Page 2 Reg 16 - MAC Specific Control Register 1 
 * Input:	dev        - pointer to the Marvell GE device.
 *              media_mode - wanted media mode
 * Returns:     PASSED/FAILED
 **************************************************************/
static int mvl_ge_mscr1_set_phy_media_mode (dev_object_t *dev, smi_t media_mode)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page   = MRV88E111N_REG_PAGE_2;
    uchar offset = MRV88E111M_P2_R16_SPECIFIC_CONTROL1_REG;
 
    /* Config Mode select field Bit[9:7] in MAC Specific Control Register 1. 
     * Bit[9:7]=000, 1000BASE-FX
     * Bit[9:7]=001, Copper GBIC 
     * Bit[9:7]=010, Auto Copper/SGMII memdia interface
     * Bit[9:7]=011, Auto Copper/1000BASE-X
     * Bit[9:7]=100, Reserved
     * Bit[9:7]=101, Copper Only
     * Bit[9:7]=110, SGMII media interface only
     * Bit[9:7]=111, 1000BASE-X only 
     */
    wr_buf = media_mode;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:%d REG:%d, Set Marvell GE PHY MAC fail\n", __FUNCTION__, __LINE__,page,offset);
        return (FAILED);
    }

    /* confirm the new value */
    test_pattern = wr_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Marvell GE PHY MAC setting fail\n", __FUNCTION__, __LINE__,page,offset);
        return (FAILED);
    }

    page   = MRV88E111N_REG_PAGE_2;
    offset = MRV88E111N_P0_R0_CONTROL_REG;

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: page:%d offset%d  fail\n", __FUNCTION__, __LINE__,page,offset);
        return (FAILED);
    }
    /* reset */
    wr_buf = rd_buf | MRV88E111M_PX_R0_B15_RESET;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: page:%d offset%d  fail\n", __FUNCTION__, __LINE__,page,offset);
        return (FAILED);
    }

    /* check reset is finished, Loopbacl filed(Bit[14]) will return to 0 after reset done */
    test_pattern = rd_buf &                            /* original value                   */
                   MRV88E111M_PX_R0_B15_RESET_DONE &   /* check Reset field return to 0    */
                   MRV88E111N_PX_R0_B14_LPBK_ENA_MASK; /* check Loopback field return to 0 */
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: page:%d offset%d  fail\n", __FUNCTION__, __LINE__,page,offset);
        return (FAILED);
    }

    return (PASSED);
}

/*********************************************************
 * Function:    mvl_ge_xcr_soft_reset
 * Note:        xcr indicate below register:
 *              (1)Copper Control Register (Page 0 Reg 0)
 *              (2)Fiber Control Register  (Page 1 Reg 0)
 *              (3)MAC Control Register    (Page 2 Reg 0)
 *
 * Description: Set the media to specific Control Register 
 * Input:	dev  - pointer to the Marvell GE device.
 *              type - Copper, Fiber or MAC
 * Returns:     PASSED/FAILED
**********************************************************/
static int mvl_ge_xcr_soft_reset (dev_object_t *dev, dev_mrvl_page_type_t type)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page;
    uchar offset;

    switch (type) {
        case MRVL_COPPER_TYPE:
            page   = MRV88E111N_REG_PAGE_0;
            offset = MRV88E111N_P0_R0_CONTROL_REG;
        break;

        case MRVL_FIBER_TYPE:
            page   = MRV88E111N_REG_PAGE_1;
            offset = MRV88E111F_P1_R0_CONTROL_REG;
        break;

        case MRVL_MAC_TYPE:
            page   = MRV88E111N_REG_PAGE_2;
            offset = MRV88E111M_P2_R0_CONTROL_REG;
        break;
       
        default:
            printf("%s:%d: Unsupported page type GE PHY soft-reset\n", __FUNCTION__, __LINE__);
        break;
    }

    wr_buf = 0x0; /* clear all fields as default */
    wr_buf |=  MRV88E111M_PX_R0_B15_RESET;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:0 REG:0, Set register data fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    test_pattern = 0x0;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:0 REG:0, Checking register data fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    msleep(MRV88E1112_SOFT_RST_BUFFERING);

    return (PASSED);
}

/*******************************************************
 * Function:    mvl_ge_cscr1_disable_linkgood
 * Note:        cscr1:Copper Specific Control Register 1
 *
 * Description: Disable Force Link Good in Copper
 * Input:	dev - pointer to the Marvell GE device.
 * Returns:     PASSED/FAILED
 *******************************************************/
static int mvl_ge_cscr1_disable_linkgood (dev_object_t *dev)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page   = MRV88E111N_REG_PAGE_0;
    uchar offset = MRV88E111N_P0_R16_SPECIFIC_CONTROL1_REG;

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d,Read original value fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    
    /* disable Force Link Good field(Bit[10]) */
    wr_buf = rd_buf &                               /* original value */
             MRV88E111M_P0_R16_FORCE_LINK_GOOD_DIS; /* Bit[10]=0      */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Disable Copper Force Link Good fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    
    /* confirm the new value */
    test_pattern = wr_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Copper Force Link Good fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/***********************************************************
 * Function:    mvl_ge_mcr_set_media_spd
 * Note:        mcr:MAC Control Register
 *
 * Description: Set the media speed to MAC Control Register 
 * Input:	dev      - pointer to the Marvell GE device.
 *              test_spd - 10Mbps/100Mbps/1000Mbps
 * Returns:     PASSED/FAILED
 ***********************************************************/
static int mvl_ge_mcr_set_media_spd(dev_object_t *dev, int test_spd)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern, media_spd;
    smi_t page   = MRV88E111N_REG_PAGE_2;
    uchar offset = MRV88E111M_P2_R0_CONTROL_REG;
 
    switch (test_spd) {
        case MRV88E1112_PHY_COP_SPD_10MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_10Mbps_MSB    | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_10Mbps_LSB; 
        break;

        case MRV88E1112_PHY_COP_SPD_100MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_100Mbps_MSB   | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_100Mbps_LSB; 
        break;

        case MRV88E1112_PHY_COP_SPD_1000MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_1000Mbps_MSB  | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_1000Mbps_LSB; 
        break;

        default:
            printf("%s:%d:Unsupported test speed\n",__FUNCTION__, __LINE__);
        break;
    }

    /* Set Reset field(Bit[15]) and Speed Select field(Bit[6,13]) in Control Register, 
     * 10Mbps:   Bit[6,13]=00, 
     * 100Mbps:  Bit[6,13]=01, 
     * 1000Mbps: Bit[6,13]=10 */
    wr_buf = MRV88E111M_PX_R0_B15_RESET | media_spd; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:2 REG:0, Set Marvell GE PHY mediag speed fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* confirm the new value */
    test_pattern = media_spd;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:2 REG:0, Checking Marvell GE PHY mediag speed fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/**************************************************************
 * Function:    mvl_ge_mcr_set_default
 * Note:        mcr:MAC Control Register
 *
 * Description: Function to set MAC Control Register as default 
 * Input:	dev - pointer to the Marvell GE device.
 * Returns:     PASSED/FAILED
 **************************************************************/
static int mvl_ge_mcr_set_default (dev_object_t *dev)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page   = MRV88E111N_REG_PAGE_2;
    uchar offset = MRV88E111M_P2_R0_CONTROL_REG;

    /* Enable Auto-Nego(Bit[12]=1), 
     * set speed as 1000Mbps(Bit[6,13]=10) */
    wr_buf = MRV88E111M_PX_R0_B12_AUTO_NEGO_ENA |
             MRV88E111M_PX_R0_B13_MEDIA_SPD_1000Mbps_LSB |
             MRV88E111M_PX_R0_B6_MEDIA_SPD_1000Mbps_MSB;

    test_pattern  = wr_buf;

    wr_buf |= MRV88E111M_PX_R0_B15_RESET; /* reset */
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:%d REG:%d, Set MAC as default fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
   
    /* confirm the new value */
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************
 * Function:    mvl_ge_ccr_set_default
 * Note:        ccr:Copper Control Register
 *
 * Description: Function to set Copper Control Register as default 
 * Input:	dev - pointer to the Marvell GE device.
 * Returns:     PASSED/FAILED
 *****************************************************************/
static int mvl_ge_ccr_set_default (dev_object_t *dev)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page   = MRV88E111N_REG_PAGE_0;
    uchar offset = MRV88E111N_P0_R0_CONTROL_REG;

    /* Enable Auto-Nego(Bit[12]=1), 
     * set speed as 1000Mbps(Bit[6,13]=10) */
    wr_buf = MRV88E111M_PX_R0_B12_AUTO_NEGO_ENA |
             MRV88E111M_PX_R0_B13_MEDIA_SPD_1000Mbps_LSB |
             MRV88E111M_PX_R0_B6_MEDIA_SPD_1000Mbps_MSB;

    test_pattern  = wr_buf;

    wr_buf |= MRV88E111M_PX_R0_B15_RESET; /* reset */
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:%d REG:%d, Set Copper as default fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
   
    /* confirm the new value */
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/************************************************************************
 * Function:    mvl_ge_ccr_ext_lpbk_spd_config
 * Note:        ccr:Copper Control Register
 *
 * Description: Function to config media speed for external loopback test 
 * Input:	dev      - pointer to the Marvell GE device.
 *              test_spd - 100Mbps/100Mbps/1000Mbps
 * Returns:     PASSED/FAILED
 ************************************************************************/
static int mvl_ge_ccr_ext_lpbk_spd_config(dev_object_t *dev, int test_spd)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern, media_spd;
    smi_t page   = MRV88E111N_REG_PAGE_0;
    uchar offset = MRV88E111N_P0_R0_CONTROL_REG;
 
    /* Config Speed Select field(Bit[6,13]) in Copper Control Register, 
     * 10Mbps:   Bit[6,13]=00, 
     * 100Mbps:  Bit[6,13]=01, 
     * 1000Mbps: Bit[6,13]=10 */
    switch (test_spd) {
        case MRV88E1112_PHY_COP_SPD_10MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_10Mbps_MSB    | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_10Mbps_LSB; 
        break;

        case MRV88E1112_PHY_COP_SPD_100MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_100Mbps_MSB   | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_100Mbps_LSB; 
        break;

        case MRV88E1112_PHY_COP_SPD_1000MBPS:
            media_spd = MRV88E111M_PX_R0_B6_MEDIA_SPD_1000Mbps_MSB  | 
                        MRV88E111M_PX_R0_B13_MEDIA_SPD_1000Mbps_LSB; 
        break;

        default:
            printf("%s:%d:Unsupported test speed\n",__FUNCTION__, __LINE__);
        break;
    }
  
    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Read Copper Control Register fail\n", __FUNCTION__, __LINE__, page,offset);
        return (FAILED);
    }
    
    wr_buf  = rd_buf |                                /* orginal value                    */
              MRV88E111M_P0_R0_B8_FULL_DUPLEX;        /* Bit[8],    set it as full-duplex */
    wr_buf &= MRV88E111M_PX_R0_B6_B13_MEDIA_SPD_MASK; /* Bit[6,13], set it as default     */
    wr_buf |= media_spd;                              /* Bit[6,13], set new media speed   */

    /* config Auto-Negotiation Enable field(Bit[12]) in Copper Control Register,
     * Enable: set Bit[12] = 1, Disable: set Bit[12] = 0 */
    if(test_spd == MRV88E1112_PHY_COP_SPD_1000MBPS) {
        wr_buf |= MRV88E111M_PX_R0_B12_AUTO_NEGO_ENA; /* Bit[12],   Enable auto-nego.     */
    } else {
        wr_buf &= MRV88E111M_PX_R0_B12_AUTO_NEGO_DIS; /* Bit[12],   Disable auto-nego.    */
    }

    test_pattern = wr_buf;

    wr_buf |= MRV88E111M_PX_R0_B15_RESET;             /* Bit[15],   Reset                  */
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:%d REG:%d, Enable Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    sleep(MRV88E1112_SPD_CFG_BUFFERING);
    /* confirm the new value */
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}
/********************************************************************
 * Function:    mvl_ge_xcr_enable_lpbk
 * Note:        xcr indicate below register:
 *              (1)Copper Control Register (Page 0 Reg 0)
 *              (2)Fiber Control Register  (Page 1 Reg 0)
 *              (3)MAC Control Register    (Page 2 Reg 0)
 *
 * Description: Enable loopback funciton to specific Control Register 
 * Input:	dev  - pointer to the Marvell GE device.
 *              type - Copper, Fiber or MAC
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mvl_ge_xcr_enable_lpbk(dev_object_t *dev, dev_mrvl_page_type_t type)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page;
    uchar offset;

    switch (type) {
        case MRVL_COPPER_TYPE:
            page   = MRV88E111N_REG_PAGE_0;
            offset = MRV88E111N_P0_R0_CONTROL_REG;
        break;

        case MRVL_FIBER_TYPE:
            page   = MRV88E111N_REG_PAGE_1;
            offset = MRV88E111F_P1_R0_CONTROL_REG;
        break;

        case MRVL_MAC_TYPE:
            page   = MRV88E111N_REG_PAGE_2;
            offset = MRV88E111M_P2_R0_CONTROL_REG;
        break;
       
        default:
            printf("%s:%d: Unsupported page type while checking loopback enable\n", __FUNCTION__, __LINE__);
        break;
    }

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:0 REG:0, Read Copper Control Register fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Enable Loopback field(Bit[14]), set this field = 1 */
    wr_buf = rd_buf | MRV88E111N_PX_R0_B14_LPBK_ENA ;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:0 REG:0, Enable Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* confirm the new value */
    test_pattern = wr_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, MRV88E111N_REG_PAGE_0, MRV88E111N_P0_R0_CONTROL_REG, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:0 REG:0, Checking Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/***********************************************************************************
 * Function:    mvl_ge_xcr_power_control
 * Note:        xssr indicate below register:
 *              (1)Copper Specific Status Register (Page 0 Reg 17)
 *              (2)Fiber Specific Status Register  (Page 1 Reg 17)
 *              (3)MAC Specific Status Register    (Page 2 Reg 17)
 *
 * Description: Power Control funciton for Copper/Fiber/MAC Specific Status Register 
 * Input:	dev        - pointer to the Marvell GE device.
 *              type       - Copper, Fiber or MAC
 *              pwr_on_off - MRVL_PWR_ON/MRVL_PWR_OFF 
 * Returns:     PASSED/FAILED
************************************************************************************/
static int mvl_ge_xcr_power_control(dev_object_t *dev, dev_mrvl_page_type_t type, dev_mrvl_pwr_type_t pwr_on_off) 
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page;
    uchar offset;
    switch (type) {
        case MRVL_COPPER_TYPE:
            page   = MRV88E111N_REG_PAGE_0;
            offset = MRV88E111N_P0_R0_CONTROL_REG;
        break;

        case MRVL_FIBER_TYPE:
            page   = MRV88E111N_REG_PAGE_1;
            offset = MRV88E111F_P1_R0_CONTROL_REG;
        break;

        case MRVL_MAC_TYPE:
            page   = MRV88E111N_REG_PAGE_2;
            offset = MRV88E111M_P2_R0_CONTROL_REG;
        break;
       
        default:
            printf("%s:%d: Unsupported page type while checking GE PHY link up\n", __FUNCTION__, __LINE__);
        break;
    }

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Read Control Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /* Power ON:  set Power Down field(Bit[11]) = 0 
     * Power OFF: set Power Down field(Bit[11]) = 1 */
    wr_buf = (pwr_on_off == MRVL_PWR_ON)? (rd_buf & MRV88E111M_PX_R0_B11_PWR_ON):
                                          (rd_buf | MRV88E111M_PX_R0_B11_PWR_OFF);
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Write Control Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    msleep(MRV88E1112_PWR_CFG_BUFFERING);
    /* confirm the new value */
    test_pattern = wr_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************************
 * Function:    mvl_ge_xssr_check_linkup
 * Note:        xssr indicate below register:
 *              (1)Copper Specific Status Register (Page 0 Reg 17)
 *              (2)Fiber Specific Status Register  (Page 1 Reg 17)
 *              (3)MAC Specific Status Register    (Page 2 Reg 17)
 *
 * Description: Check link status from  Copper/Fiber/MAC Specific Status Register 
 * Input:	dev  - pointer to the Marvell GE device.
 *              type - Copper, Fiber or MAC
 * Returns:     PASSED/FAILED
 ********************************************************************************/
static int mvl_ge_xssr_check_linkup(dev_object_t *dev, dev_mrvl_page_type_t type)
{
    uint32_t rc;
    smi_t rd_buf, test_pattern;
    smi_t page;
    uchar offset;

    switch (type) {
        case MRVL_COPPER_TYPE:
            page   = MRV88E111N_REG_PAGE_0;
            offset = MRV88E111N_P0_R17_SPECIFIC_STATUS1_REG;
        break;

        case MRVL_FIBER_TYPE:
            page   = MRV88E111N_REG_PAGE_1;
            offset = MRV88E111F_P1_R17_SPECIFIC_STATUS1_REG;
        break;

        case MRVL_MAC_TYPE:
            page   = MRV88E111N_REG_PAGE_2;
            offset = MRV88E111M_P2_R17_SPECIFIC_STATUS1_REG;
        break;
       
        default:
            printf("%s:%d: Unsupported page type while checking GE PHY link up\n", __FUNCTION__, __LINE__);
        break;
    }

    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Read Specific Status Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /* check link status field(Bit[10]) is 1 */
    test_pattern = MRV88E111M_PX_R17_B10_LINKUP;
    rc = mvl_ge_polling_reg(dev, COMPARE_AND_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Marvell GE PHY loopback fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************************************
 * Function:    mvl_ena_stub_test
 *
 * Description: Config Stub Test field(Bit[5]) in Packet Generation Register(Page 6 Reg 16), 
 *              For 10/100Mbps, P6_R16_Bit[5] = 0, disable stub test
 *              For 1000Mbps,   P6_R16_Bit[5] = 1, enable stub test
 * Input:	dev    - pointer to the Marvell GE device.
 *              option - MRVL_ENABLE/MRVL_DISABLE
 * Returns:     PASSED/FAILED
 ********************************************************************************************/
static int mvl_ena_stub_test (dev_object_t *dev, dev_mrvl_option_type_t option)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page   = MRV88E111N_REG_PAGE_6;
    uchar offset = MRV88E111N_P6_R16_PACKET_GEN_REG;
  
    /* read original value */    
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Read: PHY PAGE:%d REG:%d, Read Packet Generation Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
     
    /* set Bit[5] to config stub test field */   
    wr_buf = (option == MRVL_ENABLE)? (rd_buf | MRV88E111M_P6_R16_B5_STUB_ENA):(rd_buf & MRV88E111M_P6_R16_B5_STUB_DIS);
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Write: PHY PAGE:%d REG:%d, Write Packet Generation Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /* confirm the new value */
    test_pattern = wr_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Packet Generation Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************
 * Function:    mvl_ge_env_init
 *
 * Description: (1)Force link down CPU MAC and set it as default.
 *              (2)Force link down GE PHY MAC and set it as default.
 * Input :      dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 *******************************************************************/
static int mvl_ge_env_init(dev_object_t *dev)
{
    uint32_t rc;

    /*=====================================================*/
    /*== [Pre-setting] Init CPU for External Lpbk Test ====*/
    /*=====================================================*/
    rc = mvl_cpu_ext_lpbk_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init CPU MAC for ext. lpbk test\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=====================================================*/
    /*== [Pre-setting] Disable force link good in Copper ==*/
    /*=====================================================*/
    rc = mvl_ge_cscr1_disable_linkgood(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to disable force link good in Copper\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=============================================*/
    /*== [Pre-setting] Set MAC/Copper as default ==*/
    /*=============================================*/
    rc = mvl_ge_mcr_set_default(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set MAC as default\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    rc = mvl_ge_ccr_set_default(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set Copper as default\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/*************************************************************************
 * Function:    mvl_ge_reg_test
 *
 * Description: tests Marvell GE registers. Also check the ID of the chip.
 * Input :      dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 *************************************************************************/
static int mvl_ge_reg_test(dev_object_t *dev)
{
    uint32_t rc;
    smi_if_t smi_if, *original_smi_if;
    smi_t data, original_page_reg;
    reg_info_t *reg_ptr;
    dev_mrvl_reg_info_t *master_reg_p;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */
    smi_if.buf = &data;				/* setup test buffer */
    smi_if.offset = MRV88E111N_PA_R2_PHYID_1_REG;	/* ID offset */

    original_smi_if = mrvl_p->smi_p;	/* Save the SMI if */
    mrvl_p->smi_p = &smi_if;		/* use local SMI if */

    /* Read the ID data byte of Marvell GE */
    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
        mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
        sprintf(err_buf, "mvl_ge_reg_test() ID read failed with %#x.", rc);
        DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
        return (FAILED);
    }

    if (data != MRV88E111N_PHY_ID_1_VALUE) {
        /* ID does not match to the chip type */
        mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
        sprintf(err_buf, "mvl_ge_reg_test() ID read 0x%02x. Expect 0x%02x.",
                data, MRV88E111N_PHY_ID_1_VALUE);
        DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
        return (FAILED);
    }

    /* Get the register info table */
    master_reg_p = mvl_ge_get_master_table(dev);
    if (master_reg_p == NULL_REG_TABLE) {
        mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
        DEV_ERROR_REPORT(dev, "mvl_ge_reg_test() unable to get the table",
                         MRVL_GE_REG_TEST);
        return (FAILED);
    }

    /* Save the page register */
    smi_if.buf = &original_page_reg;
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
        mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
        sprintf(err_buf, "mvl_ge_reg_test() Page reg read failed. rc = %#x",
					rc);
        DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
        return (FAILED);
    }

    smi_if.buf = &data;		/* setup test buffer again */

    /* Setup the parameter for the registers test */
    reg_ext.param = (void *)dev;

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup page */
	smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
	data = master_reg_p->page;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	    /* If we cannot write to the page register, then we cannot recover
	     * the page register.
	     */
	    sprintf(err_buf, "mvl_ge_reg_test() Page %#x write failed rc %#x",
					data, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	    return (FAILED);
	}

	/* registers_test() will call mvl_ge_smi_rd/wr() through reg_info_t_ext
	 * struct's rd_ptr and wr_ptr.
	 */
	reg_ptr = master_reg_p->reg_p;

	if ((rc = register_tests(0, reg_ptr)) == FAILED) {
	    mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	    sprintf(err_buf, "mvl_ge_reg_test() Page %#x register_tests failed",
				master_reg_p->page);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	    return (FAILED);
	}
	master_reg_p++;
    }

    /* Restore page register */
    smi_if.offset = MRV88E111N_PA_R22_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;
    mrvl_p->smi_p = original_smi_if;		/* restore SMI if */

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
        sprintf(err_buf, "mvl_ge_reg_test() Page reg restore failed rc %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
        return (FAILED);
    }

    return (rc);

}

/*********************************************************************
 * Function:    mvl_ge_phy_mac_lpbk_test
 *
 * Description: a loopback test between CPU MAC and Marvell GE PHY MAC
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 *********************************************************************/
static int mvl_ge_phy_mac_lpbk_test (dev_object_t *dev)
{
    uint32_t rc;
    smi_t ori_media_mode;
    int test_spd, spd_ctr, spd_tb_size = sizeof(mvl_ge_copper_speed_t) / sizeof(mvl_ge_copper_speed_t[0]);

    /*====================================*/
    /*== [Pre-setting] Environment init ==*/
    /*====================================*/
    rc = mvl_ge_env_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init environment\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*============================================================*/
    /*== [Pre-setting] Read and store current GE PHY media mode ==*/
    /*============================================================*/
    rc = mvl_ge_mscr1_get_phy_media_mode (dev, &ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get original GE PHY media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Set GE PHY media to Copper mode ONLY for testing ==*/
    /*====================================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, MRV88E111M_P2_R16_B9_B7_MODE_COP_ONLY);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set GE PHY media mode as Copper Only Mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=======================================================*/
    /*== [Pre-setting] Disable that SFP module transmitter ==*/
    /*=======================================================*/
    rc = mvl_sfp_tx_disable(dev);
    if (rc != PASSED) {
        printf("%s:%d: SFP setup: fail to disable SFP Tx\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=========================================================================*/
    /*== [Testing] Entering test loop to do loopback test in different speed ==*/
    /*=========================================================================*/
    for (spd_ctr = 0; spd_ctr < spd_tb_size; spd_ctr++) {
        test_spd = mvl_ge_copper_speed_t[spd_ctr];

        /*===========================*/
        /*== 1. Config. GE PHY MAC ==*/
        /*===========================*/
        rc = mvl_ge_mcr_set_media_spd(dev, test_spd);
        if (rc != PASSED) {
            printf("%s:%d: Fail to config GE PHY media speed\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }

        /*===========================*/
        /*== 2. Config. CPU GE MAC ==*/
        /*===========================*/
        rc = mvl_cpu_mac_setup (dev, test_spd);
        if (rc != PASSED) {
            printf("%s:%d: CPU MAC setup: fail to setup CPU MAC\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
          
        /*===========================================*/
        /*== 3. Config. Enable GE PHY MAC Loopback ==*/
        /*===========================================*/
        rc = mvl_ge_xcr_enable_lpbk(dev, MRVL_COPPER_TYPE); 
        if (rc != PASSED) {
            printf("%s:%d: Fail to enable loopback function in Copper Control Register\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }

        /*==========================*/
        /*== 4. Run loopback test ==*/
        /*==========================*/
        rc =  mvl_ge_phy_tx_rx_test(dev);
        if (rc != PASSED) {
            printf("%s:%d: Loopback test: fail to run MAC loopback test\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    /*========================================================*/
    /*== [After test] Restore original GEWAN PHY media mode ==*/
    /*========================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to restore media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}


/***********************************************************************
 * Function:    mvl_ge_phy_sfp_ext_lpbk_test
 *
 * Description: an external SFP loopback test through SFP loopback cable
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
************************************************************************/
static int mvl_ge_phy_sfp_ext_lpbk_test (dev_object_t *dev)
{
    uint32_t rc;
    smi_t ori_media_mode;
    smi_t sfp_media_mode;

    /*======================================================*/
    /*== [Pre-setting] Enable that SFP module transmitter ==*/
    /*======================================================*/
    rc = mvl_sfp_tx_enable(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to enable SFP Tx\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================*/
    /*== [Pre-setting] Environment init ==*/
    /*====================================*/
    rc = mvl_ge_env_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init environment\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================*/
    /*== [Pre-setting] Setup SFP module ==*/
    /*====================================*/
    rc =  mvl_sfp_setup(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to setup SFP module\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Get the exact media mode for specific SFP module ==*/
    /*====================================================================*/
    rc =  mvl_sfp_get_media_mode(dev, &sfp_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get SFP media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    
    /*============================================================*/
    /*== [Pre-setting] Read and store current GE PHY media mode ==*/
    /*============================================================*/
    rc = mvl_ge_mscr1_get_phy_media_mode (dev, &ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get original GE PHY media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Set GE PHY media to Copper mode ONLY for testing ==*/
    /*====================================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, sfp_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set GE PHY media mode as Copper Only Mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }


    /*===================================================*/
    /*== [Pre-setting] Checking CPU side mac is linkup ==*/
    /*===================================================*/
    rc = mvl_cpu_phy_mac_check_linkup(dev);
    if (rc != PASSED) {
        printf("%s:%d: CPU side MAC doesn't link up\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*======================================================*/
    /*== [Pre-setting] Checking GE PHY side mac is linkup ==*/
    /*======================================================*/
    rc = mvl_ge_xssr_check_linkup(dev, MRVL_FIBER_TYPE); 
    if (rc != PASSED) {
        printf("%s:%d: GE PHY side MAC doesn't link up\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=================================*/
    /*== [Testing] Run loopback test ==*/
    /*=================================*/
    rc =  mvl_ge_phy_tx_rx_test(dev);
    if (rc != PASSED) {
        printf("%s:%d: Loopback test: fail to run SFP external loopback test\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=======================================================*/
    /*== [After test] Disable that SFP module transmitter ==*/
    /*=======================================================*/
    rc = mvl_sfp_tx_disable(dev);
    if (rc != PASSED) {
        printf("%s:%d: SFP setup: fail to disable SFP Tx\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*========================================================*/
    /*== [After test] Restore original GEWAN PHY media mode ==*/
    /*========================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to restore media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************
 * Function:    mvl_ge_ext_lpbk_test
 *
 * Description: an external loopback test through RJ45 loopback cable
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mvl_ge_ext_lpbk_test (dev_object_t *dev)
{
    uint32_t rc;
    smi_t ori_media_mode;
    int test_spd, spd_ctr, spd_tb_size = sizeof(mvl_ge_copper_speed_t) / sizeof(mvl_ge_copper_speed_t[0]);

    /*====================================*/
    /*== [Pre-setting] Environment init ==*/
    /*====================================*/
    rc = mvl_ge_env_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init environment\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*===============================================================*/
    /*== [Pre-setting] Read and store current GEWAN PHY media mode ==*/
    /*===============================================================*/
    rc = mvl_ge_mscr1_get_phy_media_mode (dev, &ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get original GE PHY media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Set GE PHY media to Copper mode ONLY for testing ==*/
    /*====================================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, MRV88E111M_P2_R16_B9_B7_MODE_COP_ONLY);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set GE PHY media mode as Copper Only Mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=========================================================================*/
    /*== [Testing] Entering test loop to do loopback test in different speed ==*/
    /*=========================================================================*/
    for (spd_ctr = 0; spd_ctr < spd_tb_size; spd_ctr++) {
        test_spd = mvl_ge_copper_speed_t[spd_ctr];

        /*==================================================*/
        /*== [Config] Power control: Copper ON, Fiber OFF ==*/
        /*==================================================*/
        rc = mvl_ge_xcr_power_control(dev, MRVL_FIBER_TYPE, MRVL_PWR_OFF);
        if (rc != PASSED) {
            printf("%s:%d: Fail to power off Fiber\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
        rc = mvl_ge_xcr_power_control(dev, MRVL_COPPER_TYPE, MRVL_PWR_ON);
        if (rc != PASSED) {
            printf("%s:%d: Fail to power on Copper\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }

        /*===================================================*/
        /*== [Config] Initializing Copper Control Register ==*/
        /*===================================================*/
        rc = mvl_ge_ccr_ext_lpbk_spd_config(dev, test_spd);
        if (rc != PASSED) {
            printf("%s:%d: Fail to config media speed in Copper Control Register \n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
            
        /*============================================*/
        /*== [Config] Enable stub test for 1000Mbps ==*/
        /*============================================*/
        rc = (test_spd == MRV88E1112_PHY_COP_SPD_1000MBPS)? mvl_ena_stub_test(dev, MRVL_ENABLE):PASSED;
        if (rc != PASSED) {
            printf("%s:%d: Fail to enable stub test \n", __FUNCTION__, __LINE__);
            return (FAILED);
        }

        /* a delay for preparation of hardware */
        sleep(MRV88E1112_PHY_STUB_BUFFERING);

        /*=================================*/
        /*== [Testing] Run loopback test ==*/
        /*=================================*/
        rc =  mvl_ge_phy_tx_rx_test(dev);
        if (rc != PASSED) {
            printf("%s:%d: Loopback test: fail to run external loopback test\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }
    /*====================================*/
    /*== [After test] Disable stub test ==*/
    /*====================================*/
    rc = (test_spd == MRV88E1112_PHY_COP_SPD_1000MBPS)? mvl_ena_stub_test(dev, MRVL_DISABLE):PASSED;
     if (rc != PASSED) {
         printf("%s:%d: Fail to disable stub test \n", __FUNCTION__, __LINE__);
         return (FAILED);
     }

    /*============================================================*/
    /*== [After test] Soft-reset Copper Control Register(P0_R0) ==*/
    /*============================================================*/
    rc = mvl_ge_xcr_soft_reset(dev, MRVL_COPPER_TYPE);
    if (rc != PASSED) {
        printf("%s:%d: GE soft-reset fail fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*==========================================================*/
    /*== [After test] Restore GEWAN PHY media back to default ==*/
    /*==========================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to restore media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}


/********************************************************************
 * Function:    mvl_ge_intr_test
 *
 * Description: testing the interrupt function of GE PHY
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mvl_ge_intr_test (dev_object_t *dev)
{
    uint32_t rc;
    smi_t wr_buf, test_pattern;
    smi_t page = 0;
    uchar offset = 0;

    page   = MRV88E111N_REG_PAGE_3; 
    offset = MRV88E111L_P3_R16_FUNC_CONTROL_REG;  

    /*=======================================================================*/
    /*== [Config]setting GE PHY R3_P16 with data:0x021e, disable interrupt ==*/
    /*=======================================================================*/
    printf("Setting GE PHY INT Pin(Pin.61) as default ...");
    /* write data */
    wr_buf = DISABLE_INTR; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* polling data */
    test_pattern = DISABLE_INTR;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*===================================================*/
    /*== [Config]checking the interrupt is de-asserted ==*/
    /*===================================================*/
    rc = mvl_ge_chk_intr_deassert(dev);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not de-asserted\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Done\n");

    /*======================================================================*/
    /*== [Config]setting GE PHY R3_P16 with data:0x0e1e, enable interrupt ==*/
    /*======================================================================*/
    printf("Catching GE PHY interrupt signal ...");
    /* write data */
    wr_buf = ENABLE_INTR; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* polling data */
    test_pattern = ENABLE_INTR;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*================================================*/
    /*== [Config]checking the interrupt is asserted ==*/
    /*================================================*/
    rc = mvl_ge_chk_intr_assert(dev);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not asserted\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Hit\n");

    /* put a delay makes hardware engineer to capture interrupt signal conveniently */
    sleep(MRV88E1112_PHY_INTR_BUFFERING);

    /*=======================================================================*/
    /*== [Config]setting GE PHY R3_P16 with data:0x021e, disable interrupt ==*/
    /*=======================================================================*/
    printf("Setting GE PHY INT Pin(Pin.61) as default ...");
    /* write data */
    wr_buf = DISABLE_INTR; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* polling data */
    test_pattern = DISABLE_INTR;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*===================================================*/
    /*== [Config]checking the interrupt is de-asserted ==*/
    /*===================================================*/
    rc = mvl_ge_chk_intr_deassert(dev);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not de-asserted\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Done\n");

    return (PASSED);
}

/*****************************************************
 * Function:    mvl_ge_chk_intr_assert
 *
 * Description: it will call a callout funciton to 
 *              check interrupt is asserted
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_ge_chk_intr_assert (dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    int rc = FAILED, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = (*callout_p->chk_intr_assert)();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/*****************************************************
 * Function:    mvl_ge_chk_intr_deassert
 *
 * Description: it will call a callout funciton to 
 *              check interrupt is de-asserted
 * Input :      dev - pointer to the Marvell GE device 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_ge_chk_intr_deassert (dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    int rc = FAILED, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = (*callout_p->chk_intr_deassert)();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/********************************************************************
 * Function:    util_mvl_ge_rd_reg
 *
 * Description: an utility to read GE PHY register
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int util_mvl_ge_rd_reg(dev_object_t *dev)
{
    uint32_t rc;
    smi_t rd_buf;
    smi_t page = 0;
    uchar offset = 0;

    /* let user to set page and register offset */
    page   = (smi_t)getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    offset = (uchar)getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    printf("Read  Data:0x%04x from Page:%d Reg:%d\n", rd_buf, page, offset);
    return (PASSED);
}

/********************************************************************
 * Function:    util_mvl_ge_wr_reg
 *
 * Description: an utility to write GE PHY register
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int util_mvl_ge_wr_reg(dev_object_t *dev)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf;
    smi_t page = 0;
    uchar offset = 0;

    /* let user to set page and register offset */
    page   = (smi_t)getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    offset = (uchar)getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /* let user to enter the data to write */
    wr_buf = gethex_answer("Enter write in data(0x0 ~ 0xfffff): ", rd_buf, 0, 0xffff); 
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }
    printf("Write Data:0x%04x to   Page:%d Reg:%d\n", wr_buf, page, offset);
    return (PASSED);
}

/********************************************************************
 * Function:    util_mvl_ge_set_test_mode
 *
 * Description: an utility to set test mode in 
 *              10000BASE-T Control Register(Page:0, Reg:9)
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int util_mvl_ge_set_test_mode(dev_object_t *dev)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf, test_pattern;
    smi_t page = 0;
    uchar offset = 0;
    smi_t current_test_mode = 0, wanted_test_mode;
   
    page   = MRV88E111N_REG_PAGE_0;
    offset = MRV88E111N_P0_R9_1000B_CNTL_REG;

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /* show current test mode before user enter */
    current_test_mode = rd_buf & MRV88E111M_P0_R9_TEST_MODE_MASK;
    printf("Current in: ");
    switch (current_test_mode) {
        case MRV88E111M_P0_R9_NORMAL_MODE:
            printf("Normal Mode\n");
        break;
        case MRV88E111M_P0_R9_TEST_MODE1:
            printf("Test Mode 1, Transmit Waveform Test\n");
        break;
        case MRV88E111M_P0_R9_TEST_MODE2:
            printf("Test Mode 2, Transmit Jitter Test (Master Mode)\n");
        break;
        case MRV88E111M_P0_R9_TEST_MODE3:
            printf("Test Mode 3, Transmit Jitter Test (Slave Mode)\n");
        break;
        case MRV88E111M_P0_R9_TEST_MODE4:
            printf("Test Mode 4, Transmit Distortion Test\n");
        break;
        default:
            printf("Unknown mode\n");
        break;
    }


    /* let user to enter the data to write */
    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    wanted_test_mode = (ushort)gethex_answer("Enter the test mode: ", current_test_mode >> MRV88E111M_P0_R9_MODE_SHIFT, 0, 4);

    wr_buf = wanted_test_mode << MRV88E111M_P0_R9_MODE_SHIFT; /* shift 13 bit to align Bit[15:13] */
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* reset to update test mode */
    page   = MRV88E111N_REG_PAGE_0;
    offset = MRV88E111N_P0_R0_CONTROL_REG; 
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    
    wr_buf = rd_buf | MRV88E111M_PX_R0_B15_RESET;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    test_pattern = rd_buf;
    rc = mvl_ge_polling_reg(dev, COMPARE_EQL, page, offset, test_pattern, MRV88E1112_PHY_MAX_POLLING_TIME);
    if (rc != PASSED) {
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Packet Generation Register fail\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 * Function:    if_mvl_ge_set_tx_type
 *
 * Description: an utility to set Tx type in 
 *              Copper Specific Control Register 2 (Page:0 Reg:26)
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int if_mvl_ge_set_tx_type(dev_object_t *dev, int tx_type)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf;
    smi_t page   = MRV88E111N_REG_PAGE_0;
    uchar offset = MRV88E111N_P0_R26_SPECIFIC_CONTROL2_REG;
    smi_t current_tx_type = 0, wanted_tx_type = 0;
 
    /* checking the input type is legal 
     * only support Class A ro Class B */
    if ((tx_type != MRV88E111M_TX_TYPE_A) && (tx_type != MRV88E111M_TX_TYPE_B)) {
        printf("%s:%d: Unsupported Tx type with:%d\n",
               __FUNCTION__, __LINE__, tx_type);
        return (FAILED);
    }

    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    current_tx_type = (rd_buf & MRV88E111M_P0_R26_TX_TYPE_MASK);

    wanted_tx_type = tx_type;


    wr_buf = (rd_buf & ~MRV88E111M_P0_R26_TX_TYPE_MASK) |           /* clear Bit[15] */
             (wanted_tx_type << MRV88E111M_P0_R26_TX_TYPE_SHIFT);   /* set   Bit[15] */
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 * Function:    util_mvl_ge_set_tx_type
 *
 * Description: an utility to set Tx type in 
 *              Copper Specific Control Register 2 (Page:0 Reg:26)
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int util_mvl_ge_set_tx_type(dev_object_t *dev)
{
    int tx_type, rc = FAILED;

    tx_type = (int)gethex_answer("Enter TX Type(0: Class B; 1 - Class A): ", MRV88E111M_TX_TYPE_A, 0, 1);
    if (tx_type == MRV88E111M_TX_TYPE_A) {
        printf("Setting Tx type as Class A type\n");
    } else {
        printf("Setting Tx type as Class B type\n"); 
    }
    
    rc = if_mvl_ge_set_tx_type(dev, tx_type);
    if (rc != PASSED) {
        printf("%s:%d:Failed to set Tx type\n", __FUNCTION__, __LINE__);
    }

    return (rc);
}

/********************************************************************
 * Function:    util_mvl_ge_set_vod
 *
 * Description: an utility to set Tx type in Reg:29 & Reg:30
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int util_mvl_ge_set_vod(dev_object_t *dev)
{
    uint32_t rc;
    smi_t rd_buf, wr_buf;
    smi_t page = MRV88E111N_REG_PAGE_0;
    uchar offset;
    smi_t current_vod = 0, wanted_vod = 0;

    /* Based on Marvell 88E1112 release note, MV-S300751.
     * For gigabit and 100Mbps mode, the MDI VOD output may be adjusted by
     * 1. Set reg.29 = 0x0004
     * 2. Then bit[2:0] of reg.30 represents
     *    "000" = +8%
     *    "001" = +6%
     *    "010" = +4%
     *    "011" = +2%
     *    "100" = default
     *    "101" = -2%
     *    "110" = -4%
     *    "111" = -6%
     *
     * Note: These reigster settings have no effect for 10Mbps mode.
     */

    /* 1. Set reg.29 = 0x0004 */
    offset = MRV88E111N_PA_R29_FACTORY_TEST_MODES0; /* Reg:29 */
    wr_buf = MRV88E111N_REG_PAGE_4;
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    /* 2. Get current VOD Adjust value from reg.30 bit[2:0] */
    offset = MRV88E111N_PA_R30_FACTORY_TEST_MODES1; /* Reg:30 */
    /* read original value */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, &rd_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to read Page:%d Reg:%d\n", __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }
    current_vod = rd_buf & MRV88E111N_PA_R30_VOD_MASK;

    /* let user to enter new value */
    printf("\nVOD Adjustments -\n");
    printf("    0: +8%%\n");
    printf("    1: +6%%\n");
    printf("    2: +4%%\n");
    printf("    3: +2%%\n");
    printf("    4: default\n");
    printf("    5: -2%%\n");
    printf("    6: -4%%\n");
    printf("    7: -6%%\n");
    wanted_vod = (ushort)gethex_answer("Enter VOD adjust value: ", current_vod, 0, 7);
    wr_buf = (wanted_vod & ~MRV88E111N_PA_R30_VOD_MASK) | /* clear Bit[2:0] */
             (wanted_vod & MRV88E111N_PA_R30_VOD_MASK);
    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************
 * Function:    if_mvl_ge_rd_reg
 *
 * Description: an interface to read GE PHY register
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
int if_mvl_ge_rd_reg(dev_object_t *dev, smi_t page, uchar offset, smi_t *rd_buf)
{
    return (mvl_ge_rd_wr_reg(dev, PHY_READ, page, offset, rd_buf));
}

/********************************************************************
 * Function:    if_mvl_ge_wr_reg
 *
 * Description: an interface to write GE PHY register
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
int if_mvl_ge_wr_reg(dev_object_t *dev, smi_t page, uchar offset, smi_t *wr_buf)
{
    return (mvl_ge_rd_wr_reg(dev, PHY_WRITE, page, offset, wr_buf));
}

/********************************************************************
 * Function:    mvl_ge_ext_lpbk_test_1gbps
 *
 * Description: an external loopback test through RJ45 loopback cable (For fixed speed 1GBPS)
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mvl_ge_ext_lpbk_test_1gbps (dev_object_t *dev)
{
    uint32_t rc;
    smi_t ori_media_mode;
    int test_spd;

    /*====================================*/
    /*== [Pre-setting] Environment init ==*/
    /*====================================*/
    rc = mvl_ge_env_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init environment\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*===============================================================*/
    /*== [Pre-setting] Read and store current GEWAN PHY media mode ==*/
    /*===============================================================*/
    rc = mvl_ge_mscr1_get_phy_media_mode (dev, &ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get original GE PHY media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Set GE PHY media to Copper mode ONLY for testing ==*/
    /*====================================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, MRV88E111M_P2_R16_B9_B7_MODE_COP_ONLY);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set GE PHY media mode as Copper Only Mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=========================================================================*/
    /*== [Testing] Do loopback test in 1GBPS speed ==*/
    /*=========================================================================*/
    test_spd = MRV88E1112_PHY_COP_SPD_1000MBPS;

    /*==================================================*/
    /*== [Config] Power control: Copper ON, Fiber OFF ==*/
    /*==================================================*/
    rc = mvl_ge_xcr_power_control(dev, MRVL_FIBER_TYPE, MRVL_PWR_OFF);
    if (rc != PASSED) {
        printf("%s:%d: Fail to power off Fiber\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    rc = mvl_ge_xcr_power_control(dev, MRVL_COPPER_TYPE, MRVL_PWR_ON);
    if (rc != PASSED) {
        printf("%s:%d: Fail to power on Copper\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*===================================================*/
    /*== [Config] Initializing Copper Control Register ==*/
    /*===================================================*/
    rc = mvl_ge_ccr_ext_lpbk_spd_config(dev, test_spd);
    if (rc != PASSED) {
        printf("%s:%d: Fail to config media speed in Copper Control Register \n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
            
    /*============================================*/
    /*== [Config] Enable stub test for 1000Mbps ==*/
    /*============================================*/
    rc = mvl_ena_stub_test(dev, MRVL_ENABLE);
    if (rc != PASSED) {
        printf("%s:%d: Fail to enable stub test \n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* a delay for preparation of hardware */
    sleep(MRV88E1112_PHY_STUB_BUFFERING);

    /*=================================*/
    /*== [Testing] Run loopback test ==*/
    /*=================================*/
    rc =  mvl_ge_phy_tx_rx_test(dev);
    if (rc != PASSED) {
        printf("%s:%d: Loopback test: fail to run external loopback test\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================*/
    /*== [After test] Disable stub test ==*/
    /*====================================*/
    rc = mvl_ena_stub_test(dev, MRVL_DISABLE);
     if (rc != PASSED) {
         printf("%s:%d: Fail to disable stub test \n", __FUNCTION__, __LINE__);
         return (FAILED);
     }

    /*============================================================*/
    /*== [After test] Soft-reset Copper Control Register(P0_R0) ==*/
    /*============================================================*/
    rc = mvl_ge_xcr_soft_reset(dev, MRVL_COPPER_TYPE);
    if (rc != PASSED) {
        printf("%s:%d: GE soft-reset fail fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*==========================================================*/
    /*== [After test] Restore GEWAN PHY media back to default ==*/
    /*==========================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to restore media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/*********************************************************************
 * Function:    mvl_ge_phy_mac_lpbk_test_1gbps
 *
 * Description: a loopback test between CPU MAC and Marvell GE PHY MAC (For fixed speed 1GBPS)
 * Input:       dev - Pointer to the Marvell GE device object
 * Returns:     PASSED/FAILED
 *********************************************************************/
static int mvl_ge_phy_mac_lpbk_test_1gbps (dev_object_t *dev)
{
    uint32_t rc;
    smi_t ori_media_mode;
    int test_spd;

    /*====================================*/
    /*== [Pre-setting] Environment init ==*/
    /*====================================*/
    rc = mvl_ge_env_init(dev);
    if (rc != PASSED) {
        printf("%s:%d: Fail to init environment\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*============================================================*/
    /*== [Pre-setting] Read and store current GE PHY media mode ==*/
    /*============================================================*/
    rc = mvl_ge_mscr1_get_phy_media_mode (dev, &ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to get original GE PHY media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*====================================================================*/
    /*== [Pre-setting] Set GE PHY media to Copper mode ONLY for testing ==*/
    /*====================================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, MRV88E111M_P2_R16_B9_B7_MODE_COP_ONLY);
    if (rc != PASSED) {
        printf("%s:%d: Fail to set GE PHY media mode as Copper Only Mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=======================================================*/
    /*== [Pre-setting] Disable that SFP module transmitter ==*/
    /*=======================================================*/
    rc = mvl_sfp_tx_disable(dev);
    if (rc != PASSED) {
        printf("%s:%d: SFP setup: fail to disable SFP Tx\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*=========================================================================*/
    /*== [Testing] Do loopback test in 1GBPS speed ==*/
    /*=========================================================================*/
    test_spd = MRV88E1112_PHY_COP_SPD_1000MBPS;

    /*===========================*/
    /*== 1. Config. GE PHY MAC ==*/
    /*===========================*/
    rc = mvl_ge_mcr_set_media_spd(dev, test_spd);
    if (rc != PASSED) {
        printf("%s:%d: Fail to config GE PHY media speed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*===========================*/
    /*== 2. Config. CPU GE MAC ==*/
    /*===========================*/
    rc = mvl_cpu_mac_setup (dev, test_spd);
    if (rc != PASSED) {
        printf("%s:%d: CPU MAC setup: fail to setup CPU MAC\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
          
    /*===========================================*/
    /*== 3. Config. Enable GE PHY MAC Loopback ==*/
    /*===========================================*/
    rc = mvl_ge_xcr_enable_lpbk(dev, MRVL_COPPER_TYPE); 
    if (rc != PASSED) {
        printf("%s:%d: Fail to enable loopback function in Copper Control Register\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*==========================*/
    /*== 4. Run loopback test ==*/
    /*==========================*/
    rc =  mvl_ge_phy_tx_rx_test(dev);
    if (rc != PASSED) {
        printf("%s:%d: Loopback test: fail to run MAC loopback test\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /*========================================================*/
    /*== [After test] Restore original GEWAN PHY media mode ==*/
    /*========================================================*/
    rc = mvl_ge_mscr1_set_phy_media_mode (dev, ori_media_mode);
    if (rc != PASSED) {
        printf("%s:%d: Fail to restore media mode\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}


/*-------------------------------------------------
 * $Log: dev_mrvl_ge.c,v $
 * Revision 1.3  2021/09/24 01:22:45  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.2  2019/01/10 06:26:40  wilbhuan
 * 1. The beginning of Marvell 88E111X Gigabit Ethernet Transceiver device driver.
 * 2. Supported PHYs:
 *    (1)88E1112
 *
 *-------------------------------------------------
 */
