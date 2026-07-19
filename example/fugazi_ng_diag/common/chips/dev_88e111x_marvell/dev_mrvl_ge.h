/* $Id: dev_mrvl_ge.h,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e111x_marvell/dev_mrvl_ge.h,v $
 ***********************************************************************
 * File Name: dev_mrvl_ge.h
 *
 * Description: Contains common definitions specific for the Marvell GE
 *		PHYs and Switches
 *
 * Copyright (c)2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#ifndef __DEV_MRVL_GE_H__
#define __DEV_MRVL_GE_H__

#include "dev_object.h"
#include "smi_api.h"
#include "common_utils.h"

typedef struct mrvl_ge_callout_fvt_ {
    uint32_t (*open)(smi_if_t *smi_if_p);
    uint32_t (*close)(smi_if_t *smi_if_p);
    uint32_t (*rd)(smi_if_t *smi_if_p);
    uint32_t (*wr)(smi_if_t *smi_if_p);
    int (*dev_reset)(boolean);
    void (*sfp_op)(int, int);
    int (*sfp_setup)(int);
} mrvl_ge_callout_fvt_t;

typedef struct mrvl_ge_callin_fvt_ {
    int  (*register_test)(dev_object_t *);
    int  (*set_loopback)(dev_object_t *, uint, uint, uint, uint);
    int	 (*alter_reg)(dev_object_t *);
    int  (*set_test_mode)(dev_object_t *);
} mrvl_ge_callin_fvt_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    MRVL_GE_DEV_STATE = 0,
    MRVL_GE_ATTACH,
    MRVL_GE_DETACH,
    MRVL_GE_INIT,
    MRVL_GE_SHOW,
    MRVL_GE_DESTROY,
    MRVL_GE_REG_TEST,
    MRVL_GE_RESET,
    MRVL_GE_READ,
    MRVL_GE_WRITE,
    MRVL_GE_ALTER,
    MRVL_GE_LPBK,
    MRVL_GE_PAGE,
    MRVL_GE_TEST_MODE,
}idt6v4900n_report_code_t;

/*
 * Marvell GE PHYs have multiple pages. This struct defines the registers
 * and the page associate with them.
 */
typedef struct dev_mrvl_reg_info_t_ {
    smi_t 	page;		/* page number */
    reg_info_t	*reg_p;		/* Registers table */
} dev_mrvl_reg_info_t;

/*
 * PHY registers changed for loopback.
 */
typedef struct dev_mrvl_lpbk_regs_t_ {
    smi_t page;			/* Page number */
    smi_t sp_ctl1;		/* Specific Control Register 1 */
    smi_t control;		/* Control Register */
    smi_t mac_ctrl;		/* MAC Control Register */
    smi_t mac_sp_ctl1;		/* MAC Specific Control Register 1 */
    smi_t pkt_gen;		/* Packet Generation */
} dev_mrvl_lpbk_regs_t;

/*
 * Define the Marvell GE device object structure.
 */
typedef struct dev_mrvl_ge_object_t_ {
    dev_object_t	base;
    mrvl_ge_callout_fvt_t *callout_fvt;
    mrvl_ge_callin_fvt_t  *callin_fvt;
    smi_if_t		*smi_p;		/* SMI API interface struct */
    dev_mrvl_reg_info_t *reg_info_p;	/* Registers table */
    dev_mrvl_lpbk_regs_t regs;		/* Saved registers for loopback */
    uint8_t		type;		/* PHY device type */
}dev_mrvl_ge_object_t;

/* GE type */
typedef enum {
    MRVL_GE_PHY_1112 = 0,	/* GE PHY with Copper and Optic */
    MRVL_GE_PHY_1114,		/* GE PHY with Copper only */
    MRVL_GE_PHY_INVALID,	/* Invalid GE PHY */
} dev_mrvl_ge_type_t;

typedef enum {
    ETH_MODE_FE10 = 0,
    ETH_MODE_FE100,
    ETH_MODE_GE,
    ETH_MODE_INVALID,
} dev_mrvl_ge_mode_t;

