/* $Id: dev_4359.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pmc_4359/dev_4359.h,v $
 *------------------------------------------------------------------
 *
 * pm4359.h 
 *    Data structure and defines for PMC Sierra Comet Tetra Framer
 * 
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Hoang Nguyen - Ported from pm4354.h by Bao Buu
 *------------------------------------------------------------------
 */

/****************************************************************************/
/*  COPYRIGHT (C) 2001 PMC-SIERRA, INC. ALL RIGHTS RESERVED.                */
/*--------------------------------------------------------------------------*/
/* This software embodies materials and concepts which are proprietary and  */
/* confidential to PMC-Sierra, Inc.                                         */
/* PMC-Sierra distributes this software to its customers pursuant to the    */
/* terms and conditions of the Device Driver Software License Agreement     */
/* contained in the text file software.lic that is distributed along with   */
/* the device driver software. This software can only be utilized if all    */
/* terms and conditions of the Device Driver Software License Agreement are */
/* accepted. If there are any questions, concerns, or if the Device Driver  */
/* Software License Agreement text file, software.lic, is missing please    */
/* contact PMC-Sierra for assistance.                                       */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
**
**  MODULE      : COMET-Tetra Driver
**
**  FILE        : cmq_defs.h
**
**  DESCRIPTION : Contains definitions of all device register offsets, bit 
**                offsets, and any other device dependent definitions.  Also,
**                driver enumerated type definitions are defined in this file.
**
******************************************************************************/

#ifndef __PM4359__
#define __PM4359__

/*
 * COMET Tetra Driver Constants
 */

#define CMQ_MAX_DEVS                   4
#define CMQ_MAX_INIT_PROFS             5

/*
 * COMET & COMET-Quad Register Map         
 *
 * Registers that are common to both devices have prefix 'CMQ'.  Register 
 * definitions that are specific to the COMET have prefix 'CM'.
 */

#define CMQ_GLB_CFG                             0x000
#define CMQ_CLK_MON                             0x001
#define CMQ_RX_OPTS                             0x002
#define CMQ_RX_LINE_IF_CFG                      0x003 
#define CMQ_TX_LINE_IF_CFG                      0x004 
#define CMQ_TX_FRM_BYPASS_OPTS                  0x005
#define CMQ_TX_TIMING_OPTS                      0x006
#define CMQ_INT_SRC1                            0x007
#define CMQ_INT_SRC2                            0x008
#define CMQ_INT_SRC3                            0x009
#define CMQ_MST_DIAG                            0x00A

/* master TEST for all - other offsets reserved */

#define CMQ_MST_TST                             0x00B
#define CMQ_ALOG_DIAG                           0x00C

/* revison, chip ID, PMON update for all - other offsets reserved */

#define CMQ_REV_CHIP_ID_PMON_UPD                0x00D

/* reset for all - other offsets reserved */

#define CMQ_RESET                               0x00E
#define CMQ_PRBS_POS_CTL_HDLC_CTL               0x00F

#define CMQ_CDRC_CFG                            0x010
#define CMQ_CDRC_INT_EN                         0x011
#define CMQ_CDRC_INT_STAT                       0x012
#define CMQ_CDRC_ALT_LOS                        0x013

#define CMQ_RJAT_INT_STAT                       0x014
#define CMQ_RJAT_DIV_N1_CTL                     0x015
#define CMQ_RJAT_DIV_N2_CTL                     0x016
#define CMQ_RJAT_CFG                            0x017

#define CMQ_RJAT_DIV_NX_CTL_T1                 0x2f
#define CMQ_RJAT_DIV_NX_CTL_E1                 0xff

#define CMQ_TJAT_INT_STAT                      0x018
#define CMQ_TJAT_DIV_N1_CTL                    0x019
#define CMQ_TJAT_DIV_N2_CTL                    0x01A
#define CMQ_TJAT_CFG                           0x01B

#define CMQ_TJAT_DIV_N1_CTL_1544_1544          0x2f
#define CMQ_TJAT_DIV_N1_CTL_2048_2048          0xff
#define CMQ_TJAT_DIV_N1_CTL_2048_1544          0xff
#define CMQ_TJAT_DIV_N1_CTL_1544_2048          0xc0
#define CMQ_TJAT_DIV_N1_CTL_G1544_1544         0xc0
#define CMQ_TJAT_DIV_N1_CTL_8K_1544            0x00
#define CMQ_TJAT_DIV_N1_CTL_16K_1544           0x01
#define CMQ_TJAT_DIV_N1_CTL_8K_2048            0x00

#define CMQ_TJAT_DIV_N2_CTL_1544_1544          0x2f
#define CMQ_TJAT_DIV_N2_CTL_2048_2048          0xff
#define CMQ_TJAT_DIV_N2_CTL_2048_1544          0xc0
#define CMQ_TJAT_DIV_N2_CTL_1544_2048          0xff
#define CMQ_TJAT_DIV_N2_CTL_G1544_1544         0xc0
#define CMQ_TJAT_DIV_N2_CTL_8K_1544            0xc0
#define CMQ_TJAT_DIV_N2_CTL_16K_1544           0xc0
#define CMQ_TJAT_DIV_N2_CTL_8K_2048            0xff
#define CMQ_TJAT_DIV_N2_CTL_16K_2048           0xff

#define CMQ_RX_ELST_CFG                         0x01C
#define CMQ_RX_ELST_INT_EN_STAT                 0x01D
#define CMQ_RX_ELST_IDLE_CODE                   0x01E

/* offset 0x01F Reserved */

#define CMQ_TX_ELST_CFG                         0x020
#define CMQ_TX_ELST_INT_EN_STAT                 0x021

/* offset 0x022 - 0x027 Reserved */

#define CMQ_RX_DLNK_CTL                         0x028
#define CMQ_RX_DLNK_BIT_SEL                     0x029

/* offset 0x02A - 0x02F Reserved          */

#define CMQ_BRIF_CFG                            0x030
#define CMQ_BRIF_FRM_PULSE_CFG                  0x031
#define CMQ_BRIF_PAR_FBIT_CFG                   0x032
#define CMQ_BRIF_TSLOT_OFFSET                   0x033
#define CMQ_BRIF_BIT_OFFSET                     0x034

/* offset 0x035 - 0x037 reserved */

#define CMQ_TX_DLNK_CTL                         0x038
#define CMQ_TX_DLNK_BIT_SEL                     0x039

/* offset 0x03A - 0x03F reserved             */

#define CMQ_BTIF_CFG                            0x040
#define CMQ_BTIF_FRM_PULSE_CFG                  0x041
#define CMQ_BTIF_PAR_STAT_CFG                   0x042
#define CMQ_BTIF_TSLOT_OFFSET                   0x043
#define CMQ_BTIF_BIT_OFFSET                     0x044

/* offset 0x045 - 0x047 reserved */

#define CMQ_T1_FRMR_CFG                         0x048
#define CMQ_T1_FRMR_INT_EN                      0x049
#define CMQ_T1_FRMR_STAT_INT_IND                0x04A

/* offset 0x04B reserved */

#define CMQ_IBCD_CFG                            0x04C
#define CMQ_IBCD_INT_EN_STAT                    0x04D
#define CMQ_IBCD_ACT_CODE                       0x04E
#define CMQ_IBCD_DEACT_CODE                     0x04F

#define CMQ_SIGX_CFG_CHG_SIG_STATE              0x050
#define CMQ_SIGX_COSS_25THRU30                  CMQ_SIGX_CFG_CHG_SIG_STATE
#define CMQ_SIGX_MICRO_STAT_COS                 0x051
#define CMQ_SIGX_COSS_17THRU24                  CMQ_SIGX_MICRO_STAT_COS
#define CMQ_SIGX_CHN_IND_ADDR_CTL_COS           0x052
#define CMQ_SIGX_COSS_09THRU16                  CMQ_SIGX_CHN_IND_ADDR_CTL_COS
#define CMQ_SIGX_CHN_IND_DATA_COS               0x053
#define CMQ_SIGX_COSS_01THRU08                  CMQ_SIGX_CHN_IND_DATA_COS

#define CMQ_T1_XBAS_CFG                         0x054
#define CMQ_T1_XBAS_ALRM_TX                     0x055
#define CMQ_T1_XIBC_CTL                         0x056
#define CMQ_T1_XIBC_LPBCK_CODE                  0x057

#define CMQ_PMON_INT_EN_STAT                    0x058
#define CMQ_PMON_FRM_BIT_ERR_CNT                0x059
#define CMQ_PMON_OOF_COFA_FEBE_CNT_LSB          0x05A
#define CMQ_PMON_OOF_COFA_FEEE_CNT_MSB          0x05B
#define CMQ_PMON_BIT_CRC_ERR_CNT_LSB            0x05C
#define CMQ_PMON_BIT_CRC_ERR_CNT_MSB            0x05D
#define CMQ_PMON_LCV_CNT_LSB                    0x05E
#define CMQ_PMON_LCV_CNT_MSB                    0x05F

#define CMQ_T1_ALMI_CFG                         0x060
#define CMQ_T1_ALMI_INT_EN                      0x061
#define CMQ_T1_ALMI_INT_STAT                    0x062
#define CMQ_T1_ALMI_ALRM_DET_STAT               0x063

/* Offset 0x64 reserved */

#define CMQ_T1_PDVD_INT_EN_STAT                 0x065

#define CMQ_T1_XBOC_CTL                         0x066
#define CMQ_T1_XBOC_CODE                        0x067

/* Offset 0x68 reserved */

#define CMQ_T1_XPDE_INT_EN_STAT                 0x069
#define CMQ_T1_RBOC_EN                          0x06A
#define CMQ_T1_RBOC_CODE_STAT                   0x06B

#define CMQ_TPSC_CFG                            0x06C
#define CMQ_TPSC_MICRO_ACCESS_STAT              0x06D
#define CMQ_TPSC_CHN_IND_ADDR_CTL               0x06E
#define CMQ_TPSC_CHN_IND_DATA_BUF               0x06F

#define CMQ_RPSC_CFG                            0x070
#define CMQ_RPSC_MICRO_ACCESS_STAT              0x071
#define CMQ_RPSC_CHN_IND_ADDR_CTL               0x072
#define CMQ_RPSC_CHN_IND_DATA_BUF               0x073
#define CMQ_DS0_DDLB_ELST_CFG                   0x074
#define CMQ_DS0_DDLB_ELST_INTR_EN_STAT          0x075

/* Offset 0x076 - 0x077 reserved */

#define CMQ_T1_APRM_CFG_CTL                     0x078

/* Offset 0x79 reserved */

#define CMQ_T1_APRM_INT_STAT                    0x07A
#define CMQ_T1_APRM_1SEC_OCTET2                 0x07B
#define CMQ_T1_APRM_1SEC_OCTET3                 0x07C
#define CMQ_T1_APRM_1SEC_OCTET4                 0x07D
#define CMQ_T1_APRM_1SEC_OCTET5                 0x07E
#define CMQ_T1_APRM_1SEC_OCTET6                 0x07F

#define CMQ_E1_TRAN_CFG                         0x080
#define CMQ_E1_TRAN_TX_ALRM_DIAG_CTL            0x081
#define CMQ_E1_TRAN_INTERNAT_CTL                0x082
#define CMQ_E1_TRAN_EXTRA_BITS_CTL              0x083
#define CMQ_E1_TRAN_INT_EN                      0x084
#define CMQ_E1_TRAN_INT_STAT                    0x085
#define CMQ_E1_TRAN_NAT_BIT_CODEWRD_SEL         0x086
#define CMQ_E1_TRAN_NAT_BIT_CODEWRD             0x087

/* Offset 0x088 - 0x08F reserved */

#define CMQ_E1_FRMR_FRM_ALIGN_OPT               0x090
#define CMQ_E1_FRMR_MAINT_MODE_OPT              0x091
#define CMQ_E1_FRMR_STAT_INT_EN                 0x092
#define CMQ_E1_FRMR_MAINT_ALRM_STAT_INT_EN      0x093
#define CMQ_E1_FRMR_E1_FRM_STAT_INT_IND         0x094
#define CMQ_E1_FRMR_MAINT_ALRM_STAT_INT_IND     0x095
#define CMQ_E1_FRMR_STAT                        0x096
#define CMQ_E1_FRMR_MAINT_ALRM_STAT             0x097
#define CMQ_E1_FRMR_INTER_NAT_BITS              0x098
#define CMQ_E1_FRMR_CRC_ERR_CNT_LSB             0x099
#define CMQ_E1_FRMR_CRC_ERR_CNT_MSB             0x09A
#define CMQ_E1_FRMR_NAT_BIT_CODEWRD_INT_EN      0x09B
#define CMQ_E1_FRMR_NAT_BIT_CODEWRD_INT_STAT    0x09C
#define CMQ_E1_FRMR_NAT_BIT_CODEWRD             0x09D
#define CMQ_E1_FRMR_FRM_PULSE_INT_EN            0x09E
#define CMQ_E1_FRMR_FRM_PULSE_ALRM_INT          0x09F

/* Offset 0x0A0 - 0x0A7  reserved */

#define CMQ_TDPR_CFG                            0x0A8
#define CMQ_TDPR_UPPER_TX_THRESH                0x0A9
#define CMQ_TDPR_LOWER_TX_THRESH                0x0AA
#define CMQ_TDPR_INT_EN                         0x0AB
#define CMQ_TDPR_INT_STAT_UDR_CLR               0x0AC
#define CMQ_TDPR_TX_DATA                        0x0AD

/* Offset 0x0AE - 0x0AF reserved */

#define CMQ_RX_ELST_CCS_CFG                     0x0B0
#define CMQ_RX_ELST_CCS_INT_EN_STAT             0x0B1
#define CMQ_RX_ELST_CCS_IDLE_CODE               0x0B2

/* Offset 0x0AE - 0x0AF reserved */

#define CMQ_TX_ELST_CCS_CFG                     0x0B4
#define CMQ_TX_ELST_CCS_INT_EN_STAT             0x0B5

/* Offset 0x0B6 - 0x0B7 reserved */

#define CMQ_RX_HMVIP_CCS_EN                     0x0B8

#define CMQ_TX_HMVIP_CCS_EN                     0x0B9

/* Master Test Control - other offsets reserved */

#define CMQ_MASTER_TST_CTL                      0x0BA

/* Resync Select - other offsets reserved */
#define CMQ_RSYNC_SEL                           0x0BB

/* Offset 0x0BD - 0x0BF Reserved */
#define CMQ_MASTER_INT_SRC                      0x0BC 
#define CMQ_TERM_CNTRL                          0x0BE 

/* Master Interrupt source - other offsets reserved */

#define CMQ_RDLC_CFG                            0x0C0
#define CMQ_RDLC_INT_CTL                        0x0C1
#define CMQ_RDLC_INT_STAT                       0x0C2
#define CMQ_RDLC_INT_DATA                       0x0C3
#define CMQ_RDLC_INT_PRI_ADDR_MATCH             0x0C4
#define CMQ_RDLC_INT_SEC_ADDR_MATCH             0x0C5

/* Offset 0x0C6 - 0x0D5 Reserved */

#define CMQ_CSU_CFG                             0x0D6

/* Offset 0x00D7 Reserved     */

#define CMQ_RLPS_IND_DATA_REG1                  0x0D8
#define CMQ_RLPS_IND_DATA_REG2                  0x0D9
#define CMQ_RLPS_IND_DATA_REG3                  0x0DA
#define CMQ_RLPS_IND_DATA_REG4                  0x0DB
#define CMQ_RLPS_EQ_LOOP_VOLT_REF               0x0DC
#define CMQ_RLPS_EQ_LOOP_VOLT_REF2              0x0DD

/* Offset 0x0DD - 0x0DF Reserved */

#define CMQ_PRBS_GEN_CHK_CTL                    0x0E0
#define CMQ_PRBS_CHK_INT_EN_STAT                0x0E1
#define CMQ_PRBS_PAT_SEL                        0x0E2

/* Offset 0x0E3 - Reserved  */

#define CMQ_PRBS_ERR_CNT1                       0x0E4

#define CMQ_PRBS_ERR_CNT2                       0x0E5
#define CMQ_PRBS_ERR_CNT3                       0x0E6

/* Offset 0x0E7 - 0xEF Reserved   */

#define CMQ_XLPG_LINE_DRV_CFG                   0x0F0
#define CMQ_XLPG_CTL_STAT                       0x0F1
#define CMQ_XLPG_PULSE_WFORM_STORE_WR_ADDR      0x0F2
#define CMQ_XLPG_PULSE_WFORM_STORE_WR_DATA      0x0F3

#define CMQ_XLPG_LINE_DRV_CFG_MASK              0x9F
#define CMQ_XLPG_LINE_DRV_TRI_STATE             0x00
#define CMQ_XLPG_LINE_DRV_90                    0x08
#define CMQ_XLPG_LINE_DRV_145                   0x0D
#define CMQ_XLPG_LINE_DRV_234                   0x15
#define CMQ_XLPG_LINE_DRV_HIGHZ_EN              0x80
/* COMET-Quad: */
#define CMQ_XLPG_CFG1                           0x0F4
#define CMQ_XLPG_CFG2                           0x0F5

/* COMET-Quad: */
#define CMQ_XLPG_INIT                           0x0F6

/* Offset 0x0F7 - Reserved */

#define CMQ_RLPS_CFG_STAT                       0x0F8
#define CMQ_RLPS_ALOS_DET_CLRNCE_THRESH         0x0F9
#define CMQ_RLPS_ALOS_DET_PERIOD                0x0FA
#define CMQ_RLPS_ALOS_CLRNCE_PERIOD             0x0FB
#define CMQ_RLPS_EQ_IND_ADDR                    0x0FC
#define CMQ_RLPS_EQ_RDWR_SEL                    0x0FD
#define CMQ_RLPS_EQ_LOOP_STAT_CTL               0x0FE
#define CMQ_RLPS_EQ_CFG                         0x0FF

/******************************************************************************
*
* COMET & COMET-Quad Register Bit Offsets
*
* Offsets that are common to both devices have prefix 'CMQ'.  Register 
* definitions that are specific to the COMET have prefix 'CM'.
*
******************************************************************************/

/* Global CFG - bit mask for offset 0x0000, 0x0100, 0x0200, 0x0300 */
#define CMQ_GLBL_MASK                           0xFF
#define CMQ_GLBL_PGM_IO_EN                      0x80
#define CMQ_GLBL_PGM_IO_STAT                    0x40
#define CMQ_GLBL_IBCD_IDLE                      0x20
#define CMQ_GLBL_RESYNC_DIG_LOS                 0x10
#define CMQ_GLBL_OUT_OF_MFRM_AIS                0X08
#define CMQ_GLBL_TRNK_EN                        0x04
#define CMQ_GLBL_RX_TRNK_EN                     0x02
#define CMQ_GLBL_E1_MODE                        0x01

/* CLK Monitor - bit mask for offset 0x0001, 0x0101, 0x201, 0x0301 */
#define CMQ_CLK_MON_STAT_MASK                   0x1F
#define CMQ_CLK_MON_RESERVED                    0xE0
#define CMQ_CLK_MON_TX_CLK_ACT                  0x10
#define CMQ_CLK_MON_BTIF_CLK_ACT                0x08
#define CMQ_CLK_MON_COMMON_CLK_ACT              0x04
#define CMQ_CLK_MON_BRIF_CLK_ACT                0x02
#define CMQ_CLK_MON_RX_CLK_ACT                  0x01

