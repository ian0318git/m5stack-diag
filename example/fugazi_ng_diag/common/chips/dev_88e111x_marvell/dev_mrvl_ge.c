/* $Id: dev_mrvl_ge.c,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e111x_marvell/dev_mrvl_ge.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_mrvl_ge.c
 *
 * Description:	Marvell Common PHY/Switch driver functions.
 *
 *		Marvell PHY does generate interrupt; therefore,
 *		dev_intr_enable, dev_intr_disable, dev_isr, dev_attach,
 *		dev_detach, dev_reconfig_needed, dev_restart, dev_init,
 *		dev_oper_enable, dev_oper_disable, dev_show, dev_err_report,
 *		dev_collect_crashinfo, dev_destroy are implemented.
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "dev_print.h"
#include "dev_1112.h"
#include "dev_1114.h"
#include "free.h"
#include "proto.h"
#include "strings.h"
#include "error.h"

/* #define PHY_STAT_MONITOR */ /* */

#ifdef LINUX_APP
#include <assert.h>
#endif

/* #define MEM_DEBUG  * */

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32_t mvl_ge_dev_attach(dev_object_t *dev);
static uint32_t	mvl_ge_dev_detach(dev_object_t *dev);
static uint32_t	mvl_ge_dev_reconfig(dev_object_t *, void *, boolean *);
static uint32_t	mvl_ge_dev_restart(dev_object_t *);
static uint32_t	mvl_ge_dev_init(dev_object_t *);
static uint32_t	mvl_ge_dev_oper_en(dev_object_t *);
static uint32_t	mvl_ge_dev_oper_dis(dev_object_t *);
static uint32_t mvl_ge_dev_intr_en(dev_object_t *);
static uint32_t mvl_ge_dev_intr_dis(dev_object_t *);
static uint32_t mvl_ge_dev_isr(dev_object_t *);
static uint32_t	mvl_ge_dev_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32_t	mvl_ge_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	mvl_ge_destroy(dev_object_t **);

static int mvl_ge_reg_test(dev_object_t *dev);
static int mvl_ge_set_loopback(dev_object_t *dev, uint lpbk_mode,
		uint speed, uint op, uint auto_neg);
static int mvl_ge_alt_reg(dev_object_t *dev);
static int mvl_ge_set_test_mode(dev_object_t *dev);

static int mvl_ge_smi_rd(ulong addr, int size, ulong *buf, void *param);
static int mvl_ge_smi_wr(ulong addr, int size, ulong data, void *param);
static dev_mrvl_reg_info_t *mvl_ge_get_master_table(dev_object_t *dev);
static int mvl_ge_rd_wr_reg(dev_object_t *dev, uint op, smi_t page,
		uchar offset, smi_t *data);
static int poll_sw_reset(dev_object_t *dev);
static uint32_t clear_pkt_cnt(dev_object_t *dev);

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
/* reg_info_t extension for SMI access */
static reg_info_t_ext reg_ext = {
	sizeof(smi_t), mvl_ge_smi_rd, mvl_ge_smi_wr, 0};