typedef enum {
    LOOPBACK_NONE = 0,
    LOOPBACK_MAC,		/* internal loopback at CPU */
    LOOPBACK_PHY,		/* internal loopback at marvell ge phy */
    LOOPBACK_EXT,		/* RJ45 external loopback */
    LOOPBACK_SFP,		/* SFP external loopback */
    LOOPBACK_INVALID,		/* Invalid loopback */
} dev_mrvl_ge_loopback_t;

typedef enum {
    LPBKOP_SAVE = 0,
    LPBKOP_RESTORE,
    LPBKOP_SET,
} dev_mrvl_ge_loopback_op_t;

typedef enum {
    PHY_READ = 0,
    PHY_WRITE,
} dev_mrvl_ge_smi_op_t;

/*
 * General Defines
 */
#define SW_LOG_BUF_SIZE                         384
#define INCREMENT_COUNT                         1
#define DECREMENT_COUNT                         -1
#define CONSTANT_COUNT                          0
#define ERR_BUF_SIZE				80
#define MAX_SMI_REGS				0x20
#define SMI_SHOW_MASK				0xF
#define PHY_RESET_DELAY			100	/* 100 milliseconds delay */
/* Refer to 88E1114 Rev C2 Release Notes. Section 4.8 -
 * If Auto=-Neg is turned off and the forced speed of 10 Mbps or 100 Mbps is
 * programmed by writing to Register 0, then there will be a 637 ms delay
 * before the PHY switches to the programmed speed. There is no delay if
 * switching to gigabit speed.
 * ... (Wait at least 700ms to give a little margin on top of the 637ms).
 * Diag will have the PHY_AUTO_NEG_DELAY regardless of the speed.
 */
#define PHY_AUTO_NEG_DELAY		1000	/* 700 milliseconds delay. But
						 * we extend it to 1 second */

#define SFP_ENABLE_WAIT			500	/* SFP MSA recommends 300ms */
#define GE_100FX_SETUP_TIME		10	/* Time to setup GE 100 FX */

/* Refer to "Time to link up and link down for 10/100/1000 PHYs" White Paper
 * from Marvell (December 20, 2004). -
 * The quick answer to the question is that there is no theoretical upper
 * bound, but typically link should come up within 3 seconds.
 */
#define LINK_MAX_TIME			3000	/* Link up in 3 seconds. */
#define LINK_DELAY			1	/* 1 millisecond delay */

/* Shared PHY (88E series) Registers Offsets */
#define MRV88E111N_PHYID_1_REG			2
#define MRV88E111N_PHYID_2_REG			3
#define MRV88E111N_PAGE_ADDRESS_REG		22
#define MRV88E111N_FACTORY_TEST_MODES0		29
#define MRV88E111N_FACTORY_TEST_MODES1		30
#define MRV88E111N_FACTORY_TEST_MODES2		31

/* Page 0 Register Offsets - Copper */
#define MRV88E111N_CONTROL_REG                  0
#define MRV88E111N_STATUS_REG                   1
#define MRV88E111N_AUTONEG_ADVR_REG             4
#define MRV88E111N_LINK_PART_AV_REG             5
#define MRV88E111N_AUTONEG_EXPANSION_REG        6
#define MRV88E111N_NEXT_PAGE_REG                7
#define MRV88E111N_LP_NEXT_PAGE_REG             8
#define MRV88E111N_1000B_CNTL_REG               9                              
#define MRV88E111N_1000B_STATUS_REG             10
#define MRV88E111N_EXTENDED_STATUS_REG          15
#define MRV88E111N_SPECIFIC_CONTROL1_REG        16
#define MRV88E111N_SPECIFIC_STATUS1_REG         17
#define MRV88E111N_INT_ENABLE_REG               18
#define MRV88E111N_SPECIFIC_STATUS2_REG         19
#define MRV88E111N_REC_ERROR_COUNTER_REG        21
#define MRV88E111N_SPECIFIC_CONTROL2_REG        26