/* RX Options - bit mask for offset 0x0002, 0x0102, 0x202, 0x0302 */
#define CMQ_RX_OPT_JAT_BYPASS                   0x80
#define CMQ_RX_OPT_UNFRM                        0x40
#define CMQ_RX_OPT_ELST_BYPASS                  0x20
#define CMQ_RX_OPT_RESYNC_MEM                   0x10
#define CMQ_RX_OPT_RESYNC_SEL                   0x08
#define CMQ_RX_OPT_RESYNC_8KHZ_CLK              0x08
#define CMQ_RX_OPT_RESYNC_RATE_CLK              0x00
#define CMQ_RX_OPT_WRD_ERR                      0x04
#define CMQ_RX_OPT_ALL_FAS_WRD_ERR              0x04
#define CMQ_RX_OPT_FAS_WRD_ERR                  0x00
#define CMQ_RX_OPT_CNT_NFAS                     0x02
#define CMQ_RX_OPT_CNT_COFA                     0x01

/* RX Line IF Cfg - bit mask for offset 0x0003, 0x0103, 0x203, 0x0303 */
#define CMQ_RX_LIF_CFG_AUTO_YEL                 0x80
#define CMQ_RX_LIF_CFG_AUTO_RED                 0x40
#define CMQ_RX_LIF_CFG_AUTO_OOF                 0x20
#define CMQ_RX_LIF_CFG_AUTO_AIS                 0x10
#define CMQ_RX_LIF_CFG_BPV                      0x04
/* 
 * PREP 6353: Bits 0, 1, and 3 are RESERVED on COMET-Quad devices 
 *
 */
/* Definitions of bits 0, 1, and 3 on COMET-Quad */
#define CMQ_RX_LIF_CFG_RESERVED                0x0B

/* TX Line IF Cfg - bit mask for offset 0x0004, 0x0104, 0x204, 0x0304 */
#define CMQ_TX_LIF_CFG_JAT_BYPASS              0x80
#define CMQ_TX_LIF_CFG_AIS_EN                  0x40
#define CMQ_TX_LIF_CFG_AUX_PAT                 0x20
#define CMQ_TX_LIF_CFG_UNUSED                  0x05
/* 
 * PREP 6353: Bits 1, 3, and 4 are RESERVED on COMET-Quad devices 
 *
 */
/* Definitions of bits 1, 3, and 4 on COMET-Quad */
#define CMQ_TX_LIF_CFG_RESERVED                0x1A

/* TX and bypass options -bit mask for offset 0x0005, 0x0105, 0x205, 0x0305 */
#define CMQ_TX_FRM_OPT_PATH_CRC                 0x80
#define CMQ_TX_FRM_OPT_BASIC_RAI_TYPE           0x40
#define CMQ_TX_FRM_OPT_SIG_ALIGN_EN             0x20
#define CMQ_TX_FRM_OPT_OUT_OF_CRC_MRFRM_EN      0x10
#define CMQ_TX_FRM_OPT_FRM_DIS                  0x08
#define CMQ_TX_FRM_OPT_FBIT_BYPASS              0x04
#define CMQ_TX_FRM_OPT_CRC_BYPASS               0x02
#define CMQ_TX_FRM_OPT_FDL_BYPASS               0x01

/* TX Timing Options - bit mask for offset 0x0006, 0x0106, 0x206, 0x0306 */
#define CMQ_TX_TIME_OPT_RESERVE                 0xC2
#define CMQ_TX_TIME_OPT_OUTPUT_CLK_SEL          0x30
#define CMQ_TX_TIME_OPT_OUTPUT_CLK_JAT          0x00
#define CMQ_TX_TIME_OPT_OUTPUT_CLK_INTERN       0x10
#define CMQ_TX_TIME_OPT_OUTPUT_CLK_FIFO         0x20
#define CMQ_TX_TIME_OPT_PLL_REF_CLK             0x0C
#define CMQ_TX_TIME_OPT_PLL_REF_CLK_JAT         0x00
#define CMQ_TX_TIME_OPT_PLL_REF_CLK_BTIF        0x04
#define CMQ_TX_TIME_OPT_PLL_REF_CLK_RECOVER     0x08
#define CMQ_TX_TIME_OPT_PLL_REF_CLK_COMMON      0x0C
#define CMQ_TX_TIME_OPT_ELST_BYPASS             0x01

/* INT Source 1 - bit mask for offset 0x0007, 0x0107, 0x207, 0x0307 */
#define CMQ_INT_SRC1_INT_STAT_MASK              0xFF
#define CMQ_INT_SRC1_PMON                       0x80
#define CMQ_INT_SRC1_PRBS                       0x40
#define CMQ_INT_SRC1_FRMR                       0x20
#define CMQ_INT_SRC1_SIGX                       0x10
#define CMQ_INT_SRC1_T1_APRM                    0x08
#define CMQ_INT_SRC1_TJAT                       0x04
#define CMQ_INT_SRC1_RJAT                       0x02
#define CMQ_INT_SRC1_CDRC                       0x01
/* application specific constants for INT SRC1        */
#define CMQ_LINE_IF_INT_SRC1                                   \
         (CMQ_INT_SRC1_CDRC | CMQ_INT_SRC1_RJAT | CMQ_INT_SRC1_TJAT)
#define CMQ_PMON_INT_SRC1                                      \
         (CMQ_INT_SRC1_T1_APRM | CMQ_INT_SRC1_PMON)
#define CMQ_SIGX_INT_SRC1                                      \
         (CMQ_INT_SRC1_SIGX)
#define CMQ_FRMR_INT_SRC1                                      \
         (CMQ_INT_SRC1_FRMR)
#define CMQ_SERIAL_CTL_INT_SRC1                                   \
         (CMQ_INT_SRC1_PRBS)

/* INT Source 2 - bit mask for offset 0x0008, 0x0108, 0x208, 0x0308 */
#define CMQ_INT_SRC2_INT_STAT_MASK              0xDF
#define CMQ_INT_SRC2_RX_ELST                    0x80
#define CMQ_INT_SRC2_TX_ELST                    0x08
/* COMET QUAD bit[6:4]                          */
#define CMQ_INT_SRC2_RX_ELST_CCS                0x40
#define CMQ_INT_SRC2_RESERVE                    0x20
#define CMQ_INT_SRC2_RDLC                       0x10
/* COMET QUAD bit[2:0]                          */
#define CMQ_INT_SRC2_TX_ELST_CCS                0x04
#define CMQ_INT_SRC2_XBOC                       0x02
#define CMQ_INT_SRC2_TDPR                       0x01

/* application specific constants for INT SRC2 for COMET-QUAD     */
#define CMQ_INBAND_INT_SRC2                                       \
         (CMQ_INT_SRC2_TDPR | CMQ_INT_SRC2_RDLC | CMQ_INT_SRC2_XBOC |   \
          CMQ_INT_SRC2_TX_ELST_CCS | CMQ_INT_SRC2_RX_ELST_CCS) 

#define CMQ_SYS_IF_INT_SRC2                                       \
         (CMQ_INT_SRC2_RX_ELST | CMQ_INT_SRC2_TX_ELST)

/* application specific constants for INT SRC2 for COMET       */

/* INT Source 3 - bit mask for offset 0x0009, 0x0109, 0x209, 0x0309 */
#define CMQ_INT_SRC3_INT_STAT_MASK              0xFF
#define CMQ_INT_SRC3_IBCD                       0x80
#define CMQ_INT_SRC3_PDVD                       0x40
#define CMQ_INT_SRC3_RBOC                       0x20
#define CMQ_INT_SRC3_XPDE                       0x10
#define CMQ_INT_SRC3_ALMI                       0x08
#define CMQ_INT_SRC3_TRAN                       0x04
#define CMQ_INT_SRC3_RLPS                       0x02
#define CMQ_INT_SRC3_BTIF                       0x01
/* application specific constants for INT SRC3        */
#define CMQ_SYS_IF_INT_SRC3                                       \
         (CMQ_INT_SRC3_BTIF)
#define CMQ_LINE_IF_INT_SRC3                                   \
         (CMQ_INT_SRC3_RLPS | CMQ_INT_SRC3_XPDE | CMQ_INT_SRC3_PDVD)
#define CMQ_FRMR_INT_SRC3                                      \
         (CMQ_INT_SRC3_TRAN)
#define CMQ_SERIAL_CTL_INT_SRC3                                   \
         (CMQ_INT_SRC3_ALMI)
#define CMQ_INBAND_INT_SRC3                                       \
         (CMQ_INT_SRC3_RBOC | CMQ_INT_SRC3_IBCD)

/* MASTER DIAGNOSTICS - bit mask for offset 0x000A, 0x010A, 0x020A, 0x030A */
#define CMQ_MST_DIAG_RESERVE                    0x01
#define CMQ_MST_DIAG_ALOOP                      0x40
#define CMQ_MST_DIAG_PAYLD_LPBCK                0x20
#define CMQ_MST_DIAG_LINE_LPBCK                 0x10
#define CMQ_MST_DIAG_RX_AIS                     0x08
#define CMQ_MST_DIAG_DIG_LPBCK                  0x04
#define CMQ_MST_DIAG_TX_MIMIC_FRM_PAT           0x02
#define CMQ_MST_DIAG_LP_NONE                    0x00

/* MASTER TEST - bit mask for offset 0x000B */
#define CMQ_MST_TST_HIZ_DATA                    0x02
#define CMQ_MST_TST_HIZ_IO                      0X01

/* Comet-Quad only */
#define CMQ_MST_TST_RESERVE                     0xFC

/* DIAGNOSTICS - bit mask for offset 0x000C */
#define CMQ_DIAG_TX_CLK_DATA_CFG                0xF0
#define CMQ_DIAG_RX_CLK_DATA_CFG                0x0F

/* Chip ID REV PMON update - bit mask for offset 0x000D */
#define CMQ_REV_TYPE_MASK                       0xE0
#define CMQ_REV_ID_MASK                         0x1F
#define CMQ_REV_TYPE                            0x40

/*
 * COMET-Quad Revision ID 
 */
#define CMQ_REV_ID                              0x02

/* RESET - bit mask for offset 0x000E */
#define CMQ_RESET_RESERVE                       0xFC
#define CMQ_RESET_SLICE                         0x02
#define CMQ_RESET_CHIP                          0x01

/* PRBS POS and CTL - bit mask for offset 0x000F, 0x010F, 0x020F, 0x030F */
#define CMQ_PRBS_HDLC_RESERVE                   0xC0
#define CMQ_PRBS_HDLC_DIS                       0x20
#define CMQ_PRBS_HDLC_NX56K_GEN                 0x10
#define CMQ_PRBS_HDLC_NX56K_DET                 0x08
#define CMQ_PRBS_HDLC_RX_PAT_GEN                0x04
#define CMQ_PRBS_HDLC_UNFRM_GEN                 0x02
#define CMQ_PRBS_HDLC_UNFRM_DET                 0x01

/* CDRC Cfg - bit mask for offset 0x0010, 0x0110, 0x210, 0x0310 */
#define CMQ_CDRC_CFG_RESERVE                    0x19
#define CMQ_CDRC_CFG_AMI                        0x80
#define CMQ_CDRC_CFG_LOS_THRESH_MASK            0x60
#define CMQ_CDRC_CFG_LOS_E1_PCM10               0x00
#define CMQ_CDRC_CFG_LOS_T1_PCM15               0x00
#define CMQ_CDRC_CFG_LOS_PCM15                  0x00
#define CMQ_CDRC_CFG_LOS_PCM31                  0x20
#define CMQ_CDRC_CFG_LOS_PCM63                  0x40
#define CMQ_CDRC_CFG_LOS_PCM175                 0x60
#define CMQ_CDRC_CFG_PLL_ALG_SEL                0x04
#define CMQ_CDRC_CFG_O612_COMP                  0x02

/* CDRC INT Ctl - bit mask for offset 0x0011, 0x0111, 0x211, 0x0311 */
#define CMQ_CDRC_INT_MASK                       0xF0
#define CMQ_CDRC_INT_RESERVE                    0x0F
#define CMQ_CDRC_INT_EN_LCV                     0x80
#define CMQ_CDRC_INT_EN_LOS                     0x40
#define CMQ_CDRC_INT_EN_SIG_DET                 0x20
#define CMQ_CDRC_INT_EN_ZERO_DET                0x10

/* CDRC INT Stat - bit mask for offset 0x0012, 0x0112, 0x212, 0x0312 */
#define CMQ_CDRC_INT_STAT_MASK                  0xF0
#define CMQ_CDRC_STAT_RESERVE                   0x0E
#define CMQ_CDRC_STAT_LCV_IND                   0x80
#define CMQ_CDRC_STAT_LOS_IND                   0x40
#define CMQ_CDRC_STAT_SIG_DET_IND               0x20
#define CMQ_CDRC_STAT_ZERO_DET_IND              0x10
#define CMQ_CDRC_STAT_LOS_STATE                 0x01

/* ALT LOS Stat - bit mask for offset 0x0013, 0x0113, 0x213, 0x0313 */
#define CMQ_ALT_LOS_INT_STAT_MASK               0x40
#define CMQ_ALT_LOS_INT_MASK                    0x80
#define CMQ_ALT_LOS_RESERVE                     0x3E
#define CMQ_ALT_LOS_INT_EN                      0x80
#define CMQ_ALT_LOS_IND                         0x40
#define CMQ_ALT_LOS_STATE                       0x01

/* RJAT INT Stat - bit mask for offset 0x0014, 0x0114, 0x214, 0x0314 */
#define CMQ_RJAT_INT_STAT_MASK                  0x03
#define CMQ_RJAT_INT_STAT_RESERVE               0xFC
#define CMQ_RJAT_INT_STAT_OVRN_IND              0x02
#define CMQ_RJAT_INT_STAT_UNDRN_IND             0x01

/* RJAT Ref Clk Div - bit mask for offset 0x0015, 0x0114, 0x214, 0x0314 */
#define CMQ_RJAT_REF_CLK_DIV_MASK               0xFF

/* RJAT Output Clk Div - bit mask for offset 0x0016, 0x0116, 0x216, 0x0316 */
#define CMQ_RJAT_OUTPUT_CLK_DIV_MASK            0xFF

/* RJAT Cfg - bit mask for offset 0x0017, 0x0117, 0x217, 0x0317 */
#define CMQ_RJAT_CFG_INT_MASK                   0x0C
#define CMQ_RJAT_CFG_INT_RESERVE                0xE0
#define CMQ_RJAT_CFG_CNTR_FIFO_RD_PTR           0x10
#define CMQ_RJAT_CFG_INT_EN_UNDRN               0x08
#define CMQ_RJAT_CFG_INT_EN_OVRN                0x04
#define CMQ_RJAT_CFG_FIFO_RESET                 0x02
#define CMQ_RJAT_CFG_FIFO_LIMIT                 0x01

/* TJAT INT Stat - bit mask for offset 0x0018, 0x0118, 0x218, 0x0318 */
#define CMQ_TJAT_INT_STAT_MASK                  0x03
#define CMQ_TJAT_INT_STAT_RESERVE               0xFC
#define CMQ_TJAT_INT_STAT_OVRN_IND              0x02
#define CMQ_TJAT_INT_STAT_UNDRN_IND             0x01

/* TJAT Ref Clk Div -bit mask for offset 0x0019, 0x0119, 0x219, 0x0319 */
#define CMQ_TJAT_REF_CLK_DIV_MASK               0xFF

/* TJAT Output Clk Div - bit mask for offset 0x001A, 0x011A, 0x21A, 0x031A */
#define CMQ_TJAT_OUTPUT_CLK_DIV_MASK            0xFF

/* TJAT Cfg - bit mask for offset 0x001B, 0x011B, 0x21B, 0x031B */
#define CMQ_TJAT_CFG_INT_MASK                   0x0C
#define CMQ_TJAT_CFG_INT_RESERVE                0xE0
#define CMQ_TJAT_CFG_CNTR_FIFO_RD_PTR           0x10
#define CMQ_TJAT_CFG_INT_EN_UNDRN               0x08
#define CMQ_TJAT_CFG_INT_EN_OVRN                0x04
#define CMQ_TJAT_CFG_FIFO_RESET                 0x02
#define CMQ_TJAT_CFG_FIFO_LIMIT                 0x01

/* RX ELASTIC Cfg - bit mask for offset 0x001C, 0x011C, 0x21C, 0x031C */
#define CMQ_RX_ELST_CFG_RESERVE                 0xFC
#define CMQ_RX_ELST_CFG_E1                      0x03
#define CMQ_RX_ELST_CFG_T1                      0x00

/* RX ELASTIC Int/Stat - bit mask for offset 0x001D, 0x011D, 0x21D, 0x031D */
#define CMQ_RX_ELST_INT_MASK                    0x04
#define CMQ_RX_ELST_INT_STAT_MASK               0x01
#define CMQ_RX_ELST_RESERVE                     0xF8
#define CMQ_RX_ELST_INT_EN_SLIP                 0x04
#define CMQ_RX_ELST_STAT_SLIP_DIR               0x02
#define CMQ_RX_ELST_STAT_SLIP_FULL              0x02
#define CMQ_RX_ELST_STAT_SLIP_IND               0x01

/* RX ELASTIC idle code - bit mask for offset 0x001E, 0x011E, 0x21E, 0x031E */
#define CMQ_RX_ELST_TSLOT_DATA_MASK             0xFF

/* TX ELASTIC Cfg - bit mask for offset 0x0020, 0x0120, 0x220, 0x0320 */
#define CMQ_TX_ELST_CFG_RESERVE                 0xFC
#define CMQ_TX_ELST_CFG_E1                      0x03
#define CMQ_TX_ELST_CFG_T1                      0x00

/* TX ELASTIC Cfg - bit mask for offset 0x0021, 0x0121, 0x221, 0x0321 */
#define CMQ_TX_ELST_INT_MASK                    0x04
#define CMQ_TX_ELST_INT_STAT_MASK               0x01
#define CMQ_TX_ELST_RESERVE                     0xF8
#define CMQ_TX_ELST_INT_EN_SLIP                 0x04
#define CMQ_TX_ELST_STAT_SLIP_DIR               0x02
#define CMQ_TX_ELST_STAT_SLIP_FULL              0x02
#define CMQ_TX_ELST_STAT_SLIP_IND               0x01

/* RX Data Link Ctl - bit mask for offset 0x0028, 0x0128, 0x228, 0x0328 */
#define CMQ_RX_DLNK_CTL_EXTRACT_MASK            0xC0
#define CMQ_RX_DLNK_CTL_EXTRACT_EVEN_FRM        0x80
#define CMQ_RX_DLNK_CTL_EXTRACT_ODD_FRM         0x40
#define CMQ_RX_DLNK_CTL_EN_TERM                 0x20
#define CMQ_RX_DLNK_CTL_EXTRACT_TSLOT_MASK      0x1F

/* RX Data Link bit Sel - bit mask for offset 0x0029, 0x0129, 0x229, 0x0329 */
#define CMQ_RX_DLNK_BIT_SEL_MASK                0xFF

/* COMET QUAD offset 0x2A - 2F undefined        */

/* BRIF Cfg - bit mask for offset 0x0030, 0x0130, 0x230, 0x0330 */
#define CMQ_BRIF_CFG_MASK                       0xFF
#define CMQ_BRIF_CFG_NXDS0_1                    0x80
#define CMQ_BRIF_CFG_NXDS0_0                    0x40
#define CMQ_BRIF_CFG_NXDSO_MODE_MASK            0xC0
#define CMQ_BRIF_CFG_CLK_MODE_MASK              0x20
#define CMQ_BRIF_CFG_CLK_RATE_SEL_MASK          0x03
#define CMQ_BRIF_CFG_FULL_FRM                   0x00
#define CMQ_BRIF_CFG_56K_NXDS0                  0x40
#define CMQ_BRIF_CFG_64K_NXDS0                  0x80
#define CMQ_BRIF_CFG_64K_NXDS0_FBIT             0xC0
#define CMQ_BRIF_CFG_CLK_SLAVE                  0x20
#define CMQ_BRIF_CFG_CLK_EDGE_HI_DATA           0x10
#define CMQ_BRIF_CFG_CLK_EDGE_HI_FRM            0x08
#define CMQ_BRIF_CFG_CLK_FREQ_TIMES2            0x04
#define CMQ_BRIF_CFG_CLK_RATE_1                 0x02
#define CMQ_BRIF_CFG_CLK_RATE_0                 0x01
#define CMQ_BRIF_CFG_CLK_RATE_SEL_8192          0x03
#define CMQ_BRIF_CFG_CLK_RATE_SEL_2048          0x01
#define CMQ_BRIF_CFG_CLK_RATE_SEL_1544          0x00