/* Common registers for all pages */
static reg_info_t ge_1112_cmn_reg_tbl[] = {
    {"PHY Identifier 1", MRV88E111N_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1112_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E111N_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1112_PHY_ID_2_VALUE},
    /* Page Address bit 15 - Enable automatic medium register selection */
    /* The note in 88E1112C and 88E1114 (Section 3 Registers Description)
     * - "Note that in order for the paging mechanism to work correctly
     * register 22.15 must be set to 0 to disable the automatic medium
     * register seection"
     */
    {"Page Address", MRV88E111N_PAGE_ADDRESS_REG, READ_WRITE | SAVE_RESTORE |
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
    {"Control Register", MRV88E111N_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1140},
    {"Status Register", MRV88E111N_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7F, 0x0149},
    {"Auto-Negotiation Advertisement Register", MRV88E111N_AUTONEG_ADVR_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xAFFF, 0x0001},
    {"Link Partner Ability Register - Base Page", MRV88E111N_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111N_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0004},
    {"Next Page Transmit Register", MRV88E111N_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111N_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000BASE-T Control Register", MRV88E111N_1000B_CNTL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x1F00, 0x0F00},
    {"1000BASE-T Status Register", MRV88E111N_1000B_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFCFF, 0x0000},
    {"Extended Status Register", MRV88E111N_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF000, 0x0000},
    {"Specific Control Register 1", MRV88E111N_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFE3, 0x6060},
    {"Specific Status Register 1", MRV88E111N_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0010},
    {"Interrupt Enable Register", MRV88E111N_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Specific Status Register 2", MRV88E111N_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Receive Error Counter Register", MRV88E111N_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111N_SPECIFIC_CONTROL2_REG,
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
    {"Control Register", MRV88E111F_CONTROL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x4000, 0x0100},
    {"Status Register", MRV88E111F_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7D, 0x0141},
    {"Auto-Negotiation Advertisement Register", MRV88E111F_AUTONEG_ADVR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB1E0, 0x0060},
    {"Link Partner Ability Register - Base Page", MRV88E111F_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFDE0, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111F_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x000F, 0x0004},
    {"Next Page Transmit Register", MRV88E111F_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111F_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Extended Status Register", MRV88E111F_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF000, 0x0000},
    {"Specific Control Register 1", MRV88E111F_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0E00, 0x0000},
    {"Specific Status Register 1", MRV88E111F_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFF8, 0x0010},
    {"Interrupt Enable Register", MRV88E111F_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x3F10, 0x0000},
    {"Specific Status Register 2", MRV88E111F_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x3F10, 0x0000},
    {"Receive Error Counter Register", MRV88E111F_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111F_SPECIFIC_CONTROL2_REG,
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
    {"Control Register", MRV88E111M_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1000},
    {"Specific Control Register 1", MRV88E111M_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF84, 0x0008},
    {"Specific Status Register 1", MRV88E111M_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0460, 0x0000},
    {"Interrupt Enable Register", MRV88E111M_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Status Register 2", MRV88E111M_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Control Register 2", MRV88E111M_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE090, 0x0000},
    {"End of Page 2 registers", 0, 0, {0}, 0, 0},
};

/* Page 3 - LOS, INIT, STATUS[1:0] */
static reg_info_t ge_1112_p3_reg_tbl[] = {
    {"Function Control Register", MRV88E111L_FUNC_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x021E},
    {"Polarity Control Register 1", MRV88E111L_POL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x44A0},
    {"Timer Control Register 2", MRV88E111L_TMR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x770F, 0x4105},
    {"End of Page 3 registers", 0, 0, {0}, 0, 0},
};

/* Page 4 - Non-Volatile Memory */
static reg_info_t ge_1112_p4_reg_tbl[] = {
    {"I2C Address Register", MRV88E111NV_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0100},
    {"Read Data and Status Register", MRV88E111NV_READ_DATA_STATUS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE7FF, 0x0000},
    {"Write Data and Control Register", MRV88E111NV_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF2FF, 0xA000},
    {"RAM Write Data and Control Register", MRV88E111NV_RAM_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"RAM Address Register", MRV88E111NV_RAM_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"End of Page 4 registers", 0, 0, {0}, 0, 0},
};

/* Page 5 - Virtual Cable Tester */
static reg_info_t ge_1112_p5_reg_tbl[] = {
    /* MDI[0] Virtual Cable Rester Status Register - Bit 15 (Run VCT Test)
     * is SC (self clear).
     */
    {"MDI[0] VCT Status Register", MRV88E111N_VCT_STATUS_MDI0_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x0000},
    {"MDI[1] VCT Status Register", MRV88E111N_VCT_STATUS_MDI1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x8000, 0x0000},
    {"MDI[2] VCT Status Register", MRV88E111N_VCT_STATUS_MDI2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"MDI[3] VCT Status Register", MRV88E111N_VCT_STATUS_MDI3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"1000 BASE-T Pair Skew Register", MRV88E111N_VCT_PAIR_SKEW_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000 BASE-T Pair Swap and Polarity", MRV88E111N_VCT_PAIR_SWP_POL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x007F, 0x0000},
    {"VCT DSP Distance", MRV88E111N_VCT_DSP_DISTANCE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0007, 0x0000},
    {"End of Page 5 registers", 0, 0, {0}, 0, 0},
};

/* Page 6 - Miscellaneous */
static reg_info_t ge_1112_p6_reg_tbl[] = {
    {"Packet Generation", MRV88E111N_PACKET_GEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0000},
    {"CRC Counters", MRV88E111N_CRC_CHKR_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"End of Page 6 registers", 0, 0, {0}, 0, 0},
};

/* Common registers for all pages */
static reg_info_t ge_1114_cmn_reg_tbl[] = {
    {"PHY Identifier 1", MRV88E111N_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1114_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E111N_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0xFFFF, MRV88E1114_PHY_ID_2_VALUE},
    {"Page Address", MRV88E111N_PAGE_ADDRESS_REG, READ_WRITE | SAVE_RESTORE |
	REG_ACCESS, {(unsigned long)&reg_ext}, 0x00FF, 0x0000},
    {"End of Common registers page", 0, 0, {0}, 0, 0},
};

/* Page 0 - Copper */
static reg_info_t ge_1114_p0_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) and  Bit 9 (Restart Auto-Negotiation) 
     * are SC (self clear)
     * Bit 14 (Loopback) - Loopback speed is determined by the mode the device
     * is in Registers 0_2:13 and 0_2:6. After writing 0x7140, it reads back
     * 0x3140.
     * Bits 13, 6 (Speed Selection), Bit 12 (Auto-Negotation Enable), and Bit
     *  8 (Duplex Mode) - "A write to this register bit does not take effect
     * until any one of the following also occurs: Software reset is asserted,
     * Restart Auto-Negotiation is asserted, Power down transition from power
     * down to normal operation."
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
    {"Control Register", MRV88E111N_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1140},
    {"Status Register", MRV88E111N_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7F, 0x0149},
    {"Auto-Negotiation Advertisement Register", MRV88E111N_AUTONEG_ADVR_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xAFFF, 0x0001},
    {"Link Partner Ability Register - Base Page", MRV88E111N_LINK_PART_AV_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E111N_AUTONEG_EXPANSION_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0004},
    {"Next Page Transmit Register", MRV88E111N_NEXT_PAGE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E111N_LP_NEXT_PAGE_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000BASE-T Control Register", MRV88E111N_1000B_CNTL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x1F00, 0x0F00},
    {"1000BASE-T Status Register", MRV88E111N_1000B_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFCFF, 0x0000},
    {"Extended Status Register", MRV88E111N_EXTENDED_STATUS_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x3000, 0x0000},
    {"Specific Control Register 1", MRV88E111N_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFE3, 0x6060},
    {"Specific Status Register 1", MRV88E111N_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF7F, 0x0010},
    {"Interrupt Enable Register", MRV88E111N_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Specific Status Register 2", MRV88E111N_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFF77, 0x0000},
    {"Receive Error Counter Register", MRV88E111N_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"Specific Control Register 2", MRV88E111N_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x81F2, 0x0040},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Page 1 - Not Used */
static reg_info_t ge_1114_p1_reg_tbl[] = {
    {"Receive Error Counter Register", MRV88E111F_REC_ERROR_COUNTER_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"End of Page 1 registers", 0, 0, {0}, 0, 0},
};

/* Page 2 - MAC */
static reg_info_t ge_1114_p2_reg_tbl[] = {
    /* Control Register - Bit 15 (Reset) is SC (self clear)
     * Bit 14 (Loopback) is not working. After writing 0x7040, it returns
     * 0x3040.
     * Bits 13 and 6 (Default MAC Interface and Speed), Bit 12 (MAC
     * Interface Auto-Negotiation Enable) - "Changes to these bits are
     * disruptive to the normal operation; therefore, any changes to these
     * registers must be followed by software reset to take effect."
     */
    /* Bit 11 (Power Down) will work with the registers test. But the side
     * effect will cause the loopback tests later on to fail.
     */
    {"Control Register", MRV88E111M_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x1000},
    {"Specific Control Register 1", MRV88E111M_SPECIFIC_CONTROL1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF380, 0x0288},
    {"Specific Status Register 1", MRV88E111M_SPECIFIC_STATUS1_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0460, 0x0000},
    {"Interrupt Enable Register", MRV88E111M_INT_ENABLE_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Status Register 2", MRV88E111M_SPECIFIC_STATUS2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0080, 0x0000},
    {"Specific Control Register 2", MRV88E111M_SPECIFIC_CONTROL2_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE010, 0x0000},
    {"End of Page 2 registers", 0, 0, {0}, 0, 0},
};

/* Page 3 - LOS, INIT, STATUS[1:0] */
static reg_info_t ge_1114_p3_reg_tbl[] = {
    {"Function Control Register", MRV88E111L_FUNC_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x021E},
    {"Polarity Control Register 1", MRV88E111L_POL_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x44A0},
    {"Timer Control Register 2", MRV88E111L_TMR_CONTROL_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x770F, 0x4105},
    {"End of Page 3 registers", 0, 0, {0}, 0, 0},
};

/* Page 4 - Non-Volatile Memory */
static reg_info_t ge_1114_p4_reg_tbl[] = {
    {"I2C Address Register", MRV88E111NV_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0100},
    {"Read Data and Status Register", MRV88E111NV_READ_DATA_STATUS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xE7FF, 0x0000},
    {"Write Data and Control Register", MRV88E111NV_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xF2FF, 0xA000},
    {"RAM Write Data and Control Register", MRV88E111NV_RAM_WRITE_DATA_CONTROL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"RAM Address Register", MRV88E111NV_RAM_ADDRESS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x00FF, 0x0000},
    {"End of Page 4 registers", 0, 0, {0}, 0, 0},
};

/* Page 5 - Virtual Cable Tester */
static reg_info_t ge_1114_p5_reg_tbl[] = {
    /* MDI[0] Virtual Cable Tester Status Register - Bit 15 (Run VCT Test) is
     * SC (self-clear)
     */
    {"MDI[0] VCT Status Register", MRV88E111N_VCT_STATUS_MDI0_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0000, 0x0000},
    {"MDI[1] VCT Status Register", MRV88E111N_VCT_STATUS_MDI1_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x8000, 0x0000},
    {"MDI[2] VCT Status Register", MRV88E111N_VCT_STATUS_MDI2_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"MDI[3] VCT Status Register", MRV88E111N_VCT_STATUS_MDI3_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x7FFF, 0x0000},
    {"1000 BASE-T Pair Skew Register", MRV88E111N_VCT_PAIR_SKEW_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0xFFFF, 0x0000},
    {"1000 BASE-T Pair Swap and Polarity", MRV88E111N_VCT_PAIR_SWP_POL_REG,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x007F, 0x0000},
    {"VCT DSP Distance", MRV88E111N_VCT_DSP_DISTANCE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x0007, 0x0000},
    {"End of Page 5 registers", 0, 0, {0}, 0, 0},
};

/* Page 6 - Miscellaneous */
static reg_info_t ge_1114_p6_reg_tbl[] = {
    {"Packet Generation", MRV88E111N_PACKET_GEN_REG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(unsigned long)&reg_ext},
	0x001F, 0x0000},
    {"CRC Counters", MRV88E111N_CRC_CHKR_REG,
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

/*	88E1114 */
static dev_mrvl_reg_info_t ge_1114_reg_tbl[] = {
    {MRV88E111N_REG_PAGE_0, &ge_1114_cmn_reg_tbl[0]},	/* Common registers */
    {MRV88E111N_REG_PAGE_0, &ge_1114_p0_reg_tbl[0]},	/* Page 0 */
    {MRV88E111N_REG_PAGE_1, &ge_1114_p1_reg_tbl[0]},	/* Page 1 */
    {MRV88E111N_REG_PAGE_2, &ge_1114_p2_reg_tbl[0]},	/* Page 2 */
    {MRV88E111N_REG_PAGE_3, &ge_1114_p3_reg_tbl[0]},	/* Page 3 */
    {MRV88E111N_REG_PAGE_4, &ge_1114_p4_reg_tbl[0]},	/* Page 4 */
    {MRV88E111N_REG_PAGE_5, &ge_1114_p5_reg_tbl[0]},	/* Page 5 */
    {MRV88E111N_REG_PAGE_6, &ge_1114_p6_reg_tbl[0]},	/* Page 6 */
    {0, 0},						/* End */
};

/*****************************************************************
 *
 * Name: mrvl_ge_dev_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the Marvell device.
 *	  error reporting function pointer.
 *
 * Returns: PASSED/FAILED
 *
 * Assumption:	Since this device driver is shared amongst different Marvell
 *		chips, dev_object_fvt->dev_name needs to be filled after this
 *		call.
 *
 ****************************************************************
 */
int
mrvl_ge_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "obj malloc failure in mrvl_ge_dev_create()",
			0);
	return(FAILED);
    } else {
#ifdef MEM_DEBUG
	printf("\ndev obj @ %#x\n", dev_fvt);
#endif /* MEM_DEBUG */
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
    mrvl_p->base.dev_object_fvt->dev_isr	= mvl_ge_dev_isr;
    mrvl_p->base.dev_object_fvt->dev_show	= mvl_ge_dev_show;
    mrvl_p->base.dev_object_fvt->dev_error_report = error_report_fn;
    mrvl_p->base.dev_object_fvt->dev_collect_crashinfo = mvl_ge_crsh;
    mrvl_p->base.dev_object_fvt->dev_destroy	= mvl_ge_destroy;
    mrvl_p->base.dev_object_fvt->dev_name = "Marvell GE PHY";

    if ((mrvl_p->callin_fvt = (mrvl_ge_callin_fvt_t *)
			       malloc(sizeof(mrvl_ge_callin_fvt_t))) == NULL) {
#ifdef MEM_DEBUG
	printf("freeing dev_fvt @ %#x after malloc callin\n", dev_fvt);
#endif /* MEM_DEBUG */
	free(dev_fvt);
	error_report_fn(dev, "callin malloc failure in mrvl_ge_dev_create()",
			0);
	return(FAILED);
    } else {
#ifdef MEM_DEBUG
	printf("callin_fvt @ %#x\n", mrvl_p->callin_fvt);
#endif /* MEM_DEBUG */
    }

    if ((mrvl_p->callout_fvt = (mrvl_ge_callout_fvt_t *)
			       malloc(sizeof(mrvl_ge_callout_fvt_t))) == NULL) {
#ifdef MEM_DEBUG
	printf("freeing callout @ %#x after malloc callout \n", mrvl_p->callout_fvt);
#endif /* MEM_DEBUG */
	free(mrvl_p->callin_fvt);
#ifdef MEM_DEBUG
	printf("freeing dev_fvt @ %#x\n", dev_fvt);
#endif /* MEM_DEBUG */
	free(dev_fvt);
	error_report_fn(dev, "callout malloc failure in mrvl_ge_dev_create()",
			0);
	return(FAILED);
    } else {
#ifdef MEM_DEBUG
	 printf("callout_fvt @ %#x\n", mrvl_p->callout_fvt);
#endif /* MEM_DEBUG */
    }

    mrvl_p->base.dev_state = DEV_STATE_CREATE;
    return(PASSED);
}

/*****************************************************************
 *
 * Name: mvl_ge_dev_attach()
 *
 * Description: Attach the Marvell GE device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_attach (dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    char err_buf[ERR_BUF_SIZE];

    if (mrvl_p->base.dev_state != DEV_STATE_CREATE) {
	sprintf(err_buf, "Invalid device state %#x for attach",
			  mrvl_p->base.dev_state);
	DEV_ERROR_REPORT(dev, err_buf, (uint32_t)MRVL_GE_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    mrvl_p->callin_fvt->register_test = mvl_ge_reg_test;
    mrvl_p->callin_fvt->set_loopback = mvl_ge_set_loopback;
    mrvl_p->callin_fvt->alter_reg = mvl_ge_alt_reg;
    mrvl_p->callin_fvt->set_test_mode = mvl_ge_set_test_mode;

    /* Lock the SMI device */
    if ((rc = mrvl_p->callout_fvt->open(mrvl_p->smi_p)) != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_attach() SMI open failed with rc = %#x",
							rc);
        DEV_ERROR_REPORT(dev, err_buf, (uint32_t)MRVL_GE_ATTACH);
        return(FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: mrvl_ge_dev_detach()
 *
 * Description: detach the device specific functions from the caller.
 *		All of the device specific function are connected to the
 *		dev_do_nothing() function, except for the dev_attach()
 *		function. Also, the dev_state must be assigned the value
 *		of DEV_STATE_DETACH.
 *
 *		Since, some platforms may want to detach the device, but not
 *		release the memory resources (via a free () in the
 *		dev_destroy()), this function can be executed to accomplish
 *		this task. However, before a detached device can be used again,
 *		it must be re-attached (via the dev_attach()).
 *
 * Input: Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_detach (dev_object_t *dev)
{
    uint32_t rc;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the SMI device */
    if ((rc = mrvl_p->callout_fvt->close(mrvl_p->smi_p)) != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_detach() SMI close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, mrvl_p->base.dev_object_fvt);

    mrvl_p->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: mvl_ge_dev_reconfig_needed
 *
 * Description: To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *	  void * - a device/platform specific context handle
 *	  boolean * - a pointer to a boolean
 *
 * Returns: PASSED/FAILED, context information and a boolean value.
 *	    The boolean value shall be set to TRUE if the device must be
 *	    reconfigured from scratch and it shall be set to FALSE otherwise.
 *
 * Assumptions: The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32_t
mvl_ge_dev_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: mvl_ge_dev_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the Marvell GE DEVICE
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32_t
mvl_ge_dev_restart(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *) dev;

    mrvl_p->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name: mvl_ge_dev_init()
 *
 * Description: Initializes the Marvell GE chip 
 *              
 *
 * Input: dev_object_t pointer to the Marvell GE device.
 *	  Caller has to setup the smi_p parameters with init values.
 *
 * Returns: PASSED/FAILED
 *
 * Note: Make sure base.dev_addr has been initialized to chip_base_addr
 *       before calling this function.
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_init (dev_object_t *dev)
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
    if (master_reg_p == 0) {
	DEV_ERROR_REPORT(dev, "mvl_ge_dev_init() unable to get the table",
					MRVL_GE_INIT);
	return(FAILED);
    }

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup the page */
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
	smi_reg = master_reg_p->page;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_dev_init() Page %#x write failed rc = %#x",
			smi_reg, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_INIT);
	    return(FAILED);
	}

	init_p = master_reg_p->reg_p;

	/* Call registers table init. */
	while(init_p->size.size != 0) {
	    smi_if.offset = init_p->offset;
	    smi_reg = init_p->reset_val; /* Use reset_val as init valure */

	    /* Write to the Marvell GE with platform default value */
	    rc = (*callout_p->wr)(&smi_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "mvl_ge_dev_init() write %#x @ %#x pg %#x"
				 " rc = %#x", smi_reg, smi_if.offset,
				 master_reg_p->page, rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_INIT);
		return(FAILED);
	    }
	    init_p++;
	} /* endof while init_p */
	master_reg_p++;
    } /* endof master_reg_p */

    /* Reset to page 0 */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    smi_reg = MRV88E111N_REG_PAGE_0;

    rc = (*callout_p->wr)(&smi_if);
    if (rc != PASSED) {
	DEV_ERROR_REPORT(dev, "mvl_ge_dev_init() Unable to set to page 0",
				MRVL_GE_INIT);
	return(FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_INIT;

    return(PASSED);

}

/*****************************************************************
 * Name: mvl_ge_dev_oper_en
 *
 * Description:	Enable device operation.
 *
 *		Change the state of the device from its current state to an
 *		enabled state (which implies that the device is in an
 *		operational state at the end of this function execution). Also,
 *		the dev_state must be assigned the value of DEV_STATE_ENABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to enable all or only part of the total device port's
 *		or channel's.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_oper_en(dev_object_t *dev)
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
	return(FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_ENABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: mvl_ge_dev_oper_dis
 *
 * Description:	Disable device operation.
 *
 *		Change the state of the device from its current state to an
 *		disabled state (which implies that the device is in a
 *		non-operational state at the end of this function execution).
 *		Also, the dev_state must be assigned the value of
 *		DEV_STATE_DISABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to disable all the ports or channels of a specific
 *		device
 * 
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_oper_dis(dev_object_t *dev)
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
	return(FAILED);
    }

    mrvl_p->base.dev_state = DEV_STATE_DISABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: mvl_ge__dev_intr_en
 *
 * Description: Enable the device interrupt(s).
 *		This function is expected to enable the interrupt(s) for a
 *		device with the expectation that all portions of the device
 *		are active, it is possible that some portions of a device are
 *		disabled.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_intr_en (dev_object_t *dev)
{
    smi_t smi_d;		/* Data bytes for SMI */
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Setup SMI API interface struct */
    smi_if.buf = &smi_d;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    /* Enable Interrupt Control interupts */
/* TBD */

    return (PASSED);
}

/*****************************************************************
 * Name: mvl_ge_dev_intr_dis
 *
 * Description: Disable the device interrupt(s).
 *		This function is expected to disable ALL of the device
 *		interrupt(s).
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_intr_dis (dev_object_t *dev)
{
    smi_t smi_d;	/* Data bytes for SMI */
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Setup SMI API interface struct */
    smi_d = 0;		/* Disable all interrupts */
    smi_if.buf = &smi_d;
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

    /* Disable Interrupt Control interrupts */
/* TBD */

    return (PASSED);
}

/*****************************************************************
 * Name: mvl_ge_dev_isr
 *
 * Description: Process and handle the interrupt(s) on this device.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_isr (dev_object_t *dev)
{
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Setup SMI API interface struct */
    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;

#if 0 /* TBD */
    /*
     * Read the interrupt status registers.
     * *** Need mechanism to pass status back to caller. print them out for now
     */
    for (i = 0, new_i2c_if.offset = IDT32021_INTERRUPT1_STS;
	 i < IDT32021_NUM_INTRS; i++, new_i2c_if.offset++) {
	 new_i2c_if.buf = &intr[i];

	/* Read the interrupt status from the IDT82V32021 */
	rc = callout_p->rd(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_dev_isr() read from %#x rc %#x\n",
				smi_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ISR);
	    return(FAILED);
	}
	printf("\nMarvell GE Interrupt Status %d = %#x\n", intr[i]);

	/* Write the same value back to clear */
	rc = callout_p->wr(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_dev_isr() write to %#x rc %#x\n",
				smi_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ISR);
	    return(FAILED);
	}

    }
#endif /* 0 TBD */

    /* Mask off the interrupts to prevent solid interrupts */
    return(mvl_ge_dev_intr_dis(dev));
}

/*****************************************************************
 * Name: mvl_ge_dev_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *	  A device print function vector
 *	  A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32_t
mvl_ge_dev_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32_t i, rc;
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
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_show() Page reg read failed. rc = %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	return(FAILED);
    }

    smi_if.buf = &reg_d;	/* Buffer address */

    /* Get the register info table */
    master_reg_p = mvl_ge_get_master_table(dev);
    if (master_reg_p == 0) {
	DEV_ERROR_REPORT(dev, "mvl_ge_dev_show() unable to get the table",
					MRVL_GE_SHOW);
	return(FAILED);
    }

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup the page */
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
	reg_d = master_reg_p->page;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    /* If we cannot write to the page register, then we cannot recover
	     * the page register.
	     */
	    sprintf(err_buf, "mvl_ge_dev_show() Page %#x write failed rc %#x",
					reg_d, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	    return(FAILED);
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
		    return(FAILED);
		}

		dev_print("%s @ %#.2x = %#.4x\n",
		       reg_p->name, reg_p->offset, reg_d);
		reg_p++;
	    } /* endof while */
	    break;
	case DEV_SHOW_BRIEF:
	    for (i = 0; i < MAX_SMI_REGS; i++) {
		/* Read the data bytes of Marvell GE */
		smi_if.offset = i;
		rc = (*callout_p->rd)(&smi_if);

		if (rc != PASSED) {
		    sprintf(err_buf, "mvl_ge_dev_show() brief read @ %x failed"
				     " rc = %#x", i, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
		    return(FAILED);
		}
		dev_print("%#.4x ", reg_d);
		if ((i & SMI_SHOW_MASK) == SMI_SHOW_MASK) {
		    dev_print("\n");
		}
	    } /* endof for */
	    break;
	default:
	    assert(!"mvl_ge_dev_show");
	    return(FAILED);
	    break;
	} /* endof switch */
	master_reg_p++;		/* Point to the next page */
    } /* endof while */

    /* Restore the page register */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_dev_show() Page reg restore failed rc %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_SHOW);
	return(FAILED);
    }

    return(PASSED);
}

