/* $Id: diag_bcm_lib.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_bcm_lib.h - Fugazi Boradcom chip library header file
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_BCM_LIB_H__
#define __FUGAZI_BCM_LIB_H__

#include <stdint.h>
#include <unistd.h>

#include "linux_pci.h"
#include "diag_common.h"
#include "common_utils.h"
#include "diag_bnxt.h"
#include "diag_miura.h"

#define _XOPEN_SOURCE           500
#define NR_FUGAZI_BNXT          6       /* Total 6 BCM57412 MAC on Fugazi */
#define FUGAZI_PORT_SPEED_1G    1000
#define FUGAZI_PORT_SPEED_10G   10000
#define BCM57412_PCI_VENDOR     0x14e4
#define BCM57412_PCI_DEVICE     0x16d6
#define FUGAZI_LANE_TO_MIURA(lane)    (1 << (lane % 2))
#define FUGAZI_IF_SIDE_TO_MIURA(if_side)                  \
    ((if_side == FUGAZI_IF_SIDE_SYS) ?                    \
     FUGAZI_MIURA_SYS_SIDE : FUGAZI_MIURA_LINE_SIDE)
#define BCM82757_PHY_ID         0x10
#define BCM82757_PHY_ID_1       0x14
#define BCM82757_PHY_ID_2       0x16
#define MDIO_PHY_ID_C45         0x8000
#define MIURA_PHY_ID_1          0x0
#define MIURA_PHY_ID_2          0x1
#define DIGITAL_PMD             0x1
#define REMOTE_PMD              0x2


/* PCI mapping to BCM57412 MAC, and MAC to 1G/10G PHYs */
typedef enum {
    FUGAZI_MAC_10G_PHY_0,
    FUGAZI_MAC_10G_PHY_1,
    FUGAZI_MAC_1G_PHY_0,       /* 2 - 1st 1G PHY */
    FUGAZI_MAC_1G_PHY_1,
    FUGAZI_MAC_1G_PHY_2,
    FUGAZI_MAC_1G_PHY_3,
    MAX_FUGAZI_MAC
} fugazi_mac_phy_t;;

typedef enum {
    FUGAZI_IF_SIDE_LINE,
    FUGAZI_IF_SIDE_SYS,
    MAX_NR_FUGAZI_IF_SIDE
} fugazi_if_side_t;

typedef enum {
    FUGAZI_LANE_0,
    FUGAZI_LANE_1,
    FUGAZI_LANE_2,
    FUGAZI_LANE_3,
    MAX_NR_FUGAZI_LANE
} fugazi_lane_t;

typedef enum {
    FUGAZI_LED_OFF,
    FUGAZI_LED_GREEN,
    FUGAZI_LED_RED,
    FUGAZI_LED_AMBER,
    FUGAZI_LED_YELLOW,
    FUGAZI_LED_BLUE,
    MAX_NR_FUGAZI_LED
} fugazi_led_t;

typedef enum {
    FUGAZI_VMARG_NORMAL,
    FUGAZI_VMARG_HIGH,
    FUGAZI_VMARG_LOW,
    MAX_NR_FUGAZI_VMARG
} fugazi_vmarg_t;

typedef enum {
    FUGAZI_PRBS_7,
    FUGAZI_PRBS_9,
    FUGAZI_PRBS_11,
    FUGAZI_PRBS_15,
    FUGAZI_PRBS_23,
    FUGAZI_PRBS_31,
    MAX_NR_FUGAZI_PRBS
} fugazi_prbs_t;


struct fugazi {
    struct fugazi_bnxt bnxt[NR_FUGAZI_BNXT];
    struct fugazi_miura miura;
};

#define FUGAZI_MIURA_DEV_PMA_PMD       1
#define FUGAZI_MIURA_DEV_PCS           3
#define FUGAZI_MIURA_DEV_CL73_AN       7

#define PHYMOD_MIURA_DIRECT_ACC    0x000000
#define MIURA_PM_TSCE_BASEADR 0x42000000
#define PHYMOD_MIURA_TSCE_BASE_ADDR 0x20000
#define MIURA_MERLIN_BASEADR    0x48000000