/* BRIF Frm Pulse Cfg - bit mask for offset 0x0031, 0x0131, 0x231, 0x0331 */
#define CMQ_BRIF_FRM_PULSE_CFG_24_TSLOTS        0x80
#define CMQ_BRIF_FRM_PULSE_CFG_INV              0x40
#define CMQ_BRIF_FRM_PULSE_CFG_MODE             0X20
#define CMQ_BRIF_FRM_PULSE_SLAVE                0x20
#define CMQ_BRIF_FRM_PULSE_MASTER               0x00
#define CMQ_BRIF_FRM_PULSE_EN_ALT_FDL           0x10
#define CMQ_BRIF_FRM_PULSE_CFG_MASK             0x0E
#define CMQ_BRIF_FRM_PULSE_CFG_OVHD             0x08
#define CMQ_BRIF_FRM_PULSE_CFG_COMP_MFRM        0x06
#define CMQ_BRIF_FRM_PULSE_CFG_SIG_MFRM         0x04
#define CMQ_BRIF_FRM_PULSE_CFG_CRC_MFRM         0x02
#define CMQ_BRIF_FRM_PULSE_CFG_ROHM             0x08
#define CMQ_BRIF_FRM_PULSE_CFG_BRXSMFP          0x04
#define CMQ_BRIF_FRM_PULSE_CFG_BRXCMFP          0x02
#define CMQ_BRIF_FRM_PULSE_CFG_ALTBRFP          0x01
#define CMQ_BRIF_FRM_PULSE_CFG_OUTPUT           0x00
#define CMQ_BRIF_FRM_PULSE_CFG_T1_SF_ESF        0x04
#define CMQ_BRIF_FRM_PULSE_CFG_T1_193_BITS      0x00
#define CMQ_BRIF_FRM_PULSE_EN_ALT               0x01

/* BRIF FBIT parity Cfg - bit mask for offset 0x0032, 0x0132, 0x232, 0x0332 */
#define CMQ_BRIF_PAR_FBIT_CFG_ODD                  0x80
#define CMQ_BRIF_PAR_FBIT_CFG_PARITY_INS_EN        0x40
#define  CMQ_BRIF_PAR_FBIT_CFG_FIX_POLARITY_MASK   0x30
#define CMQ_BRIF_PAR_FBIT_CFG_FIXF                 0x20
#define CMQ_BRIF_PAR_FBIT_CFG_HIGH                 0x10
#define CMQ_BRIF_PAR_FBIT_CFG_FIX_POLARITY_NONE    0x00
#define CMQ_BRIF_PAR_FBIT_CFG_PAR_EXTEND           0x08
/* COMETQ bit maps for bit[1:0]                 */
#define CMQ_BRIF_PAR_FBIT_CFG_RESERVE           0x02
#define CMQ_BRIF_PAR_FBIT_CFG_TRI_HMVIP         0x01

/* BRIF TSlot Offset - bit mask for offset 0x0033, 0x0133, 0x233, 0x0333 */
#define CMQ_BRIF_TSLOT_RESERVE                  0x80
#define CMQ_BRIF_TSLOT_BYTE_OFFSET_MASK         0x7F
#define CMQ_BRIF_TSLOT_HMVIP_QUADRANT_1         0x00
#define CMQ_BRIF_TSLOT_HMVIP_QUADRANT_2         0x01
#define CMQ_BRIF_TSLOT_HMVIP_QUADRANT_3         0x02
#define CMQ_BRIF_TSLOT_HMVIP_QUADRANT_4         0x03

/* BRIF FP Bit Offset - bit mask for offset 0x0034, 0x0134, 0x234, 0x0334 */
#define CMQ_BRIF_FP_BIT_OFFSET_RESERVE          0xF0
#define CMQ_BRIF_FP_BIT_OFFSET_EN               0x08
#define CMQ_BRIF_FP_BIT_OFFSET_MASK             0x07

/* TX Data Link Ctl - bit mask for offset 0x0038, 0x0138, 0x238, 0x0338 */
#define CMQ_TX_DLNK_CTL_INS_MASK                0xC0
#define CMQ_TX_DLNK_CTL_INS_EVEN_FRM            0x80
#define CMQ_TX_DLNK_CTL_INS_ODD_FRM             0x40
#define CMQ_TX_DLNK_CTL_EN_INS                  0x20
#define CMQ_TX_DLNK_CTL_INS_TSLOT_MASK          0x1F

/* TX Data Link Sel Mask - bit mask for offset 0x0039, 0x0139, 0x239, 0x0339 */
#define CMQ_TX_DLNK_BIT_SEL_MASK                0xFF

/* COMET QUAD offset 0x3A - 3F undefined        */

/* BTIF Cfg - bit mask for offset 0x0040, 0x0140, 0x240, 0x0340 */
#define CMQ_BTIF_CFG_MASK                       0xFF
#define CMQ_BTIF_CFG_NXDSO_MODE_MASK            0xC0
#define CMQ_BTIF_CFG_CLK_RATE_SEL_MASK          0x03
#define CMQ_BTIF_CFG_FULL_FRM                   0x00
#define CMQ_BTIF_CFG_NXDS0_1                    0x80
#define CMQ_BTIF_CFG_NXDS0_0                    0x40
#define CMQ_BTIF_CFG_CLK_MODE_MASK              0x20
#define CMQ_BTIF_CFG_CLK_RATE_1                 0x02
#define CMQ_BTIF_CFG_CLK_RATE_0                 0x01
#define CMQ_BTIF_CFG_56K_NXDS0                  0x40
#define CMQ_BTIF_CFG_64K_NXDS0                  0x80
#define CMQ_BTIF_CFG_64K_NXDS0_FBIT             0xC0
#define CMQ_BTIF_CFG_CLK_SLAVE                  0x20
#define CMQ_BTIF_CFG_CLK_EDGE_HI_DATA           0x10
#define CMQ_BTIF_CFG_CLK_EDGE_HI_FRM            0x08
#define CMQ_BTIF_CFG_CLK_FREQ_TIMES2            0x04
#define CMQ_BTIF_CFG_CLK_RATE_SEL_8192          0x03
#define CMQ_BTIF_CFG_CLK_RATE_SEL_2048          0x01
#define CMQ_BTIF_CFG_CLK_RATE_SEL_1544          0x00

/* BTIF Frame Pulse Cfg bit mask for offset 0x0041, 0x0141, 0x241, 0x0341 */
#define CMQ_BTIF_FRM_PULSE_CFG_RESERVE             0x70
#define CMQ_BTIF_FRM_PULSE_CFG_24_TSLOTS           0x80
#define CMQ_BTIF_FRM_PULSE_CFG_INV                 0x08
#define CMQ_BTIF_FRM_PULSE_ESF_EN                  0x04
#define CMQ_BTIF_FRM_PULSE_T1_ESF_ALIGN            0x02  
#define CMQ_BTIF_FRM_PULSE_E1_MFRM_ALIGN           0x02
#define CMQ_BTIF_FRM_PULSE_E1_FRM_ALIGN            0x00
#define CMQ_BTIF_FRM_PULSE_SLAVE                   0x01
#define CMQ_BTIF_FRM_PULSE_MASTER                  0x00
#define CMQ_BTIF_FRM_PULSE_TYPE                    0x02
#define CMQ_BTIF_FRM_PULSE_CFG_MODE                0x01

/* BTIF Frame Parity Cfg - bit mask for offset 0x0042, 0x0142, 0x242, 0x0342 */
#define CMQ_BTIF_PAR_INT_MASK                      0x40
#define CMQ_BTIF_PAR_INT_STAT_MASK                 0x30
#define CMQ_BTIF_PAR_CFG_ODD                       0x80
#define CMQ_BTIF_PAR_CFG_INT_EN                    0x40
#define CMQ_BTIF_PAR_CFG_DATA_ERR_IND              0x20
#define CMQ_BTIF_PAR_CFG_SIG_ERR_IND               0x10
#define CMQ_BTIF_PAR_CFG_EXT                       0x08
#define CMQ_BTIF_PAR_CFG_RESERVE                   0x07

/* BTIF TSlot Offset - bit mask for offset 0x0043, 0x0143, 0x243, 0x0343 */
#define CMQ_BTIF_TSLOT_BYTE_OFFSET_MASK            0x7F
#define CMQ_BTIF_TSLOT_HMVIP_QUADRANT_1            0x00
#define CMQ_BTIF_TSLOT_HMVIP_QUADRANT_2            0x01
#define CMQ_BTIF_TSLOT_HMVIP_QUADRANT_3            0x02
#define CMQ_BTIF_TSLOT_HMVIP_QUADRANT_4            0x03

/* BTIF FP Bit Offset - bit mask for offset 0x0044, 0x0144, 0x244, 0x0344 */
#define CMQ_BTIF_FP_BIT_OFFSET_RESERVE             0xF0
#define CMQ_BTIF_FP_BIT_OFFSET_EN                  0x08
#define CMQ_BTIF_FP_BIT_OFFSET_MASK                0x07

/* T1 Framer Cfg - bit mask for offset 0x0048, 0x0148, 0x248, 0x0348 */
#define CMQ_T1_FRMR_CFG_RESERVE                    0x01
#define CMQ_T1_FRMR_CFG_ERR_DET_MASK               0xC0
#define CMQ_T1_FRMR_CFG_ERR_DET_2OF6               0x80
#define CMQ_T1_FRMR_CFG_ERR_DET_2OF5               0x40
#define CMQ_T1_FRMR_CFG_ERR_DET_2OF4               0x00
#define CMQ_T1_FRMR_CFG_SF_MODE                    0x00
#define CMQ_T1_FRMR_CFG_T1DM_MODE                  0x04
#define CMQ_T1_FRMR_CFG_SLC96_MODE                 0x08
#define CMQ_T1_FRMR_CFG_T1DM_MODE2                 0x0C /* same as MODE1 */
#define CMQ_T1_FRMR_CFG_ESF_FDL_MODE               0x10
#define CMQ_T1_FRMR_CFG_MODE_RESERVE1              0x14
#define CMQ_T1_FRMR_CFG_MODE_RESERVE2              0x18
#define CMQ_T1_FRMR_CFG_MODE_RESERVE3              0x1C
#define CMQ_T1_FRMR_CFG_MODE_MASK                  0x1C
#define CMQ_T1_FRMR_CFG_JPN_EN                     0x02
#define CMQ_T1_FRMR_CFG_ESF_CRC_ALGO               0x20
   
/* T1 Framer INT enable - bit mask for offset 0x0049, 0x0149, 0x249, 0x0349 */
#define CMQ_T1_FRMR_INT_MASK                    0x3F
#define CMQ_T1_FRMR_INT_RESERVE                    0xC0
#define CMQ_T1_FRMR_INT_EN_CHG_ALIGN               0x20
#define CMQ_T1_FRMR_INT_EN_FERR                    0x10
#define CMQ_T1_FRMR_INT_EN_BIT_ERR_EVENT           0x08
#define CMQ_T1_FRMR_INT_EN_SEVR_ERR                0x04
#define CMQ_T1_FRMR_INT_EN_MIMIC_DET               0x02
#define CMQ_T1_FRMR_INT_EN_IN_FRM                  0x01
   
/* T1 Framer INT stat - bit mask for offset 0x004A, 0x014A, 0x24A, 0x034A */
#define CMQ_T1_FRMR_INT_STAT_MASK                  0xFC
#define CMQ_T1_FRMR_STAT_CHG_ALIGN_IND             0x80
#define CMQ_T1_FRMR_STAT_ERR_IND                   0x40
#define CMQ_T1_FRMR_STAT_BIT_ERR_EVENT_IND         0x20
#define CMQ_T1_FRMR_STAT_SEVR_ERR_IND              0x10
#define CMQ_T1_FRMR_STAT_MIMIC_DET_IND             0x08
#define CMQ_T1_FRMR_STAT_IN_FRM_IND                0x04
#define CMQ_T1_FRMR_STAT_MIMIC_FRM_STATE           0x02
#define CMQ_T1_FRMR_STAT_IN_FRM_STATE              0x01

/* IBCD Cfg - bit mask for offset 0x004C, 0x014C, 0x24C, 0x034C */
#define CMQ_IBCD_CFG_RESERVE                       0xF0
#define CMQ_IBCD_CFG_DEACT_CD_LEN_MASK             0x0C
#define CMQ_IBCD_CFG_DEACT_CD_LEN8                 0x0C
#define CMQ_IBCD_CFG_DEACT_CD_LEN7                 0x08
#define CMQ_IBCD_CFG_DEACT_CD_LEN6                 0x04
#define CMQ_IBCD_CFG_DEACT_CD_LEN5                 0x00
#define CMQ_IBCD_CFG_ACT_CD_LEN_MASK               0x03
#define CMQ_IBCD_CFG_ACT_CD_LEN8                   0x03
#define CMQ_IBCD_CFG_ACT_CD_LEN7                   0x02
#define CMQ_IBCD_CFG_ACT_CD_LEN6                   0x01
#define CMQ_IBCD_CFG_ACT_CD_LEN5                   0x00

/* IBCD INT enable/stat bit mask for offset 0x004D, 0x014D, 0x24D, 0x034D */
#define CMQ_IBCD_INT_MASK                          0x30
#define CMQ_IBCD_INT_STAT_MASK                     0x0C
#define CMQ_IBCD_STAT_LPBCK_ACT_PRES               0x80
#define CMQ_IBCD_STAT_LPBCK_DEACT_PRES             0x40
#define CMQ_IBCD_INT_EN_LPBCK_ACT                  0x20
#define CMQ_IBCD_INT_EN_LPBCK_DEACT                0x10
#define CMQ_IBCD_STAT_LPBCK_ACT_IND                0x08
#define CMQ_IBCD_STAT_LPBCK_DEACT_IND              0x04
#define CMQ_IBCD_STAT_LPBCK_ACT_STATE              0x02
#define CMQ_IBCD_STAT_LPBCK_DEACT_STATE            0x01

/* IBCD Activate code - bit mask for offset 0x004E, 0x014E, 0x24E, 0x034E */
#define CMQ_IBCD_LPBCK_ACT_CD_MASK                 0xFF
                                          
/* IBCD DeActivate code - bit mask for offset 0x004F, 0x014F, 0x24F, 0x034F */
#define CMQ_IBCD_LPBCK_DEACT_CD_MASK               0xFF

/* SIGX Cfg - bit mask for offset 0x0050, 0x0150, 0x250, 0x0350 */
#define CMQ_SIX_CFG_INT_MASK                       0x20
#define CMQ_SIGX_CFG_RESERVE                       0x98
/* bit 6 Poll enable or indirect access */
#define CMQ_SIGX_CFG_COSS_POLL_EN                  0x40
#define CMQ_SIGX_CFG_COSS_IND_ACCESS               0x00
#define CMQ_SIGX_CFG_COSS_INT_EN                   0x20
#define CMQ_SIGX_CFG_ESF_MODE                      0x04
/* bit 1 indirect or direct Reg access */
#define CMQ_SIGX_CFG_IND_ACCESS                    0x02
#define CMQ_SIGX_CFG_DIR_ACCESS                    0x00
#define CMQ_SIGX_CFG_PER_TSLOT_CHN_CFG_EN          0x01

/* SIGX COSS - bit mask for offset 0x0051, 0x0151, 0x0251, 0x0351 */
#define CMQ_SIGX_COSS_RESERVE                   0x80
#define CMQ_SIGX_COSS_IND_ACCESS_MODE           0x00
/* MODE bit is shared in both modes of operation */
#define CMQ_SIGX_COSS_MODE                      0x40
/* operational mode of this Reg */
#define CMQ_SIGX_COSS_INT_STAT_MASK1               0x3F
#define CMQ_SIGX_COSS_30                           0x20
#define CMQ_SIGX_COSS_29                           0x10
#define CMQ_SIGX_COSS_28                           0x08
#define CMQ_SIGX_COSS_27                           0x04
#define CMQ_SIGX_COSS_26                           0x02
#define CMQ_SIGX_COSS_25                           0x01

/* SIGX TimeSlot Ind Stat - bit mask for offset 0x0051, 0x0151, 0x0251, 0x0351 */
/* bit mask apply if bit 6 is clr in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_TSLOT_IND_STAT_RESERVE            0x7F
#define CMQ_SIGX_TSLOT_IND_STAT_RD_BUSY            0x80

/* SIGX COSS Change - bit mask for offset 0x0051, 0x0151, 0x251, 0x0351 */
/* bit mask apply if bit 6 is set in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_COSS_INT_STAT_MASK2               0xFF
#define CMQ_SIGX_COSS_24                           0x80
#define CMQ_SIGX_COSS_23                           0x40
#define CMQ_SIGX_COSS_22                           0x20
#define CMQ_SIGX_COSS_21                           0x10
#define CMQ_SIGX_COSS_20                           0x08
#define CMQ_SIGX_COSS_19                           0x04
#define CMQ_SIGX_COSS_18                           0x02
#define CMQ_SIGX_COSS_17                           0x01

/* SIGX TSlot Ind Addr Ctl - bit mask for offset 0x0052, 0x0152, 0x252, 0x0352 */
/* bit mask apply if bit 6 is clr in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_IND_ADDR_CTL_RW_MASK              0x80
#define CMQ_SIGX_IND_ADDR_CTL_RD                   0x80
#define CMQ_SIGX_IND_ADDR_CTL_WR                   0x00
#define CMQ_SIGX_IND_ADDR_CTL_ADDR_MASK            0x7F

/*SIGX COSS Change - bit mask for offset 0x0052, 0x0152, 0x252, 0x0352 */
/* bit mask apply if bit 6 is set in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_COSS_INT_STAT_MASK3               0xFF
#define CMQ_SIGX_COSS_16                           0x80
#define CMQ_SIGX_COSS_15                           0x40
#define CMQ_SIGX_COSS_14                           0x20
#define CMQ_SIGX_COSS_13                           0x10
#define CMQ_SIGX_COSS_12                           0x08
#define CMQ_SIGX_COSS_11                           0x04
#define CMQ_SIGX_COSS_10                           0x02
#define CMQ_SIGX_COSS_9                            0x01

/* bit mask for offset 0x0053, 0x0153, 0x253, 0x0353 */
/* bit mask apply if bit 6 is clr in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_TSLOT_IND_DATA_BUF                0xFF

/* bit mask for offset 0x0053, 0x0153, 0x253, 0x0353 */
/* bit mask apply if bit 6 is set in reg 0x0050, 0x0150, 0x0250, 0x0350 */
#define CMQ_SIGX_COSS_INT_STAT_MASK4               0xFF
#define CMQ_SIGX_COSS_8                            0x80
#define CMQ_SIGX_COSS_7                            0x40
#define CMQ_SIGX_COSS_6                            0x20
#define CMQ_SIGX_COSS_5                            0x10
#define CMQ_SIGX_COSS_4                            0x08
#define CMQ_SIGX_COSS_3                            0x04
#define CMQ_SIGX_COSS_2                            0x02
#define CMQ_SIGX_COSS_1                            0x01