/*****************************************************************
 * Name: mvl_ge_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *        A crash print function vector.
 *        A verbosity level.
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 *
 *****************************************************************/
static uint32_t
mvl_ge_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("mvl_ge_crsh(): No Crash info available for Marvell GE\n");
    return(PASSED);
}

/*****************************************************************
 * Name: mvl_ge_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
mvl_ge_destroy(dev_object_t **dev)
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

    if (mrvl_p->callout_fvt) {
#ifdef MEM_DEBUG
	printf("freeing callout @ %#x\n", mrvl_p->callout_fvt);
#endif /* MEM_DEBUG */
	free(mrvl_p->callout_fvt);	/* Free callout struct */
    }

    if (mrvl_p->callin_fvt) {
#ifdef MEM_DEBUG
	printf("freeing callin @ %#x\n", mrvl_p->callin_fvt);
#endif /* MEM_DEBUG */
	free(mrvl_p->callin_fvt);	/* Free callin struct */
    }

#ifdef MEM_DEBUG
    printf("freeing object @ %#x\n", mrvl_p->base.dev_object_fvt);
#endif /* MEM_DEBUG */
    free(mrvl_p->base.dev_object_fvt);	/* Free dev_object_t */

}

/**********************************************************************
 *
 * Function: mvl_ge_reg_test
 *
 * This function: tests Marvell GE registers.
 *		  Also check the ID of the chip.
 *
 * Input : dev - Pointer to the Marvell GE device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
mvl_ge_reg_test(dev_object_t *dev)
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
    smi_if.offset = MRV88E111N_PHYID_1_REG;	/* ID offset */

    original_smi_if = mrvl_p->smi_p;	/* Save the SMI if */
    mrvl_p->smi_p = &smi_if;		/* use local SMI if */

    /* Read the ID data byte of Marvell GE */
    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	sprintf(err_buf, "mvl_ge_reg_test() ID read failed with %#x.", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	return(FAILED);
    }

    if (data != MRV88E111N_PHY_ID_1_VALUE) {
	/* ID does not match to the chip type */
	mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	sprintf(err_buf, "mvl_ge_reg_test() ID read 0x%02x. Expect 0x%02x.",
            data, MRV88E111N_PHY_ID_1_VALUE);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	return(FAILED);
    }

    /* Get the register info table */
    master_reg_p = mvl_ge_get_master_table(dev);
    if (master_reg_p == 0) {
	mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	DEV_ERROR_REPORT(dev, "mvl_ge_reg_test() unable to get the table",
					MRVL_GE_REG_TEST);
	return(FAILED);
    }

    /* Save the page register */
    smi_if.buf = &original_page_reg;
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	mrvl_p->smi_p = original_smi_if;	/* restore SMI if */
	sprintf(err_buf, "mvl_ge_reg_test() Page reg read failed. rc = %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	return(FAILED);
    }

    smi_if.buf = &data;		/* setup test buffer again */

    /* Setup the parameter for the registers test */
    reg_ext.param = (void *)dev;

    /* Traverse through all pages */
    while (master_reg_p->reg_p) {
	/* Setup page */
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
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
	    return(FAILED);
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
	    return(FAILED);
	}
	master_reg_p++;
    }

    /* Restore page register */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;
    mrvl_p->smi_p = original_smi_if;		/* restore SMI if */

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_reg_test() Page reg restore failed rc %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_REG_TEST);
	return(FAILED);
    }

    return (rc);

}