#define BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_0_CONTROLr (0x00018a82 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO1_0_CONTROLr (0x00018a84 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_1_CONTROLr (0x00018a86 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO1_1_CONTROLr (0x00018a88 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_CTRL_LED_OPMODEr (0x00018ba0 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_CTRL_LED_PARAMSr (0x00018ba1 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_PAD_CNTRL_MOD_ABS_0_STATUSr (0x00018a6f | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_PAD_CNTRL_LASI_0_CONTROLr (0x00018a4e | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_LASI_1_CONTROLr (0x00018a50 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_CTRL_PORT0_CONFIGr (0x00018b03 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_CTRL_PORT1_CONFIGr (0x00018b05 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_TSCE_XGXS_AN_X4_ENSr (0x0000c180 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_AN_X4_LOC_DEV_CL37_BASE_ABILr (0x0000c181 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)

#define BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_CTRL5r (0x0000d0a5 | MIURA_MERLIN_BASEADR)
#define BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL1r (0x0000d110 | MIURA_MERLIN_BASEADR)
#define BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL2r (0x0000d111 | MIURA_MERLIN_BASEADR)

#define BCMI_MIURA_PRBS_GENERATOR_CONTROL (0x0000d0d1 | MIURA_MERLIN_BASEADR)

/* general ctrl regs */
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_CLOCK_SCALER_CTRLr (0x00018200 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GEN_CONTROL1r (0x00018201 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GEN_CONTROL2r (0x00018202 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GEN_CONTROL3r (0x00018203 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_MICRO_BOOT_MDIO_PORr (0x000182fe | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_MICRO_BOOT_BOOT_PORr (0x000182ff | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_CTRL_CHIP_IDr (0x00018b00 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_CTRL_CHIP_REVISIONr (0x00018b01 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_CTRL_CHIP_CONFIGr (0x00018b02 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_MODULE_CNTRL_CONTROLr (0x00018700 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_MODULE_CNTRL_STATUSr (0x00018701 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_PAD_CNTRL_MDIO1_CONTROLr (0x00018a30 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_MDIO2_CONTROLr (0x00018a32 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_ADR0_CONTROLr (0x00018a34 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_ADR1_CONTROLr (0x00018a36 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_PAD_CNTRL_ADR2_CONTROLr (0x00018a38 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_M0ACCESS_ADDR_MST_CRAM_MEM_ADD_CTRLr (0x00018400 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_M0ACCESS_ADDR_MST_DRAM_MEM_ADD_CTRLr (0x00018401 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_M0ACCESS_ADDR_MST_VT_ADD_CTRLr (0x00018402 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_M0ACCESS_ADDR_SLV1_CRAM_MEM_ADD_CTRLr (0x00018403 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_LMI_LMI_RESET_CTRLr (0x00019000 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_LMI_LMI_CMDr (0x00019001 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_LMI_LMI_ADDRr (0x00019002 | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_LMI_LMI_DATAr (0x00019003 | PHYMOD_MIURA_DIRECT_ACC)

#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Cr (0x0001826c | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Dr (0x0001826d | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Er (0x0001826e | PHYMOD_MIURA_DIRECT_ACC)
#define BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Fr (0x0001826f | PHYMOD_MIURA_DIRECT_ACC)

/*  tsce regs */
#define BCMI_TSCE_XGXS_RX_X4_PCS_CTL0r (0x0000c130 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_TX_X4_PCS_STSr (0x0000c121 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_PMA_CTL0r (0x0000c137 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_PCS_LATCH_STS1r (0x0000c152 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_PCS_LATCH_STS0r (0x0000c153 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_PCS_LIVE_STSr (0x0000c154 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)

#define BCMI_TSCE_XGXS_PMD_X1_STSr (0x00009012 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PMD_X4_CTLr (0x0000c010 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PMD_X4_MODEr (0x0000c011 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PMD_X4_STSr (0x0000c012 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)

#define BCMI_TSCE_XGXS_RX_X4_FEC0r (0x0000c131 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC1r (0x0000c132 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_CORRBLKSLr (0x0000c157 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_CORRBLKSHr (0x0000c158 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_UNCORRBLKSLr (0x0000c159 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_UNCORRBLKSHr (0x0000c15a | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_BURST_ERR_STSLr (0x0000c142 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_BURST_ERR_STSHr (0x0000c143 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_RX_X4_FEC_DBG_ERRAHr (0x0000c141 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PKTGEN_CRCERRCNTr (0x00009033 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PATGEN_TXPKTCNT_Ur (0x0000c040 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PATGEN_TXPKTCNT_Lr (0x0000c041 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PATGEN_RXPKTCNT_Ur (0x0000c042 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)
#define BCMI_TSCE_XGXS_PATGEN_RXPKTCNT_Lr (0x0000c043 | PHYMOD_MIURA_TSCE_BASE_ADDR | MIURA_PM_TSCE_BASEADR)

extern int fugazi_init(struct fugazi *);
extern void fugazi_exit(struct fugazi *);

extern int fugazi_bcm82757_read(struct fugazi *, fugazi_lane_t, 
                                fugazi_if_side_t, uint32_t, uint32_t, uint32_t *);
extern int fugazi_bcm82757_write(struct fugazi *, fugazi_lane_t, 
                                 fugazi_if_side_t , uint32_t, uint32_t, uint32_t);
extern int fugazi_bcm82757_read_mdio(struct fugazi *, fugazi_lane_t, 
                                     fugazi_if_side_t, uint32_t, uint32_t, uint32_t *);
extern int fugazi_bcm82757_write_mdio(struct fugazi *, fugazi_lane_t, 
                                      fugazi_if_side_t , uint32_t, uint32_t, uint32_t);
extern int fugazi_bcm82757_dump(struct fugazi *, fugazi_lane_t, fugazi_if_side_t);
extern int fugazi_bcm82757_mac_dump(struct fugazi *, fugazi_lane_t, fugazi_if_side_t);
extern int fugazi_bcm82757_link_status(struct fugazi *, fugazi_lane_t, 
                                       fugazi_if_side_t, unsigned int *);
extern int fugazi_bcm82757_display_eye_scan(struct fugazi *, fugazi_lane_t, 
                                            fugazi_if_side_t);
extern int fugazi_bcm82757_loopback_set(struct fugazi *, fugazi_lane_t , 
                                        fugazi_if_side_t ,unsigned int,  
                                        unsigned int );
extern int fugazi_bcm82757_loopback_get(struct fugazi *, fugazi_lane_t , 
                                        fugazi_if_side_t ,unsigned int,  
                                        unsigned int* );
extern int fugazi_bcm82757_prbs_set(struct fugazi *, fugazi_lane_t, 
                                    fugazi_if_side_t, unsigned int, fugazi_prbs_t, unsigned int);
extern int fugazi_bcm82757_prbs_clear_rx_stat(struct fugazi *, fugazi_lane_t, 
                                              fugazi_if_side_t);
extern int fugazi_bcm82757_prbs_check(struct fugazi *, fugazi_lane_t, 
                                      fugazi_if_side_t);
extern int fugazi_bcm82757_prbs_clear_error(struct fugazi *fugazi,
									fugazi_lane_t, fugazi_if_side_t );
extern int fugazi_bcm82757_firmware_lane_set(struct fugazi *,
                                             fugazi_lane_t , fugazi_if_side_t ,
                                             bcm_plp_pm_firmware_lane_config_t *);
extern int fugazi_bcm82757_firmware_lane_get(struct fugazi *,
                                             fugazi_lane_t , fugazi_if_side_t ,
                                             bcm_plp_pm_firmware_lane_config_t *);
extern int fugazi_bcm82757_cl73_set(struct fugazi *, fugazi_lane_t, 
                                    fugazi_if_side_t, unsigned int, int);
extern void fugazi_bcm82757_config_macsec_cleanup(struct fugazi *, fugazi_lane_t);
extern int fugazi_bcm82757_config_macsec_bypass(struct fugazi *, fugazi_lane_t, int);
extern int fugazi_bcm82757_init(struct fugazi *);
extern int fugazi_bcm82757_show_fw_version(struct fugazi *, unsigned int *, unsigned int *);
extern int fugazi_bcm82757_interrupt_set(struct fugazi *, fugazi_lane_t, 
                                         fugazi_if_side_t , unsigned int, 
                                         unsigned int);
extern int fugazi_bcm82757_interrupt_get(struct fugazi *, fugazi_lane_t , 
                                         fugazi_if_side_t , unsigned int , 
                                         uint32_t *);
extern int fugazi_bcm82780_recover_clock( struct fugazi *, int, int );
extern int fugazi_bcm82757_regs_dump(struct fugazi *, fugazi_lane_t);
extern int fugazi_bcm82757_tx_analog_get(struct fugazi *, fugazi_lane_t, bcm_plp_tx_t *);
extern int fugazi_bcm82757_tx_analog_set(struct fugazi *, fugazi_lane_t, bcm_plp_tx_t *);
extern void force_line_side_intf_lrm(int);
extern int bcm82757_power_get (struct fugazi *, fugazi_lane_t, fugazi_if_side_t,
                               unsigned int *, unsigned int *);
extern int bcm82757_power_set (struct fugazi *, fugazi_lane_t, fugazi_if_side_t,
                               unsigned int, unsigned int);

extern boolean bcm82757_fw_downloaded;
extern struct fugazi *fugazi_struct;

#endif /* __FUGAZI_BCM_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_bcm_lib.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2021/05/04 18:40:27  pdoong
 * Change config bcm82757 cl37 mode from directly register write to all bcm82757 API
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.24  2020/08/25 04:21:35  pdoong
 * Add bcm82757_power_get()/bcm82757_power_set() functions to get/Set power state.
 *
 * Revision 1.1.6.23  2020/08/19 09:11:50  iachang
 * PRRQ CSCvo59196-4 : BCM82757 10G PHY code review
 *
 * Revision 1.1.6.22  2020/08/06 02:16:06  pdoong
 * clean code for BCM54194 1G PHY
 *
 * Revision 1.1.6.21  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.20  2020/06/08 06:54:39  iachang
 * Program Aikido FPGA DEV key utility
 * Program I211 and BCM57412 MAC address utility.
 *
 * Revision 1.1.6.19  2020/03/25 01:07:49  iachang
 * BCM82757 register read/write utility and register test, using mdio directly access instead of call 10G PHY Broadcom's API.
 *
 * Revision 1.1.6.18  2020/02/25 08:02:49  iachang
 * Modify BCM82757 LASI test utility.
 *
 * Revision 1.1.6.17  2020/01/17 06:30:26  iachang
 * Skip BCM82757 initial with SyncE and BCM57412 submenu and add in SyncE Recovered Clock Test.
 *
 * Revision 1.1.6.16  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.15  2019/10/04 06:05:22  iachang
 * BCM82757 force line side SFP LRM utility
 *
 * Revision 1.1.6.14  2019/09/16 11:23:42  iachang
 * CSCvr24877 : Display PRBS error count when inject error from external
 *
 * Revision 1.1.6.13  2019/08/30 22:00:21  pdoong
 * Add Clear error counter option to clear PRBS error counter.
 *
 * Revision 1.1.6.12  2019/08/29 20:49:32  pdoong
 * Add BCM82757 Analog utility
 *
 * Revision 1.1.6.11  2019/08/02 03:32:38  iachang
 * Add BCM82757 Regs dump utility
 * Add packet count check when BCM82757 loopback test failed.
 *
 * Revision 1.1.6.10  2019/06/14 23:58:31  pdoong
 * Add configure bcm82757 10G PHY to generate recovered clock output
 *
 * Revision 1.1.6.9  2019/06/13 14:21:20  iachang
 * Add BCM82757 interrupt utility
 *
 * Revision 1.1.6.8  2019/05/14 02:01:54  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.7  2019/04/17 22:44:16  iachang
 * Modify BCM82757 PHY ID for P1A2 board.
 *
 * Revision 1.1.6.6  2019/04/06 01:07:49  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.5  2019/04/01 22:34:07  iachang
 * Support 2nd BCM82757 utility.
 *
 * Revision 1.1.6.4  2019/03/18 23:16:14  iachang
 * Bing up 2'nd BCM82757 PHY FW download and external loopback.
 *
 * Revision 1.1.6.3  2019/03/14 21:46:47  iachang
 * Bring up BCM82757 first PHY.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:24  letsai
 * Initial check in.
 *
 *
 * $Endlog$
 * */