/* indirect register offsets within SIGX block 
   0x10 - 0x5F 
*/
#define CMQ_SIGX_IND_CUR_SIG_OFFSET_BASEADDR       0x10
#define CMQ_SIGX_IND_DLY_SIG_OFFSET_BASEADDR       0x20
#define CMQ_SIGX_IND_CFG_OFFSET_BASEADDR           0x40
#define CMQ_SIGX_IND_TSLOT_MAX                     0x20

/* bit mask apply if bit 6 is set in reg 0x0050, 0x0150, 0x0250, 0x0350 */
/* indirect register offsets within SIGX block 
   0x20 - 0x5F    bit mask for signal and data
*/

/* SIGX indirect 0x40-0x5F - bit mask */
#define CMQ_SIGX_IND_SIG_BIT_RESERVE               0xF0
#define CMQ_SIGX_IND_SIG_BIT_MASK                  0x0F
#define CMQ_SIGX_IND_SIG_BITA                      0x08     
#define CMQ_SIGX_IND_SIG_BITB                      0x04     
#define CMQ_SIGX_IND_SIG_BITC                      0x02     
#define CMQ_SIGX_IND_SIG_BITD                      0x01

/* SIGX indirect 0x20-0x3F - bit mask */
#define CMQ_SIGX_IND_COND_DATA_RESERVE             0xF0
#define CMQ_SIGX_IND_COND_DATA_MASK                0x0F

/* XBAS Cfg - bit mask for offset 0x0054, 0x0154, 0x254, 0x0354 */
#define CMQ_T1_XBAS_CFG_TRNK                       0x80
#define CMQ_T1_XBAS_CFG_JPN_EN                     0x40
#define CMQ_T1_XBAS_CFG_B8ZS_EN                    0x20
#define CMQ_T1_XBAS_CFG_SF_MODE                    0x00
#define CMQ_T1_XBAS_CFG_T1DM_MODE                  0x04
#define CMQ_T1_XBAS_CFG_SLC96_MODE                 0x08
#define CMQ_T1_XBAS_CFG_T1DM_MODE2                 0x0C /* same as MODE1 */
#define CMQ_T1_XBAS_CFG_ESF_FDL_MODE               0x10
#define CMQ_T1_XBAS_CFG_MODE_RESERVE1              0x14
#define CMQ_T1_XBAS_CFG_MODE_RESERVE2              0x18
#define CMQ_T1_XBAS_CFG_MODE_RESERVE3              0x1C
#define CMQ_T1_XBAS_CFG_MODE_MASK                  0x1C
#define CMQ_T1_XBAS_CFG_ZSUP_FMT_MASK              0x03
#define CMQ_T1_XBAS_CFG_ZSUP_FMT_NONE              0x00
#define CMQ_T1_XBAS_CFG_ZSUP_FMT_GTE               0x01
#define CMQ_T1_XBAS_CFG_ZSUP_FMT_DDS               0x02
#define CMQ_T1_XBAS_CFG_ZSUP_FMT_BELL              0x03

/* XBAS Alarm TX - bit mask for offset 0x0055, 0x0155, 0x255, 0x0355 */
#define CMQ_T1_XBAS_ALRM_RESERVE                   0xFC
#define CMQ_T1_XBAS_ALRM_TX_YEL                    0x02
#define CMQ_T1_XBAS_ALRM_TX_AIS                    0x01
                           
/* XBIC Ctl - bit mask for offset 0x0056, 0x0156, 0x256, 0x0356 */
#define CMQ_T1_XIBC_CTL_RESERVE                    0x3C
#define CMQ_T1_XIBC_CTL_EN                         0x80
#define CMQ_T1_XIBC_CTL_UNFRM_EN                   0x40
#define CMQ_T1_XIBC_CTL_CD_LEN_MASK                0x03
#define CMQ_T1_XIBC_CTL_CD_LEN5                    0x00
#define CMQ_T1_XIBC_CTL_CD_LEN6                    0x01
#define CMQ_T1_XIBC_CTL_CD_LEN7                    0x02
#define CMQ_T1_XIBC_CTL_CD_LEN8                    0x03
                                       
/* XBIC LoopBack Code - bit mask for offset 0x0057, 0x0157, 0x257, 0x0357 */
#define CMQ_T1_XIBC_LPBCK_CD_MASK                  0xFF

/* PMON INT enable/stat bit mask for offset 0x0058, 0x0158, 0x258, 0x0358 */
#define CMQ_PMON_INT_MASK                          0x04
#define CMQ_PMON_INT_STAT_MASK                     0x03
#define CMQ_PMON_INT_EN                            0x04
#define CMQ_PMON_INT_STAT_XFER_IND                 0x02
#define CMQ_PMON_INT_STAT_OVRN_IND                 0x01

/* PMON Frm ErrCnt - bit mask for offset 0x0059, 0x0159, 0x259, 0x0359 */
#define CMQ_PMON_STAT_FRM_ERR_CNT_RESERVE          0x80
#define CMQ_PMON_STAT_FRM_ERR_CNT_MASK             0x7F

/* PMON FEBE Cnt LSB - bit mask for offset 0x005A, 0x015A, 0x25A, 0x035A */
#define CMQ_PMON_OOF_COFA_FEBE_CNT_LSB_MASK        0xFF

/* PMON FEBE Cnt MSB - bit mask for offset 0x005B, 0x015B, 0x25B, 0x035B */
#define CMQ_PMON_OOF_COFA_FEBE_MSB_RESERVE         0xFC
#define CMQ_PMON_OOF_COFA_FEBE_CNT_MSB_MASK        0x03

/* PMON BIT ErrCnt LSB - bit mask for offset 0x005C, 0x015C, 0x25C, 0x035C */
#define CMQ_PMON_BITERR_CRCERR_CNT_LSB_MASK        0xFF
                                       
/* PMON BIT ErrCnt MSB - bit mask for offset 0x005D, 0x015D, 0x25D, 0x035D */
#define CMQ_PMON_BITERR_CRCERR_CNT_RESERVE         0xFC
#define CMQ_PMON_BITERR_CRCERR_CNT_MSB_MASK        0x03

/* PMON LCV Cnt LSB - bit mask for offset 0x005E, 0x015E, 0x25E, 0x035E */
#define CMQ_PMON_LCV_CNT_LSB_MASK                  0xFF

/* PMON LCV Cnt MSB - bit mask for offset 0x005F, 0x015F, 0x25F, 0x035F */
#define CMQ_PMON_LCV_CNT_RESERVE                   0xE0
#define CMQ_PMON_LCV_CNT_MSB_MASK                  0x1F

/* ALMI Cfg - bit mask for offset 0x0060, 0x0160, 0x260, 0x0360 */
#define CMQ_ALMI_CFG_RESERVE                       0xE3
#define CMQ_T1_ALMI_CFG_SF_MODE                    0x00
#define CMQ_T1_ALMI_CFG_T1DM_MODE                  0x04
#define CMQ_T1_ALMI_CFG_SLC96_MODE                 0x08
#define CMQ_T1_ALMI_CFG_T1DM_MODE2                 0x0C /* same as MODE1 */
#define CMQ_T1_ALMI_CFG_ESF_FDL_MODE               0x10
#define CMQ_T1_ALMI_CFG_MODE_RESERVE1              0x14
#define CMQ_T1_ALMI_CFG_MODE_RESERVE2              0x18
#define CMQ_T1_ALMI_CFG_MODE_RESERVE3              0x1C
#define CMQ_T1_ALMI_CFG_MODE_MASK                  0x1C

/* ALMI INT enable - bit mask for offset 0x0061, 0x0161, 0x261, 0x0361 */
#define CMQ_ALMI_INT_MASK                          0x07
#define CMQ_ALMI_RESERVE                           0xE0
#define CMQ_ALMI_EN_FAST_DEASERT_ALRM              0x10
#define CMQ_ALMI_EN_ACCEL                          0x08
#define CMQ_ALMI_INT_EN_YEL                        0x04
#define CMQ_ALMI_INT_EN_RED                        0x02
#define CMQ_ALMI_INT_EN_AIS                        0x01

/* ALMI INT status - bit mask for offset 0x0062, 0x0162, 0x262, 0x0362 */
#define CMQ_ALMI_INT_STAT_MASK                     0x38
#define CMQ_ALMI_INT_STAT_RESERVE                  0xC0
#define CMQ_ALMI_INT_STAT_YEL_IND                  0x20
#define CMQ_ALMI_INT_STAT_RED_IND                  0x10
#define CMQ_ALMI_INT_STAT_AIS_IND                  0x08
#define CMQ_ALMI_INT_STAT_YEL_STATE                0x04
#define CMQ_ALMI_INT_STAT_RED_STATE                0x02
#define CMQ_ALMI_INT_STAT_AIS_STATE                0x01

/* ALMI Detect Stat - bit mask for offset 0x0063, 0x0163, 0x263, 0x0363 */
#define CMQ_ALMI_ALRM_STAT_RESERVE                 0xF8
#define CMQ_ALMI_ALRM_STAT_RED_DET                 0x04
#define CMQ_ALMI_ALRM_STAT_YEL_DET                 0x02
#define CMQ_ALMI_ALRM_STAT_AIS_DET                 0x01

/* T1 PDVD INT / stat -
   bit mask for offset 0x0065, 0x0165, 0x265, 0x0365 */
#define CMQ_T1_PDVD_INT_MASK                       0x03
#define CMQ_T1_PDVD_INT_STAT_MASK                  0x0C
#define CMQ_T1_PDVD_RESERVE                        0xE0
#define CMQ_T1_PDVD_PLS_DNSTY_VLTN_STATE           0x10
#define CMQ_T1_PDVD_STAT_Z16_DNSTY_DET_IND         0x08
#define CMQ_T1_PDVD_STAT_PLS_DNSTY_VLTN_IND        0x04
#define CMQ_T1_PDVD_INT_EN_Z16_DNSTY_DET           0x02
#define CMQ_T1_PDVD_INT_EN_PLS_DNSTY_VLTN          0x01

/* T1 TX BOC code - bit mask for offset 0x0066, 0x0166, 0x266, 0x0366 */
#define CMQ_T1_XBOC_CTL_INT_MASK                   0x40
#define CMQ_T1_XBOC_CTL_STAT_MASK                  0X80
#define CMQ_T1_XBOC_CTL_INT_STAT_SMP_RDY_IND       0x80
#define CMQ_T1_XBOC_CTL_INT_EN_SMP_RDY             0x40
#define CMQ_T1_XBOC_CTL_UPD_RDY                    0x20
#define CMQ_T1_XBOC_CTL_RESERVE                    0x10
#define CMQ_T1_XBOC_CTL_BOC_REPEAT_CNT_MASK        0X0F

/* T1 TX BOC code - bit mask for offset 0x0067, 0x0167, 0x267, 0x0367 */
#define CMQ_T1_XBOC_RESERVE                        0xC0
#define CMQ_T1_XBOC_CD_MASK                        0x3F

/* T1 XPDE INT/stat - bit mask for offset 0x0069, 0x0169, 0x269, 0x0369 */
#define CMQ_T1_XPDE_INT_MASK                       0x83
#define CMQ_T1_XPDE_INT_STAT_MASK                  0x2C
#define CMQ_T1_XPDE_INT_EN_STUFF                   0x80
#define CMQ_T1_XPDE_STAT_STUFF_STATE               0x40
#define CMQ_T1_XPDE_STAT_STUFF_IND                 0x20
#define CMQ_T1_XPDE_STAT_PLS_DNSTY_VLTN_STATE      0x10
#define CMQ_T1_XPDE_STAT_Z16_DET_IND               0x08
#define CMQ_T1_XPDE_STAT_PLS_DNSTY_VLTN_IND        0x04
#define CMQ_T1_XPDE_INT_EN_Z16_DET                 0x02
#define CMQ_T1_XPDE_INT_EN_PLS_DNSTY_VLTN          0x01

/* T1 RBOC enable - bit mask for offset 0x006A, 0x016A, 0x26A, 0x036A */
#define CMQ_T1_RBOC_INT_MASK                       0x05
#define CMQ_T1_RBOC_RESERVE                        0xF8
#define CMQ_T1_RBOC_INT_EN_IDLE                    0x04
#define CMQ_T1_RBOC_EN_VALID_CRITERIA_4OF5         0x02
#define CMQ_T1_RBOC_EN_VALID_CRITERIA_8OF10        0x00
#define CMQ_T1_RBOC_INT_EN_BOC                     0x01

/* T1 RBOC stat - bit mask for offset 0x006B, 0x016B, 0x26B, 0x036B */
#define CMQ_T1_RBOC_INT_STAT_MASK                  0xC0
#define CMQ_T1_RBOC_INT_STAT_IDLE_IND              0x80
#define CMQ_T1_RBOC_INT_STAT_IND                   0x40
#define CMQ_T1_RBOC_STAT_BOC_CD_MASK               0x3F

/* TPSC Cfg - bit mask for offset 0x006C, 0x016C, 0x26C, 0x036C */
#define CMQ_TPSC_CFG_RESERVE                       0xFC
#define CMQ_TPSC_CFG_IND_ACCESS                    0x02
#define CMQ_TPSC_CFG_EN_PER_SER_CHN                0x01

/* TPSC micro access stat - bit mask for offset 0x006D, 0x016D, 0x26D, 0x036D */
#define CMQ_TPSC_MICRO_ACCESS_RESERVE              0x7F
#define CMQ_TPSC_MICRO_ACCESS_STAT_BUSY            0x80

/* 
 * TPSC micro access addr ctl - 
 * bit mask for offset 0x006E, 0x016E, 0x26E, 0x036E
 */
#define CMQ_TPSC_CHN_IND_ADDR_CTL_RD               0x80
#define CMQ_TPSC_CHN_IND_ADDR_CTL_WR               0x00
#define CMQ_TPSC_CHN_IND_ADDR_MASK                 0x7F

/* TPSC micro Ind Data Buf -
bit mask for offset 0x006F, 0x016F, 0x26F, 0x036F */
#define CMQ_TPSC_CHN_IND_DATA_BUF_MASK             0xFF

/*
 * Generic bit mask for indirect access
 */
#define CMQ_CFG_RESERVE                       CMQ_TPSC_CFG_RESERVE
#define CMQ_CFG_IND_ACCESS                    CMQ_TPSC_CFG_IND_ACCESS
#define CMQ_CFG_EN_PER_SER_CHN                CMQ_TPSC_CFG_EN_PER_SER_CHN
#define CMQ_MICRO_ACCESS_RESERVE              CMQ_TPSC_MICRO_ACCESS_RESERVE
#define CMQ_MICRO_ACCESS_STAT_BUSY            CMQ_TPSC_MICRO_ACCESS_STAT_BUSY
#define CMQ_CHN_IND_ADDR_CTL_RD               CMQ_TPSC_CHN_IND_ADDR_CTL_RD
#define CMQ_CHN_IND_ADDR_CTL_WR               CMQ_TPSC_CHN_IND_ADDR_CTL_WR
#define CMQ_CHN_IND_ADDR_MASK                 CMQ_TPSC_CHN_IND_ADDR_MASK
#define CMQ_CHN_IND_DATA_BUF_MASK             CMQ_TPSC_CHN_IND_DATA_BUF_MASK

/* TPSC Framer Indirect Reg Offset */
#define CMQ_TPSC_IND_OFFSET                        0x20

/* TPSC Indirect Reg PCM Data Ctl 0x20-0x3F*/
#define CMQ_TPSC_IND_DATA_CTL                      0x20

/* TPSC Indirect Reg Idle Code byte  0x40-0x5F*/
#define CMQ_TPSC_IND_IDLE_CODE                     0x40

/* TPSC Indirect Reg Sig Ctl 0x60-0x7F */
#define CMQ_TPSC_IND_SIG_CTL                       0x60
 
/* TPSC PCM Data Ctl - indirect register bit maps 0x20 - 0x3F*/
#define CMQ_TPSC_IND_PCM_DCTL_INV_DATA             0x80
#define CMQ_TPSC_IND_PCM_DCTL_IDLE                 0x40
#define CMQ_TPSC_IND_PCM_DCTL_DMW                  0x20
#define CMQ_TPSC_IND_PCM_DCTL_INV_SIG              0x10
#define CMQ_TPSC_IND_PCM_DCTL_TST                  0x08
#define CMQ_TPSC_IND_PCM_DCTL_LP                   0x04
#define CMQ_TPSC_IND_PCM_DCTL_ZSUP_FMT_MASK        0x03
#define CMQ_TPSC_IND_PCM_DCTL_ZSUP_FMT_NONE        0x00
#define CMQ_TPSC_IND_PCM_DCTL_ZSUP_FMT_GTE         0x01
#define CMQ_TPSC_IND_PCM_DCTL_ZSUP_FMT_DDS         0x02
#define CMQ_TPSC_IND_PCM_DCTL_ZSUP_FMT_BELL        0x03

/* TPSC Idle Code byte - indirect register bit maps 0x40 - 0x5F */
#define CMQ_TPSC_IND_PCM_DCTL_IDLE_CD_SUB_MASK     0xFF

/* TPSC Sig Ctl- indirect register bit maps 0x60 - 0x7F*/
#define CMQ_TPSC_IND_PCM_SCTL_NO_CHG               0x00
#define CMQ_TPSC_IND_PCM_SCTL_INV_ODD              0x20
#define CMQ_TPSC_IND_PCM_SCTL_INV_EVEN             0x40
#define CMQ_TPSC_IND_PCM_SCTL_INV_ALL              0x60   
#define CMQ_TPSC_IND_PCM_SCTL_SUB_IDLE             0x80   
#define CMQ_TPSC_IND_PCM_SCTL_SUB_ALAW             0xC0   
#define CMQ_TPSC_IND_PCM_SCTL_SUB_ULAW             0xE0
#define CMQ_TPSC_IND_PCM_SCTL_SRC_SIGBIT           0x10
#define CMQ_TPSC_IND_PCM_SCTL_SRC_BCKPLN_SIGBIT    0x00
#define CMQ_TPSC_IND_PCM_SCTL_SIGBIT_MASK          0xFF
   

/* RPSC Cfg - bit mask for offset 0x0070, 0x0170, 0x270, 0x0370 */
#define CMQ_RPSC_CFG_RESERVE                       0xFC
#define CMQ_RPSC_CFG_IND_ACCESS                    0x02
#define CMQ_RPSC_CFG_EN_PER_SER_CHN                0x01

/* RPSC micro access stat - 
bit mask for offset 0x0071, 0x0171, 0x271, 0x0371 */
#define CMQ_RPSC_MICRO_ACCESS_RESERVE              0x7F
#define CMQ_RPSC_MICRO_ACCESS_STAT_BUSY            0x80

/* RPSC micro access addr ctl -
bit mask for offset 0x0072, 0x0172, 0x272, 0x0372 */
#define CMQ_RPSC_CHN_IND_ADDR_CTL_RD               0x80
#define CMQ_RPSC_CHN_IND_ADDR_CTL_WR               0x00
#define CMQ_RPSC_CHN_IND_ADDR_MASK                 0x7F

/* RPSC micro Ind Data Buf -
bit mask for offset 0x0073, 0x0173, 0x273, 0x0373 */
#define CMQ_RPSC_CHN_IND_DATA_BUF_MASK             0xFF

/* Bit mask for offset 0x0074, 0x0174, 0x0274, 0x0374 */
#define DS0_DDLB_ELST_IR                           0x02
#define DS0_DDLB_ELST_OR                           0x01

/* Bit mask for offset 0x0075, 0x0175, 0x0275, 0x0375 */
#define DS0_DDLB_SLIPE_INTR_EN                     0x04