/**********************************************************************
 *
 * Function:	mvl_ge_alt_reg
 *
 * This function: provides altering Marvell GE registers.
 *
 * Input:	dev - Pointer to the Marvell GE device object
 *
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int
mvl_ge_alt_reg(dev_object_t *dev)
{
    uint32_t rc, page_rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d, original_page_reg;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */

    /* Get the original Page register */
    smi_if.buf = &original_page_reg;
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_alt_reg() Page reg read failed. rc = %#x",
					rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ALTER);
	return(FAILED);
    }

    /* Ready to setup the page */
    smi_if.buf = &reg_d;	/* Buffer address */

    reg_d = gethex_answer("Enter the page: ", MRV88E111N_REG_PAGE_0,
			MRV88E111N_REG_PAGE_0, MRV88E111N_REG_PAGE_6);

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	/* If we cannot write to the page register, then we cannot recover
	 * the page register.
	 */
	sprintf(err_buf, "mvl_ge_alt_reg() Page %#x write failed rc %#x",
					reg_d, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ALTER);
	return(FAILED);
    }

    /* Got the page setup. Get the register */
    smi_if.offset = gethex_answer("Enter the register offset: ", 0,
			0, MAX_SMI_REGS - 1);

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	/* Unable to read, recover the page */
	sprintf(err_buf, "mvl_ge_alt_reg() Unable to read register. rc = %#x",
					rc);
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
	smi_if.buf = &original_page_reg;
	(*callout_p->wr)(&smi_if);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ALTER);
	return(FAILED);
    }

    /* Got the current data. Get the new data */
    reg_d = gethex_answer("Enter the new data: ", reg_d, 0, 0xFFFF);

    /* Write the new data */
    rc = (*callout_p->wr)(&smi_if);

    /* Recover the page */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;
    page_rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_alt_reg() Write failed with rc %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ALTER);
	return(FAILED);
    }

    if (page_rc != PASSED) {
	sprintf(err_buf, "mvl_ge_alt_reg() Unable to recover page %#x rc %#x",
					original_page_reg, page_rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_ALTER);
	return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function:	mvl_ge_set_test_mode
 *
 * This function: provides PHY test mode for Marvell GE PHYs.
 *
 * Input:	dev - Pointer to the Marvell GE device object
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int
mvl_ge_set_test_mode(dev_object_t *dev)
{
    uint32_t rc, page_rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d, original_page_reg, test_mode;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */

    /* Get the original Page register */
    smi_if.buf = &original_page_reg;
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    rc = (*callout_p->rd)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_test_mode() Page reg read failed. "
			 "rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_TEST_MODE);
	return(FAILED);
    }

    /* Got the current mode. */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_0,
			  MRV88E111N_1000B_CNTL_REG, &reg_d);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_test_mode() Unable to read test mode. "
			 "rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_TEST_MODE);
	return(FAILED);
    }

    test_mode = (reg_d & PHY_GT_CTL_TEST_MASK) >> (PHY_GT_CTL_TEST_SHIFT);

    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = gethex_answer("Enter the test mode: ", test_mode, 0, 
						       PHY_GT_CTL_TEST_MAX);

    /* Write the new data */
    reg_d &= (~PHY_GT_CTL_TEST_MASK); /* clear the test mode */
    reg_d |= (test_mode << PHY_GT_CTL_TEST_SHIFT);

    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_0,
			   MRV88E111N_1000B_CNTL_REG, &reg_d);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_test_mode() Unable to write test mode. "
			 "rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_TEST_MODE);
	return(FAILED);
    }

    /* Recover the page */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    smi_if.buf = &original_page_reg;
    page_rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_test_mode() Write failed with rc %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_TEST_MODE);
	return(FAILED);
    }

    if (page_rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_test_mode() Unable to recover page %#x. "
			 "rc = %#x", original_page_reg, page_rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_TEST_MODE);
	return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: mvl_ge_smi_rd
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
static int
mvl_ge_smi_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32_t rc;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)param;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t smi_buf;
    char err_buf[ERR_BUF_SIZE];
    uint8_t *data_buf = (uint8_t *)buf;

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
	return(FAILED);
    } else {
	/* Got the data */
        data_buf[2] = smi_buf >> 8 & 0xFF;
        data_buf[3] = smi_buf & 0xFF;
	    return(PASSED);
    }
}