/* Page 1 Register Offsets - Fiber */
#define MRV88E111F_CONTROL_REG			0
#define MRV88E111F_STATUS_REG			1
#define MRV88E111F_AUTONEG_ADVR_REG		4
#define MRV88E111F_LINK_PART_AV_REG		5
#define MRV88E111F_AUTONEG_EXPANSION_REG	6
#define MRV88E111F_NEXT_PAGE_REG		7
#define MRV88E111F_LP_NEXT_PAGE_REG		8
#define MRV88E111F_EXTENDED_STATUS_REG		15
#define MRV88E111F_SPECIFIC_CONTROL1_REG	16
#define MRV88E111F_SPECIFIC_STATUS1_REG		17
#define MRV88E111F_INT_ENABLE_REG		18
#define MRV88E111F_SPECIFIC_STATUS2_REG		19
#define MRV88E111F_REC_ERROR_COUNTER_REG	21
#define MRV88E111F_SPECIFIC_CONTROL2_REG	26

/* Page 2 Register Offsets - MAC */
#define MRV88E111M_CONTROL_REG			0
#define MRV88E111M_SPECIFIC_CONTROL1_REG	16
#define MRV88E111M_SPECIFIC_STATUS1_REG		17
#define MRV88E111M_INT_ENABLE_REG		18
#define MRV88E111M_SPECIFIC_STATUS2_REG		19
#define MRV88E111M_SPECIFIC_CONTROL2_REG	26

/* Page 3 Register Offsets - LOS, INIT, STATUS[1:0] */
#define MRV88E111L_FUNC_CONTROL_REG		16
#define MRV88E111L_POL_CONTROL_REG		17
#define MRV88E111L_TMR_CONTROL_REG		18

/* Page 4 Register Offsets - Non-Volatile Memory */
#define MRV88E111NV_ADDRESS			16
#define MRV88E111NV_READ_DATA_STATUS		17
#define MRV88E111NV_WRITE_DATA_CONTROL		18
#define MRV88E111NV_RAM_WRITE_DATA_CONTROL	19
#define MRV88E111NV_RAM_ADDRESS			20

/* Page 5 Register Offsets - VCT */
#define MRV88E111N_VCT_STATUS_MDI0_REG		16
#define MRV88E111N_VCT_STATUS_MDI1_REG		17
#define MRV88E111N_VCT_STATUS_MDI2_REG		18
#define MRV88E111N_VCT_STATUS_MDI3_REG		19
#define MRV88E111N_VCT_PAIR_SKEW_REG		20
#define MRV88E111N_VCT_PAIR_SWP_POL_REG		21
#define MRV88E111N_VCT_DSP_DISTANCE		26

/* Page 6 Register Offsets */
#define MRV88E111N_PACKET_GEN_REG		16
#define MRV88E111N_CRC_CHKR_REG			17

#define MRV88E111N_CONTROL_PHY_RESET_VALUE	0x8000
#define MRV88E111N_CONTROL_PHY_RESET		0x8000
#define MRV88E111N_CONTROL_LOOPBACK		0x4000
#define MRV88E111N_CONTROL_FORCE_10_L		0x0000
#define MRV88E111N_CONTROL_FORCE_100_L          0x2000
#define MRV88E111N_CONTROL_FORCE_1000_L		0x0040
#define MRV88E111N_CONTROL_AUTONEG_ENABLE	0x1000
#define MRV88E111N_CONTROL_POWER_DOWN		0x0800
#define MRV88E111N_CONTROL_ISOLATE		0x0400
#define MRV88E111N_CONTROL_RESTART_AUTONEG	0x0200
#define MRV88E111N_CONTROL_FULL_DUPLEX		0x0100
#define MRV88E111N_CONTROL_COLLISION_TEST	0x0080
#define MRV88E111N_CONTROL_FORCE_1000_M		0x0040
#define MRV88E111N_CONTROL_FORCE_10_M		0x0000
#define MRV88E111N_CONTROL_SPEED_MASK		0x2040

#define MRV88E111N_PHY_ID_1_VALUE		0x0141

#define MRV88E111N_STATUS_AUTONEG_CHK		0x0008
#define MRV88E111N_STATUS_LINK_UP		0x0004