/* RPSC Framer Indirect Reg Offset */
#define CMQ_RPSC_IND_OFFSET                        0x20

/* RPSC Indirect Reg PCM Data Ctl 0x20-0x3F*/
#define CMQ_RPSC_IND_DATA_CTL                      0x20

/* RPSC Indirect Reg data trunken  0x40-0x5F*/
#define CMQ_RPSC_IND_DATA_TRNK                     0x40

/* RPSC Indirect Reg Sig trunken 0x60-0x7F */
#define CMQ_RPSC_IND_SIG_TRNK                      0x60

/* RPSC PCM Data Ctl - indirect register bit maps 0x20 - 0x3F*/
#define CMQ_RPSC_IND_PCM_RESERVE                   0x03
#define CMQ_RPSC_IND_PCM_DCTL_TEST_PAT             0x80
#define CMQ_RPSC_IND_PCM_DCTL_TRNK_DATA_COND       0x40
#define CMQ_RPSC_IND_PCM_DCTL_TRNK_SIG_COND        0x20
#define CMQ_RPSC_IND_PCM_SCTL_SUB_ALAW             0x10   
#define CMQ_RPSC_IND_PCM_SCTL_SUB_ULAW             0x08
#define CMQ_RPSC_IND_PCM_SCTL_SIG_INV              0x04

/* RPSC data Trunk Cond - indirect register bit maps 0x40 - 0x5F */
#define CMQ_RPSC_IND_PCM_DCTL_TRNK_CD_SUB_MASK     0xFF

/* RPSC Sig Trunk Cond - indirect register bit maps 0x60 - 0x7F*/
#define CMQ_RPSC_IND_PCM_SCTL_SIGBIT_MASK          0xFF

/* T1 APRM Cfg - bit mask for offset 0x0078, 0x0178, 0x278, 0x0378 */
#define CMQ_T1_APRM_CFG_CTL_INT_MASK               0x02
#define CMQ_T1_APRM_CFG_CTL_RESERVE                0xE0
#define CMQ_T1_APRM_CFG_CTL_SYNC_MASK              0x18
#define CMQ_T1_APRM_CFG_CTL_CONT_CRC               0x04
#define CMQ_T1_APRM_CFG_CTL_INT_EN                 0x02
#define CMQ_T1_APRM_CFG_CTL_EN_AUTO_UPD            0x01

/* COMET QUAD  offset 0x79, 0x179, 0x279, 0x379 undefined   */

/* APRM INT stat - bit mask for offset 0x007A, 0x017A, 0x27A, 0x037A */
#define CMQ_T1_APRM_INT_STAT_MASK                  0x01
#define CMQ_T1_APRM_STAT_RESERVE                   0xFE
#define CMQ_T1_APRM_INT_STAT_IND                   0x01

/* APRM 1 sec content octet 2 -
bit mask for offset 0x007B, 0x017B, 0x27B, 0x037B */
#define CMQ_T1_APRM_OCTET2_SAP_ID_MASK             0xFC
#define CMQ_T1_APRM_OCTET2_CARRIER_CMD_RESP        0x04
#define CMQ_T1_APRM_OCTET2_EXTEND_ADDR             0x01

/* APRM 1 sec content octet 3 -
bit mask for offset 0x007C, 0x017C, 0x27C, 0x037C */
#define CMQ_T1_APRM_OCTET3_TERM_END_PT_MASK        0xFE
#define CMQ_T1_APRM_OCTET3_EXTEND_ADDR             0x01

/* APRM 1 sec content octet 4 - 
bit mask for offset 0x007D, 0x017D, 0x27D, 0x037D */
#define CMQ_T1_APRM_OCTET4_CTL_MASK                0xFF

/* APRM 1 sec content octet 5 -
bit mask for offset 0x007E, 0x017E, 0x27E, 0x037E */
#define CMQ_T1_APRM_OCTET5_CRC_GT5_LT10            0x80
#define CMQ_T1_APRM_OCTET5_LCV_IN_SEC              0x40
#define CMQ_T1_APRM_OCTET5_CRC_GT10_LT100          0x20
#define CMQ_T1_APRM_OCTET5_SYNC_MASK               0x18
#define CMQ_T1_APRM_OCTET5_CRC_GT100_LE319         0x04
#define CMQ_T1_APRM_OCTET5_SLIP_IN_SEC             0x02
#define CMQ_T1_APRM_OCTET5_CRC_GE320               0x01

/* APRM 1 sec content octet 6 -
 bit mask for offset 0x007F, 0x017F, 0x27F, 0x037F */
#define CMQ_T1_APRM_OCTET6_FRM_BER_IN_SEC          0x80
#define CMQ_T1_APRM_OCTET6_SEV_ERR_FRM_IN_SEC      0x40
#define CMQ_T1_APRM_OCTET6_PYLD_LPBCK              0x20
#define CMQ_T1_APRM_OCTET6_CRC_EQ1                 0x10
#define CMQ_T1_APRM_OCTET6_RESERVE                 0x08
#define CMQ_T1_APRM_OCTET6_REP_CNT_MASK            0x03

/* E1 TRAN Cfg - bit mask for offset 0x0080, 0x0180, 0x280, 0x0380 */
#define CMQ_E1_TRN_CFG_AMI_LINE_CD                 0x80
#define CMQ_E1_TRN_CFG_SIG_INS_DIS                 0x00
#define CMQ_E1_TRN_CFG_SIG_CCS_EN                  0x20
#define CMQ_E1_TRN_CFG_SIG_CAS_EN                  0x60
#define CMQ_E1_TRN_CFG_SIG_EN_MASK                 0x60
#define CMQ_E1_TRN_CFG_EN_GEN_CRC_MFRM             0x10
#define CMQ_E1_TRN_CFG_FRM_DIS                     0x08
#define CMQ_E1_TRN_CFG_FEBE_DIS                    0x04
#define CMQ_E1_TRN_CFG_INS_NAT_INTER_DIS           0x02
#define CMQ_E1_TRN_CFG_INS_EXTRA_BIT_DIS           0x01

/* E1 TRAN Alarm Diag Ctl
 - bit mask for offset 0x0081, 0x0181, 0x281, 0x0381 */
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_EN_TRNK         0x80
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_FRM_PAT_INV     0x40
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_SPOILER_INS     0x20
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_MFRM_PAT_INV    0x10
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_EN_RAI          0x08
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_EN_YEL          0x04
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_EN_TSLOT16_AIS  0x02
#define CMQ_E1_TRN_TX_ALM_DIAG_CTL_EN_AIS          0x01

/* E1 TRAN International and national control
- bit mask for offset 0x0082, 0x0182, 0x282, 0x0382 */
#define CMQ_E1_TRN_INTERNAT_NAT_CTL_RESERVE        0x3F
#define CMQ_E1_TRN_INTERNAT_NAT_CTL_MASK           0xC0
#define CMQ_E1_TRN_INTERNAT_NAT_CTL_SI1_SHIFT      6
#define CMQ_E1_TRN_INTERNAT_NAT_CTL_SI0_SHIFT      6

#define CMQ_E1_TRN_INTERNAT_NAT_CTL_SI0             0x01
#define CMQ_E1_TRN_INTERNAT_NAT_CTL_SI1             0x02

/* E1 TRAN Extra Bit Ctl 
- bit mask for offset 0x0083, 0x0183, 0x0283, 0x0383 */
#define CMQ_E1_TRN_EXTRA_BIT_CTL_RESERVE           0xF4
#define CMQ_E1_TRN_EXTRA_BIT_CTL_MASK              0x0B
#define CMQ_E1_TRN_EXTRA_BIT_CT_X1_SHIFT           3
#define CMQ_E1_TRN_EXTRA_BIT_CT_X3_SHIFT           0
#define CMQ_E1_TRN_EXTRA_BIT_CT_X4_SHIFT           2

#define CMQ_E1_TRN_EXTRA_BIT_CT_X1                 0x01
#define CMQ_E1_TRN_EXTRA_BIT_CT_X3                 0x02
#define CMQ_E1_TRN_EXTRA_BIT_CT_X4                 0x04

/* E1 TRAN INT - bit mask for offset 0x0084, 0x0184, 0x284, 0x0384 */
#define CMQ_E1_TRN_INT_MASK                        0x1F
#define CMQ_E1_TRN_INT_RESERVE                     0xE0
#define CMQ_E1_TRN_INT_EN_SIG_MFRM                 0x10
#define CMQ_E1_TRN_INT_EN_NFAS                     0x08
#define CMQ_E1_TRN_INT_EN_MFRM                     0x04
#define CMQ_E1_TRN_INT_EN_SMFRM                    0x02
#define CMQ_E1_TRN_INT_EN_FRM                      0x01

/* E1 TRAN INT Stat - bit mask for offset 0x0085, 0x0185, 0x285, 0x0385 */
#define CMQ_E1_TRN_INT_STAT_MASK                   0x1F
#define CMQ_E1_TRN_INT_STAT_RESERVE                0xE0
#define CMQ_E1_TRN_INT_STAT_SIG_MFRM_IND           0x10
#define CMQ_E1_TRN_INT_STAT_NFAS_IND               0x08
#define CMQ_E1_TRN_INT_STAT_MFRM_IND               0x04
#define CMQ_E1_TRN_INT_STAT_SMFRM_IND              0x02
#define CMQ_E1_TRN_INT_STAT_FRM_IND                0x01

/* E1 TRAN national code word sel 
- bit mask for offset 0x0086, 0x0186, 0x286, 0x0386 */
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_RESERVE         0x1F
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SHIFT           5
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_MASK            0xE0
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SA4             0X03
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SA5             0X04
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SA6             0X05
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SA7             0X06
#define CMQ_E1_TRN_NAT_CD_WORD_SEL_SA8             0X07

/* E1 TRAN National Bits Codeword 
- bit mask for offset 0x0087, 0x0187, 0x287, 0x0387 */
#define CMQ_E1_TRN_NAT_CD_WORD_INS_BIT_MASK        0xF0
#define CMQ_E1_TRN_NAT_CD_WORD_BITS_MASK           0x0F
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_EN1_SHIFT       7
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_EN2_SHIFT       5
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_EN3_SHIFT       3
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_EN4_SHIFT       1
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_VAL1_SHIFT      3
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_VAL2_SHIFT      1
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_VAL3_SHIFT      1
#define CMQ_E1_TRN_NAT_CD_WORD_SAX_VAL4_SHIFT      3

#define CMQ_E1_TRN_NAT_CD_WORD_SA1                  0x01
#define CMQ_E1_TRN_NAT_CD_WORD_SA2                  0x02
#define CMQ_E1_TRN_NAT_CD_WORD_SA3                  0x04
#define CMQ_E1_TRN_NAT_CD_WORD_SA4                  0x08

/* E1 FRM Align Opts - bit mask for offset 0x0090, 0x0190, 0x290, 0x0390 */
#define CMQ_E1_FRM_OPT_CRC_EN                      0x80
#define CMQ_E1_FRM_OPT_CAS_DIS                     0x40
#define CMQ_E1_FRM_OPT_CRC_2NO_CRC                 0x20
#define CMQ_E1_FRM_OPT_RESERVE                     0x01
#define CMQ_E1_FRM_OPT_RESERVE1                    0x08
#define CMQ_E1_FRM_OPT_RESYNC                      0x04
#define CMQ_E1_FRM_OPT_CRC_THRESH_RESYNC           0x02
#define CMQ_E1_FRM_OPT_REFRM_DIS                   0x01

/* E1 FRM Maint Opt - bit mask for offset 0x0091, 0x0191, 0x291, 0x0391 */
#define CMQ_E1_FRM_MAINT_OPT_RESERVE               0x84
#define CMQ_E1_FRM_MAINT_OPT_SUP_OOF_CRITERIA_MASK 0x70
#define CMQ_E1_FRM_MAINT_OPT_NFAS_OOF_CRITERIA     0x40
#define CMQ_E1_FRM_MAINT_OPT_OOMF_CRITERIA         0x20
#define CMQ_E1_FRM_MAINT_OPT_OOMF_TSLOT16_CRITERIA 0x10
#define CMQ_E1_FRM_MAINT_OPT_RAI_CRITERIA          0x08
#define CMQ_E1_FRM_MAINT_OPT_AIS_CRITERIA          0x02
#define CMQ_E1_FRM_MAINT_OPT_EXCESS_CRC_ERR_RESYNC 0x01

/* E1 FRM INT enable - bit mask for offset 0x0092, 0x0192, 0x292, 0x0392 */
#define CMQ_E1_FRM_INT_EN_MASK                     0xFF
#define CMQ_E1_FRM_INT_EN_CRC_2_NONCRC             0x80
#define CMQ_E1_FRM_INT_EN_OOF                      0x40
#define CMQ_E1_FRM_INT_EN_OOSMF                    0x20
#define CMQ_E1_FRM_INT_EN_OOCMF                    0x10
#define CMQ_E1_FRM_INT_EN_COFA                     0x08
#define CMQ_E1_FRM_INT_EN_FER                      0x04
#define CMQ_E1_FRM_INT_EN_SMFER                    0x02
#define CMQ_E1_FRM_INT_EN_CMFER                    0x01
                                          
/* E1 FRM Maint INT enable - bit mask for offset 0x0093, 0x0193, 0x293, 0x0393 */
#define CMQ_E1_FRM_MAINT_ALRM_INT_MASK             0xEF
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_RAI           0x80
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_RMAI          0x40
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_AISD          0x20
#define CMQ_E1_FRM_MAINT_ALRM_RESERVE              0x10
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_RED           0x08
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_AIS           0x04
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_FEBE          0x02
#define CMQ_E1_FRM_MAINT_ALRM_INT_EN_CRC           0x01

/* E1 FRM INT stat bit mask for offset 0x0094, 0x0194, 0x294, 0x0394 */
#define CMQ_E1_FRM_STAT_INT_MASK                   0xFF
#define CMQ_E1_FRM_STAT_INT_CRC_2_NONCRC_IND       0x80
#define CMQ_E1_FRM_STAT_INT_OOF_IND                0x40
#define CMQ_E1_FRM_STAT_INT_OOSMF_IND              0x20
#define CMQ_E1_FRM_STAT_INT_OOCMF_IND              0x10
#define CMQ_E1_FRM_STAT_INT_COFA_IND               0x08
#define CMQ_E1_FRM_STAT_INT_FER_IND                0x04
#define CMQ_E1_FRM_STAT_INT_SMFER_IND              0x02
#define CMQ_E1_FRM_STAT_INT_CMFER_IND              0x01

/* E1 FRM maint INT stat mask 
- bit mask for offset 0x0095, 0x0195, 0x0295, 0x0395 */
#define CMQ_E1_FRM_MAINT_ALRM_INT_STAT_MASK        0xEF
#define CMQ_E1_FRM_MAINT_ALRM_INT_RAI_IND          0x80
#define CMQ_E1_FRM_MAINT_ALRM_INT_RMAI_IND         0x40
#define CMQ_E1_FRM_MAINT_ALRM_INT_AISD_IND         0x20
#define CMQ_E1_FRM_MAINT_ALRM_INT_RED_IND          0x08
#define CMQ_E1_FRM_MAINT_ALRM_INT_AIS_IND          0x04
#define CMQ_E1_FRM_MAINT_ALRM_INT_FEBE_IND         0x02
#define CMQ_E1_FRM_MAINT_ALRM_INT_CRC_IND          0x01

/* E1 FRM Status - bit mask for offset 0x0096, 0x0196, 0x296, 0x0396 */
#define CMQ_E1_FRM_STAT_CRC_2_NONCRC_STATE         0x80
#define CMQ_E1_FRM_STAT_OOF_STATE                  0x40
#define CMQ_E1_FRM_STAT_OOSMF_STATE                0x20
#define CMQ_E1_FRM_STAT_OOCMF_STATE                0x10
#define CMQ_E1_FRM_STAT_OOOF_STATE                 0x08
#define CMQ_E1_FRM_STAT_RAICCRC_STATE              0x04
#define CMQ_E1_FRM_STAT_CFEBE_STATE                0x02
#define CMQ_E1_FRM_STAT_V52LINK_STATE              0x01

/* E1 FRM maint stat - bit mask for offset 0x0097, 0x0197, 0x297, 0x0397 */
#define CMQ_E1_FRM_MAINT_ALRM_STAT_RESERVE         0x13
#define CMQ_E1_FRM_MAINT_ALRM_STAT_RAI_STATE       0x80
#define CMQ_E1_FRM_MAINT_ALRM_STAT_RMAI_STATE      0x40
#define CMQ_E1_FRM_MAINT_ALRM_STAT_AISD_STATE      0x20
#define CMQ_E1_FRM_MAINT_ALRM_STAT_RED_STATE       0x08
#define CMQ_E1_FRM_MAINT_ALRM_STAT_AIS_STATE       0x04

/* E1 FRMR Tslot international and national bits -
bit mask for offset 0x0098, 0x0198, 0x298, 0x0398 */
#define CMQ_E1_FRM_TSLOT0_INTERNAT_BIT_MASK        0xC0
#define CMQ_E1_FRM_TSLOT0_RAI_BIT_MASK             0x40
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_MASK             0x1F
#define CMQ_E1_FRM_TSLOT0_INAT_BIT_SI1             0x80
#define CMQ_E1_FRM_TSLOT0_INAT_BIT_SI0             0x40
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_SA4              0x10
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_SA5              0x08
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_SA6              0x04
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_SA7              0x02
#define CMQ_E1_FRM_TSLOT0_NAT_BIT_SA8              0x01

#define CMQ_E1_FRM_TSLOT0_INAT_BIT_SI1_SHIFT       6
#define CMQ_E1_FRM_TSLOT0_INAT_BIT_SI0_SHIFT       6
#define CMQ_E1_FRM_TSLOT_NAT_BIT_SA4_SHIFT         4
#define CMQ_E1_FRM_TSLOT_NAT_BIT_SA5_SHIFT         2
#define CMQ_E1_FRM_TSLOT_NAT_BIT_SA6_SHIFT         0
#define CMQ_E1_FRM_TSLOT_NAT_BIT_SA7_SHIFT         2
#define CMQ_E1_FRM_TSLOT_NAT_BIT_SA8_SHIFT         4

/* E1 FRMR CRC ErrCnt LSB - 
bit mask for offset 0x0099, 0x0199, 0x0299, 0x0399 */
#define CMQ_E1_FRM_CRCERR_LSB_MASK                 0xFF

/* E1 FRMR CRC ErrCnt MSB -
bit mask for offset 0x009A, 0x019A, 0x029A, 0x039A */
#define CMQ_E1_FRM_CRCERR_OVRN                     0x80
#define CMQ_E1_FRM_CRCERR_UPD_AVAIL                0x40
#define CMQ_E1_FRM_CRCERR_EXTRA_BIT_MASK           0x2C
#define CMQ_E1_FRM_CRCERR_RMAI_BIT_MASK            0x10
#define CMQ_E1_FRM_CRCERR_MSB_MASK                 0x03
#define CMQ_E1_FRM_CRCERR_X1                       0x20
#define CMQ_E1_FRM_CRCERR_X3                       0x08
#define CMQ_E1_FRM_CRCERR_X4                       0x04
#define CMQ_E1_FRM_CRCERR_Y_BIT                    0x10

#define CMQ_E1_FRM_CRCERR_X1_SHIFT                 5
#define CMQ_E1_FRM_CRCERR_X3_SHIFT                 1
#define CMQ_E1_FRM_CRCERR_X4_SHIFT                 1
#define CMQ_E1_FRM_CRCERR_Y_SHIFT                  3