/**********************************************************************
 *
 * Function: mvl_ge_smi_wr
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
static int
mvl_ge_smi_wr(ulong addr, int size, ulong data, void *param)
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
	return(FAILED);
    } else {
	/* Data written */
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function:	mvl_ge_get_master_table
 *
 * This function: returns the pointer to the beginning of register table
 *		  array.
 *
 * Input:	dev_object_t pointer to the Marvell GE device
 *
 * Output:	Pointer to the beginning of the register table array.
 *
 **********************************************************************
 */
static dev_mrvl_reg_info_t
*mvl_ge_get_master_table(dev_object_t *dev)
{
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;

    /* Check if the user provided the table */
    if (mrvl_p->reg_info_p) {
	/* User provided the table */
	return(mrvl_p->reg_info_p);
    } else {
	/* Use default table provided in this file */
	switch(mrvl_p->type) {
	case MRVL_GE_PHY_1112:
	    /* 88E1112 Copper and Fiber */
	    return(&ge_1112_reg_tbl[0]);
	    break;
	case MRVL_GE_PHY_1114:
	    /* 88E1114 Copper */
	    return(&ge_1114_reg_tbl[0]);
	    break;
	default:
	    /* Unknown type */
	    assert(!"mvl_ge_get_master_table");
	    return(0);
	    break;
	} /* endof switch */
    }
}

/**********************************************************************
 *
 * Function:	mvl_ge_set_loopback
 *
 * This function: sets the loopback mode to the Marvell GE device.
 *
 * Input:	dev_object_t pointer to the Marvell GE device.
 *		loopback mode - dev_mrvl_ge_loopback_t.
 *		speed - dev_mrvl_ge_mode_t.
 *		op - dev_mrvl_ge_loopback_op_t.
 *		auto_neg - TRUE to auto negotiate on the MAC side.
 *
 * Output:	None
 *
 **********************************************************************
 */
static int
mvl_ge_set_loopback(dev_object_t *dev, uint lpbk_mode,
		    uint speed, uint op, uint auto_neg)
{
    uint32_t rc;
    uint i;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d, mac_mode, phy_page, link_mask;
    char err_buf[ERR_BUF_SIZE];

#ifdef PHY_DEBUG
    printf("mode %#x, speed %#x, op %#x\n",
	    lpbk_mode, speed, op);
#endif /* PHY_DEBUG */

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;	/* Setup SMI interface */

    switch (op) {
    case LPBKOP_SAVE:
	/* Get the original Page register */
	smi_if.buf = &mrvl_p->regs.page;
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

	rc = (*callout_p->rd)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Page reg read rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Get the MAC control register */
	rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_2,
			      MRV88E111M_CONTROL_REG, &mrvl_p->regs.mac_ctrl);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() MAC control read failed. "
			     "rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Get the MAC Specific Control 1 register */
	rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_2,
		   MRV88E111M_SPECIFIC_CONTROL1_REG, &mrvl_p->regs.mac_sp_ctl1);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() MAC specific control 1 "
			     "read failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Get the Packet Generation register */
	rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_6,
			      MRV88E111N_PACKET_GEN_REG, &mrvl_p->regs.pkt_gen);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Pkt Gen reg read failed. "
			     "rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Always enable packet CRC checking */
	mrvl_p->regs.pkt_gen |= PHY_PKT_CRC_EN;

	/* Make sure the Stub bit is cleared from last SFP or external
	 * loopback. Without the clearing, link won't come up sometimes.
	 */
	reg_d = mrvl_p->regs.pkt_gen & (~PHY_PKT_STUB_EN);

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_6,
			      MRV88E111N_PACKET_GEN_REG, &reg_d);

	if (rc != PASSED) {
	    sprintf(err_buf, "%s() Stub clearing write failed. rc = %#x",
			     __FUNCTION__, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	break;
    case LPBKOP_SET:
	/* Setup MAC of the PHY side first */
	mac_mode = 0;

	/* Section 2.4 of 1114 data sheet (2/27/2007) -
	 * Before performing any other operations the 88E114 device requires 
	 * the following register initialization with sequences after power up,
	 * or the following at hardware reset:
	 * 1. Register 22 = 0x0002
	`* 2. Register 16_2.9:7 = 0x5.
	 * 3. Register 22 = 0x8000.
	 * 4. Register 0.15 = 0x1.
	 * Step 1 is already done. Make sure step 2 is handled here. Step 4
	 * will be done later.
	 */
	 /* Marvell "recommends to force the mode to copper only mode for any
	  * loopback test (internal or external stub loopback)." Refer to
	  * email dated 9/29/2008 4:30 PM
	  */
	 if (lpbk_mode == LOOPBACK_SFP) {
	    /* SFP Encoding type of 8B10B and SONET scrambled will use
	     * Mode of 11 which is converted to MAC Specific Control
	     * register 1's Mode Select bits of 011 (Auto Copper/1000Base-X)
	     * 4B5B will use mode of 00 - 000 (100Base-FX). Instead of
	     * calling the SFP I2C to get the mode, we are using
	     * the speed passed to the common device driver. 100M for
	     * 4B5B. 1G for others.
	     */
	    switch (speed) {
	    case ETH_MODE_FE100:
		if (callout_p->sfp_setup) {
		    /* GLC-GE-100FX uses SGMII */
		    mac_mode = MAC_SP_CTL1_SGM_MODE;
		} else {
		    mac_mode = MAC_SP_CTL1_FX_MODE;
		}

		break;
	    case ETH_MODE_GE:
		mac_mode = MAC_SP_CTL1_A_X_MODE;
		break;
	    default:
		sprintf(err_buf, "mvl_ge_set_loopback() invalid speed %#x "
				 "for SFP loopback setup", speed);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
		break;
	    } /* endof switch */
	} else {
	    mac_mode = MAC_SP_CTL1_CU_MODE;
	} /* endof if lpbk_mode type */

	reg_d = mrvl_p->regs.mac_sp_ctl1 & (~MAC_SP_CTL1_MODE_MASK);
	reg_d |= mac_mode; /* */

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_2,
			      MRV88E111M_SPECIFIC_CONTROL1_REG, &reg_d);;
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() MAC specific control 1"
			     " register write failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	} /* endof if rc */

	/* MAC does not have duplex bit. Set the speed in MAC */
	/* Turn off Auto-Neg per 88E1114 Rev C2 Release notes, page 5 */
	reg_d = mrvl_p->regs.mac_ctrl & ~(PHY_CTRL_MASK | PHY_CTRL_AUTO_NEG |
					  PHY_CTRL_PWR_DWN);
	if (auto_neg == TRUE) {
	    reg_d |= (PHY_CTRL_RESET | PHY_CTRL_AUTO_NEG);
	} else {
	    reg_d |= PHY_CTRL_RESET;	/* Reset to keep new config */
	}

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_2,
			      MRV88E111M_CONTROL_REG, &reg_d);
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() write %#x to MAC control "
			     "failed. rc = %#x", reg_d, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	if (poll_sw_reset(dev) == FAILED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Unable to reset after "
			     "disabling MAC auto neg");
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	switch(speed) {
	case ETH_MODE_FE10:
	    reg_d |= PHY_CTRL_10;
	    break;
	case ETH_MODE_FE100:
	    reg_d |= PHY_CTRL_100;
	    break;
	case ETH_MODE_GE:
	    reg_d |= PHY_CTRL_1000;
	    break;
	}