#define MRV88E111N_STATUS1_SPEED		0xC000
#define MRV88E111N_STATUS1_SPEED_1000		0x8000
#define MRV88E111N_STATUS1_SPEED_100		0x4000
#define MRV88E111N_STATUS1_SPEED_10		0x0000
#define MRV88E111N_STATUS1_LINK			0x0400
#define MRV88E111N_RESET_TIMEOUT		15	/* TRESET 10 ms min */

#define MRV88E111N_SPEED_1000BT			1000
#define MRV88E111N_SPEED_100BT			100
#define MRV88E111N_SPEED_10BT			10

#define MRV88E111N_REG_PAGE_0			0
#define MRV88E111N_REG_PAGE_1			1
#define MRV88E111N_REG_PAGE_2			2
#define MRV88E111N_REG_PAGE_3			3
#define MRV88E111N_REG_PAGE_4			4
#define MRV88E111N_REG_PAGE_5			5
#define MRV88E111N_REG_PAGE_6			6

/* Switch (98DX series) Registers Offsets */
#define MRV98DX_STATUS		0x1F	/* SMI Read-Write Status Register */
#define MRV98DX_WR_ADDR_MSB	0x00	/* SMI Write Address MSBs Register */
#define MRV98DX_WR_ADDR_LSB	0x01	/* SMI Write Address LSBs Register */
#define MRV98DX_WR_DATA_MSB	0x02	/* SMI Write Data MSBs Register */
#define MRV98DX_WR_DATA_LSB	0x03	/* SMI Write Data LSBs Register */
#define MRV98DX_RD_ADDR_MSB	0x04	/* SMI Read Address MSBs Register */
#define MRV98DX_RD_ADDR_LSB	0x05	/* SMI Read Address LSBs Register */
#define MRV98DX_RD_DATA_MSB	0x06	/* SMI Read Data MSBs Register */
#define MRV98DX_RD_DATA_LSB	0x07	/* SMI Read Data LSBs Register */

/* Bits defines for SMI Read-Write Status Register */
#define MRV98DX_SMI_WR_DONE	0x0002	/* SMI Write Done */
#define MRV98DX_SMI_RD_RDY	0x0001	/* SMI Read Ready */

/* Control Register (offset 0) Bits defines */
#define PHY_CTRL_RESET		MRV88E111N_CONTROL_PHY_RESET
#define PHY_CTRL_LOOPBACK	MRV88E111N_CONTROL_LOOPBACK
#define PHY_CTRL_MASK		MRV88E111N_CONTROL_SPEED_MASK
#define PHY_CTRL_1000		MRV88E111N_CONTROL_FORCE_1000_L
#define PHY_CTRL_100		MRV88E111N_CONTROL_FORCE_100_L
#define PHY_CTRL_10		MRV88E111N_CONTROL_FORCE_10_L
#define PHY_CTRL_DUPLEX		MRV88E111N_CONTROL_FULL_DUPLEX
#define PHY_CTRL_AUTO_NEG	MRV88E111N_CONTROL_AUTONEG_ENABLE
#define PHY_CTRL_PWR_DWN	MRV88E111N_CONTROL_POWER_DOWN

/* Link Partner Ability Register (offset 5 - Page 1) Bits defines */
#define LPA_SGMII_LINK		0x8000	/* SGMII Link up */

/* 1000BASE-T Control Register (offset 9) Bits defines */
#define PHY_GT_CTL_TEST_MASK	0xE000	/* Test Modes */
#define PHY_GT_CTL_NORMAL	0x0000	/* Normal mode */
#define PHY_GT_CTL_WV_TEST	0x2000	/* Test mode 1 - Waveform test */
#define PHY_GT_CTL_JT_MS	0x4000	/* Test mode 2 - Jitter (Master) */
#define PHY_GT_CTL_JT_SL	0x6000	/* Test mode 3 - Jitter (Slave) */
#define PHY_GT_CTL_DS_TEST	0x8000	/* Test mode 4 - Distortion test */
#define PHY_GT_CTL_TEST_MAX	PHY_GT_CTL_DS_TEST
#define PHY_GT_CTL_TEST_SHIFT	13	/* Test mode bits shift counts */