/* E1 FRMR national Code word sel INT-
bit mask for offset 0x009B, 0x019B, 0x029B, 0x039B */
#define CMQ_E1_FRM_NAT_CD_WORD_INT_MASK            0x1F
#define CMQ_E1_FRM_NAT_CD_SEL_WORD_SA4             0x80
#define CMQ_E1_FRM_NAT_CD_SEL_WORD_SA5             0xA0
#define CMQ_E1_FRM_NAT_CD_SEL_WORD_SA6             0xC0
#define CMQ_E1_FRM_NAT_CD_SEL_WORD_SA7             0xE0
#define CMQ_E1_FRM_NAT_CD_SEL_WORD_SA8             0x00
#define CMQ_E1_FRMT_NAT_CD_SEL_WORD_MASK           0xE0
#define CMQ_E1_FRM_NAT_CD_WORD_INT_EN_SA4          0x10
#define CMQ_E1_FRM_NAT_CD_WORD_INT_EN_SA5          0x08
#define CMQ_E1_FRM_NAT_CD_WORD_INT_EN_SA6          0x04
#define CMQ_E1_FRM_NAT_CD_WORD_INT_EN_SA7          0x02
#define CMQ_E1_FRM_NAT_CD_WORD_INT_EN_SA8          0x01

/* E1 FRMR National Bit code word INT status 
- bit mask for offset 0x009C, 0x019C, 0x029C, 0x039C */
#define CMQ_E1_FRM_NAT_CD_WORD_INT_STAT_MASK       0x1F
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_RESERVE        0xE0
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_SA4_IND        0x10
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_SA5_IND        0x08
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_SA6_IND        0x04
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_SA7_IND        0x02
#define CMQ_E1_FRM_NAT_CD_WORD_STAT_SA8_IND        0x01

/* E1 FRMR National Bit Code word -
bit mask for offset 0x009D, 0x019D, 0x029D, 0x039D */
#define CMQ_E1_FRM_NAT_CD_WORD_RESERVE             0xF0
#define CMQ_E1_FRM_NAT_CD_WORD_MASK                0x0F
#define CMQ_E1_FRM_NAT_CD_WORD_SAX1                0x08
#define CMQ_E1_FRM_NAT_CD_WORD_SAX2                0x04
#define CMQ_E1_FRM_NAT_CD_WORD_SAX3                0x02
#define CMQ_E1_FRM_NAT_CD_WORD_SAX4                0x01

#define CMQ_E1_FRM_NAT_CD_WORD_SAX1_SHIFT          3
#define CMQ_E1_FRM_NAT_CD_WORD_SAX2_SHIFT          1
#define CMQ_E1_FRM_NAT_CD_WORD_SAX3_SHIFT          1
#define CMQ_E1_FRM_NAT_CD_WORD_SAX4_SHIFT          3

/* E1 FRMR frame pulse/alarm/V5.2 Link ID Int -
bit mask for offset 0x009E, 0x019E, 0x029E, 0x039E */
#define CMQ_E1_FRM_PLSE_ALRM_INT_MASK              0xFF
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_OOOF           0x80
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_RAI_CCRC       0x40
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_FEBE           0x20
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_V25LNKID       0x10
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_BRFP           0x08
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_ICSMFP         0x04
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_ICMFP          0x02
#define CMQ_E1_FRM_PLSE_ALRM_INT_EN_ISMFP          0x01

/* E1 FRMR Frame Pulse/Alarm Int - 
bit mask for offset 0x009F, 0x019F, 0x029F, 0x039F */
#define CMQ_E1_FRM_PLSE_ALRM_INT_STAT_MASK         0xFF
#define CMQ_E1_FRM_PLSE_ALRM_OOOF_IND              0x80
#define CMQ_E1_FRM_PLSE_ALRM_RAI_CCRC_IND          0x40
#define CMQ_E1_FRM_PLSE_ALRM_FEBE_IND              0x20
#define CMQ_E1_FRM_PLSE_ALRM_V25LNKID_IND          0x10
#define CMQ_E1_FRM_PLSE_ALRM_BRFP_IND              0x08
#define CMQ_E1_FRM_PLSE_ALRM_ICSMFP_IND            0x04
#define CMQ_E1_FRM_PLSE_ALRM_ICMFP_IND             0x02
#define CMQ_E1_FRM_PLSE_ALRM_ISMFP_IND             0x01

/* TDPR Cfg - bit mask for offset 0x00A8, 0x01A8, 0x02A8, 0x03A8 */
#define CMQ_TDPR_CFG_RESERVE                       0x10
#define CMQ_TDPR_CFG_FLG_SEQ_SHR                   0x80
#define CMQ_TDPR_CFG_FIFO_CLR                      0x40
#define CMQ_TDPR_CFG_EN_PMON_REP                   0x20
#define CMQ_TDPR_CFG_EOM                           0x08
#define CMQ_TDPR_CFG_ABRT                          0x04
#define CMQ_TDPR_CFG_FCS_CRC_CHK                   0x02
#define CMQ_TDPR_CFG_START                         0x01

/* TDPR upper Tx Threshold - 
bit mask for offset 0x00A9, 0x01A9, 0x02A9, 0x03A9 */
#define CMQ_TDPR_FIFO_UPPER_RESERVE                0x80
#define CMQ_TDPR_FIFO_UPPER_THRESH_MASK            0x7F

/* TDPR lower Tx Threshold -
bit mask for offset 0x00AA, 0x01AA, 0x02AA, 0x03AA */
#define CMQ_TDPR_FIFO_LOWER_RESERVE                0x80
#define CMQ_TDPR_FIFO_LOWER_THRESH_MASK            0x7F

/* TDPR INT enable - bit mask for offset 0x00AB, 0x01AB, 0x02AB, 0x03AB */
#define CMQ_TDPR_INT_MASK                          0x1F
#define CMQ_TDPR_INT_RESERVE                       0xE0
#define CMQ_TDPR_INT_EN_PMON_REP                   0x10
#define CMQ_TDPR_INT_EN_FIFO_FULL_LVL_THRESH       0x08
#define CMQ_TDPR_INT_EN_FIFO_OVRN_THRESH           0x04
#define CMQ_TDPR_INT_EN_FIFO_UNDRN_LVL_THRESH      0x02
#define CMQ_TDPR_INT_EN_FIFO_LOW_LVL_THRESH        0x01

/* TDPR INT stat 
- bit mask for offset 0x00AC, 0x01AC, 0x02AC, 0x03AC */
#define CMQ_TDPR_INT_STAT_RESERVE                     0x80    
#define CMQ_TDPR_INT_STAT_MASK                        0x7F
#define CMQ_TDPR_STAT_FIFO_FULL_IND                   0x40
#define CMQ_TDPR_STAT_FIFO_BELOW_LOW_LVL_THRESH_IND   0x20
#define CMQ_TDPR_STAT_PMON_REP_IND                    0x10
#define CMQ_TDPR_STAT_FULL_LVL_THRESH_IND             0x08
#define CMQ_TDPR_STAT_OVRN_THRESH_IND                 0x04
#define CMQ_TDPR_STAT_UNDRN_LVL_THRESH_IND            0x02
#define CMQ_TDPR_STAT_LOW_LVL_THRESH_IND              0x01

/* TDPR TX Data - bit mask for offset 0x00AD, 0x01AD, 0x02AD, 0x03AD */
#define CMQ_TDPR_DATA_BUF_MASK                     0xFF

/* RX-Elastic CCS Cfg - bit mask for offset 0x00B0, 0x01B0, 0x02B0, 0x03B0 */
#define CMQ_RX_ELST_CCS_CFG_RESERVE                0xFC
#define CMQ_RX_ELST_CCS_CFG_INP_E1_RATE_EN         0x02
#define CMQ_RX_ELST_CCS_CFG_OUTP_RATE_EN           0x01
#define CMQ_RX_ELST_CCS_CFG_E1                     0x03
#define CMQ_RX_ELST_CCS_CFG_T1                     0x01

/* RX-Elastic CCS Int Stat 
- bit mask for offset 0x00B1, 0x01B1, 0x02B1, 0x03B1 */
#define CMQ_RX_ELST_CSS_INT_MASK                   0x04
#define CMQ_RX_ELST_CSS_INT_STAT_MASK              0x01
#define CMQ_RX_ELST_CCS_RESERVE                    0xF8
#define CMQ_RX_ELST_CCS_INT_EN_SLIP                0x04
#define CMQ_RX_ELST_CCS_STAT_SLIP_DIR              0x02
#define CMQ_RX_ELST_CCS_STAT_SLIP_FULL             0x02
#define CMQ_RX_ELST_CCS_STAT_SLIP_IND              0x01

/* RX-ELST CCS idle Code 
- bit mask for offset 0x00B2, 0x01B2, 0x02B2, 0x03B2 */
#define CMQ_RX_ELST_IDLE_CD_SUB_MASK               0xFF

/* TX-ELST CCS Cfg - bit mask for offset 0x00B4, 0x01B4, 0x02B4, 0x03B4 */
#define CMQ_TX_ELST_CCS_CFG_RESERVE                0xFC
#define CMQ_TX_ELST_CCS_CFG_INP_E1_RATE_EN         0x02
#define CMQ_TX_ELST_CCS_CFG_OUTP_RATE_EN           0x01
#define CMQ_TX_ELST_CCS_CFG_E1                     0x03
#define CMQ_TX_ELST_CCS_CFG_T1                     0x02

/* TX-Elastic CCS Int Stat 
- bit mask for offset 0x00B1, 0x01B1, 0x02B1, 0x03B1 */
#define CMQ_TX_ELST_CSS_INT_MASK                   0x04
#define CMQ_TX_ELST_CSS_INT_STAT_MASK              0x01
#define CMQ_TX_ELST_CCS_EN_RESERVE                 0xF8
#define CMQ_TX_ELST_CCS_INT_EN_SLIP                0x04
#define CMQ_TX_ELST_CCS_STAT_SLIP_DIR              0x02
#define CMQ_TX_ELST_CCS_STAT_SLIP_FULL             0x02
#define CMQ_TX_ELST_CCS_STAT_SLIP_IND              0x01

 /* RX HMVIP/CCS Enable - bit mask for offset 0x00B8, 0x01B8, 0x02B8, 0x03B8 */
#define CMQ_RX_HMVIP_CSS_RESERVE                   0xF8
#define CMQ_RX_HMVIP_CCS_EN_TRNK_COND              0x04
#define CMQ_RX_HMVIP_CCS_EXT_EN                    0x02
#define CMQ_RX_HMVIP_EN                            0x01

 /* TX HMVIP/CCS Enable - bit mask for offset 0x00B9, 0x01B9, 0x02B9, 0x03B9 */
#define CMQ_TX_HMVIP_CSS_EN_RESERVE                0xE0
#define CMQ_TX_HMVIP_CCS_EN_TSLOT31                0x10
#define CMQ_TX_HMVIP_CCS_EN_TSLOT16                0x08
#define CMQ_TX_HMVIP_CCS_EN_TSLOT15                0x04
#define CMQ_TX_HMVIP_CCS_INS_EN                    0x02
#define CMQ_TX_HMVIP_EN                            0x01

 /* Master Test Ctl - bit mask for offset 0x00BA, 0x01BA, 0x02BA, 0x03BA */
#define CMQ_MST_TST_CTL_QUAD_MASK                  0x03
#define CMQ_MST_TST_CTL_RESERVE                    0xFC
#define CMQ_MST_TST_CTL_QUAD0                      0x00
#define CMQ_MST_TST_CTL_QUAD1                      0x01
#define CMQ_MST_TST_CTL_QUAD2                      0x02
#define CMQ_MST_TST_CTL_QUAD3                      0x03

/* RSYNC Sel - bit mask for offset 0x00BB, 0x01BB, 0x02BB, 0x03BB */
#define CMQ_RESYNC_CTL_QUAD_MASK                   0x03
#define CMQ_RESYNC_CTL_RESERVE                     0xFC
#define CMQ_RESYNC_CTL_QUAD0                       0x00
#define CMQ_RESYNC_CTL_QUAD1                       0x01
#define CMQ_RESYNC_CTL_QUAD2                       0x02
#define CMQ_RESYNC_CTL_QUAD3                       0x03

 /* QUAD master Int Src - bit mask for offset 0x00BC, 0x01BC, 0x02BC, 0x03BC */
#define CMQ_MST_QUAD_INT_SRC_MASK                  0x0F
#define CMQ_MST_QUAD_INT_SRC_RESERVE               0xF0
#define CMQ_MST_QUAD_INT_QUAD0                     0x00
#define CMQ_MST_QUAD_INT_QUAD1                     0x01
#define CMQ_MST_QUAD_INT_QUAD2                     0x02
#define CMQ_MST_QUAD_INT_QUAD3                     0x03
 
 /* QUAD term control - bit mask for offset 0x00BE, 0x01BE, 0x02BE, 0x03BE */
#define CMQ_TERM_CNTRL_RLIU_EN                     0x10
#define CMQ_TERM_CNTRL_RXTERM_EN                   0x04
#define CMQ_TERM_CNTRL_RXTERM_E1_75                0x02
#define CMQ_TERM_CNTRL_RXTERM_E1                   0x01

/* RDLC Cfg - bit mask for offset 0x00C0, 0x01C0, 0x02C0, 0x03C0 */
#define CMQ_RDLC_CFG_RESERVE                       0xF0
#define CMQ_RDLC_CFG_EN_ADDR_MATCH                 0x08
#define CMQ_RDLC_CFG_ADDR_MATCH_MASK               0x04
#define CMQ_RDLC_CFG_TERM_RECEPTION                0x02
#define CMQ_RDLC_CFG_EN                            0x01

/* RDLC INT ctl - bit mask for offset 0x00C1, 0x01C1, 0x02C1, 0x03C1 */
#define CMQ_RDLC_CTL_INT_MASK                      0x80
#define CMQ_RDLC_CTL_INT_EN                        0x80
#define CMQ_RDLC_CTL_FIFO_LVL_MASK                 0x7F

/* bit mask for offset 0x00C2, 0x01C2, 0x02C2, 0x03C2 */
#define CMQ_RDLC_STAT_INT_STAT_MASK                0x01
#define CMQ_RDLC_STAT_FIFO_EMPTY                   0x80
#define CMQ_RDLC_STAT_FIFO_OVRN                    0x40
#define CMQ_RDLC_STAT_CHG_OF_LNK_STAT              0x20
#define CMQ_RDLC_STAT_PKT_NORM                     0x10
#define CMQ_RDLC_STAT_PKT_BYTE_STATUS              0x0E
#define CMQ_RDLC_STAT_PKT_BYTE_DATA                0x00
#define CMQ_RDLC_STAT_PKT_BYTE_DLINK_ACT           0x02
#define CMQ_RDLC_STAT_PKT_BYTE_DLINK_INACT         0x04
#define CMQ_RDLC_STAT_PKT_BYTE_RESERVE             0x06
#define CMQ_RDLC_STAT_PKT_BYTE_LAST                0x08
#define CMQ_RDLC_STAT_PKT_BYTE_NON_INTEGER         0x0A
#define CMQ_RDLC_STAT_PKT_BYTE_CRC_ERR             0x0C
#define CMQ_RDLC_STAT_PKT_BYTE_CRC_ERR_NON_INTEGER 0x0E
#define CMQ_RDLC_STAT_INT_STAT_IND                 0x01

/* RDLC Data - bit mask for offset 0x00C3, 0x01C3, 0x02C3, 0x03C3 */
#define CMQ_RDLC_DATA_MASK                         0xFF

/* RDLC Primary Addr match 
 - bit mask for offset 0x00C4, 0x01C4, 0x02C4, 0x03C4 */
#define CMQ_RDLC_PRI_ADR_MASK                      0xFF

/* RDLC Secondaray Address Match 
- bit mask for offset 0x00C5, 0x01C5, 0x02C5, 0x03C5 */
#define CMQ_RDLC_SEC_ADR_MASK                      0xFF
                                       
/* CSU Cfg - bit mask for offset 0x00D6, 0x01D6, 0x02D6, 0x03D6 */
#define CMQ_CSU_CFG_RESERVE                        0x30
#define CMQ_CSU_CFG_RESET                          0x80
#define CMQ_CSU_CFG_IDDQ_TST                       0x40
#define CMQ_CSU_CFG_FREQ_LOCK                      0x08
#define CMQ_CSU_CFG_XCLK_TX_MASK                   0x07
#define CMQ_CSU_CFG_XCLK2048_TX2048                0x00
#define CMQ_CSU_CFG_XCLK1544_TX1544                0x01
#define CMQ_CSU_CFG_XCLK2048_TX1544                0x07

/* RLPS IND data - bit mask for offset 0x00D8, 0x01D8, 0x02D8, 0x02D8 */
/* RLPS IND data - bit mask for offset 0x00D9, 0x01D9, 0x02D9, 0x02D9 */
/* RLPS IND data - bit mask for offset 0x00DA, 0x01DA, 0x02DA, 0x02DA */
/* RLPS IND data - bit mask for offset 0x00DB, 0x01DB, 0x02DB, 0x02DB */
#define CMQ_RLPS_EQ_IND_DATA_MASK                  00xFF

/* RLPS Equalization Volatage Data -
bit mask for offset 0x00DC, 0x01DC, 0x02DC, 0x03DC */
#define CMQ_RLPS_EQ_VOLT_REF_RESERVE               0xC0
#define CMQ_RLPS_EQ_VOLT_REF_MASK                  0x3F

/* PRBS Generator/Checker Ctl -
bit mask for offset 0x00E0, 0x01E0, 0x02E0, 0x03E0 */
#define CMQ_PRBS_GEN_CHK_CTL_RESERVE               0xC8
#define CMQ_PRBS_GEN_CHK_CTL_QRSS_ZSUP             0x20
#define CMQ_PRBS_GEN_CHK_CTL_TX_INV                0x08
#define CMQ_PRBS_GEN_CHK_CTL_RX_INV                0x04
#define CMQ_PRBS_GEN_CHK_CTL_PAT_AUTO_SYNC         0x02
#define CMQ_PRBS_GEN_CHK_CTL_PAT_MAN_SYNC          0x01

/* PRBS Checker Int Enable/Stat - 
bit mask for offset 0x00E1, 0x01E1, 0x02E1, 0x03E1 */
#define CMQ_PRBS_CHK_INT_MASK                      0xE0
#define CMQ_PRBS_CHK_INT_STAT_MASK                 0x0E
#define CMQ_PRBS_CHK_INT_EN_SYNC                   0x80
#define CMQ_PRBS_CHK_INT_EN_BIT_ERR_DET            0x40
#define CMQ_PRBS_CHK_INT_EN_CNT_XFER               0x20
#define CMQ_PRBS_CHK_STAT_SYNC_STATE               0x10
#define CMQ_PRBS_CHK_STAT_SYNC_STATE               0x10
#define CMQ_PRBS_CHK_STAT_SYNC_IND                 0x08
#define CMQ_PRBS_CHK_STAT_BIT_ERR_IND              0x04
#define CMQ_PRBS_CHK_STAT_CNT_XFER_IND             0x02
#define CMQ_PRBS_CHK_STAT_CNT_XFER_OVRN            0x01

/* PRBS Pattern Select -
bit mask for offset 0x00E2, 0x01E2, 0x02E2, 0x03E2 */
#define CMQ_PRBS_PAT_SEL_RESERVE                   0xFC
#define CMQ_PRBS_PAT_SEL_MASK                      0x03
#define CMQ_PRBS_PAT_SEL_2TO15MINUS1               0x00
#define CMQ_PRBS_PAT_SEL_2TO20MINUS1               0x01
#define CMQ_PRBS_PAT_SEL_2TO11MINUS1               0x02