#ifdef PHY_DEBUG
	printf("writing %#x ( %#x ) to offset %#x\n", *smi_if.buf,
				reg_d, smi_if.offset);
#endif /* PHY_DEBUG */

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_2,
			      MRV88E111M_CONTROL_REG, &reg_d);
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() write %#x to MAC control "
			     "failed. rc = %#x", reg_d, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	if (poll_sw_reset(dev) == FAILED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Unable to reset after "
			     "setting the MAC speed");
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	break;
    case LPBKOP_RESTORE:
#ifdef PHY_STAT_MONITOR
	/* Print the CRC counter */
	rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_6,
			      MRV88E111N_CRC_CHKR_REG, &reg_d);
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() restore. CRC counter read "
			     "failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}
	prpass(testpass, "CRC counter = %#x before restores", reg_d);
#endif /* PHY_STAT_MONITOR */

	break;
    } /* endof switch op */

    /* CSCsv40919 requires to use Class A in the Copper */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_0,
			  MRV88E111N_SPECIFIC_CONTROL2_REG, &reg_d);
    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_loopback() Copper Specific Control 2 "
			 "register read failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);
    }

    reg_d &= (~PG0_SP_CTL2_TX_MASK);	/* clear the mask */
    reg_d |= PG0_SP_CTL2_TX_A;		/* set Class A */

    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_0,
			  MRV88E111N_SPECIFIC_CONTROL2_REG, &reg_d);
    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_set_loopback() Copper Specific Control 2 "
			 "register write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);
    }

    /******* From this point on, the page is the PHY page, and don't modify
     ******* other page registers.
     * If needed, use phy_page setup here.
     */

    /* Ready to setup the page */
    smi_if.buf = &reg_d;	/* Buffer address */
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    /* Setup the page table */
    switch(mrvl_p->type) {
    case MRVL_GE_PHY_1112:
	/* 88E1112 Copper and Fiber */
	if (lpbk_mode == LOOPBACK_SFP) {
	    /* SFP loopback mode */
	    phy_page = MRV88E111N_REG_PAGE_1;
	} else {
	    /* RJ45 loopback mode */
	    phy_page = MRV88E111N_REG_PAGE_0;
	}
	break;
    case MRVL_GE_PHY_1114:
	/* 88E1114 Copper */
	phy_page = MRV88E111N_REG_PAGE_0;
	break;
    default:
	/* Unknown type */
	assert(!"mvl_ge_set_loopback");
	phy_page = MRV88E111N_REG_PAGE_0;
	break;
    } /* endof switch */

    reg_d = phy_page;
    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	/* If we cannot write to the page register, then we cannot recover
	 * the page register.
	 */
	sprintf(err_buf, "mvl_ge_set_loopback() Set Page failed rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);
    }

    switch(op) {
    case LPBKOP_SAVE:
	/* Save PHY Specific Control register 1 */
	smi_if.buf = &mrvl_p->regs.sp_ctl1;
	smi_if.offset = MRV88E111N_SPECIFIC_CONTROL1_REG;

	rc = (*callout_p->rd)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() read special ctrl rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Save Control register */
	smi_if.buf = &mrvl_p->regs.control;
	smi_if.offset = MRV88E111N_CONTROL_REG;

	rc = (*callout_p->rd)(&smi_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() read Control reg rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	return(rc);
	break;
    case LPBKOP_RESTORE:
	/* Restore PHY Specific Control register 1 */
	smi_if.buf = &mrvl_p->regs.sp_ctl1;
	smi_if.offset = MRV88E111N_SPECIFIC_CONTROL1_REG;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() write special ctl rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Restore Control register */
	reg_d = mrvl_p->regs.control | PHY_CTRL_RESET;
	smi_if.buf = &reg_d;
	smi_if.offset = MRV88E111N_CONTROL_REG;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() write control reg rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	if (poll_sw_reset(dev) == FAILED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Unable to reset after "
			     "restoring PHY control register");
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}   /* endof if poll */

	/* Restore the MAC control register */
	reg_d = mrvl_p->regs.mac_ctrl | PHY_CTRL_RESET;

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_2,
			      MRV88E111M_CONTROL_REG, &reg_d);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() MAC control write failed. "
			     "rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_2,
		   MRV88E111M_SPECIFIC_CONTROL1_REG, &mrvl_p->regs.mac_sp_ctl1);
	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() MAC specific control 1 "
			     " write failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Restore the Packet Generation register */
	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_6,
			      MRV88E111N_PACKET_GEN_REG, &mrvl_p->regs.pkt_gen);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Pkt Gen reg write failed. "
			     "rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Restore the page register */
	smi_if.buf = &mrvl_p->regs.page;
	smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

	rc = (*callout_p->wr)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "mvl_ge_set_loopback() Page reg write rc = %#x",
					rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	}

	return (rc);
	break;
    case LPBKOP_SET:
	/* Configure the PHY side */
	/******* Note that the page is set to the media at this point ****/
	smi_if.buf = &reg_d;	/* Setup the write buffer */

	if (lpbk_mode == LOOPBACK_PHY) {
	    smi_if.offset = MRV88E111N_CONTROL_REG;

	    /* Turn off auto negotiate and set the soft reset */
	    reg_d = mrvl_p->regs.control & (~(PHY_CTRL_AUTO_NEG |
					     PHY_CTRL_MASK | PHY_CTRL_PWR_DWN));
	    reg_d |= (PHY_CTRL_RESET | PHY_CTRL_DUPLEX);

	    /* Set the speed */
	    switch(speed) {
	    case ETH_MODE_FE10:
		reg_d |= PHY_CTRL_10;
		break;
	    case ETH_MODE_FE100:
		reg_d |= PHY_CTRL_100;
		break;
	    case ETH_MODE_GE:
		reg_d |= PHY_CTRL_1000;
		break;
	    } /* endof switch */

	    rc = (*callout_p->wr)(&smi_if);
	    if (rc != PASSED) {
		sprintf(err_buf, "mvl_ge_set_loopback() Reset (%#x) "
				 "failed. rc = %#x", reg_d, rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    } /* endof if rc */

	    if (poll_sw_reset(dev) == FAILED) {
		sprintf(err_buf, "mvl_ge_set_loopback() Unable to reset after "
				 "disabling auto negtiate");
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    }   /* endof if poll */

	    reg_d &= (~PHY_CTRL_RESET);
	    reg_d |= PHY_CTRL_LOOPBACK;

	    rc = (*callout_p->wr)(&smi_if);
	    if (rc == PASSED) {
		/* Reset the Packet CRC counter */
		rc = clear_pkt_cnt(dev);

		if (rc != PASSED) {
		    return(FAILED);
		}

		/* Read status 2 and check if any bits set */
		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
				      MRV88E111N_SPECIFIC_STATUS2_REG, &reg_d);
		if (rc != PASSED) {
		    sprintf(err_buf, "%s() Stat 2 read rc = %#x",
				     __FUNCTION__, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		    return(FAILED);
		}
		prpass(testpass, "Status 2 after link up = %#x", reg_d);

#ifdef PHY_STAT_MONITOR
		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
				      MRV88E111N_REC_ERROR_COUNTER_REG, &reg_d);
		if (rc != PASSED) {
		    sprintf(err_buf, "%s() Rx Count read rc = %#x",
				     __FUNCTION__, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		    return(FAILED);
		}
		prpass(testpass, "Receive Error Counter = %#x", reg_d);

		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_6,
				      MRV88E111N_CRC_CHKR_REG, &reg_d);
		if (rc != PASSED) {
		    sprintf(err_buf, "%s() CRC Counter read rc = %#x",
				     __FUNCTION__, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		    return(FAILED);
		}
		prpass(testpass, "CRC counter = %#x after link up", reg_d);

#endif /* PHY_STAT_MONITOR */

		return(PASSED);
	    } else {
		sprintf(err_buf, "mvl_ge_set_loopback() write loopback to "
				 " control register failed. rc = %#x", rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    }
	} else {	/* External loopback */

	    /* Copper needs to disable Auto MDI and energy detect */
	    smi_if.offset = MRV88E111N_SPECIFIC_CONTROL1_REG;
	    reg_d = mrvl_p->regs.sp_ctl1 & ~(PHY_SP_CTL1_ENERGY_DET +
					     PHY_SP_CTL1_MDIX_MASK);

	    /* For 88E1112. Check for SFP loopback */
	    if (lpbk_mode == LOOPBACK_SFP) {
		/* Set the SIGDET Polarity. From Marvell - */
		/* "On the SFP/Fiber transceiver, there is a pin called LOS
		 * or SD. Typically SFP use LOS (loss of signal) while Fiber
		 * transceiver uses SD (signal detect). They have opposite
		 * polarity when active. ..."
		 */
		 reg_d |= PHY_SP_CTL1_SIGLOS;
	    } /* endof if lpbk_mode */

	    rc = (*callout_p->wr)(&smi_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "mvl_ge_set_loopback() write %#x to special "
				 "control 1 reg. failed. rc = %#x",
				  reg_d, rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    } /* endof if rc */

	    /* Setup PHY speed, and loopback mode */
	    reg_d = mrvl_p->regs.control & ~(PHY_CTRL_LOOPBACK | PHY_CTRL_MASK |
					  PHY_CTRL_AUTO_NEG | PHY_CTRL_PWR_DWN);

	    /* Set Full duplex, and also reset to keep new config. */
	    reg_d |= (PHY_CTRL_RESET | PHY_CTRL_DUPLEX);

	    switch(speed) {
	    case ETH_MODE_FE10:
		reg_d |= PHY_CTRL_10;
		break;
	    case ETH_MODE_FE100:
		reg_d |= PHY_CTRL_100;
		break;
	    case ETH_MODE_GE:
		reg_d |= PHY_CTRL_1000;
		break;
	    } /* endof switch */

	    if ((lpbk_mode == LOOPBACK_SFP) && callout_p->sfp_setup){
		/* GE-100FX has to use AutoNegotiation */
		/* Needs to setup the SFP - GLC-FE-100FX */
		if ((*callout_p->sfp_setup)(smi_if.smi_dev) != PASSED) {
		    sprintf(err_buf, "%s() Unable to setup SFP for operation",
				      __FUNCTION__);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		    return(FAILED);
		}
		mdelay(LINK_DELAY);
		reg_d |= PHY_CTRL_AUTO_NEG;
	    }

	    smi_if.offset = MRV88E111N_CONTROL_REG;
#ifdef PHY_DEBUG
	    printf("writing %#x ( %#x ) to offset %#x\n", *smi_if.buf,
			reg_d, smi_if.offset);
#endif /* PHY_DEBUG */

	    rc = (*callout_p->wr)(&smi_if);
	    if (rc != PASSED) {
		sprintf(err_buf, "mvl_ge_set_loopback() write %#x to control "
				 "register failed. rc = %#x", reg_d, rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    } /* endof if rc */

	    if (poll_sw_reset(dev) == FAILED) {
		sprintf(err_buf, "mvl_ge_set_loopback() Unable to reset after "
				 "setting PHY control");
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    }	/* endof if poll */

#ifdef PHY_DEBUG
	    if ((*callout_p->rd)(&smi_if) != PASSED) {
		printf("cannot read it back\n");
		return(FAILED);
	    }
	    printf(" control = %#x ( %#x )\n", *smi_if.buf, reg_d);
#endif /* PHY_DEBUG */

	} /* endof if lpbk_mode */

	/* Section 2.3.4.1 of 1112 data sheet (2/21/2007) -
	 * For 10BASE-T and 100BASE-TX modes, the loopback test requires no
	 * register writes. For 1000BASE-T mode, register 16_6.5 must be set
	 * to 1 to enable the external loopback. ...
	 */
	if (((lpbk_mode == LOOPBACK_EXT) || (lpbk_mode == LOOPBACK_SFP)) &&
	     (speed == ETH_MODE_GE)) {
	    /* External loopback with 1000BASE-T */
	    /* Instead of reading the register then write, use the one in the
	     * saved during the LPBKOP_SAVE step.
	     */
	    reg_d = mrvl_p->regs.pkt_gen | PHY_PKT_STUB_EN;
	} else {
	    reg_d = mrvl_p->regs.pkt_gen & (~PHY_PKT_STUB_EN);
	}

	/* Set the stub first */
	rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_6,
			      MRV88E111N_PACKET_GEN_REG, &reg_d);
	if (rc != PASSED) {
	    sprintf(err_buf, "%s() stub setup failed. rc = %#x",
			     __FUNCTION__, rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	    return(FAILED);
	}

	/* Reset the Packet CRC counter */
	rc = clear_pkt_cnt(dev);

	if (rc != PASSED) {
	    return(FAILED);
	}

	if (lpbk_mode == LOOPBACK_SFP) {
	    /* Enable the SFP */
	    (*callout_p->sfp_op)(smi_if.smi_dev, ENABLE);
	}

	/* Needs to reset the page */
	/* check linkup */
	smi_if.offset = MRV88E111N_SPECIFIC_STATUS1_REG;

	for (i = 0; i < (LINK_MAX_TIME * 2); i++) {
	    if ((lpbk_mode == LOOPBACK_SFP) && callout_p->sfp_setup){
		/* GLC-GE-100FX SFP uses SGMII mode. With SGMII mode, different
		 * status register nad bit are checked for the link up
		 */
		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
				      MRV88E111N_LINK_PART_AV_REG, &reg_d);
		link_mask = LPA_SGMII_LINK;
	    } else {
		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
				      MRV88E111N_SPECIFIC_STATUS1_REG, &reg_d);
		link_mask = PHY_SP_STA1_LINKUP;
	    }

	    if (rc != PASSED) {
		sprintf(err_buf, "mvl_ge_set_loopback() linkup read rc = %#x",
				rc);
		DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		return(FAILED);
	    }

	    if (reg_d & link_mask) {
		/* Link up */
                /* Read status 2 and check if any bits set */
                rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
                                      MRV88E111N_SPECIFIC_STATUS2_REG, &reg_d);
                if (rc != PASSED) {
                    sprintf(err_buf, "mvl_ge_set_loopback() Stat 2 read rc = "
                                     "%#x", rc);
                    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
                    return(FAILED);
                }
                if (reg_d & PG0_SP_ST2_ERR_MSK) {
                    printf("\nStatus 2 after link up = %#x\n", reg_d);
                }

#ifdef PHY_STAT_MONITOR
                rc = mvl_ge_rd_wr_reg(dev, PHY_READ, phy_page,
                                      MRV88E111N_REC_ERROR_COUNTER_REG, &reg_d);
                if (rc != PASSED) {
                    sprintf(err_buf, "mvl_ge_set_loopback() Rx Count read rc = "
                                     "%#x", rc);
                    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
                    return(FAILED);
                }
                if (reg_d) {
                    printf("\nReceive Error Counter = %#x\n", reg_d);
                }

		rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_6,
				      MRV88E111N_CRC_CHKR_REG, &reg_d);
		if (rc != PASSED) {
		    sprintf(err_buf, "%s() CRC Counter read rc = %#x",
				     __FUNCTION__, rc);
		    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
		    return(FAILED);
		}
		prpass(testpass, "CRC counter = %#x after link up", reg_d);

#endif /* PHY_STAT_MONITOR */

#ifdef PHY_DEBUG
		printf("\n Link up after %d ms\n", i);
#endif /* PHY_DEBUG */
		return(PASSED);
	    }
	    mdelay(LINK_DELAY);
	}