/* Specific Control 1 Register (offset 16) Bits defines */
#define PHY_SP_CTL1_SIGLOS	0x0200	/* SIGDET polarity for SFP */
#define PHY_SP_CTL1_ENERGY_DET	0x0180	/* Energy detect mask */
#define PHY_SP_CTL1_MDIX_MASK	0x0060	/* MDI Crossover mask */

#define MAC_SP_CTL1_MODE_MASK	0x0380	/* Mode select mask */
#define MAC_SP_CTL1_FX_MODE	0x0000	/* 100Base-FX */
#define MAC_SP_CTL1_GBIC_MODE	0x0080	/* Copper GBIC */
#define MAC_SP_CTL1_A_SGM_MODE	0x0100	/* Auto Copper/SGMII media interface */
#define MAC_SP_CTL1_A_X_MODE	0x0180	/* Auto Copper/1000Base-X */
#define MAC_SP_CTL1_CU_MODE	0x0280	/* Copper only */
#define MAC_SP_CTL1_SGM_MODE	0x0300	/* SGMII media interface only */
#define MAC_SP_CTL1_X_MODE	0x0380	/* 1000Base-X only */

/* Specific Status 1 Register (offset 17) Bits defines */
#define PHY_SP_STA1_LINKUP	MRV88E111N_STATUS1_LINK

/* Packet Generation Register (offset 16 - Page 6) Bits defines */
#define PHY_PKT_STUB_EN		0x0020	/* Enable stub test */
#define PHY_PKT_CRC_EN		0x0010	/* Enable CRC checker */

/* Copper Specific Status Register 2 (offset 19 - Page 0) Bits defines */
#define PG0_SP_ST2_AN_ERR       0x8000  /* Copper Auto-Negotiation Error */
#define PG0_SP_ST2_SP_CHG	0x4000	/* Copper Speed Changed */
#define PG0_SP_ST2_DPX_CHG	0x2000	/* Copper Duplex Changed */
#define PG0_SP_ST2_PG_RX	0x1000	/* Copper Page Received */
#define PG0_SP_ST2_AN_CMP	0x0800	/* Copper Auto-Negotiation Completed */
#define PG0_SP_ST2_LNK_CHG	0x0400	/* Copper Link Status Changed */
#define PG0_SP_ST2_SYM_ERR	0x0200	/* Copper Symbol Error */
#define PG0_SP_ST2_FLS_CRR	0x0100	/* Copper False Carrier */
#define PG0_SP_ST2_MDI_CHG	0x0040	/* MDI Crossover Changed */
#define PG0_SP_ST2_DNS_INT	0x0020	/* Downshift Interrupt */
#define PG0_SP_ST2_ED_CHG	0x0010	/* Energy Detect Chagned */
#define PG0_SP_ST2_DTE_CHG	0x0004	/* DTE power detection status changed */
#define PG0_SP_ST2_POL_CHG	0x0002	/* Polarity Changed */
#define PG0_SP_ST2_JAB		0x0001	/* Jabber */
#define PG0_SP_ST2_ERR_MSK	(PG0_SP_ST2_AN_ERR | PG0_SP_ST2_SYM_ERR | \
				 PG0_SP_ST2_FLS_CRR | PG0_SP_ST2_JAB)

/* Copper Specific Control Register 2 (offset 26 - Page 0) Bits defines */
#define PG0_SP_CTL2_TX_MASK	0x8000	/* Transmitter type mask */
#define PG0_SP_CTL2_TX_A	0x8000	/* Class A */
#define PG0_SP_CTL2_TX_B	0x0000	/* Class B */

/* MAC Specific Control Register 2 (offset 26 - Page 2) Bits defines */
/* SGMII MAC Interface Output Amplitude defines */
#define MAC_SP_CTL2_AMP_P5	0x0000	/* 0.50V */
#define MAC_SP_CTL2_AMP_P6	0x0001	/* 0.60V */
#define MAC_SP_CTL2_AMP_P7	0x0002	/* 0.70V */
#define MAC_SP_CTL2_AMP_P8	0x0003	/* 0.80V */
#define MAC_SP_CTL2_AMP_P9	0x0004	/* 0.90V */
#define MAC_SP_CTL2_AMP_1	0x0005	/* 1.00V */
#define MAC_SP_CTL2_AMP_1P1	0x0006	/* 1.10V */
#define MAC_SP_CTL2_AMP_1P2	0x0007	/* 1.20V */