/* PRBS ErrCnt Mask - bit mask for offset 0x00E4, 0x01E4, 0x02E4, 0x03E4 */
/* PRBS ErrCnt Mask - bit mask for offset 0x00E5, 0x01E5, 0x02E5, 0x03E5 */
/* PRBS ErrCnt Mask - bit mask for offset 0x00E6, 0x01E6, 0x02E6, 0x03E6 */
#define CMQ_PRBS_ERR_CNT_MASK                      0xFF

/* XLPG Line Drv Cfg - bit mask for offset 0x00F0, 0x01F0, 0x02F0, 0x03F0 */
#define CMQ_XLPG_LINE_DRV_CFG_HIGHZ                0x80
#define CMQ_XLPG_LINE_DRV_CFG_TLIU_EN              0x40
#define CMQ_XLPG_LINE_DRV_CFG_RESERVE              0x20
#define CMQ_XLPG_LINE_DRV_CFG_WAVEFORM_SCALE_MASK  0x1F

/* XLPG Ctl/Stat - bit mask for offset 0x00F1, 0x01F1, 0x02F1, 0x03F1 */
#define CMQ_XLPG_CTL_RESERVE                       0xF8
#define CMQ_XLPG_CTL_WAVEFORM_OVRFLW_STAT          0x04
#define CMQ_XLPG_CTL_TX_SW_AIS_INS                 0x02
#define CMQ_XLPG_CTL_TX_AIS_INS_EN                 0x01

/* XLPG pulse Waveform storage Write Address -
bit mask for offset 0x00F2, 0x01F2, 0x02F2, 0x03F2 */
#define CMQ_XLPG_CTL_WAVEFORM_IND_ADDR_SAMPLE_MASK 0xF8
#define CMQ_XLPG_CTL_WAVEFORM_IND_ADDR_SAMPLE_SHIFT 3
#define CMQ_XLPG_CTL_WAVEFORM_IND_ADDR_UNIT_MASK   0x07

/* XPLG pulse Waveform Storage Data 
- bit mask for offset 0x00F3, 0x01F3, 0x02F3, 0x03F3 */
#define CMQ_XLPG_WAVEFORM_STORE_DATA_RESERVE       0x80
#define CMQ_XLPG_WAVEFORM_STORE_DATA_MASK          0x7F

/* COMET-Quad: XLPG Configuration #1  
   COMET-Quad bit mask for offset 0x00F4, 0x01F4, 
                                  0x02F4, 0x03F4 */
#define CMQ_XLPG_CFG1_RESERVE                      0x01
#define CMQ_XLPG_CFG1_MASK                         0x7E
#define CMQ_XLPG_CFG1_DIR_NEG                      0x80

/* COMET-Quad: XLPG Configuration #2
   COMET-Quad bit mask for offset 0x00F5, 0x01F5, 
                                  0x02F5, 0x03F5 */
#define CMQ_XLPG_CFG2_RESERVE                      0x01
#define CMQ_XLPG_CFG2_MASK                         0x7E
#define CMQ_XLPG_CFG2_DIR_NEG                      0x80

/* COMET-Quad: XLPG Initialization 
- bit mask for offset 0x00F6, 0x01F6, 0x02F6, 0x03F6 */
#define CMQ_XLPG_INIT_DATA_SEL_POS                 0x04
#define CMQ_XLPG_INIT_DATA_SEL_NEG                 0x02
#define CMQ_XLPG_INIT_ACTIVATE                     0x01
#define CMQ_XLPG_INIT_UNUSED                       0xF1

/* RLPS Cfg and Stat - bit mask for offset 0x00F8, 0x01F8, 0x02F8, 0x03F8 */

/* COMET and COMET-Quad: */
#define CMQ_RLPS_CFG_ALOG_LOS_IND                  0x80
#define CMQ_RLPS_CFG_ALOG_LOS_STATE                0x40
#define CMQ_RLPS_CFG_INT_EN_ALOG_LOS               0x20
#define CMQ_RLPS_CFG_EN_SQUELCH                    0x10

/* COMET-Quad only: */
#define CMQ_RLPS_CFG_RESERVE                       0x01

/* RLPS ALOS Detection/Clearance Thresholds -
bit mask for offset 0x00F9, 0x01F9, 0x02F9, 0x03F9 */
#define CMQ_RLPS_ALOS_RESERVE                      0x88
#define CMQ_RLPS_ALOS_CLRNC_THRESH_MASK            0x70
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB35            0x70
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB31            0x60
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB30            0x50
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB25            0x40
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB22            0x30
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB20            0x20
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB145           0x10
#define CMQ_RLPS_ALOS_CLRNC_THRESH_DB09            0x00
#define CMQ_RLPS_ALOS_DET_THRESH_MASK              0x07
#define CMQ_RLPS_ALOS_DET_THRESH_DB35              0x07
#define CMQ_RLPS_ALOS_DET_THRESH_DB31              0x06
#define CMQ_RLPS_ALOS_DET_THRESH_DB30              0x05
#define CMQ_RLPS_ALOS_DET_THRESH_DB25              0x04
#define CMQ_RLPS_ALOS_DET_THRESH_DB22              0x03
#define CMQ_RLPS_ALOS_DET_THRESH_DB20              0x02
#define CMQ_RLPS_ALOS_DET_THRESH_DB145             0x01
#define CMQ_RLPS_ALOS_DET_THRESH_DB09              0X00

/* RLPS ALOS Detection Period - 
bit mask for offset 0x00FA, 0x01FA, 0x02FA, 0x03FA */
#define CMQ_RLPS_ALOS_DET_PERIOD_MASK              0xFF
 
/* RLPS ALOS Clearance Period -
bit mask for offset 0x00FB, 0x01FB, 0x02FB, 0x03FB */
#define CMQ_RLPS_ALOS_CLRNC_PERIOD_MASK            0xFF

/* RLPS Equalization Ind Addr - 
bit mask for offset 0x00FC, 0x01FC, 0x02FC, 0x03FC */
#define CMQ_RLPS_EQ_IND_ADDR_MASK                  0xFF

/* RLPS Equalization RW Sel -
bit mask for offset 0x00FD, 0x01FD, 0x02FD, 0x03FD */
#define CMQ_RLPS_EQ_RESERVE                        0xEF
#define CMQ_RLPS_EQ_RD                             0x80
#define CMQ_RLPS_EQ_WR                             0x00

/* RLPS Equalizer Loop Stat and Ctl 
- bit mask for offset 0x00FE, 0x01FE, 0x02FE, 0x03FE */
#define CMQ_RLPS_EQ_LOOP_LOCATION                  0xFF

/* RLPS Equalizer Configuration
- bit mask for offset 0x00FF, 0x01FF, 0x02FF, 0x03FF */
#define CMQ_RLPS_EQ_CFG_EQ_EN                      0x08
#define CMQ_RLPS_EQ_CFG_VALU                       0x0B

/******************************************************************************
*  COMET-quad DPR event codes 
******************************************************************************/

/* ---------event1 field of the dpv----------- */

/* line side interface events */

#define CMQ_EVENT_CDRC_LCV                          0x00000001
#define CMQ_EVENT_CDRC_LOS                          0x00000002
#define CMQ_EVENT_CDRC_LINE_CODE_SIG                0x00000004
#define CMQ_EVENT_CDRC_CON_16ZERO                   0x00000008
#define CMQ_EVENT_CDRC_ALT_LOS                      0x00000010
#define CMQ_EVENT_RJAT_FIFO_UNDRUN                  0x00000020
#define CMQ_EVENT_RJAT_FIFO_OVRRUN                  0x00000040
#define CMQ_EVENT_TJAT_FIFO_UNDRUN                  0x00000080
#define CMQ_EVENT_TJAT_FIFO_OVRRUN                  0x00000100
#define CMQ_EVENT_PDVD_CON_16ZERO_VIOLT             0x00000200
#define CMQ_EVENT_PDVD_PULSE_DENSITY_VIOLT          0x00000400
#define CMQ_EVENT_XPDE_BIT_STUFF                    0x00000800
#define CMQ_EVENT_XPDE_CON_16ZERO_VIOLT             0x00001000
#define CMQ_EVENT_XPDE_PULSE_DENSITY_VIOLT          0x00002000
#define CMQ_EVENT_RLPS_ALOS                         0x00004000

/* system side interface events */

#define CMQ_EVENT_RX_ELST_SLIP_EMPTY                0x00008000
#define CMQ_EVENT_RX_ELST_SLIP_FULL                 0x00010000
#define CMQ_EVENT_TX_ELST_SLIP_EMPTY                0x00020000
#define CMQ_EVENT_TX_ELST_SLIP_FULL                 0x00040000
#define CMQ_EVENT_BTIF_DATA_PAR_ERR                 0x00080000
#define CMQ_EVENT_BTIF_SIG_PAR_ERR                  0x00100000
#define CMQ_EVENT_RX_ELST_CCS_SLIP_FULL             0x00200000
#define CMQ_EVENT_RX_ELST_CCS_SLIP_EMPTY            0x00400000
#define CMQ_EVENT_TX_ELST_CCS_SLIP_FULL             0x00800000
#define CMQ_EVENT_TX_ELST_CCS_SLIP_EMPTY            0x01000000

/* SigX event */
#define CMQ_EVENT_SIGX_COS_STATE                    0x02000000

/* PMON events */

#define CMQ_EVENT_APRM_DATA_RDY                     0x04000000
#define CMQ_EVENT_PMON_XFER_CNT_UPD                 0x08000000
#define CMQ_EVENT_PMON_XFER_CNT_OVRRUN              0x10000000

/* serial controller events */

#define CMQ_EVENT_PRBS_PAT_SYNC                     0x20000000
#define CMQ_EVENT_PRBS_BIT_ERR                      0x40000000
#define CMQ_EVENT_PRBS_XFER_UPD                     0x80000000

/*--------- event2 field of the dpv -------------*/

/* E1 framer events */
#define CMQ_EVENT_E1_FRMR_RAI_ALARM                 0x00000001
#define CMQ_EVENT_E1_FRMR_RMAI_ALARM                0x00000002
#define CMQ_EVENT_E1_FRMR_AIS_ALARM                 0x00000004
#define CMQ_EVENT_E1_FRMR_AISD_ALARM                0x00000008
#define CMQ_EVENT_E1_FRMR_FEBE_ALARM                0x00000010
#define CMQ_EVENT_E1_FRMR_CRC_ALARM                 0x00000020
#define CMQ_EVENT_E1_FRMR_OOF_ALARM                 0x00000040
#define CMQ_EVENT_E1_FRMR_RAI_CONT_CRC_ALARM        0x00000080
#define CMQ_EVENT_E1_FRMR_CONT_FEBE_ALARM           0x00000100
#define CMQ_EVENT_E1_FRMR_V52LINKID_ALARM           0x00000200
#define CMQ_EVENT_E1_FRMR_BR_FRM_PLS_ALARM          0x00000400
#define CMQ_EVENT_E1_FRMR_CRC_SUBMFRM_PLS_ALARM     0x00000800
#define CMQ_EVENT_E1_FRMR_CRC_MFRM_PLS_ALARM        0x00001000
#define CMQ_EVENT_E1_FRMR_MFRM_PLS_ALARM            0x00002000
#define CMQ_EVENT_E1_FRMR_RED_ALARM                 0x00004000
#define CMQ_EVENT_E1_FRMR_CRC2NCRC                  0x00008000
#define CMQ_EVENT_E1_FRMR_OOF                       0x00010000
#define CMQ_EVENT_E1_FRMR_OOF_SMFRM                 0x00020000
#define CMQ_EVENT_E1_FRMR_OOF_CRC_MFRM              0x00040000
#define CMQ_EVENT_E1_FRMR_COFA                      0x00080000
#define CMQ_EVENT_E1_FRMR_ERR                       0x00100000
#define CMQ_EVENT_E1_FRMR_SMFRM_ERR                 0x00200000
#define CMQ_EVENT_E1_FRMR_CRC_MFRM_ERR              0x00400000
#define CMQ_EVENT_E1_FRMR_SA4_IND                   0x00800000
#define CMQ_EVENT_E1_FRMR_SA5_IND                   0x01000000
#define CMQ_EVENT_E1_FRMR_SA6_IND                   0x02000000
#define CMQ_EVENT_E1_FRMR_SA7_IND                   0x04000000
#define CMQ_EVENT_E1_FRMR_SA8_IND                   0x08000000

/*-------- event3 field of the dpv --------*/

/* E1 Tran and T1 framer events */

#define CMQ_EVENT_E1_TRAN_SIGMFRM_BNDRY             0x00000001
#define CMQ_EVENT_E1_TRAN_NFAS_BNDRY                0x00000002
#define CMQ_EVENT_E1_TRAN_MFRM_BNDRY                0x00000004
#define CMQ_EVENT_E1_TRAN_SUBMFRM_BNDRY             0x00000008
#define CMQ_EVENT_E1_TRAN_FRM_BNDRY                 0x00000010
#define CMQ_EVENT_T1_FRMR_COFA                      0x00000020
#define CMQ_EVENT_T1_FRMR_ERR                       0x00000040
#define CMQ_EVENT_T1_FRMR_BIT_ERR                   0x00000080
#define CMQ_EVENT_T1_FRMR_SER_FRM                   0x00000100
#define CMQ_EVENT_T1_FRMR_MIMIC_FRM                 0x00000200
#define CMQ_EVENT_T1_FRMR_INFRM                     0x00000400

/* alarm and inband communications events */
#define CMQ_EVENT_IBCD_LPBCK_ACT_CODE               0x00000800
#define CMQ_EVENT_IBCD_LPBCK_DEACT_CODE             0x00001000
#define CMQ_EVENT_T1_RBOC_IDLE                      0x00002000
#define CMQ_EVENT_T1_RBOC_DETECT                    0x00004000
#define CMQ_EVENT_T1_XBOC_REPEAT                    0x00008000
#define CMQ_EVENT_RDLC_EVENT                        0x00010000
#define CMQ_EVENT_TDPR_FIFO_FILL_LOWLVL_THRESH      0x00020000
#define CMQ_EVENT_TDPR_FIFO_UNDRUN                  0x00040000
#define CMQ_EVENT_TDPR_FIFO_OVRRUN                  0x00080000
#define CMQ_EVENT_TDPR_FIFO_FULL                    0x00100000
#define CMQ_EVENT_TDPR_PMON_RPT_RDY                 0x00200000
#define CMQ_EVENT_ALMI_YELLOW_ALARM                 0x00400000    
#define CMQ_EVENT_ALMI_RED_ALARM                    0x00800000
#define CMQ_EVENT_ALMI_AIS_ALARM                    0x01000000

/******************************************************************************
*
* COMET & COMET-Quad Constants & Ranges
*
******************************************************************************/

/* COMET & COMET-Quad timeslot ranges */
#define CMQ_MAX_TSLOT                           0x20
#define CMQ_MAX_COSS_TSLOTS                     0x1F

/* COMET & COMET-Quad transmit pulse waveform sizes */
#define CMQ_XLPG_MAX_UNITS                      0x05
#define CMQ_XLPG_MAX_SAMPLES                    0x18

/* COMET & COMET-Quad RLPS equalizer RAM size */
#define CMQ_RLPS_EQUALIZER_RAM_SIZE             256

/* COMET & COMET-Quad RLPS voltage reference 1 values */
#define CMQ_RLPS_VREF_VALU                      0x26 

/* COMET & COMET-Quad RLPS voltage reference 2 values */
#define CMQ_RLPS_VREF_E1                        0x03 
#define CMQ_RLPS_VREF_T1                        0xc3

/* Indirect register wait time (in loop iterations) */
#define CMQ_INDIRECT_ACCESS_WAIT                100

/* HDLC tx and rx register access delay (in loop iterations) */
#define CMQ_HDLC_ACCESS_WAIT                    20

/* Register tests constants */
#define CMQ_REG_TEST_NUM_VALUES                 0x04
#define CMQ_REG_TEST_VAL1                       0x55
#define CMQ_REG_TEST_VAL2                       0xAA
#define CMQ_REG_TEST_VAL3                       0x5A
#define CMQ_REG_TEST_VAL4                       0xA5

#define INVALID                                 0
#define BUSY_WAIT_MAX                           2  /* in ms */
#define BUSY_WAIT_TIME                          1  /* in ms */
#define PM4359_SIGX_NUM_IND_REG                 0x50
#define PM4359_NUM_IND_REG                      0x60

/* 
 * PREP 6692: minimum RJAT and TJAT output clock divisor value
 */ 
#define CMQ_MIN_JAT_OUTPUT_DIV                  0x0F

/* Indirect RLPS register wait time:
   Note:  we must delay more than 3 clock line 
          rate (1.544MHz) cycles  */
#define CMQ_RLPS_INDIRECT_WAIT                  100

/* COMETQ - offset of each T1/E1 framer */
#define CMQ_FRM_OFFSET                          0x100

/* COMETQ - offset of each HDLC */
#define CMQ_HDLC_OFFSET                         CMQ_FRM_OFFSET

#ifdef PMC4358  /* 8 ports Comet */

/* COMETQ - number of framers       */
#define CMQ_MAX_FRAMERS                         0x08
/* COMETQ - number of HDLCs            */
#define CMQ_MAX_HDLCS                           0x08

#else   /* 4 ports Comet */

/* COMETQ - number of framers       */
#define CMQ_MAX_FRAMERS                         0x04
/* COMETQ - number of HDLCs            */
#define CMQ_MAX_HDLCS                           0x04

#endif

enum
{
   CMQ_TJAT_OUTPUT_CLK_INTERN_JAT,
   CMQ_TJAT_OUTPUT_CLK_CTCLK,
   CMQ_TJAT_OUTPUT_CLK_FIFO_INPUT
};

enum
{
   CMQ_TJAT_PLL_REF_CLK_FIFO_INPUT,
   CMQ_TJAT_PLL_REF_CLK_BACKPLANE,
   CMQ_TJAT_PLL_REF_CLK_RECOVERED,
   CMQ_TJAT_PLL_REF_CLK_CTCLK
};

enum
{
   CMQ_CHAN_RATE_NX56K,
   CMQ_CHAN_RATE_NX64K
};

enum
{
   CMQ_RECOVER_CLK_LOW_FREQ_JAT,
   CMQ_RECOVER_CLK_HIGH_FREQ_JAT
};

enum
{
    CMQ_LOS_THRESH_PCM_10_HDB3,
    CMQ_LOS_THRESH_PCM_15_B8ZS,
    CMQ_LOS_THRESH_PCM_15_AMI,
    CMQ_LOS_THRESH_PCM_31,
    CMQ_LOS_THRESH_PCM_63,
    CMQ_LOS_THRESH_PCM_175
};

enum
{ 
    CMQ_XCLK_2048_TXCLK_2048,
    CMQ_XCLK_1544_TXCLK_1544,
    CMQ_XCLK_2048_TXCLK_1544
};

enum
{
  CMQ_RX_ALOS_9DB_THRESH,
  CMQ_RX_ALOS_14_5DB_THRESH,
  CMQ_RX_ALOS_20DB_THRESH,
  CMQ_RX_ALOS_22DB_THRESH,
  CMQ_RX_ALOS_25DB_THRESH,
  CMQ_RX_ALOS_30DB_THRESH,
  CMQ_RX_ALOS_31DB_THRESH,
  CMQ_RX_ALOS_35DB_THRESH
};