#ifdef PHY_DEBUG
	printf("special status 1 = %#x ( %#x )\n", *smi_if.buf, reg_d);
#endif /* PHY_DEBUG */
	sprintf(err_buf, "mvl_ge_set_loopback() %s loopback linkup timedout",
			 (lpbk_mode == LOOPBACK_SFP) ? "SFP" : "External");
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);

    } /* endof if op */

    /* should not reach this point. If it does, return failed */
    return(FAILED);
}

/**********************************************************************
 *
 * Function:	mvl_ge_rd_wr_reg
 *
 * This function: reads or writes a register of specified page.
 *
 * Input:	dev_object_t pointer to the Marvell GE device.
 *		op - read or write.
 *		page - Page number.
 *		offset - register offset.
 *		data - Points to data of read/write
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int
mvl_ge_rd_wr_reg(dev_object_t *dev, uint op, smi_t page, uchar offset,
		smi_t *data)
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
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;

    rc = (*callout_p->wr)(&smi_if);

    if (rc != PASSED) {
	sprintf(err_buf, "mvl_ge_rd_wr_reg() set page %#x failed. rc = %#x",
				page, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_PAGE);
	return(FAILED);
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

    return(rc);
}

/**********************************************************************
 *
 * Function:	poll_sw_reset
 *
 * This function: polls the Soft Reset bit in the control register.
 *
 * Input:	dev_object_t pointer to the Marvell GE device.
 *
 * Output:	PASSED/FAILED
 *
 * Assumption:	The caller sets up the page of the Control register to be
 *		polled.
 *
 **********************************************************************
 */