/* Prototypes */
extern int mrvl_ge_dev_create (dev_object_t *dev,
				dev_error_report_t error_report_fn);

#endif /* __DEV_MRVL_GE_H__ */

/*
 *------------------------------------------------------------------
 * $Log: dev_mrvl_ge.h,v $
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
 * Revision 1.2  2009/10/27 23:02:11  siyen
 * Clear Power Down bit in case ROMMON/BIOS sets it for power management.
 *
 * Revision 1.1.1.1  2009/10/17 02:05:28  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.11.6.1.2.1  2009/10/14 01:06:57  huyhoang
 * + Add DiagLinux support to ngd-diag-rep repository
 *
 * Revision 1.11.6.1  2009/06/04 09:30:00  sctsai
 * Sync with informers2-tag-060209 repository.
 *
 * Revision 1.11  2009/05/20 21:05:49  aosulliv
 * Sync of xformers-branch to ngd-diags-rep
 *
 * Revision 1.9.2.7  2009/06/01 17:31:24  siyen
 * Sync up to the main trunk for GLC-GE-100FX SFP support.
 *
 * Revision 1.9.2.6  2009/06/01 07:43:09  sctsai
 * Sync with informers2-postld-tag-060109 repository.
 *
 * Revision 1.9.2.5.2.1  2009/05/21 05:16:41  sctsai
 * The changes:
 * 1.change "menu" back to big-endian.
 * 2.compile "library(diag/lib)" as big-endian by ICC.
 * 3.after linking by GNU(ld), calling "bepostld to handle .initdata section.
 *
 * Revision 1.9.2.5  2009/04/24 01:05:16  sctsai
 * Sync with informers-tag-042209 repository.
 *
 * Revision 1.9.2.4  2009/04/09 17:20:31  sctsai
 * The changes:
 * 1.Create a new header file "endians.h" to support both gcc & icc.
 *   .c file should include it at the beginning, if need.
 * 2.Change INFORMERS_ICC to INTEL_ICC
 * 3.Move all x86 related files to le directory.
 * 4.Temporarily, change FPGA path to /auto/sp-engops/diags/pld_icc.
 *
 * Revision 1.9.2.3  2009/03/30 16:25:56  sctsai
 * Fixed GE PHY test.
 *
 * Revision 1.9.2.2  2009/03/16 22:48:18  sctsai
 * Sync with informers-tag-031309-sync repository.
 *
 * Revision 1.5.2.6  2009/04/18 19:41:10  siyen
 * Extend Link Up to to 1 second per Xformers HW request.
 *
 * Revision 1.5.2.5  2009/03/13 22:37:03  ptong
 * Sync with ngd-informers-031209 repository.
 *
 * Revision 1.5.2.4  2009/01/23 02:24:27  shhuang
 *  Sync with ngd-informers-012109 repository.
 *
 * Revision 1.10  2009/03/04 21:25:00  aosulliv
 * Sync xformers-tag-030409 to ngd-diags-rep
 *
 * Revision 1.9  2009/01/19 18:51:14  aosulliv
 * sync of xformers-tag-011609 to main ngd-diags-rep
 *
 * Revision 1.8  2008/11/13 21:06:47  aosulliv
 * Sync xformers-tag-121208 to the ngd diag repository
 *
 * Revision 1.3.2.6  2008/10/31 18:02:04  siyen
 * Set to Class A for copper per hardware request. (CSCsv40919)
 *
 * Revision 1.3.2.5  2008/09/16 23:11:23  siyen
 * Enable CRC packet/error counters for debugging.
 *
 * Revision 1.3.2.4  2008/07/13 20:55:34  siyen
 * Added GE PHY loopback support for Cavium based platforms. (CSCsq64012)
 *
 * Revision 1.3.2.3  2008/06/13 19:50:39  siyen
 * Added additional PHY config per hardware change.
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
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