enum
{
   CMQ_RX_EQ_FREQ_T1_24_125KHZ,  /* 24.125 kHz */
   CMQ_RX_EQ_FREQ_T1_12_063KHZ,  /* 12.063 kHz */
   CMQ_RX_EQ_FREQ_T1_8_0417KHZ,  /* 8.0417 kHz */
   CMQ_RX_EQ_FREQ_T1_6_0313KHZ,  /* 6.01313 kHz */
   CMQ_RX_EQ_FREQ_T1_4_8250KHZ,  /* 4.8250 kHz */
   CMQ_RX_EQ_FREQ_T1_4_0208KHZ,  /* 4.0208 kHz */
   CMQ_RX_EQ_FREQ_T1_3_4464KHZ,  /* 3.4464 kHz */
   CMQ_RX_EQ_FREQ_T1_3_0156KHZ,  /* 3.0156 kHz */
   CMQ_RX_EQ_FREQ_E1_32_000KHZ,  /* 32.000 kHz */
   CMQ_RX_EQ_FREQ_E1_16_000KHZ,  /* 16.000 kHz */
   CMQ_RX_EQ_FREQ_E1_10_667KHZ,  /* 10.667 kHz */
   CMQ_RX_EQ_FREQ_E1_8_000KHZ,   /* 8.000 kHz */
   CMQ_RX_EQ_FREQ_E1_6_40KHZ,    /* 6.40 kHz */
   CMQ_RX_EQ_FREQ_E1_5_333KHZ,   /* 5.333 kHz */
   CMQ_RX_EQ_FREQ_E1_4_5714KHZ,  /* 4.5714 kHz */
   CMQ_RX_EQ_FREQ_E1_4_0KHZ      /* 4.0 kHz */
};

enum
{
   CMQ_RX_EQ_VALID_PERIOD_32,
   CMQ_RX_EQ_VALID_PERIOD_64,
   CMQ_RX_EQ_VALID_PERIOD_128,
   CMQ_RX_EQ_VALID_PERIOD_256
};

enum
{ 
   CMQ_TX_FUSE_DATA_USER_DEFINED,
   CMQ_TX_FUSE_DATA_LIU_FUSE
};

enum
{
   CMQ_BACKPLANE_TX_CLOCK_MASTER_FULL_T1E1,
   CMQ_BACKPLANE_TX_CLOCK_MASTER_Nx64,
   CMQ_BACKPLANE_TX_CLOCK_MASTER_CLEAR_CHAN,
   CMQ_BACKPLANE_TX_CLOCK_SLAVE_FULL_T1E1,
   CMQ_BACKPLANE_TX_CLOCK_SLAVE_CLEAR_CHAN,
   CMQ_BACKPLANE_TX_CLOCK_SLAVE_HMVIP,              /* COMET-Quad only */
   CMQ_BACKPLANE_TX_CLOCK_SLAVE_FULL_T1E1_HMVIP_CCS /* COMET-Quad only */
};

enum
{
   CMQ_BACKPLANE_RX_CLOCK_MASTER_FULL_T1E1,
   CMQ_BACKPLANE_RX_CLOCK_MASTER_Nx64,
   CMQ_BACKPLANE_RX_CLOCK_MASTER_CLEAR_CHAN,
   CMQ_BACKPLANE_RX_CLOCK_SLAVE_FULL_T1E1,
   CMQ_BACKPLANE_RX_CLOCK_SLAVE_HMVIP,              /* COMET-Quad only */
   CMQ_BACKPLANE_RX_CLOCK_SLAVE_FULL_T1E1_HMVIP_CCS /* COMET-Quad only */
};

enum
{
     CMQ_BACKPLANE_FULL_FRAME_MODE,
   CMQ_BACKPLANE_NX56K_MODE,
   CMQ_BACKPLANE_NX64K_MODE,
   CMQ_BACKPLANE_NX64K_E1_MODE
};

enum
{
   CMQ_BACKPLANE_CLK_RATE_1544,
   CMQ_BACKPLANE_CLK_RATE_2048,
   CMQ_BACKPLANE_CLK_RATE_8192
};

enum
{
   CMQ_BACKPLANE_CLK_MASTER,
   CMQ_BACKPLANE_CLK_SLAVE
};

enum
{
   CMQ_BACKPLANE_TIMESLOT_MAP_3_OF_4,
   CMQ_BACKPLANE_TIMESLOT_MAP_24_OF_32
};

enum
{
   CMQ_BACKPLANE_RX_FP_T1_HIGH_ON_SF_ESF,
   CMQ_BACKPLANE_RX_FP_T1E1_HIGH_EVERY_FRAME,
   CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_CRC_MFRM,
   CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_SIG_MFRM,
   CMQ_BACKPLANE_RX_FP_E1_COMP_MFRM,   
   CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_OVERHEAD
};

enum
{
   CMQ_BACKPLANE_HMVIP_MODE,
   CMQ_BACKPLANE_HMVIP_CCS,
   CMQ_BACKPLANE_HMVIP_DISABLE
};

enum
{
   CMQ_TX_LBO_T1_LONG_HAUL_0DB = 0,
   CMQ_TX_LBO_T1_LONG_HAUL_7_5DB,
   CMQ_TX_LBO_T1_LONG_HAUL_15DB,
   CMQ_TX_LBO_T1_LONG_HAUL_22_5DB,
   CMQ_TX_LBO_T1_SHORT_HAUL_110FT,
   CMQ_TX_LBO_T1_SHORT_HAUL_220FT,          /* 5 */
   CMQ_TX_LBO_T1_SHORT_HAUL_330FT,
   CMQ_TX_LBO_T1_SHORT_HAUL_440FT,
   CMQ_TX_LBO_T1_SHORT_HAUL_550FT,
   CMQ_TX_LBO_T1_SHORT_HAUL_660FT,
   CMQ_TX_LBO_E1_75OHM,            /* 10 */
   CMQ_TX_LBO_E1_120OHM,
   CMQ_TX_LBO_RETAIN_CURRENT     /* indicates user does not want 
                                    to update the table */
};

enum
{
   CMQ_RX_LINE_EQ_RAM_T1 = 0,
   CMQ_RX_LINE_EQ_RAM_E1,
   CMQ_RX_LINE_EQ_RETAIN_CURRENT /* indicates user does not want
                                   to update the table */
};

enum
{    
   CMQ_BRIF_E1_FP_OUTP,
   CMQ_BRIF_E1_CRC_MFRM_OUTP,
   CMQ_BRIF_E1_SIG_MFRM_OUTP,
   CMQ_BRIF_E1_COMP_MFRM_OUTP,
   CMQ_BRIF_E1_OVHD_OUTP,
   CMQ_BRIF_T1_PULSE_ON_SF_ESF,
   CMQ_BRIF_T1_PULSE_EVERY_FRAME
};

enum
{
   CMQ_BRIF_BIT_FIX_POL_ZERO,
   CMQ_BRIF_BIT_FIX_POL_ONE,
   CMQ_BRIF_BIT_FIX_POL_DISABLE,     
   CMQ_BRIF_BIT_FIX_POL_MAX     
};

enum 
{
    CMQ_FRM_MODE_E1,
    CMQ_FRM_MODE_E1_CRC_MFRM,
    CMQ_FRM_MODE_E1_UNFRAMED,
    CMQ_FRM_MODE_T1_SF,
    CMQ_FRM_MODE_T1_DM,
    CMQ_FRM_MODE_T1_SLC96,
    CMQ_FRM_MODE_T1_DM_FDL,
    CMQ_FRM_MODE_T1_ESF,
    CMQ_FRM_MODE_T1_SF_JPN_ALARM,
    CMQ_FRM_MODE_T1_DM_JPN_ALARM,
    CMQ_FRM_MODE_T1_SLC96_JPN_ALARM,
    CMQ_FRM_MODE_T1_DM_FDL_JPN_ALARM,
    CMQ_FRM_MODE_T1_JT_G704,
    CMQ_FRM_MODE_T1_UNFRAMED
};

enum
{
    CMQ_E1_SIG_INS_NONE,
    CMQ_E1_SIG_INS_HDLC_CCS,
    CMQ_E1_SIG_INS_CAS
};

enum
{
   CMQ_T1_ZSUP_NONE,
   CMQ_T1_ZSUP_GTE,
   CMQ_T1_ZSUP_DDS,
   CMQ_T1_ZSUP_BELL
};

enum
{
   CMQ_T1_OOF_2OF4,
   CMQ_T1_OOF_2OF5,
   CMQ_T1_OOF_2OF6
};

enum
{
  CMQ_LINE_CODE_AMI,
  CMQ_LINE_CODE_HDB3_E1,
  CMQ_LINE_CODE_B8ZS_T1
};

enum
{
   CMQ_MODE_T1 = 0,
   CMQ_MODE_E1,
};

enum
{
   CMQ_T1_ESF_FRAME_ALGO_ONE_CANDIDATE,
   CMQ_T1_ESF_FRAME_ALGO_CRC_6
};

enum
{
   CMQ_E1_OOF_NFAS,
   CMQ_E1_OOF_FAS
};

enum
{
    CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_NONE,
    CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_ZERO_1_MFRM, 
    CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_ZERO_2_MFRM
};
        
enum
{
   CMQ_E1_AIS_CRIT_3Z_IN_512BITS,
   CMQ_E1_AIS_CRIT_2_PERIODS_3Z_IN_512BITS
};

enum
{
   CMQ_E1_RAI_CRIT_ALL_A_1,
   CMQ_E1_RAI_CRIT_4_CONSEC_A_1
};

enum
{
   CMQ_ALARM_INS_RX_AIS,
   CMQ_ALARM_INS_TX_YELLOW,
   CMQ_ALARM_INS_TX_AIS,
   CMQ_ALARM_INS_TX_E1_Y_BIT,
   CMQ_ALARM_INS_TX_E1_CHAN16
};

enum
{
   CMQ_E1_NAT_BIT_SA4,
   CMQ_E1_NAT_BIT_SA5,
   CMQ_E1_NAT_BIT_SA6,
   CMQ_E1_NAT_BIT_SA7,
   CMQ_E1_NAT_BIT_SA8
};

enum
{
   CMQ_TDPR_ACTION_ABORT,
   CMQ_TDPR_ACTION_END_ABORT,
   CMQ_TDPR_ACTION_EOM,
   CMQ_TDPR_ACTION_FIFOCLR
};

enum
{
   CMQ_RX_BOC_VALID_4OF5, 
   CMQ_RX_BOC_VALID_8OF10
};

/* Diagnostic LOOP Identifiers */

enum
{ 
   CMQ_LOOPBACK_NONE,
   CMQ_LOOPBACK_DIGITAL,
   CMQ_LOOPBACK_LINE,
   CMQ_LOOPBACK_PAYLOAD
};

/* Auto Performance Monitoring actions    */
enum
{
   CMQ_AUTO_PMON_UPDATE_DISABLE, 
   CMQ_AUTO_PMON_UPDATE_ENABLE,
   CMQ_AUTO_PMON_UPDATE_MAN 
};

enum
{
   CMQ_PRGD_PAT_8_BIT,
   CMQ_PRGD_PAT_7_BIT
};

enum
{
    CMQ_PSEUDO_RANDOM_PAT_2_TO_3RD_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_4th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_5th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_6th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_7th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_FRAC_T1_ACTIVATE,  
    CMQ_PSEUDO_RANDOM_PAT_FRAC_T1_DEACTIVATE, 
    CMQ_PSEUDO_RANDOM_PAT_2_TO_9th_MINUS1_O_153,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_10th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_11th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_11th_MINUS1_O_152,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_15th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_15th_MINUS1_O_151,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_17th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_18th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_20th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_20th_MINUS1_O_153,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_20th_MINUS1_O_151,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_21th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_22th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_23th_MINUS1_O_151,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_25th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_28th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_29th_MINUS1,  
    CMQ_PSEUDO_RANDOM_PAT_2_TO_31ST_MINUS1,
    CMQ_PSEUDO_RANDOM_PAT_ALL_ONES,
    CMQ_PSEUDO_RANDOM_PAT_ALL_ZEROS,
    CMQ_PSEUDO_RANDOM_PAT_ALT_ONES_AND_ZEROS,
    CMQ_PSEUDO_RANDOM_PAT_DOUBLE_ALT_ONES_AND_ZEROS,
    CMQ_PSEUDO_RANDOM_PAT_3_IN_24,
    CMQ_PSEUDO_RANDOM_PAT_1_IN_16,
    CMQ_PSEUDO_RANDOM_PAT_1_IN_8,
    CMQ_PSEUDO_RANDOM_PAT_1_IN_4,
    CMQ_PSEUDO_RANDOM_PAT_INBAND_LOOPBACK_ACTIVATE,  
    CMQ_PSEUDO_RANDOM_PAT_INBAND_LOOPBACK_DEACTIVATE
};

enum
{
    CMQ_ERROR_RATE_OFF,
    CMQ_ERROR_RATE_SINGLE,
    CMQ_ERROR_RATE_10_TO_MINUS1,    
    CMQ_ERROR_RATE_10_TO_MINUS2,    
    CMQ_ERROR_RATE_10_TO_MINUS3,    
    CMQ_ERROR_RATE_10_TO_MINUS4,    
    CMQ_ERROR_RATE_10_TO_MINUS5,    
    CMQ_ERROR_RATE_10_TO_MINUS6,    
    CMQ_ERROR_RATE_10_TO_MINUS7
};

typedef struct cmq_framer_conf_t_ {
    /* T1 Tx Params */
    uchar    quad_num;
    uchar    op_mode;
    uchar    frm_mode;           /* reg 0x54 */
    uchar    zcode_suppr_format; /* reg 0x54 */
    uchar    sf_align_en;        /* reg 0x05 */
    uchar    out_of_frame;       /* reg 0x48 */
    uchar    frm_esf_algo;       /* reg 0x48 */
    uchar    ccofa_en;           /* reg 0x48 */
    uchar    ts16Signaling;      /* reg 0x80 */
    uchar    nat_bit_en;         /* reg 0x80 */
    uchar    xtra_bit_en;        /* reg 0x80 */
    uchar    febee_en;           /* reg 0x80 */
    uchar    cas_align_en;       /* reg 0x90 */
    uchar    crc2ncrc_en;        /* reg 0x90 */
    uchar    no_refrm_err_en;    /* reg 0x90 */
    uchar    refrm_xs_crc_en;    /* reg 0x90 */
    uchar    los_bit_2crit_en;   /* reg 0x91 */
    uchar    mfrm_los_alig_crit; /* reg 0x91 */
    uchar    ais_criteria;       /* reg 0x91 */
    uchar    rai_criteria;       /* reg 0x91 */
    uchar    mult_faseo_en;      /* reg 0x02 */
    uchar    nfas_err_en;        /* reg 0x02 */
    uchar    rjat_byp_en;        /* reg 0x02 */
    uchar    rx_elst_byp;        /* reg 0x02 */
    uchar    tjat_byp_en;        /* reg 0x04 */
    uchar    tjat_ref_div;       /* reg ox19 */
    uchar    tjat_output_div;    /* reg 0x1A */
    uchar    tjat_limit_ov_under;/* reg 0x1B */
    uchar    tjat_fifo_cent;     /* reg 0x1B */
    uchar    rjat_ref_div;       /* reg ox15 */
    uchar    rjat_output_div;    /* reg 0x16 */
    uchar    rjat_limit_ov_under;/* reg 0x17 */
    uchar    rjat_fifo_cent;     /* reg 0x17 */
    uchar    out_clock;          /* reg 0x06 */
    uchar    pll_ref_clock;      /* reg 0x06 */
    uchar    tx_elst_byp;        /* reg 0x06 */
    uchar    recover_clk_sel;    /* reg 0x10 */
    uchar    los_thres;          /* reg 0x10 */
    uchar    brif_mas_mode;      /* reg 0x30 */
    uchar    brif_data_mode;     /* reg 0x30 */
    uchar    brif_clkx2;         /* reg 0x30 */
    uchar    brif_data_rate;     /* reg 0x30 */
    uchar    brif_de_hi;         /* reg 0x30 */
    uchar    brif_fe_hi;         /* reg 0x30 */
    uchar    synth_tx_freq;      /* reg 0xD6 */
    uchar    btif_mas_mode;      /* reg 0x40 */
    uchar    btif_data_mode;     /* reg 0x40 */
    uchar    btif_clkx2;         /* reg 0x40 */
    uchar    btif_data_rate;     /* reg 0x40 */
    uchar    btif_de_hi;         /* reg 0x40 */
    uchar    btif_fe_hi;         /* reg 0x40 */
    uchar    r_fp_mas_mode;      /* reg 0x31 */
    uchar    r_fp_mode;          /* reg 0x31 */
    uchar    r_fp_inv_en;        /* reg 0x31 */
    uchar    r_alt_fdl_en;       /* reg 0x31 */
    uchar    r_tslot_map_format; /* reg 0x31 */
    uchar    r_par_ins_en;       /* reg 0x32 */
    uchar    r_odd_par;          /* reg 0x32 */
    uchar    r_ext_par_en;       /* reg 0x32 */
    uchar    r_fbit_fix;         /* reg 0x32 */
    uchar    r_fbit_pol;         /* reg 0x32 */
    uchar    r_fp_frm_offset;    /* reg 0x33 */
    uchar    r_fp_bit_offset_en; /* reg 0x34 */
    uchar    r_fpBitOffset;      /* reg 0x34 */
    uchar    t_fp_mas_mode;      /* reg 0x41 */
    uchar    t_fp_inv_en;        /* reg 0x41 */
    uchar    t_t1_esf_align;     /* reg 0x41 */
    uchar    t_tslot_map_format; /* reg 0x41 */
    uchar    t_odd_par;          /* reg 0x42 */
    uchar    t_ext_par_en;       /* reg 0x42 */
    uchar    t_fp_frm_offset;    /* reg 0x43 */
    uchar    t_fp_bitoffset_en;  /* reg 0x44 */
    uchar    t_fp_bitoffset;     /* reg 0x44 */
    uchar    squelch_en;         /* reg 0xF8 */
    uchar    alos_thres;         /* reg 0xF9 */
    uchar    alos_det_period;    /* reg 0xFA */
    uchar    alos_clr_period;    /* reg 0xFB */
    uchar    xlpg_highz_en;      /* reg 0xF0 */
    uchar    xlpg_line_drv_val;  /* reg 0xF0 */
} cmq_framer_conf_t;

typedef struct dev_4359_callout_fvt_ {
    uchar (*rd_frm_reg)(ulong, ulong, uchar);
    void  (*wr_frm_reg)(ulong, ulong, uchar, uchar);
} dev_4359_callout_fvt_t;

typedef struct dev_4359_callin_fvt_ {
    int   (*register_test)(dev_object_t *, int);
    void  (*dev_reset)(dev_object_t *);
    void  (*set_cfg_info)(dev_object_t *, uchar, uchar);
    void  (*set_loopback)(dev_object_t *, uchar, uchar);
    int   (*rd_ind_reg)(dev_object_t *, ulong, int, uchar *);
    int   (*wr_ind_reg)(dev_object_t *, ulong, int, uchar);
    int   (*init_xpsc)(dev_object_t *, uchar, ushort);
    int   (*dump_xpsc)(dev_object_t *, ulong, ushort);
    int   (*read_rlps)(dev_object_t *, ulong);
    void  (*ycable_enab)(dev_object_t *, uchar, ulong);
} dev_4359_callin_fvt_t;

/*
 * Define the PMC 4359 device object structure.
 */
typedef struct dev_4359_object_t_ {
    dev_object_t           base;
    dev_4359_callout_fvt_t *callout_fvt;
    dev_4359_callin_fvt_t  *callin_fvt;
    cmq_framer_conf_t *cfg_info_p;  /* Framer configuration info */
    uchar bus_width;
}dev_4359_object_t;

extern void pmc4359_dev_create (dev_object_t *dev, 
				dev_error_report_t error_report_fn);
extern void msleep(int msecs);
#endif

/******** History ******** 
$Log: dev_4359.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