static int
poll_sw_reset(dev_object_t *dev)
{
    uint32_t rc;
    int i;
    smi_if_t smi_if;
    dev_mrvl_ge_object_t *mrvl_p = (dev_mrvl_ge_object_t *)dev;
    mrvl_ge_callout_fvt_t *callout_p = mrvl_p->callout_fvt;
    smi_t reg_d;
    char err_buf[ERR_BUF_SIZE];

    smi_if.smi_dev = mrvl_p->smi_p->smi_dev;    /* Setup SMI interface */

    smi_if.buf = &reg_d;
    smi_if.offset = MRV88E111N_CONTROL_REG;

    for (i = 0; i < PHY_RESET_DELAY; i++) {
	rc = (*callout_p->rd)(&smi_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "poll_sw_reset() failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_PAGE);
	    return(FAILED);
	}
	if ((reg_d & PHY_CTRL_RESET) == 0 ) {
	    /* Out of reset */
	    /* Refer to 88E1114 Release Notes, section 4.8 */
	    mdelay(PHY_AUTO_NEG_DELAY);
	    return(PASSED);
	}
	mdelay(LINK_DELAY);	/* Poll every millisecond */
    }

    return(FAILED);	/* Timedout */

}

/**********************************************************************
 *
 * Function:	clear_pkt_cnt
 *
 * This function: clears the Packet CRC counter register.
 *
 * Input:	dev_object_t pointer to the Marvell GE device.
 *
 * Output:	PASSED/FAILED
 *
 * Assumption:	The caller is required to have the device created.
 *
 **********************************************************************
 */
static uint32_t
clear_pkt_cnt(dev_object_t *dev)
{
    uint32_t rc;
    smi_t reg_d;
    char err_buf[ERR_BUF_SIZE];

    /* Read the register */
    rc = mvl_ge_rd_wr_reg(dev, PHY_READ, MRV88E111N_REG_PAGE_6,
			  MRV88E111N_PACKET_GEN_REG, &reg_d);
    if (rc != PASSED) {
	sprintf(err_buf, "%s Unable to read Packet CRC register. rc = %#x",
			 __FUNCTION__, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);
    }

    /* Setup the packet generation register, with CRC counter cleared */
    reg_d &= (~PHY_PKT_CRC_EN);

    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_6,
			  MRV88E111N_PACKET_GEN_REG, &reg_d);
    if (rc != PASSED) {
	sprintf(err_buf, "%s clear CRC checker failed. rc = %#x",
			 __FUNCTION__, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
	return(FAILED);
    }

    /* Re-enable the counter */
    reg_d |= PHY_PKT_CRC_EN;

    rc = mvl_ge_rd_wr_reg(dev, PHY_WRITE, MRV88E111N_REG_PAGE_6,
			  MRV88E111N_PACKET_GEN_REG, &reg_d);
    if (rc != PASSED) {
	sprintf(err_buf, "%s CRC checker enable failed. rc = %#x",
			 __FUNCTION__, rc);
	DEV_ERROR_REPORT(dev, err_buf, MRVL_GE_LPBK);
    }
    return(rc);
}

/******** History ******** 
 * $Log: dev_mrvl_ge.c,v $
 * Revision 1.2  2013/10/08 08:48:25  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:58:01  tirawan
 * First Woodlawn linux integration
 *
 * Revision 1.2  2013/03/27 04:49:44  kuangik
 * Code cleanup after -Wall
 *
 * Revision 1.1  2013/03/13 06:42:07  kuangik
 * Add for the first time
 *
 * Revision 1.6  2013/03/08 09:35:46  kuangik
 * Clear warning
 *
 * Revision 1.5  2013/03/06 11:19:30  kuangik
 * Fix for register access
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
 * Revision 1.4.24.1  2011/03/11 21:38:41  mcharon
 * initial support informers linux
 *
 * Revision 1.4  2009/12/17 02:16:24  siyen
 * Removal of cterr() (CSCtd86918).
 *
 * Revision 1.3  2009/12/16 18:33:00  siyen
 * GLC-GE-100FX SFP Link timeout fix (CSCtd86918).
 *
 * Revision 1.2  2009/10/27 23:01:48  siyen
 * Clear Power Down bit in case ROMMON/BIOS sets it for power management.
 *
 * Revision 1.1.1.1  2009/10/17 02:05:27  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.10.6.5.2.1  2009/10/14 01:06:57  huyhoang
 * + Add DiagLinux support to ngd-diag-rep repository
 *
 * Revision 1.10.6.5  2009/09/28 23:57:57  siyen
 * Added loopback type during link time out failure.
 *
 * Revision 1.10.6.4  2009/08/04 03:37:14  iachang
 * Fixed "PHY 88E1112 register test" issue
 *
 * Revision 1.10.6.3  2009/06/20 01:53:16  siyen
 * Remove REV1_BRINGUP
 *
 * Revision 1.10.6.2  2009/06/08 12:43:49  sctsai
 * Sync with ngd-informers3-060509 repository.
 *
 * Revision 1.10.6.1  2009/06/04 09:30:00  sctsai
 * Sync with informers2-tag-060209 repository.
 *
 * Revision 1.11  2009/06/02 22:48:21  siyen
 * Clear Stub bit used for external and SFP loopback before each loopback test. (CSCsz95764).
 *
 * Revision 1.10  2009/05/20 21:05:49  aosulliv
 * Sync of xformers-branch to ngd-diags-rep
 *
 * Revision 1.8.2.4  2009/06/01 17:31:24  siyen
 * Sync up to the main trunk for GLC-GE-100FX SFP support.
 *
 * Revision 1.8.2.3  2009/04/09 17:20:31  sctsai
 * The changes:
 * 1.Create a new header file "endians.h" to support both gcc & icc.
 *   .c file should include it at the beginning, if need.
 * 2.Change INFORMERS_ICC to INTEL_ICC
 * 3.Move all x86 related files to le directory.
 * 4.Temporarily, change FPGA path to /auto/sp-engops/diags/pld_icc.
 *
 * Revision 1.8.2.2  2009/03/16 22:48:18  sctsai
 * Sync with informers-tag-031309-sync repository.
 *
 * Revision 1.5.2.7  2009/03/13 22:37:02  ptong
 * Sync with ngd-informers-031209 repository.
 *
 * Revision 1.5.2.6  2009/03/07 04:55:23  siyen
 * Skip some registers test to avoid watchdog timer interrupts for GE PHY0.
 *
 * Revision 1.5.2.5  2009/02/19 03:54:11  siyen
 * Added malloc/free debug capability
 *
 * Revision 1.5.2.4  2009/02/18 22:52:14  siyen
 * Skip NVRAM bits during registers tests. They are self cleared.
 *
 * Revision 1.5.2.3  2009/01/23 02:24:26  shhuang
 *  Sync with ngd-informers-012109 repository.
 *
 * Revision 1.9  2009/02/20 22:33:48  aosulliv
 * Sync of xformers-tag-022009 to main
 *
 * Revision 1.8  2009/01/19 18:51:14  aosulliv
 * sync of xformers-tag-011609 to main ngd-diags-rep
 *
 * Revision 1.7  2008/11/13 21:06:46  aosulliv
 * Sync xformers-tag-121208 to the ngd diag repository
 *
 * Revision 1.3.2.9  2008/12/20 01:14:25  siyen
 * Clear status and counter after PHY loopback setup. (CSCso95664).
 *
 * Revision 1.3.2.8  2008/11/06 22:47:28  siyen
 * Clear packet stub for all tests other than external loopback.
 *
 * Revision 1.3.2.7  2008/10/31 18:02:03  siyen
 * Set to Class A for copper per hardware request. (CSCsv40919)
 *
 * Revision 1.3.2.6  2008/10/04 00:23:18  siyen
 * Disable MDIX and Energy detect for both 1112 and 1114 per Marvell recommendation. (CSCsr23763).
 *
 * Revision 1.3.2.5  2008/09/30 21:39:11  siyen
 * Use copper mode only for RJ45 and internal loopback. Also changed configuration ordering per Marvell recommendation. (CSCsr23763).
 *
 * Revision 1.3.2.4  2008/09/16 23:11:22  siyen
 * Enable CRC packet/error counters for debugging.
 *
 * Revision 1.3.2.3  2008/07/13 20:55:34  siyen
 * Added GE PHY loopback support for Cavium based platforms. (CSCsq64012)
 *
 * Revision 1.3.2.2  2008/04/15 02:12:53  siyen
 * Added GLC-GE-100FX SFP supports.
 *
 * Revision 1.3.2.1  2008/03/18 01:01:10  siyen
 * Added Missing PHY config for SFP loopback with ROMMON.
 *
 * Revision 1.3  2008/02/29 02:22:35  siyen
 * Added Loopback tests supports, and the test mode utility.
 *
 * Revision 1.2  2008/01/17 17:39:07  siyen
 * Added loopback setup supports.
 *
 * Revision 1.1  2007/12/26 22:16:56  siyen
 * Initial check-in.
 *
 * $Endlog$
 */
