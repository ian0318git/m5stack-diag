/* $Id: ag_mg_regs_car.h,v 1.1 2012/04/18 18:08:25 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_car.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_car.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_car.h
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * This file defines Clock And Reset (CAR) registers.
 * The CAR module consists of a clock generator, reset generator, a soft repair
 * logic block, a speedometer control circuit, and an APB slave.
 *
 * These registers are accessible from the DSP Subsystem(s) (DSS) and from
 * the Packet Processor Block (PPB). Register addresses defined in this
 * file are for code running on the ARM accessing the CAR.
 *
 * See "Physical register addresses" below for macos
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *  PLL1CTL
 *  PLL1SEL
 *  PLL2CTL
 *  CLKCTL
 *  CLKOUT
 *  DSSEN
 *  CHIP_WAKE
 *  TDM0IO
 *  TDM1IO
 *  TDM2IO
 *  TDM3IO
 *  TDM4IO
 *  TDM5IO
 *  SSSMII0IO
 *  SSSMII1IO
 *  SSPIO
 *  GPIOIO
 *  CLOCKSIO
 *  DDR3IO
 *  FLASHIO
 *  SERDESIO
 *  DDR3_SWAP
 *  DDR3_KILL_FORCE
 *  RSTPROT
 *  RESSTAT
 *  RSTCTL
 *  CHIPID
 *  DEVICEID
 *  HOST_RESET
 *  ARMRSTCTL
 *  CHIP_RESET
 *  PFUSE0
 *  PFUSE1
 *  PFUSE2
 *  PFUSE3
 *  PFUSE4
 *  PFUSE5
 *  PFUSE6
 *  PFUSE7
 *  PFUSE8
 *  PFUSE9
 *  PFUSE10
 *  PFUSE11
 *  PFUSE12
 *  PFUSE13
 *  PFUSE14
 *  PFUSE15
 *  PFUSE16
 *  PFUSE17
 *  PFUSE18
 *  PFUSE19
 *  PFUSE20
 *  PFUSE21
 *  PFUSE22
 *  PFUSE23
 *  PFUSE24
 *  PFUSE25
 *  PFUSE26
 *  PFUSE27
 *  PFUSE28
 *  PFUSE29
 *  PFUSE30
 *  PFUSE31
 *  PFUSE32
 *  PFUSE33
 *  PFUSE34
 *  PFUSE35
 *  PFUSE36
 *  PFUSE37
 *  PFUSE38
 *  PFUSE39
 *  PFUSE40
 *  PFUSE41
 *  PFUSE42
 *  PFUSE43
 *  PFUSE44
 *  PFUSE45
 *  PFUSE46
 *  PFUSE47
 *  PFUSE48
 *  PFUSE49
 *  PFUSE50
 *  PFUSE51
 *  PFUSE52
 *  PFUSE53
 *  PFUSE54
 *  PFUSE55
 *  PFUSE56
 *  PFUSE57
 *  PFUSE58
 *  PFUSE59
 *  PFUSE60
 *  PFUSE61
 *  PFUSE62
 *  PFUSE63
 *  PFUSE64
 *  PFUSE65
 *  PFUSE66
 *  PFUSE67
 *  PFUSE68
 *  PFUSE69
 *  PFUSE70
 *  PFUSE71
 *  PFUSE72
 *  PFUSE73
 *  PFUSE74
 *  PFUSE75
 *  PFUSE76
 *  PFUSE77
 *  PFUSE78
 *  PFUSE79
 *  PFUSE80
 *  PFUSE81
 *  PFUSE82
 *  PFUSE83
 *  PFUSE84
 *  PFUSE85
 *  PFUSE86
 *  PFUSE87
 *  PFUSE88
 *  PFUSE89
 *  PFUSE90
 *  PFUSE91
 *  PFUSE92
 *  PFUSE93
 *  PFUSE94
 *  PFUSE95
 *  PFUSE96
 *  PFUSE97
 *  PFUSE98
 *  PFUSE99
 *  PFUSE100
 *  PFUSE101
 *  PFUSE102
 *  PFUSE103
 *  PFUSE104
 *  PFUSE105
 *  PFUSE106
 *  PFUSE107
 *  PFUSE108
 *  PFUSE109
 *  PFUSE110
 *  PFUSE111
 *  PFUSE112
 *  PFUSE113
 *  PFUSE114
 *  PFUSE115
 *  PFUSE116
 *  PFUSE117
 *  PFUSE118
 *  PFUSE119
 *  PFUSE110
 *  PFUSE121
 *  PFUSE122
 *  PFUSE123
 *  PFUSE124
 *  PFUSE125
 *  PFUSE126
 *  PFUSE127
 *  SPEEDSTART
 *  SPEEDDUR
 *  SPEEDDATA0H
 *  SPEEDDATA0S
 *  SPEEDDATA0L
 *  SPEEDDATA1H
 *  SPEEDDATA1S
 *  SPEEDDATA1L
 *  SPEEDDATA2H
 *  SPEEDDATA2S
 *  SPEEDDATA2L
 *  SPEEDDATA3H
 *  SPEEDDATA3S
 *  SPEEDDATA3L
 *  SPEEDDATA4H
 *  SPEEDDATA4S
 *  SPEEDDATA4L
 *  SPEEDDATA5H
 *  SPEEDDATA5S
 *  SPEEDDATA5L
 *  SPEEDDATA6H
 *  SPEEDDATA6S
 *  SPEEDDATA6L
 *  SPEEDDATA7H
 *  SPEEDDATA7S
 *  SPEEDDATA7L
 *  SPEEDDATA8H
 *  SPEEDDATA8S
 *  SPEEDDATA8L
 *  TEMPDATA0
 *  TEMPDATA1
 *  TEMPDATA2
 *  TEMPDATA3
 *  TEMPDATA4
 *  TEMPDATA5
 *  TEMPDATA6
 *  TEMPDATA7
 *  TEMPDATA8
 *  VOLTDATA0
 *  VOLTDATA1
 *  VOLTDATA2
 *  VOLTDATA3
 *  VOLTDATA4
 *  VOLTDATA5
 *  VOLTDATA6
 *  VOLTDATA7
 *  VOLTDATA8
 *  RTC0
 *  RTC1
 *  RTCCTL
 *  TURBOMODE
 *  TURBOSELECT
 *  FBCNT_END
 *  TURBOFORCE
 *  PCIE_RC_EP_N
 *  SRIO_LANE_SEL
 *  PCIE_TX_FIFOEN
 *  SRIO0_MODE
 *  SRIO_PEFCAR
 *  SRIO1_MODE
 *  SRIO_AI
 *  SRIO_AVI
 *  SRIO_AR
 *  SRIO0_PORT_NUM
 *  SRIO1_PORT_NUM
 *  MATRIX_SECURE
 *  GIGE_SERDES_UPDATE
 *  SRIO_SERDES_UPDATE
 *  GIGE_TX_FIFO_SEL
 *  SRIO_TX_FIFO_SEL
 *  SERDES_TRIMDONE
 *  UHD_DELAY
 *  DSS_L2CACHE_EN
 *  PEI_INT_GEN
 *  PEI_INT_MASK
 *  PEI_INT_CLEAR
 *  GIGE_SERDES_PD
 *  GIGE_RX_LOS_SEL
 *  SRIO_RX_LOS_SEL
 *  DSS_RPR
 *  ARM_RPR
 *  IROM_RPR
 *  SYS_RPR
 *  PPB_RPR
 *  RPR_RST
 *  DEBUG_TRIG_CTRL
 *  DDR3_STAT
 *  DDR3_REFRESH
 *  FEED_FORWARD_STAT
 *  FEED_FORWARD_KEY
 *  MBIST_DSS_STAT0
 *  MBIST_DSS_STAT1
 *  MBIST_ARM_STAT
 *  MBIST_SYS_STAT0
 *  MBIST_SYS_STAT1
 *  MBIST_PPB_STAT
 *
 * Prefix naming conventions :
 *   AG_MG_REGS_XXX_RA : register/memory physical address
 *   AG_MG_REGS_XXX_RO : register/memory address offset
 *   AG_MG_REGS_XXX_RM : register mask
 *   AG_MG_REGS_XXX_BO : bit/field offset from LSB
 *   AG_MG_REGS_XXX_BM : bit/field mask
 *   AG_MG_REGS_XXX_U  : bitfields in C union typedef
 *   AG_MG_REGS_XXX_S  : registers in C struct typedef
 *   AG_MG_REGS_XX_RPT : number of identical registers in array
 *   AG_MG_REGS_XX_IVL : interval between registers in array
 *
 * NOTE: user may redefine ag_mg_regs_register
 *       in ag_mg_regs_regops.h if necessary
 * NOTE: access mode of individual bit fields matches that
 *       of containing register unless indicated otherwise
 */

#ifndef AG_MG_REGS_CAR_REGISTERS_H
#define AG_MG_REGS_CAR_REGISTERS_H

#include "ag_mg_regs_regops.h"
/*
 * Generated by HSI Designer release 2.3.5.
 */





/*
 * Initialization value: 0x0000131C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PLL1CTL_RO                                               0x00000000
#define AG_MG_REGS_PLL1CTL_RM                                               0x0001FFFF

#define AG_MG_REGS_PLL1CTL_PLL1F_BO                                         0
#define AG_MG_REGS_PLL1CTL_PLL1F_BM                                         0x000000FF

#define AG_MG_REGS_PLL1CTL_PLL1Z_BO                                         8
#define AG_MG_REGS_PLL1CTL_PLL1Z_BM                                         0x00000700

#define AG_MG_REGS_PLL1CTL_PLL1P_BO                                         11
#define AG_MG_REGS_PLL1CTL_PLL1P_BM                                         0x00001800

#define AG_MG_REGS_PLL1CTL_PLL1EN_BO                                        13
#define AG_MG_REGS_PLL1CTL_PLL1EN_BM                                        0x00002000

#define AG_MG_REGS_PLL1CTL_PLL1CARLOCK_BO                                   14
#define AG_MG_REGS_PLL1CTL_PLL1CARLOCK_BM                                   0x00004000

#define AG_MG_REGS_PLL1CTL_PLL1BYP_BO                                       15
#define AG_MG_REGS_PLL1CTL_PLL1BYP_BM                                       0x00008000

#define AG_MG_REGS_PLL1CTL_PLL1LOCK_BO                                      16
#define AG_MG_REGS_PLL1CTL_PLL1LOCK_BM                                      0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PLL1CTL_U
{
    struct
    {
        ag_mg_regs_register
            pll1f : 8,
            pll1z : 3,
            pll1p : 2,
            pll1en : 1,
            pll1carlock : 1,
            pll1byp : 1,
            pll1lock : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pll1ctl_u;
#endif


/*
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PLL1SEL_RO                                               0x00000004
#define AG_MG_REGS_PLL1SEL_RM                                               0x00000003

#define AG_MG_REGS_PLL1SEL_PLL1SEL_BO                                       0
#define AG_MG_REGS_PLL1SEL_PLL1SEL_BM                                       0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PLL1SEL_U
{
    struct
    {
        ag_mg_regs_register
            pll1sel : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pll1sel_u;
#endif


/*
 * Initialization value: 0x00001313  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PLL2CTL_RO                                               0x00000008
#define AG_MG_REGS_PLL2CTL_RM                                               0x0001FFFF

#define AG_MG_REGS_PLL2CTL_PLL2F_BO                                         0
#define AG_MG_REGS_PLL2CTL_PLL2F_BM                                         0x000000FF

#define AG_MG_REGS_PLL2CTL_PLL2Z_BO                                         8
#define AG_MG_REGS_PLL2CTL_PLL2Z_BM                                         0x00000700

#define AG_MG_REGS_PLL2CTL_PLL2P_BO                                         11
#define AG_MG_REGS_PLL2CTL_PLL2P_BM                                         0x00001800

#define AG_MG_REGS_PLL2CTL_PLL2EN_BO                                        13
#define AG_MG_REGS_PLL2CTL_PLL2EN_BM                                        0x00002000

#define AG_MG_REGS_PLL2CTL_PLL2CARLOCK_BO                                   14
#define AG_MG_REGS_PLL2CTL_PLL2CARLOCK_BM                                   0x00004000

#define AG_MG_REGS_PLL2CTL_PLL2LOCK_BO                                      16
#define AG_MG_REGS_PLL2CTL_PLL2LOCK_BM                                      0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PLL2CTL_U
{
    struct
    {
        ag_mg_regs_register
            pll2f : 8,
            pll2z : 3,
            pll2p : 2,
            pll2en : 1,
            pll2carlock : 1,
            fill1 : 1,
            pll2lock : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pll2ctl_u;
#endif


/*
 * Initialization value: 0x0000FFFF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CLKCTL_RO                                                0x00000010
#define AG_MG_REGS_CLKCTL_RM                                                0x00001FFF

#define AG_MG_REGS_CLKCTL_PCIE_CLKEN_BO                                     0
#define AG_MG_REGS_CLKCTL_PCIE_CLKEN_BM                                     0x00000001

#define AG_MG_REGS_CLKCTL_GIGE1_CLKEN_BO                                    1
#define AG_MG_REGS_CLKCTL_GIGE1_CLKEN_BM                                    0x00000002

#define AG_MG_REGS_CLKCTL_GIGE0_CLKEN_BO                                    2
#define AG_MG_REGS_CLKCTL_GIGE0_CLKEN_BM                                    0x00000004

#define AG_MG_REGS_CLKCTL_DDR3_CLKEN_BO                                     3
#define AG_MG_REGS_CLKCTL_DDR3_CLKEN_BM                                     0x00000008

#define AG_MG_REGS_CLKCTL_TDM_CLKEN_BO                                      4
#define AG_MG_REGS_CLKCTL_TDM_CLKEN_BM                                      0x00000010

#define AG_MG_REGS_CLKCTL_ARM_CLKEN_BO                                      5
#define AG_MG_REGS_CLKCTL_ARM_CLKEN_BM                                      0x00000020

#define AG_MG_REGS_CLKCTL_SSP_CLKEN_BO                                      6
#define AG_MG_REGS_CLKCTL_SSP_CLKEN_BM                                      0x00000040

#define AG_MG_REGS_CLKCTL_UART_CLKEN_BO                                     7
#define AG_MG_REGS_CLKCTL_UART_CLKEN_BM                                     0x00000080

#define AG_MG_REGS_CLKCTL_SRIO0_CLKEN_BO                                    8
#define AG_MG_REGS_CLKCTL_SRIO0_CLKEN_BM                                    0x00000100

#define AG_MG_REGS_CLKCTL_SRIO1_CLKEN_BO                                    9
#define AG_MG_REGS_CLKCTL_SRIO1_CLKEN_BM                                    0x00000200

#define AG_MG_REGS_CLKCTL_FLASH_CLKEN_BO                                    10
#define AG_MG_REGS_CLKCTL_FLASH_CLKEN_BM                                    0x00000400

#define AG_MG_REGS_CLKCTL_PPB_CLKEN_BO                                      11
#define AG_MG_REGS_CLKCTL_PPB_CLKEN_BM                                      0x00000800

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CLKCTL_U
{
    struct
    {
        ag_mg_regs_register
            pcie_clken : 1,
            gige1_clken : 1,
            gige0_clken : 1,
            ddr3_clken : 1,
            tdm_clken : 1,
            arm_clken : 1,
            ssp_clken : 1,
            uart_clken : 1,
            srio0_clken : 1,
            srio1_clken : 1,
            flash_clken : 1,
            ppb_clken : 1,
            fill0 : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_clkctl_u;
#endif


/*
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CLKOUT_RO                                                0x00000014
#define AG_MG_REGS_CLKOUT_RM                                                0x0000003F

#define AG_MG_REGS_CLKOUT_CLKOUT_BO                                         0
#define AG_MG_REGS_CLKOUT_CLKOUT_BM                                         0x0000003F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CLKOUT_U
{
    struct
    {
        ag_mg_regs_register
            clkout : 6,
            fill0 : 26;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_clkout_u;
#endif


/*
 * DSSEN (DSS Enable Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DSSEN_RO                                                 0x00000018
#define AG_MG_REGS_DSSEN_RM                                                 0x000000FF

#define AG_MG_REGS_DSSEN_DSS0EN_BO                                          0
#define AG_MG_REGS_DSSEN_DSS0EN_BM                                          0x00000001

#define AG_MG_REGS_DSSEN_DSS1EN_BO                                          1
#define AG_MG_REGS_DSSEN_DSS1EN_BM                                          0x00000002

#define AG_MG_REGS_DSSEN_DSS2EN_BO                                          2
#define AG_MG_REGS_DSSEN_DSS2EN_BM                                          0x00000004

#define AG_MG_REGS_DSSEN_DSS3EN_BO                                          3
#define AG_MG_REGS_DSSEN_DSS3EN_BM                                          0x00000008

#define AG_MG_REGS_DSSEN_L2CACHE_EN_BO                                      4
#define AG_MG_REGS_DSSEN_L2CACHE_EN_BM                                      0x00000010

#define AG_MG_REGS_DSSEN_SP2702_MODE_BO                                     5
#define AG_MG_REGS_DSSEN_SP2702_MODE_BM                                     0x00000020

#define AG_MG_REGS_DSSEN_ARMEN_BO                                           6
#define AG_MG_REGS_DSSEN_ARMEN_BM                                           0x000000C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DSSEN_U
{
    struct
    {
        ag_mg_regs_register
            dss0en : 1,
            dss1en : 1,
            dss2en : 1,
            dss3en : 1,
            l2cache_en : 1,
            sp2702_mode : 1,
            armen : 2,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dssen_u;
#endif


/*
 * CHIP_WAKE (Chip Wake Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CHIP_WAKE_RO                                             0x0000001C
#define AG_MG_REGS_CHIP_WAKE_RM                                             0x00000001

#define AG_MG_REGS_CHIP_WAKE_CHIP_WAKE_BO                                   0
#define AG_MG_REGS_CHIP_WAKE_CHIP_WAKE_BM                                   0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CHIP_WAKE_U
{
    struct
    {
        ag_mg_regs_register
            chip_wake : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_chip_wake_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM0IO_RO                                                0x00000020
#define AG_MG_REGS_TDM0IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM0IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM0IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM0IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM0IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM0IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM0IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM0IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM0IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM0IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM0IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM0IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM0IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM0IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM0IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM0IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM0IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM0IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM0IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM0IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm0io_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM1IO_RO                                                0x00000024
#define AG_MG_REGS_TDM1IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM1IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM1IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM1IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM1IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM1IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM1IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM1IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM1IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM1IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM1IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM1IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM1IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM1IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM1IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM1IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM1IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM1IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM1IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM1IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm1io_u;
#endif


/*
 * TDM2IO (CAR TDM2 I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM2IO_RO                                                0x00000028
#define AG_MG_REGS_TDM2IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM2IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM2IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM2IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM2IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM2IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM2IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM2IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM2IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM2IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM2IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM2IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM2IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM2IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM2IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM2IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM2IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM2IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM2IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM2IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm2io_u;
#endif


/*
 * TDM3IO (CAR TDM3 I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM3IO_RO                                                0x0000002C
#define AG_MG_REGS_TDM3IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM3IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM3IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM3IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM3IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM3IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM3IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM3IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM3IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM3IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM3IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM3IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM3IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM3IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM3IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM3IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM3IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM3IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM3IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM3IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm3io_u;
#endif


/*
 * TDM4IO (CAR TDM4 I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM4IO_RO                                                0x00000030
#define AG_MG_REGS_TDM4IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM4IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM4IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM4IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM4IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM4IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM4IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM4IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM4IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM4IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM4IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM4IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM4IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM4IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM4IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM4IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM4IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM4IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM4IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM4IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm4io_u;
#endif


/*
 * TDM5IO (CAR TDM5 I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TDM5IO_RO                                                0x00000034
#define AG_MG_REGS_TDM5IO_RM                                                0x00003FF8

#define AG_MG_REGS_TDM5IO_ODSC_BO                                           3
#define AG_MG_REGS_TDM5IO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_TDM5IO_CDSC_BO                                           5
#define AG_MG_REGS_TDM5IO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_TDM5IO_IPD50_BO                                          7
#define AG_MG_REGS_TDM5IO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_TDM5IO_OPD50_BO                                          8
#define AG_MG_REGS_TDM5IO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_TDM5IO_BPD50_BO                                          9
#define AG_MG_REGS_TDM5IO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_TDM5IO_IPU50_BO                                          10
#define AG_MG_REGS_TDM5IO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_TDM5IO_OPU50_BO                                          11
#define AG_MG_REGS_TDM5IO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_TDM5IO_BPU50_BO                                          12
#define AG_MG_REGS_TDM5IO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_TDM5IO_LPMODE_BO                                         13
#define AG_MG_REGS_TDM5IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TDM5IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tdm5io_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SSSMII0IO_RO                                             0x00000038
#define AG_MG_REGS_SSSMII0IO_RM                                             0x00003FF8

#define AG_MG_REGS_SSSMII0IO_ODSC_BO                                        3
#define AG_MG_REGS_SSSMII0IO_ODSC_BM                                        0x00000018

#define AG_MG_REGS_SSSMII0IO_CDSC_BO                                        5
#define AG_MG_REGS_SSSMII0IO_CDSC_BM                                        0x00000060

#define AG_MG_REGS_SSSMII0IO_IPD50_BO                                       7
#define AG_MG_REGS_SSSMII0IO_IPD50_BM                                       0x00000080

#define AG_MG_REGS_SSSMII0IO_OPD50_BO                                       8
#define AG_MG_REGS_SSSMII0IO_OPD50_BM                                       0x00000100

#define AG_MG_REGS_SSSMII0IO_BPD50_BO                                       9
#define AG_MG_REGS_SSSMII0IO_BPD50_BM                                       0x00000200

#define AG_MG_REGS_SSSMII0IO_IPU50_BO                                       10
#define AG_MG_REGS_SSSMII0IO_IPU50_BM                                       0x00000400

#define AG_MG_REGS_SSSMII0IO_OPU50_BO                                       11
#define AG_MG_REGS_SSSMII0IO_OPU50_BM                                       0x00000800

#define AG_MG_REGS_SSSMII0IO_BPU50_BO                                       12
#define AG_MG_REGS_SSSMII0IO_BPU50_BM                                       0x00001000

#define AG_MG_REGS_SSSMII0IO_LPMODE_BO                                      13
#define AG_MG_REGS_SSSMII0IO_LPMODE_BM                                      0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SSSMII0IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sssmii0io_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SSSMII1IO_RO                                             0x0000003C
#define AG_MG_REGS_SSSMII1IO_RM                                             0x00003FF8

#define AG_MG_REGS_SSSMII1IO_ODSC_BO                                        3
#define AG_MG_REGS_SSSMII1IO_ODSC_BM                                        0x00000018

#define AG_MG_REGS_SSSMII1IO_CDSC_BO                                        5
#define AG_MG_REGS_SSSMII1IO_CDSC_BM                                        0x00000060

#define AG_MG_REGS_SSSMII1IO_IPD50_BO                                       7
#define AG_MG_REGS_SSSMII1IO_IPD50_BM                                       0x00000080

#define AG_MG_REGS_SSSMII1IO_OPD50_BO                                       8
#define AG_MG_REGS_SSSMII1IO_OPD50_BM                                       0x00000100

#define AG_MG_REGS_SSSMII1IO_BPD50_BO                                       9
#define AG_MG_REGS_SSSMII1IO_BPD50_BM                                       0x00000200

#define AG_MG_REGS_SSSMII1IO_IPU50_BO                                       10
#define AG_MG_REGS_SSSMII1IO_IPU50_BM                                       0x00000400

#define AG_MG_REGS_SSSMII1IO_OPU50_BO                                       11
#define AG_MG_REGS_SSSMII1IO_OPU50_BM                                       0x00000800

#define AG_MG_REGS_SSSMII1IO_BPU50_BO                                       12
#define AG_MG_REGS_SSSMII1IO_BPU50_BM                                       0x00001000

#define AG_MG_REGS_SSSMII1IO_LPMODE_BO                                      13
#define AG_MG_REGS_SSSMII1IO_LPMODE_BM                                      0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SSSMII1IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sssmii1io_u;
#endif


/*
 * SSPIO (SSP I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SSPIO_RO                                                 0x00000040
#define AG_MG_REGS_SSPIO_RM                                                 0x00003FF8

#define AG_MG_REGS_SSPIO_ODSC_BO                                            3
#define AG_MG_REGS_SSPIO_ODSC_BM                                            0x00000018

#define AG_MG_REGS_SSPIO_CDSC_BO                                            5
#define AG_MG_REGS_SSPIO_CDSC_BM                                            0x00000060

#define AG_MG_REGS_SSPIO_IPD50_BO                                           7
#define AG_MG_REGS_SSPIO_IPD50_BM                                           0x00000080

#define AG_MG_REGS_SSPIO_OPD50_BO                                           8
#define AG_MG_REGS_SSPIO_OPD50_BM                                           0x00000100

#define AG_MG_REGS_SSPIO_BPD50_BO                                           9
#define AG_MG_REGS_SSPIO_BPD50_BM                                           0x00000200

#define AG_MG_REGS_SSPIO_IPU50_BO                                           10
#define AG_MG_REGS_SSPIO_IPU50_BM                                           0x00000400

#define AG_MG_REGS_SSPIO_OPU50_BO                                           11
#define AG_MG_REGS_SSPIO_OPU50_BM                                           0x00000800

#define AG_MG_REGS_SSPIO_BPU50_BO                                           12
#define AG_MG_REGS_SSPIO_BPU50_BM                                           0x00001000

#define AG_MG_REGS_SSPIO_LPMODE_BO                                          13
#define AG_MG_REGS_SSPIO_LPMODE_BM                                          0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SSPIO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sspio_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIOIO_RO                                                0x00000044
#define AG_MG_REGS_GPIOIO_RM                                                0x00003FF8

#define AG_MG_REGS_GPIOIO_ODSC_BO                                           3
#define AG_MG_REGS_GPIOIO_ODSC_BM                                           0x00000018

#define AG_MG_REGS_GPIOIO_CDSC_BO                                           5
#define AG_MG_REGS_GPIOIO_CDSC_BM                                           0x00000060

#define AG_MG_REGS_GPIOIO_IPD50_BO                                          7
#define AG_MG_REGS_GPIOIO_IPD50_BM                                          0x00000080

#define AG_MG_REGS_GPIOIO_OPD50_BO                                          8
#define AG_MG_REGS_GPIOIO_OPD50_BM                                          0x00000100

#define AG_MG_REGS_GPIOIO_BPD50_BO                                          9
#define AG_MG_REGS_GPIOIO_BPD50_BM                                          0x00000200

#define AG_MG_REGS_GPIOIO_IPU50_BO                                          10
#define AG_MG_REGS_GPIOIO_IPU50_BM                                          0x00000400

#define AG_MG_REGS_GPIOIO_OPU50_BO                                          11
#define AG_MG_REGS_GPIOIO_OPU50_BM                                          0x00000800

#define AG_MG_REGS_GPIOIO_BPU50_BO                                          12
#define AG_MG_REGS_GPIOIO_BPU50_BM                                          0x00001000

#define AG_MG_REGS_GPIOIO_LPMODE_BO                                         13
#define AG_MG_REGS_GPIOIO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIOIO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpioio_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CLOCKSIO_RO                                              0x00000048
#define AG_MG_REGS_CLOCKSIO_RM                                              0x00003FF8

#define AG_MG_REGS_CLOCKSIO_ODSC_BO                                         3
#define AG_MG_REGS_CLOCKSIO_ODSC_BM                                         0x00000018

#define AG_MG_REGS_CLOCKSIO_CDSC_BO                                         5
#define AG_MG_REGS_CLOCKSIO_CDSC_BM                                         0x00000060

#define AG_MG_REGS_CLOCKSIO_IPD50_BO                                        7
#define AG_MG_REGS_CLOCKSIO_IPD50_BM                                        0x00000080

#define AG_MG_REGS_CLOCKSIO_OPD50_BO                                        8
#define AG_MG_REGS_CLOCKSIO_OPD50_BM                                        0x00000100

#define AG_MG_REGS_CLOCKSIO_BPD50_BO                                        9
#define AG_MG_REGS_CLOCKSIO_BPD50_BM                                        0x00000200

#define AG_MG_REGS_CLOCKSIO_IPU50_BO                                        10
#define AG_MG_REGS_CLOCKSIO_IPU50_BM                                        0x00000400

#define AG_MG_REGS_CLOCKSIO_OPU50_BO                                        11
#define AG_MG_REGS_CLOCKSIO_OPU50_BM                                        0x00000800

#define AG_MG_REGS_CLOCKSIO_BPU50_BO                                        12
#define AG_MG_REGS_CLOCKSIO_BPU50_BM                                        0x00001000

#define AG_MG_REGS_CLOCKSIO_LPMODE_BO                                       13
#define AG_MG_REGS_CLOCKSIO_LPMODE_BM                                       0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CLOCKSIO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_clocksio_u;
#endif


/*
 * DDR3IO (DDR3 I/O Control Register)
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DDR3IO_RO                                                0x0000004C
#define AG_MG_REGS_DDR3IO_RM                                                0x00002000

#define AG_MG_REGS_DDR3IO_LPMODE_BO                                         13
#define AG_MG_REGS_DDR3IO_LPMODE_BM                                         0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DDR3IO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 13,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ddr3io_u;
#endif


/*
 * Initialization value: 0x00001428  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_FLASHIO_RO                                               0x00000050
#define AG_MG_REGS_FLASHIO_RM                                               0x00003FF8

#define AG_MG_REGS_FLASHIO_ODSC_BO                                          3
#define AG_MG_REGS_FLASHIO_ODSC_BM                                          0x00000018

#define AG_MG_REGS_FLASHIO_CDSC_BO                                          5
#define AG_MG_REGS_FLASHIO_CDSC_BM                                          0x00000060

#define AG_MG_REGS_FLASHIO_IPD50_BO                                         7
#define AG_MG_REGS_FLASHIO_IPD50_BM                                         0x00000080

#define AG_MG_REGS_FLASHIO_OPD50_BO                                         8
#define AG_MG_REGS_FLASHIO_OPD50_BM                                         0x00000100

#define AG_MG_REGS_FLASHIO_BPD50_BO                                         9
#define AG_MG_REGS_FLASHIO_BPD50_BM                                         0x00000200

#define AG_MG_REGS_FLASHIO_IPU50_BO                                         10
#define AG_MG_REGS_FLASHIO_IPU50_BM                                         0x00000400

#define AG_MG_REGS_FLASHIO_OPU50_BO                                         11
#define AG_MG_REGS_FLASHIO_OPU50_BM                                         0x00000800

#define AG_MG_REGS_FLASHIO_BPU50_BO                                         12
#define AG_MG_REGS_FLASHIO_BPU50_BM                                         0x00001000

#define AG_MG_REGS_FLASHIO_LPMODE_BO                                        13
#define AG_MG_REGS_FLASHIO_LPMODE_BM                                        0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_FLASHIO_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            odsc : 2,
            cdsc : 2,
            ipd50 : 1,
            opd50 : 1,
            bpd50 : 1,
            ipu50 : 1,
            opu50 : 1,
            bpu50 : 1,
            lpmode : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_flashio_u;
#endif


/*
 * Initialization value: 0x02000A00  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SERDESIO_RO                                              0x00000054
#define AG_MG_REGS_SERDESIO_RM                                              0x07FFFFFF

#define AG_MG_REGS_SERDESIO_SGMIIEN50_BO                                    0
#define AG_MG_REGS_SERDESIO_SGMIIEN50_BM                                    0x00000001

#define AG_MG_REGS_SERDESIO_SGMIIAC_BO                                      1
#define AG_MG_REGS_SERDESIO_SGMIIAC_BM                                      0x00000002

#define AG_MG_REGS_SERDESIO_SGMIILP_BO                                      2
#define AG_MG_REGS_SERDESIO_SGMIILP_BM                                      0x00000004

#define AG_MG_REGS_SERDESIO_SRIOEN50_BO                                     3
#define AG_MG_REGS_SERDESIO_SRIOEN50_BM                                     0x00000008

#define AG_MG_REGS_SERDESIO_SRIOAC_BO                                       4
#define AG_MG_REGS_SERDESIO_SRIOAC_BM                                       0x00000010

#define AG_MG_REGS_SERDESIO_SRIOLP_BO                                       5
#define AG_MG_REGS_SERDESIO_SRIOLP_BM                                       0x00000020

#define AG_MG_REGS_SERDESIO_PCIEEN50_BO                                     6
#define AG_MG_REGS_SERDESIO_PCIEEN50_BM                                     0x00000040

#define AG_MG_REGS_SERDESIO_PCIEAC_BO                                       7
#define AG_MG_REGS_SERDESIO_PCIEAC_BM                                       0x00000080

#define AG_MG_REGS_SERDESIO_PCIELP_BO                                       8
#define AG_MG_REGS_SERDESIO_PCIELP_BM                                       0x00000100

#define AG_MG_REGS_SERDESIO_TRIM_SETTING_BO                                 9
#define AG_MG_REGS_SERDESIO_TRIM_SETTING_BM                                 0x00000E00

#define AG_MG_REGS_SERDESIO_TRIM_RESISTOR_BO                                12
#define AG_MG_REGS_SERDESIO_TRIM_RESISTOR_BM                                0x0000F000

#define AG_MG_REGS_SERDESIO_TRIM_TST_MODE_BO                                16
#define AG_MG_REGS_SERDESIO_TRIM_TST_MODE_BM                                0x00010000

#define AG_MG_REGS_SERDESIO_TRIM_COUNT_BO                                   17
#define AG_MG_REGS_SERDESIO_TRIM_COUNT_BM                                   0x07FE0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SERDESIO_U
{
    struct
    {
        ag_mg_regs_register
            sgmiien50 : 1,
            sgmiiac : 1,
            sgmiilp : 1,
            srioen50 : 1,
            srioac : 1,
            sriolp : 1,
            pcieen50 : 1,
            pcieac : 1,
            pcielp : 1,
            trim_setting : 3,
            trim_resistor : 4,
            trim_tst_mode : 1,
            trim_count : 10,
            fill0 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_serdesio_u;
#endif

/*
 * Initialization value: 0x02000A00  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DDR3_SWAP_RO                                      0x00000058
#define AG_MG_REGS_DDR3_SWAP_RM                                      0xFFFFFFFF

#define AG_MG_REGS_DDR3_SWAP_BO                                    0
#define AG_MG_REGS_DDR3_SWAP_BM                                    0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DDR3_SWAP_U
{
    struct
    {
        ag_mg_regs_register
			ddr3_swap : 1,
			fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ddr3_swap_u;
#endif



/*
 * Initialization value: 0x02000A00  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DDR3_KILL_FORCE_RO                                      0x0000005c
#define AG_MG_REGS_DDR3_KILL_FORCE_RM                                      0xFFFFFFFF

#define AG_MG_REGS_DDR3_KILL_FORCE_ON_BO                                    0
#define AG_MG_REGS_DDR3_KILL_FORCE_ON_BM                                    0x00000001

#define AG_MG_REGS_DDR3_KILL_FORCE_MODE_BO                                  1
#define AG_MG_REGS_DDR3_KILL_FORCE_MODE_BM                                  0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DDR3_KILL_FORCE_U
{
    struct
    {
        ag_mg_regs_register
			ddr3_kill_force_on : 1,
			ddr3_kill_force_mode : 1,
			fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ddr3_kill_force_u;
#endif

/*
 * RSTPROT (Reset Protection KEY)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RSTPROT_RO                                               0x00000060
#define AG_MG_REGS_RSTPROT_RM                                               0xFFFFFFFF

#define AG_MG_REGS_RSTPROT_RSTPROT_BO                                       0
#define AG_MG_REGS_RSTPROT_RSTPROT_BM                                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RSTPROT_U
{
    struct
    {
        ag_mg_regs_register
            rstprot;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rstprot_u;
#endif


/*
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_RESSTAT_RO                                               0x00000064
#define AG_MG_REGS_RESSTAT_RM                                               0x0000000F

#define AG_MG_REGS_RESSTAT_HWRST_BO                                         0
#define AG_MG_REGS_RESSTAT_HWRST_BM                                         0x00000001

#define AG_MG_REGS_RESSTAT_CHIPRST_BO                                       1
#define AG_MG_REGS_RESSTAT_CHIPRST_BM                                       0x00000002

#define AG_MG_REGS_RESSTAT_WDRST1_BO                                        2
#define AG_MG_REGS_RESSTAT_WDRST1_BM                                        0x00000004

#define AG_MG_REGS_RESSTAT_WDRST2_BO                                        3
#define AG_MG_REGS_RESSTAT_WDRST2_BM                                        0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RESSTAT_U
{
    struct
    {
        ag_mg_regs_register
            hwrst : 1,
            chiprst : 1,
            wdrst1 : 1,
            wdrst2 : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_resstat_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RSTCTL_RO                                                0x00000068
#define AG_MG_REGS_RSTCTL_RM                                                0x007FFFFF

#define AG_MG_REGS_RSTCTL_SWRESET_UART_BO                                   0
#define AG_MG_REGS_RSTCTL_SWRESET_UART_BM                                   0x00000001

#define AG_MG_REGS_RSTCTL_SWRESET_MC0_BO                                    1
#define AG_MG_REGS_RSTCTL_SWRESET_MC0_BM                                    0x00000002

#define AG_MG_REGS_RSTCTL_SWRESET_MS0_BO                                    2
#define AG_MG_REGS_RSTCTL_SWRESET_MS0_BM                                    0x00000004

#define AG_MG_REGS_RSTCTL_SWRESET_MDIO0_BO                                  3
#define AG_MG_REGS_RSTCTL_SWRESET_MDIO0_BM                                  0x00000008

#define AG_MG_REGS_RSTCTL_SWRESET_MC1_BO                                    4
#define AG_MG_REGS_RSTCTL_SWRESET_MC1_BM                                    0x00000010

#define AG_MG_REGS_RSTCTL_SWRESET_MS1_BO                                    5
#define AG_MG_REGS_RSTCTL_SWRESET_MS1_BM                                    0x00000020

#define AG_MG_REGS_RSTCTL_SWRESET_MDIO1_BO                                  6
#define AG_MG_REGS_RSTCTL_SWRESET_MDIO1_BM                                  0x00000040

#define AG_MG_REGS_RSTCTL_SWRESET_PCE0_BO                                   7
#define AG_MG_REGS_RSTCTL_SWRESET_PCE0_BM                                   0x00000080

#define AG_MG_REGS_RSTCTL_SWRESET_PCE1_BO                                   8
#define AG_MG_REGS_RSTCTL_SWRESET_PCE1_BM                                   0x00000100

#define AG_MG_REGS_RSTCTL_SWRESET_TXD0_BO                                   9
#define AG_MG_REGS_RSTCTL_SWRESET_TXD0_BM                                   0x00000200

#define AG_MG_REGS_RSTCTL_SWRESET_TXD1_BO                                   10
#define AG_MG_REGS_RSTCTL_SWRESET_TXD1_BM                                   0x00000400

#define AG_MG_REGS_RSTCTL_SWRESET_PCIE_BO                                   11
#define AG_MG_REGS_RSTCTL_SWRESET_PCIE_BM                                   0x00000800

#define AG_MG_REGS_RSTCTL_SWRESET_SSP_BO                                    12
#define AG_MG_REGS_RSTCTL_SWRESET_SSP_BM                                    0x00001000

#define AG_MG_REGS_RSTCTL_SWRESET_SRIO0_BO                                  13
#define AG_MG_REGS_RSTCTL_SWRESET_SRIO0_BM                                  0x00002000

#define AG_MG_REGS_RSTCTL_SWRESET_SRIO1_BO                                  14
#define AG_MG_REGS_RSTCTL_SWRESET_SRIO1_BM                                  0x00004000

#define AG_MG_REGS_RSTCTL_SWRESET_FLASH_BO                                  15
#define AG_MG_REGS_RSTCTL_SWRESET_FLASH_BM                                  0x00008000

#define AG_MG_REGS_RSTCTL_SWRESET_DDR_BO                                    16
#define AG_MG_REGS_RSTCTL_SWRESET_DDR_BM                                    0x00010000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM0_BO                                   17
#define AG_MG_REGS_RSTCTL_SWRESET_TDM0_BM                                   0x00020000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM1_BO                                   18
#define AG_MG_REGS_RSTCTL_SWRESET_TDM1_BM                                   0x00040000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM2_BO                                   19
#define AG_MG_REGS_RSTCTL_SWRESET_TDM2_BM                                   0x00080000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM3_BO                                   20
#define AG_MG_REGS_RSTCTL_SWRESET_TDM3_BM                                   0x00100000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM4_BO                                   21
#define AG_MG_REGS_RSTCTL_SWRESET_TDM4_BM                                   0x00200000

#define AG_MG_REGS_RSTCTL_SWRESET_TDM5_BO                                   22
#define AG_MG_REGS_RSTCTL_SWRESET_TDM5_BM                                   0x00400000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RSTCTL_U
{
    struct
    {
        ag_mg_regs_register
            swreset_uart : 1,
            swreset_mc0 : 1,
            swreset_ms0 : 1,
            swreset_mdio0 : 1,
            swreset_mc1 : 1,
            swreset_ms1 : 1,
            swreset_mdio1 : 1,
            swreset_pce0 : 1,
            swreset_pce1 : 1,
            swreset_txd0 : 1,
            swreset_txd1 : 1,
            swreset_pcie : 1,
            swreset_ssp : 1,
            swreset_srio0 : 1,
            swreset_srio1 : 1,
            swreset_flash : 1,
            swreset_ddr : 1,
            swreset_tdm0 : 1,
            swreset_tdm1 : 1,
            swreset_tdm2 : 1,
            swreset_tdm3 : 1,
            swreset_tdm4 : 1,
            swreset_tdm5 : 1,
            fill0 : 9;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rstctl_u;
#endif


/*
 * Initialization value: 0x20000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CHIPID_RO                                                0x0000006C
#define AG_MG_REGS_CHIPID_RM                                                0xE000001F

#define AG_MG_REGS_CHIPID_CHIPID_BO                                         0
#define AG_MG_REGS_CHIPID_CHIPID_BM                                         0x0000001F

#define AG_MG_REGS_CHIPID_REV_ID_BO                                         29
#define AG_MG_REGS_CHIPID_REV_ID_BM                                         0xE0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CHIPID_U
{
    struct
    {
        ag_mg_regs_register
            chipid : 5,
            fill0 : 24,
            rev_id : 3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_chipid_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEVICEID_RO                                              0x00000070
#define AG_MG_REGS_DEVICEID_RM                                              0x000000FF

#define AG_MG_REGS_DEVICEID_DEVICEID_BO                                     0
#define AG_MG_REGS_DEVICEID_DEVICEID_BM                                     0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICEID_U
{
    struct
    {
        ag_mg_regs_register
            deviceid : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_deviceid_u;
#endif


/*
 * HOST_RESET (Host Reset Control)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_HOST_RESET_RO                                            0x00000074
#define AG_MG_REGS_HOST_RESET_RM                                            0x00000003

#define AG_MG_REGS_HOST_RESET_HOST_RESET_ARM0_BO                            0
#define AG_MG_REGS_HOST_RESET_HOST_RESET_ARM0_BM                            0x00000001

#define AG_MG_REGS_HOST_RESET_HOST_RESET_ARM1_BO                            1
#define AG_MG_REGS_HOST_RESET_HOST_RESET_ARM1_BM                            0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HOST_RESET_U
{
    struct
    {
        ag_mg_regs_register
            host_reset_arm0 : 1,
            host_reset_arm1 : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_host_reset_u;
#endif


/*
 * ARMRSTCTL (ARM Reset Control)
 * Initialization value: 0x00000008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ARMRSTCTL_RO                                             0x00000078
#define AG_MG_REGS_ARMRSTCTL_RM                                             0x0000003F

#define AG_MG_REGS_ARMRSTCTL_ARM0WDBYP_BO                                   0
#define AG_MG_REGS_ARMRSTCTL_ARM0WDBYP_BM                                   0x00000001

#define AG_MG_REGS_ARMRSTCTL_ARM1WDBYP_BO                                   1
#define AG_MG_REGS_ARMRSTCTL_ARM1WDBYP_BM                                   0x00000002

#define AG_MG_REGS_ARMRSTCTL_ARM0SWRST_BO                                   2
#define AG_MG_REGS_ARMRSTCTL_ARM0SWRST_BM                                   0x00000004

#define AG_MG_REGS_ARMRSTCTL_ARM1SWRST_BO                                   3
#define AG_MG_REGS_ARMRSTCTL_ARM1SWRST_BM                                   0x00000008

#define AG_MG_REGS_ARMRSTCTL_ARM0PWRDN_BO                                   4
#define AG_MG_REGS_ARMRSTCTL_ARM0PWRDN_BM                                   0x00000010

#define AG_MG_REGS_ARMRSTCTL_ARM1PWRDN_BO                                   5
#define AG_MG_REGS_ARMRSTCTL_ARM1PWRDN_BM                                   0x00000020

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ARMRSTCTL_U
{
    struct
    {
        ag_mg_regs_register
            arm0wdbyp : 1,
            arm1wdbyp : 1,
            arm0swrst : 1,
            arm1swrst : 1,
            arm0pwrdn : 1,
            arm1pwrdn : 1,
            fill0 : 26;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_armrstctl_u;
#endif


/*
 * CHIP_RESET (Chip Reset Control)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CHIP_RESET_RO                                            0x0000007C
#define AG_MG_REGS_CHIP_RESET_RM                                            0x00000001

#define AG_MG_REGS_CHIP_RESET_CHIP_RESET_BO                                 0
#define AG_MG_REGS_CHIP_RESET_CHIP_RESET_BM                                 0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CHIP_RESET_U
{
    struct
    {
        ag_mg_regs_register
            chip_reset : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_chip_reset_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PFUSE0_RO                                                0x00000080
#define AG_MG_REGS_PFUSE0_RM                                                0xFFFFFFFF

#define AG_MG_REGS_PFUSE0_PFUSE_BO                                          0
#define AG_MG_REGS_PFUSE0_PFUSE_BM                                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PFUSE0_U
{
    struct
    {
        ag_mg_regs_register
            pfuse;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pfuse0_u;
#endif


/*
 * DSS_RPR (DSS RAM Repair)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DSS_RPR_RO                                               0x00000500
#define AG_MG_REGS_DSS_RPR_RM                                               0x0000000F

#define AG_MG_REGS_DSS_RPR_DSS0RPR_EN_BO                                    0
#define AG_MG_REGS_DSS_RPR_DSS0RPR_EN_BM                                    0x00000001

#define AG_MG_REGS_DSS_RPR_DSS1RPR_EN_BO                                    1
#define AG_MG_REGS_DSS_RPR_DSS1RPR_EN_BM                                    0x00000002

#define AG_MG_REGS_DSS_RPR_DSS2RPR_EN_BO                                    2
#define AG_MG_REGS_DSS_RPR_DSS2RPR_EN_BM                                    0x00000004

#define AG_MG_REGS_DSS_RPR_DSS3RPR_EN_BO                                    3
#define AG_MG_REGS_DSS_RPR_DSS3RPR_EN_BM                                    0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DSS_RPR_U
{
    struct
    {
        ag_mg_regs_register
            dss0rpr_en : 1,
            dss1rpr_en : 1,
            dss2rpr_en : 1,
            dss3rpr_en : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dss_rpr_u;
#endif


/*
 * ARM_RPR (ARM RAM Repair)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ARM_RPR_RO                                               0x00000504
#define AG_MG_REGS_ARM_RPR_RM                                               0x00000001

#define AG_MG_REGS_ARM_RPR_ARMRPR_EN_BO                                     0
#define AG_MG_REGS_ARM_RPR_ARMRPR_EN_BM                                     0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ARM_RPR_U
{
    struct
    {
        ag_mg_regs_register
            armrpr_en : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_arm_rpr_u;
#endif


/*
 * IROM_RPR (IROM RAM Repair)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_IROM_RPR_RO                                              0x00000508
#define AG_MG_REGS_IROM_RPR_RM                                              0x00000003

#define AG_MG_REGS_IROM_RPR_ROMRPR_CPR_BO                                   0
#define AG_MG_REGS_IROM_RPR_ROMRPR_CPR_BM                                   0x00000001

#define AG_MG_REGS_IROM_RPR_ROMRPR_EN_BO                                    1
#define AG_MG_REGS_IROM_RPR_ROMRPR_EN_BM                                    0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_IROM_RPR_U
{
    struct
    {
        ag_mg_regs_register
            romrpr_cpr : 1,
            romrpr_en : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_irom_rpr_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SYS_RPR_RO                                               0x0000050C
#define AG_MG_REGS_SYS_RPR_RM                                               0x00000001

#define AG_MG_REGS_SYS_RPR_SYSRPR_EN_BO                                     0
#define AG_MG_REGS_SYS_RPR_SYSRPR_EN_BM                                     0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SYS_RPR_U
{
    struct
    {
        ag_mg_regs_register
            sysrpr_en : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sys_rpr_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PPB_RPR_RO                                               0x00000510
#define AG_MG_REGS_PPB_RPR_RM                                               0x00000001

#define AG_MG_REGS_PPB_RPR_PPBRPR_EN_BO                                     0
#define AG_MG_REGS_PPB_RPR_PPBRPR_EN_BM                                     0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PPB_RPR_U
{
    struct
    {
        ag_mg_regs_register
            ppbrpr_en : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ppb_rpr_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RPR_RST_RO                                               0x00000514
#define AG_MG_REGS_RPR_RST_RM                                               0x00000001

#define AG_MG_REGS_RPR_RST_RPR_RST_BO                                       0
#define AG_MG_REGS_RPR_RST_RPR_RST_BM                                       0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RPR_RST_U
{
    struct
    {
        ag_mg_regs_register
            rpr_rst : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rpr_rst_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEBUG_TRIG_CTRL_RO                                       0x00000518
#define AG_MG_REGS_DEBUG_TRIG_CTRL_RM                                       0x00000003

#define AG_MG_REGS_DEBUG_TRIG_CTRL_PPB2DSS_DBG_TRIG_EN_BO                   0
#define AG_MG_REGS_DEBUG_TRIG_CTRL_PPB2DSS_DBG_TRIG_EN_BM                   0x00000001

#define AG_MG_REGS_DEBUG_TRIG_CTRL_DSS2PPB_DBG_TRIG_EN_BO                   1
#define AG_MG_REGS_DEBUG_TRIG_CTRL_DSS2PPB_DBG_TRIG_EN_BM                   0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEBUG_TRIG_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            ppb2dss_dbg_trig_en : 1,
            dss2ppb_dbg_trig_en : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_debug_trig_ctrl_u;
#endif


/*
 * DDR3_STAT (DDR3 Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DDR3_STAT_RO                                             0x0000051C
#define AG_MG_REGS_DDR3_STAT_RM                                             0x000000FF

#define AG_MG_REGS_DDR3_STAT_REFRESH_IN_PROCESS_BO                          0
#define AG_MG_REGS_DDR3_STAT_REFRESH_IN_PROCESS_BM                          0x00000001

#define AG_MG_REGS_DDR3_STAT_Q_ALMOST_FULL_BO                               1
#define AG_MG_REGS_DDR3_STAT_Q_ALMOST_FULL_BM                               0x00000002

#define AG_MG_REGS_DDR3_STAT_REFRESH_ACK_BO                                 2
#define AG_MG_REGS_DDR3_STAT_REFRESH_ACK_BM                                 0x00000004

#define AG_MG_REGS_DDR3_STAT_CKE_STATUS_BO                                  3
#define AG_MG_REGS_DDR3_STAT_CKE_STATUS_BM                                  0x00000008

#define AG_MG_REGS_DDR3_STAT_CONTROLLER_BUSY_BO                             4
#define AG_MG_REGS_DDR3_STAT_CONTROLLER_BUSY_BM                             0x00000010

#define AG_MG_REGS_DDR3_STAT_PORT_BUSY_BO                                   5
#define AG_MG_REGS_DDR3_STAT_PORT_BUSY_BM                                   0x00000020

#define AG_MG_REGS_DDR3_STAT_ECC_DATAOUT_CORRECTED_BO                       6
#define AG_MG_REGS_DDR3_STAT_ECC_DATAOUT_CORRECTED_BM                       0x00000040

#define AG_MG_REGS_DDR3_STAT_ECC_DATAOUT_UNCORRECTED_BO                     7
#define AG_MG_REGS_DDR3_STAT_ECC_DATAOUT_UNCORRECTED_BM                     0x00000080

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DDR3_STAT_U
{
    struct
    {
        ag_mg_regs_register
            refresh_in_process : 1,
            q_almost_full : 1,
            rfresh_ack : 1,
            cke_status : 1,
            controller_busy : 1,
            port_busy : 1,
            ecc_dataout_corrected : 1,
            ecc_dataout_uncorrected : 1,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ddr3_stat_u;
#endif


/*
 * DDR3_REFRESH (DDR3 Refresh Pulse)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Write Only
 */
#define AG_MG_REGS_DDR3_REFRESH_RO                                          0x00000520
#define AG_MG_REGS_DDR3_REFRESH_RM                                          0x00000001

#define AG_MG_REGS_DDR3_REFRESH_DDR3_REFRESH_BO                             0
#define AG_MG_REGS_DDR3_REFRESH_DDR3_REFRESH_BM                             0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DDR3_REFRESH_U
{
    struct
    {
        ag_mg_regs_register
            ddr3_refresh : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ddr3_refresh_u;
#endif


/*
 * FEED_FORWARD_STAT (Feed Forward Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_FEED_FORWARD_STAT_RO                                     0x00000528
#define AG_MG_REGS_FEED_FORWARD_STAT_RM                                     0x00000001

#define AG_MG_REGS_FEED_FORWARD_STAT_FEED_FORWARD_STAT_BO                   0
#define AG_MG_REGS_FEED_FORWARD_STAT_FEED_FORWARD_STAT_BM                   0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_FEED_FORWARD_STAT_U
{
    struct
    {
        ag_mg_regs_register
            feed_forward_stat : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_feed_forward_stat_u;
#endif


/*
 * FEED_FORWARD_KEY (Feed Forware Key Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_FEED_FORWARD_KEY_RO                                      0x0000052C
#define AG_MG_REGS_FEED_FORWARD_KEY_RM                                      0x0000000F

#define AG_MG_REGS_FEED_FORWARD_KEY_FEED_FORWARD_KEY_BO                     0
#define AG_MG_REGS_FEED_FORWARD_KEY_FEED_FORWARD_KEY_BM                     0x0000000F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_FEED_FORWARD_KEY_U
{
    struct
    {
        ag_mg_regs_register
            feed_forward_key : 4,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_feed_forward_key_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_DSS_STAT0_RO                                       0x00000530
#define AG_MG_REGS_MBIST_DSS_STAT0_RM                                       0xFFFFFFFF

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_0_BO                          0
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_0_BM                          0x00000003

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_1_BO                          2
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_1_BM                          0x0000000C

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_2_BO                          4
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_2_BM                          0x00000030

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_3_BO                          6
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_3_BM                          0x000000C0

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_4_BO                          8
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_4_BM                          0x00000300

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_5_BO                          10
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_5_BM                          0x00000C00

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_6_BO                          12
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_6_BM                          0x00003000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_7_BO                          14
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS0_7_BM                          0x0000C000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_0_BO                          16
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_0_BM                          0x00030000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_1_BO                          18
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_1_BM                          0x000C0000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_2_BO                          20
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_2_BM                          0x00300000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_3_BO                          22
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_3_BM                          0x00C00000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_4_BO                          24
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_4_BM                          0x03000000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_5_BO                          26
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_5_BM                          0x0C000000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_6_BO                          28
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_6_BM                          0x30000000

#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_7_BO                          30
#define AG_MG_REGS_MBIST_DSS_STAT0_MBIST_DSS1_7_BM                          0xC0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_DSS_STAT0_U
{
    struct
    {
        ag_mg_regs_register
            mbist_dss0_0 : 2,
            mbist_dss0_1 : 2,
            mbist_dss0_2 : 2,
            mbist_dss0_3 : 2,
            mbist_dss0_4 : 2,
            mbist_dss0_5 : 2,
            mbist_dss0_6 : 2,
            mbist_dss0_7 : 2,
            mbist_dss1_0 : 2,
            mbist_dss1_1 : 2,
            mbist_dss1_2 : 2,
            mbist_dss1_3 : 2,
            mbist_dss1_4 : 2,
            mbist_dss1_5 : 2,
            mbist_dss1_6 : 2,
            mbist_dss1_7 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_dss_stat0_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_DSS_STAT1_RO                                       0x00000534
#define AG_MG_REGS_MBIST_DSS_STAT1_RM                                       0xFFFFFFFF

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_0_BO                          0
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_0_BM                          0x00000003

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_1_BO                          2
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_1_BM                          0x0000000C

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_2_BO                          4
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_2_BM                          0x00000030

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_3_BO                          6
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_3_BM                          0x000000C0

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_4_BO                          8
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_4_BM                          0x00000300

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_5_BO                          10
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_5_BM                          0x00000C00

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_6_BO                          12
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_6_BM                          0x00003000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_7_BO                          14
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS2_7_BM                          0x0000C000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_0_BO                          16
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_0_BM                          0x00030000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_1_BO                          18
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_1_BM                          0x000C0000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_2_BO                          20
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_2_BM                          0x00300000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_3_BO                          22
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_3_BM                          0x00C00000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_4_BO                          24
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_4_BM                          0x03000000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_5_BO                          26
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_5_BM                          0x0C000000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_6_BO                          28
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_6_BM                          0x30000000

#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_7_BO                          30
#define AG_MG_REGS_MBIST_DSS_STAT1_MBIST_DSS3_7_BM                          0xC0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_DSS_STAT1_U
{
    struct
    {
        ag_mg_regs_register
            mbist_dss2_0 : 2,
            mbist_dss2_1 : 2,
            mbist_dss2_2 : 2,
            mbist_dss2_3 : 2,
            mbist_dss2_4 : 2,
            mbist_dss2_5 : 2,
            mbist_dss2_6 : 2,
            mbist_dss2_7 : 2,
            mbist_dss3_0 : 2,
            mbist_dss3_1 : 2,
            mbist_dss3_2 : 2,
            mbist_dss3_3 : 2,
            mbist_dss3_4 : 2,
            mbist_dss3_5 : 2,
            mbist_dss3_6 : 2,
            mbist_dss3_7 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_dss_stat1_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_ARM_STAT_RO                                        0x00000538
#define AG_MG_REGS_MBIST_ARM_STAT_RM                                        0xFFFFFFFF

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM0_BO                             0
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM0_BM                             0x00000003

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM1_BO                             2
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM1_BM                             0x0000000C

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM2_BO                             4
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM2_BM                             0x00000030

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM3_BO                             6
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM3_BM                             0x000000C0

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM4_BO                             8
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM4_BM                             0x00000300

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM5_BO                             10
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM5_BM                             0x00000C00

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM6_BO                             12
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM6_BM                             0x00003000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM7_BO                             14
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM7_BM                             0x0000C000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM8_BO                             16
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM8_BM                             0x00030000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM9_BO                             18
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM9_BM                             0x000C0000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM10_BO                            20
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM10_BM                            0x00300000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM11_BO                            22
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM11_BM                            0x00C00000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM12_BO                            24
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM12_BM                            0x03000000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM13_BO                            26
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM13_BM                            0x0C000000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM14_BO                            28
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM14_BM                            0x30000000

#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM15_BO                            30
#define AG_MG_REGS_MBIST_ARM_STAT_MBIST_ARM15_BM                            0xC0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_ARM_STAT_U
{
    struct
    {
        ag_mg_regs_register
            mbist_arm0 : 2,
            mbist_arm1 : 2,
            mbist_arm2 : 2,
            mbist_arm3 : 2,
            mbist_arm4 : 2,
            mbist_arm5 : 2,
            mbist_arm6 : 2,
            mbist_arm7 : 2,
            mbist_arm8 : 2,
            mbist_arm9 : 2,
            mbist_arm10 : 2,
            mbist_arm11 : 2,
            mbist_arm12 : 2,
            mbist_arm13 : 2,
            mbist_arm14 : 2,
            mbist_arm15 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_arm_stat_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_SYS_STAT0_RO                                       0x0000053C
#define AG_MG_REGS_MBIST_SYS_STAT0_RM                                       0x00FFFFFF

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS0_BO                            0
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS0_BM                            0x00000003

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS1_BO                            2
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS1_BM                            0x0000000C

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS2_BO                            4
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS2_BM                            0x00000030

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS3_BO                            6
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS3_BM                            0x000000C0

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS4_BO                            8
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS4_BM                            0x00000300

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS5_BO                            10
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS5_BM                            0x00000C00

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS6_BO                            12
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS6_BM                            0x00003000

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS7_BO                            14
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS7_BM                            0x0000C000

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS8_BO                            16
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS8_BM                            0x00030000

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS9_BO                            18
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS9_BM                            0x000C0000

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS10_BO                           20
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS10_BM                           0x00300000

#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS11_BO                           22
#define AG_MG_REGS_MBIST_SYS_STAT0_MBIST_SYS11_BM                           0x00C00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_SYS_STAT0_U
{
    struct
    {
        ag_mg_regs_register
            mbist_sys0 : 2,
            mbist_sys1 : 2,
            mbist_sys2 : 2,
            mbist_sys3 : 2,
            mbist_sys4 : 2,
            mbist_sys5 : 2,
            mbist_sys6 : 2,
            mbist_sys7 : 2,
            mbist_sys8 : 2,
            mbist_sys9 : 2,
            mbist_sys10 : 2,
            mbist_sys11 : 2,
            fill0 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_sys_stat0_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_SYS_STAT1_RO                                       0x00000540
#define AG_MG_REGS_MBIST_SYS_STAT1_RM                                       0x0000FFFF

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS12_0_BO                         0
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS12_0_BM                         0x00000003

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS12_1_BO                         2
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS12_1_BM                         0x0000000C

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS13_0_BO                         4
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS13_0_BM                         0x00000030

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS13_1_BO                         6
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS13_1_BM                         0x000000C0

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS14_0_BO                         8
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS14_0_BM                         0x00000300

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS14_1_BO                         10
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS14_1_BM                         0x00000C00

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS15_0_BO                         12
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS15_0_BM                         0x00003000

#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS15_1_BO                         14
#define AG_MG_REGS_MBIST_SYS_STAT1_MBIST_SYS15_1_BM                         0x0000C000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_SYS_STAT1_U
{
    struct
    {
        ag_mg_regs_register
            mbist_sys12_0 : 2,
            mbist_sys12_1 : 2,
            mbist_sys13_0 : 2,
            mbist_sys13_1 : 2,
            mbist_sys14_0 : 2,
            mbist_sys14_1 : 2,
            mbist_sys15_0 : 2,
            mbist_sys15_1 : 2,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_sys_stat1_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MBIST_PPB_STAT_RO                                        0x00000544
#define AG_MG_REGS_MBIST_PPB_STAT_RM                                        0x000000FF

#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB0_BO                             0
#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB0_BM                             0x00000003

#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB1_BO                             2
#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB1_BM                             0x0000000C

#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB2_BO                             4
#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB2_BM                             0x00000030

#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB3_BO                             6
#define AG_MG_REGS_MBIST_PPB_STAT_MBIST_PPB3_BM                             0x000000C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MBIST_PPB_STAT_U
{
    struct
    {
        ag_mg_regs_register
            mbist_ppb0 : 2,
            mbist_ppb1 : 2,
            mbist_ppb2 : 2,
            mbist_ppb3 : 2,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mbist_ppb_stat_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SPEEDSTART_RO                                            0x00000300
#define AG_MG_REGS_SPEEDSTART_RM                                            0x0000000F

#define AG_MG_REGS_SPEEDSTART_SPEEDEN_BO                                    0
#define AG_MG_REGS_SPEEDSTART_SPEEDEN_BM                                    0x00000001

#define AG_MG_REGS_SPEEDSTART_SPEEDTEMP_BO                                  1
#define AG_MG_REGS_SPEEDSTART_SPEEDTEMP_BM                                  0x00000002

#define AG_MG_REGS_SPEEDSTART_SPEEDVOLT_BO                                  2
#define AG_MG_REGS_SPEEDSTART_SPEEDVOLT_BM                                  0x00000004

#define AG_MG_REGS_SPEEDSTART_AVSEN_BO                                      3
#define AG_MG_REGS_SPEEDSTART_AVSEN_BM                                      0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDSTART_U
{
    struct
    {
        ag_mg_regs_register
            speeden : 1,
            speedtemp : 1,
            speedvolt : 1,
            avsen : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speedstart_u;
#endif


/*
 * Initialization value: 0xFFFFFFFF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SPEEDDUR_RO                                              0x00000304
#define AG_MG_REGS_SPEEDDUR_RM                                              0xFFFFFFFF

#define AG_MG_REGS_SPEEDDUR_SPEEDDUR_BO                                     0
#define AG_MG_REGS_SPEEDDUR_SPEEDDUR_BM                                     0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDUR_U
{
    struct
    {
        ag_mg_regs_register
            speeddur;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddur_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA0H_RO                                           0x00000310
#define AG_MG_REGS_SPEEDDATA0H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA0H_SPEEDDATA0H_BO                               0
#define AG_MG_REGS_SPEEDDATA0H_SPEEDDATA0H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA0H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata0h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata0h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA0S_RO                                           0x00000314
#define AG_MG_REGS_SPEEDDATA0S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA0S_SPEEDDATA0S_BO                               0
#define AG_MG_REGS_SPEEDDATA0S_SPEEDDATA0S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA0S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata0s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata0s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA0L_RO                                           0x00000318
#define AG_MG_REGS_SPEEDDATA0L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA0L_SPEEDDATA0L_BO                               0
#define AG_MG_REGS_SPEEDDATA0L_SPEEDDATA0L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA0L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata0l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata0l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA1H_RO                                           0x0000031C
#define AG_MG_REGS_SPEEDDATA1H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA1H_SPEEDDATA1H_BO                               0
#define AG_MG_REGS_SPEEDDATA1H_SPEEDDATA1H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA1H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata1h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata1h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA1S_RO                                           0x00000320
#define AG_MG_REGS_SPEEDDATA1S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA1S_SPEEDDATA1S_BO                               0
#define AG_MG_REGS_SPEEDDATA1S_SPEEDDATA1S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA1S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata1s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata1s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA1L_RO                                           0x00000324
#define AG_MG_REGS_SPEEDDATA1L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA1L_SPEEDDATA1L_BO                               0
#define AG_MG_REGS_SPEEDDATA1L_SPEEDDATA1L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA1L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata1l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata1l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA2H_RO                                           0x00000328
#define AG_MG_REGS_SPEEDDATA2H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA2H_SPEEDDATA2H_BO                               0
#define AG_MG_REGS_SPEEDDATA2H_SPEEDDATA2H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA2H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata2h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata2h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA2S_RO                                           0x0000032C
#define AG_MG_REGS_SPEEDDATA2S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA2S_SPEEDDATA2S_BO                               0
#define AG_MG_REGS_SPEEDDATA2S_SPEEDDATA2S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA2S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata2s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata2s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA2L_RO                                           0x00000330
#define AG_MG_REGS_SPEEDDATA2L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA2L_SPEEDDATA2L_BO                               0
#define AG_MG_REGS_SPEEDDATA2L_SPEEDDATA2L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA2L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata2l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata2l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA3H_RO                                           0x00000334
#define AG_MG_REGS_SPEEDDATA3H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA3H_SPEEDDATA3H_BO                               0
#define AG_MG_REGS_SPEEDDATA3H_SPEEDDATA3H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA3H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata3h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata3h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA3S_RO                                           0x00000338
#define AG_MG_REGS_SPEEDDATA3S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA3S_SPEEDDATA3S_BO                               0
#define AG_MG_REGS_SPEEDDATA3S_SPEEDDATA3S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA3S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata3s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata3s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA3L_RO                                           0x0000033C
#define AG_MG_REGS_SPEEDDATA3L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA3L_SPEEDDATA3L_BO                               0
#define AG_MG_REGS_SPEEDDATA3L_SPEEDDATA3L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA3L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata3l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata3l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA4H_RO                                           0x00000340
#define AG_MG_REGS_SPEEDDATA4H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA4H_SPEEDDATA4H_BO                               0
#define AG_MG_REGS_SPEEDDATA4H_SPEEDDATA4H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA4H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata4h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata4h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA4S_RO                                           0x00000344
#define AG_MG_REGS_SPEEDDATA4S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA4S_SPEEDDATA4S_BO                               0
#define AG_MG_REGS_SPEEDDATA4S_SPEEDDATA4S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA4S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata4s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata4s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA4L_RO                                           0x00000348
#define AG_MG_REGS_SPEEDDATA4L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA4L_SPEEDDATA4L_BO                               0
#define AG_MG_REGS_SPEEDDATA4L_SPEEDDATA4L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA4L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata4l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata4l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA5H_RO                                           0x0000034C
#define AG_MG_REGS_SPEEDDATA5H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA5H_SPEEDDATA5H_BO                               0
#define AG_MG_REGS_SPEEDDATA5H_SPEEDDATA5H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA5H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata5h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata5h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA5S_RO                                           0x00000350
#define AG_MG_REGS_SPEEDDATA5S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA5S_SPEEDDATA5S_BO                               0
#define AG_MG_REGS_SPEEDDATA5S_SPEEDDATA5S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA5S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata5s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata5s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA5L_RO                                           0x00000354
#define AG_MG_REGS_SPEEDDATA5L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA5L_SPEEDDATA5L_BO                               0
#define AG_MG_REGS_SPEEDDATA5L_SPEEDDATA5L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA5L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata5l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata5l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA6H_RO                                           0x00000358
#define AG_MG_REGS_SPEEDDATA6H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA6H_SPEEDDATA6H_BO                               0
#define AG_MG_REGS_SPEEDDATA6H_SPEEDDATA6H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA6H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata6h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata6h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA6S_RO                                           0x0000035C
#define AG_MG_REGS_SPEEDDATA6S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA6S_SPEEDDATA6S_BO                               0
#define AG_MG_REGS_SPEEDDATA6S_SPEEDDATA6S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA6S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata6s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata6s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA6L_RO                                           0x00000360
#define AG_MG_REGS_SPEEDDATA6L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA6L_SPEEDDATA6L_BO                               0
#define AG_MG_REGS_SPEEDDATA6L_SPEEDDATA6L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA6L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata6l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata6l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA7H_RO                                           0x00000364
#define AG_MG_REGS_SPEEDDATA7H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA7H_SPEEDDATA7H_BO                               0
#define AG_MG_REGS_SPEEDDATA7H_SPEEDDATA7H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA7H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata7h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata7h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA7S_RO                                           0x00000368
#define AG_MG_REGS_SPEEDDATA7S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA7S_SPEEDDATA7S_BO                               0
#define AG_MG_REGS_SPEEDDATA7S_SPEEDDATA7S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA7S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata7s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata7s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA7L_RO                                           0x0000036C
#define AG_MG_REGS_SPEEDDATA7L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA7L_SPEEDDATA7L_BO                               0
#define AG_MG_REGS_SPEEDDATA7L_SPEEDDATA7L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA7L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata7l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata7l_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA8H_RO                                           0x00000370
#define AG_MG_REGS_SPEEDDATA8H_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA8H_SPEEDDATA8H_BO                               0
#define AG_MG_REGS_SPEEDDATA8H_SPEEDDATA8H_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA8H_U
{
    struct
    {
        ag_mg_regs_register
            speeddata8h;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata8h_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA8S_RO                                           0x00000374
#define AG_MG_REGS_SPEEDDATA8S_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA8S_SPEEDDATA8S_BO                               0
#define AG_MG_REGS_SPEEDDATA8S_SPEEDDATA8S_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA8S_U
{
    struct
    {
        ag_mg_regs_register
            speeddata8s;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata8s_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SPEEDDATA8L_RO                                           0x00000378
#define AG_MG_REGS_SPEEDDATA8L_RM                                           0xFFFFFFFF

#define AG_MG_REGS_SPEEDDATA8L_SPEEDDATA8L_BO                               0
#define AG_MG_REGS_SPEEDDATA8L_SPEEDDATA8L_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SPEEDDATA8L_U
{
    struct
    {
        ag_mg_regs_register
            speeddata8l;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_speeddata8l_u;
#endif


/*
 * TEMPDATA0 (Temperature 0 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA0_RO                                             0x0000037C
#define AG_MG_REGS_TEMPDATA0_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA0_TEMPDATA0_BO                                   0
#define AG_MG_REGS_TEMPDATA0_TEMPDATA0_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA0_U
{
    struct
    {
        ag_mg_regs_register
            tempdata0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata0_u;
#endif


/*
 * TEMPDATA1 (Temperature 1 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA1_RO                                             0x00000380
#define AG_MG_REGS_TEMPDATA1_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA1_TEMPDATA1_BO                                   0
#define AG_MG_REGS_TEMPDATA1_TEMPDATA1_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA1_U
{
    struct
    {
        ag_mg_regs_register
            tempdata1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata1_u;
#endif


/*
 * TEMPDATA2 (Temperature 2 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA2_RO                                             0x00000384
#define AG_MG_REGS_TEMPDATA2_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA2_TEMPDATA2_BO                                   0
#define AG_MG_REGS_TEMPDATA2_TEMPDATA2_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA2_U
{
    struct
    {
        ag_mg_regs_register
            tempdata2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata2_u;
#endif


/*
 * TEMPDATA3 (Temperature 3 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA3_RO                                             0x00000388
#define AG_MG_REGS_TEMPDATA3_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA3_TEMPDATA3_BO                                   0
#define AG_MG_REGS_TEMPDATA3_TEMPDATA3_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA3_U
{
    struct
    {
        ag_mg_regs_register
            tempdata3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata3_u;
#endif


/*
 * TEMPDATA4 (Temperature 4 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA4_RO                                             0x0000038C
#define AG_MG_REGS_TEMPDATA4_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA4_TEMPDATA4_BO                                   0
#define AG_MG_REGS_TEMPDATA4_TEMPDATA4_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA4_U
{
    struct
    {
        ag_mg_regs_register
            tempdata4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata4_u;
#endif


/*
 * TEMPDATA5 (Temperature 5 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA5_RO                                             0x00000390
#define AG_MG_REGS_TEMPDATA5_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA5_TEMPDATA5_BO                                   0
#define AG_MG_REGS_TEMPDATA5_TEMPDATA5_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA5_U
{
    struct
    {
        ag_mg_regs_register
            tempdata5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata5_u;
#endif


/*
 * TEMPDATA6 (Temperature 6 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA6_RO                                             0x00000394
#define AG_MG_REGS_TEMPDATA6_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA6_TEMPDATA6_BO                                   0
#define AG_MG_REGS_TEMPDATA6_TEMPDATA6_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA6_U
{
    struct
    {
        ag_mg_regs_register
            tempdata6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata6_u;
#endif


/*
 * TEMPDATA7 (Temperature 7 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA7_RO                                             0x00000398
#define AG_MG_REGS_TEMPDATA7_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA7_TEMPDATA7_BO                                   0
#define AG_MG_REGS_TEMPDATA7_TEMPDATA7_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA7_U
{
    struct
    {
        ag_mg_regs_register
            tempdata7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata7_u;
#endif


/*
 * TEMPDATA8 (Temperature 8 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TEMPDATA8_RO                                             0x0000039C
#define AG_MG_REGS_TEMPDATA8_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TEMPDATA8_TEMPDATA8_BO                                   0
#define AG_MG_REGS_TEMPDATA8_TEMPDATA8_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEMPDATA8_U
{
    struct
    {
        ag_mg_regs_register
            tempdata8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tempdata8_u;
#endif


/*
 * VOLTDATA0 (Voltage 0 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA0_RO                                             0x000003A0
#define AG_MG_REGS_VOLTDATA0_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA0_VOLTDATA0_BO                                   0
#define AG_MG_REGS_VOLTDATA0_VOLTDATA0_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA0_U
{
    struct
    {
        ag_mg_regs_register
            voltdata0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata0_u;
#endif


/*
 * VOLTDATA1 (Voltage 1 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA1_RO                                             0x000003A4
#define AG_MG_REGS_VOLTDATA1_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA1_VOLTDATA1_BO                                   0
#define AG_MG_REGS_VOLTDATA1_VOLTDATA1_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA1_U
{
    struct
    {
        ag_mg_regs_register
            voltdata1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata1_u;
#endif


/*
 * VOLTDATA2 (Voltage 2 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA2_RO                                             0x000003A8
#define AG_MG_REGS_VOLTDATA2_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA2_VOLTDATA2_BO                                   0
#define AG_MG_REGS_VOLTDATA2_VOLTDATA2_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA2_U
{
    struct
    {
        ag_mg_regs_register
            voltdata2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata2_u;
#endif


/*
 * VOLTDATA3 (Voltage 3 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA3_RO                                             0x000003AC
#define AG_MG_REGS_VOLTDATA3_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA3_VOLTDATA3_BO                                   0
#define AG_MG_REGS_VOLTDATA3_VOLTDATA3_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA3_U
{
    struct
    {
        ag_mg_regs_register
            voltdata3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata3_u;
#endif


/*
 * VOLTDATA4 (Voltage 4 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA4_RO                                             0x000003B0
#define AG_MG_REGS_VOLTDATA4_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA4_VOLTDATA4_BO                                   0
#define AG_MG_REGS_VOLTDATA4_VOLTDATA4_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA4_U
{
    struct
    {
        ag_mg_regs_register
            voltdata4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata4_u;
#endif


/*
 * VOLTDATA5 (Voltage 5 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA5_RO                                             0x000003B4
#define AG_MG_REGS_VOLTDATA5_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA5_VOLTDATA5_BO                                   0
#define AG_MG_REGS_VOLTDATA5_VOLTDATA5_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA5_U
{
    struct
    {
        ag_mg_regs_register
            voltdata5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata5_u;
#endif


/*
 * VOLTDATA6 (Voltage 6 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA6_RO                                             0x000003B8
#define AG_MG_REGS_VOLTDATA6_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA6_VOLTDATA6_BO                                   0
#define AG_MG_REGS_VOLTDATA6_VOLTDATA6_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA6_U
{
    struct
    {
        ag_mg_regs_register
            voltdata6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata6_u;
#endif


/*
 * VOLTDATA7 (Voltage 7 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA7_RO                                             0x000003BC
#define AG_MG_REGS_VOLTDATA7_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA7_VOLTDATA7_BO                                   0
#define AG_MG_REGS_VOLTDATA7_VOLTDATA7_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA7_U
{
    struct
    {
        ag_mg_regs_register
            voltdata7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata7_u;
#endif


/*
 * VOLTDATA8 (Voltage 8 Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VOLTDATA8_RO                                             0x000003C0
#define AG_MG_REGS_VOLTDATA8_RM                                             0xFFFFFFFF

#define AG_MG_REGS_VOLTDATA8_VOLTDATA8_BO                                   0
#define AG_MG_REGS_VOLTDATA8_VOLTDATA8_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VOLTDATA8_U
{
    struct
    {
        ag_mg_regs_register
            voltdata8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_voltdata8_u;
#endif


/*
 * RTC0 (RTC LSB Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RTC0_RO                                                  0x00000400
#define AG_MG_REGS_RTC0_RM                                                  0xFFFFFFFF

#define AG_MG_REGS_RTC0_RTC0_BO                                             0
#define AG_MG_REGS_RTC0_RTC0_BM                                             0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RTC0_U
{
    struct
    {
        ag_mg_regs_register
            rtc0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rtc0_u;
#endif


/*
 * RTC1 (RTC MSB Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RTC1_RO                                                  0x00000404
#define AG_MG_REGS_RTC1_RM                                                  0xFFFFFFFF

#define AG_MG_REGS_RTC1_RTC1_BO                                             0
#define AG_MG_REGS_RTC1_RTC1_BM                                             0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RTC1_U
{
    struct
    {
        ag_mg_regs_register
            rtc1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rtc1_u;
#endif


/*
 * RTCCTL (RTC Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RTCCTL_RO                                                0x00000408
#define AG_MG_REGS_RTCCTL_RM                                                0x00000003

#define AG_MG_REGS_RTCCTL_RTCEN_BO                                          0
#define AG_MG_REGS_RTCCTL_RTCEN_BM                                          0x00000001

#define AG_MG_REGS_RTCCTL_RTCRST_BO                                         1
#define AG_MG_REGS_RTCCTL_RTCRST_BM                                         0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RTCCTL_U
{
    struct
    {
        ag_mg_regs_register
            rtcen : 1,
            rtcrst : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rtcctl_u;
#endif


/*
 * TURBOMODE (Turbo Mode Enable Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TURBOMODE_RO                                             0x00000410
#define AG_MG_REGS_TURBOMODE_RM                                             0xFFFFFFFF

#define AG_MG_REGS_TURBOMODE_TURBOMODE_BO                                   0
#define AG_MG_REGS_TURBOMODE_TURBOMODE_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TURBOMODE_U
{
    struct
    {
        ag_mg_regs_register
            turbomode;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_turbomode_u;
#endif


/*
 * TURBOSELECT (Turbo Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TURBOSELECT_RO                                           0x00000414
#define AG_MG_REGS_TURBOSELECT_RM                                           0x00000001

#define AG_MG_REGS_TURBOSELECT_TURBOSELECT_BO                               0
#define AG_MG_REGS_TURBOSELECT_TURBOSELECT_BM                               0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TURBOSELECT_U
{
    struct
    {
        ag_mg_regs_register
            turboselect : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_turboselect_u;
#endif


/*
 * FBCNT_END (CAR PLL Turbo Mode Feedback Counter End Register)
 * Initialization value: 0x0000001F  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_FBCNT_END_RO                                             0x00000418
#define AG_MG_REGS_FBCNT_END_RM                                             0x000000FF

#define AG_MG_REGS_FBCNT_END_FBCNT_END_BO                                   0
#define AG_MG_REGS_FBCNT_END_FBCNT_END_BM                                   0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_FBCNT_END_U
{
    struct
    {
        ag_mg_regs_register
            fbcnt_end : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_fbcnt_end_u;
#endif


/*
 * TURBOFORCE (CAR Turbo Mode Force Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TURBOFORCE_RO                                            0x0000041C
#define AG_MG_REGS_TURBOFORCE_RM                                            0x00000001

#define AG_MG_REGS_TURBOFORCE_TURBOFORCE_BO                                 0
#define AG_MG_REGS_TURBOFORCE_TURBOFORCE_BM                                 0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TURBOFORCE_U
{
    struct
    {
        ag_mg_regs_register
            turboforce : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_turboforce_u;
#endif


/*
 * PCIE_RC_EP_N (PCIe Root Complex/End Point Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_RC_EP_N_RO                                          0x00000420
#define AG_MG_REGS_PCIE_RC_EP_N_RM                                          0x00000001

#define AG_MG_REGS_PCIE_RC_EP_N_PCIE_RC_EP_N_BO                             0
#define AG_MG_REGS_PCIE_RC_EP_N_PCIE_RC_EP_N_BM                             0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_RC_EP_N_U
{
    struct
    {
        ag_mg_regs_register
            pcie_rc_ep_n : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_rc_ep_n_u;
#endif


/*
 * SRIO_LANE_SEL (SRIO Lane Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_LANE_SEL_RO                                         0x00000424
#define AG_MG_REGS_SRIO_LANE_SEL_RM                                         0x00000001

#define AG_MG_REGS_SRIO_LANE_SEL_SRIO_LANE_SEL_BO                           0
#define AG_MG_REGS_SRIO_LANE_SEL_SRIO_LANE_SEL_BM                           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_LANE_SEL_U
{
    struct
    {
        ag_mg_regs_register
            srio_lane_sel : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_lane_sel_u;
#endif


/*
 * PCIE_TX_FIFOEN (PCIe Transmit FIFO Enable Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_TX_FIFOEN_RO                                        0x00000428
#define AG_MG_REGS_PCIE_TX_FIFOEN_RM                                        0x00000003

#define AG_MG_REGS_PCIE_TX_FIFOEN_PCIE_TX_FIFOEN_BO                         0
#define AG_MG_REGS_PCIE_TX_FIFOEN_PCIE_TX_FIFOEN_BM                         0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_TX_FIFOEN_U
{
    struct
    {
        ag_mg_regs_register
            pcie_tx_fifoen : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_tx_fifoen_u;
#endif


/*
 * SRIO0_MODE (sRIO 0 Mode Register)
 * Initialization value: 0x00000003  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO0_MODE_RO                                            0x00000430
#define AG_MG_REGS_SRIO0_MODE_RM                                            0x00000003

#define AG_MG_REGS_SRIO0_MODE_SRIO0_MODE_BO                                 0
#define AG_MG_REGS_SRIO0_MODE_SRIO0_MODE_BM                                 0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO0_MODE_U
{
    struct
    {
        ag_mg_regs_register
            srio0_mode : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio0_mode_u;
#endif


/*
 * SRIO_PEFCAR (sRIO 0 ID Width Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_PEFCAR_RO                                           0x00000434
#define AG_MG_REGS_SRIO_PEFCAR_RM                                           0x00000001

#define AG_MG_REGS_SRIO_PEFCAR_SRIO_PEFCAR_BO                               0
#define AG_MG_REGS_SRIO_PEFCAR_SRIO_PEFCAR_BM                               0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_PEFCAR_U
{
    struct
    {
        ag_mg_regs_register
            srio_pefcar : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_pefcar_u;
#endif


/*
 * SRIO1_MODE (sRIO 1 Mode Register)
 * Initialization value: 0x00000003  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO1_MODE_RO                                            0x00000438
#define AG_MG_REGS_SRIO1_MODE_RM                                            0x00000003

#define AG_MG_REGS_SRIO1_MODE_SRIO1_MODE_BO                                 0
#define AG_MG_REGS_SRIO1_MODE_SRIO1_MODE_BM                                 0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO1_MODE_U
{
    struct
    {
        ag_mg_regs_register
            srio1_mode : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio1_mode_u;
#endif


/*
 * SRIO_AI (sRIO Assembly Identity Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_AI_RO                                               0x0000043C
#define AG_MG_REGS_SRIO_AI_RM                                               0x0000FFFF

#define AG_MG_REGS_SRIO_AI_SRIO_AI_BO                                       0
#define AG_MG_REGS_SRIO_AI_SRIO_AI_BM                                       0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_AI_U
{
    struct
    {
        ag_mg_regs_register
            srio_ai : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_ai_u;
#endif


/*
 * SRIO_AVI (sRIO Assembly Vendor Identity)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_AVI_RO                                              0x00000440
#define AG_MG_REGS_SRIO_AVI_RM                                              0x0000FFFF

#define AG_MG_REGS_SRIO_AVI_SRIO_AVI_BO                                      0
#define AG_MG_REGS_SRIO_AVI_SRIO_AVI_BM                                      0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_AVI_U
{
    struct
    {
        ag_mg_regs_register
            srio_avi : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_avi_u;
#endif


/*
 * SRIO_AR (sRIO Assembly Revision Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_AR_RO                                               0x00000444
#define AG_MG_REGS_SRIO_AR_RM                                               0x0000FFFF

#define AG_MG_REGS_SRIO_AR_SRIO_AR_BO                                       0
#define AG_MG_REGS_SRIO_AR_SRIO_AR_BM                                       0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_AR_U
{
    struct
    {
        ag_mg_regs_register
            srio_ar : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_ar_u;
#endif


/*
 * SRIO0_PORT_NUM (sRIO 0 Port Number)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_AG_MG_REGS_SRIO0_PORT_RO                             0x00000448
#define AG_MG_REGS_AG_MG_REGS_SRIO0_PORT_RM                             0x0000000F

#define AG_MG_REGS_AG_MG_REGS_SRIO0_PORT_NUM_SRIO0_PORT_BO              0
#define AG_MG_REGS_AG_MG_REGS_SRIO0_PORT_NUM_SRIO0_PORT_BM              0x0000000F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO0_PORT_NUM_U
{
    struct
    {
        ag_mg_regs_register
            srio0_port_num : 4,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio0_port_num_u;
#endif


/*
 * SRIO1_PORT_NUM (sRIO 1 Port Number)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_AG_MG_REGS_SRIO1_PORT_RO                             0x0000044C
#define AG_MG_REGS_AG_MG_REGS_SRIO1_PORT_RM                             0x0000000F

#define AG_MG_REGS_AG_MG_REGS_SRIO1_PORT_NUM_SRIO1_PORT_BO              0
#define AG_MG_REGS_AG_MG_REGS_SRIO1_PORT_NUM_SRIO1_PORT_BM              0x0000000F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO1_PORT_NUM_U
{
    struct
    {
        ag_mg_regs_register
            srio1_port_num : 4,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio1_port_num_u;
#endif


/*
 * MATRIX_SECURE (Bus Matrix Secure Mode)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MATRIX_SECURE_RO                                         0x00000450
#define AG_MG_REGS_MATRIX_SECURE_RM                                         0x0001FFFF

#define AG_MG_REGS_MATRIX_SECURE_DSS0_DMI_BO            0
#define AG_MG_REGS_MATRIX_SECURE_DSS0_DMI_BM            0x00000001
#define AG_MG_REGS_MATRIX_SECURE_DSS1_DMI_BO            1
#define AG_MG_REGS_MATRIX_SECURE_DSS1_DMI_BM            0x00000002
#define AG_MG_REGS_MATRIX_SECURE_DSS2_DMI_BO            2
#define AG_MG_REGS_MATRIX_SECURE_DSS2_DMI_BM            0x00000004
#define AG_MG_REGS_MATRIX_SECURE_DSS3_DMI_BO            3
#define AG_MG_REGS_MATRIX_SECURE_DSS3_DMI_BM            0x00000008
#define AG_MG_REGS_MATRIX_SECURE_DDR3_DATA_BO           4
#define AG_MG_REGS_MATRIX_SECURE_DDR3_DATA_BM           0x00000010
#define AG_MG_REGS_MATRIX_SECURE_DDR3_CONFIG_BO         5
#define AG_MG_REGS_MATRIX_SECURE_DDR3_CONFIG_BM         0x00000020
#define AG_MG_REGS_MATRIX_SECURE_SRIO_0_DATA_BO         6
#define AG_MG_REGS_MATRIX_SECURE_SRIO_0_DATA_BM         0x00000040
#define AG_MG_REGS_MATRIX_SECURE_SRIO_1_DATA_BO         7
#define AG_MG_REGS_MATRIX_SECURE_SRIO_1_DATA_BM         0x00000080
#define AG_MG_REGS_MATRIX_SECURE_PCIE_DATA_BO           8
#define AG_MG_REGS_MATRIX_SECURE_PCIE_DATA_BM           0x00000100
#define AG_MG_REGS_MATRIX_SECURE_PCIE_CONFIG_BO         9
#define AG_MG_REGS_MATRIX_SECURE_PCIE_CONFIG_BM         0x00000200
#define AG_MG_REGS_MATRIX_SECURE_DDR3_PHY_BO            10
#define AG_MG_REGS_MATRIX_SECURE_DDR3_PHY_BM            0x00000400
#define AG_MG_REGS_MATRIX_SECURE_RESERVED_BO            11
#define AG_MG_REGS_MATRIX_SECURE_RESERVED_BM            0x00000800
#define AG_MG_REGS_MATRIX_SECURE_NAND_FLASH_BO          12
#define AG_MG_REGS_MATRIX_SECURE_NAND_FLASH_BM          0x00001000
#define AG_MG_REGS_MATRIX_SECURE_TRNG_BO                13
#define AG_MG_REGS_MATRIX_SECURE_TRNG_BM                0x00002000
#define AG_MG_REGS_MATRIX_SECURE_GBE_SERDES_BO          14
#define AG_MG_REGS_MATRIX_SECURE_GBE_SERDES_BM          0x00004000
#define AG_MG_REGS_MATRIX_SECURE_SRIO_SERDES_BO         15
#define AG_MG_REGS_MATRIX_SECURE_SRIO_SERDES_BM         0x00008000
#define AG_MG_REGS_MATRIX_SECURE_PCIE_SERDES_BO         16
#define AG_MG_REGS_MATRIX_SECURE_PCIE_SERDES_BM         0x00010000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MATRIX_SECURE_U
{
    struct
    {
        ag_mg_regs_register
            dss0_dmi : 1,
            dss1_dmi : 1,
            dss2_dmi : 1,
            dss3_dmi : 1,
            ddr3_data : 1,
            ddr3_config : 1,
            srio_0_data : 1,
            srio_1_data : 1,
            pcie_data : 1,
            pcie_config : 1,
            ddr3_phy : 1,
            fill1 : 1,
            nand_flash : 1,
            trng : 1,
            gbe_serdes : 1,
            srio_serdes : 1,
            pcie_serdes : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_matrix_secure_u;
#endif


/*
 * GIGE_SERDES_UPDATE (GIGE SerDes FIFO Mode Update Register)
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GIGE_SERDES_UPDATE_RO                                    0x00000454
#define AG_MG_REGS_GIGE_SERDES_UPDATE_RM                                    0x00000001

#define AG_MG_REGS_GIGE_SERDES_UPDATE_GIGE_SERDES_UPDATE_BO                 0
#define AG_MG_REGS_GIGE_SERDES_UPDATE_GIGE_SERDES_UPDATE_BM                 0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GIGE_SERDES_UPDATE_U
{
    struct
    {
        ag_mg_regs_register
            gige_serdes_update : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gige_serdes_update_u;
#endif


/*
 * SRIO_SERDES_UPDATE (sRIO SerDes FIFO Mode Update Register)
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_SERDES_UPDATE_RO                                    0x00000458
#define AG_MG_REGS_SRIO_SERDES_UPDATE_RM                                    0x00000001

#define AG_MG_REGS_SRIO_SERDES_UPDATE_SRIO_SERDES_UPDATE_BO                 0
#define AG_MG_REGS_SRIO_SERDES_UPDATE_SRIO_SERDES_UPDATE_BM                 0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_SERDES_UPDATE_U
{
    struct
    {
        ag_mg_regs_register
            srio_serdes_update : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_serdes_update_u;
#endif


/*
 * GIGE_TX_FIFO_SEL (GIGE Transmit FIFO Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GIGE_TX_FIFO_SEL_RO                                      0x0000045C
#define AG_MG_REGS_GIGE_TX_FIFO_SEL_RM                                      0x00000001

#define AG_MG_REGS_GIGE_TX_FIFO_SEL_GIGE_TX_FIFO_SEL_BO                     0
#define AG_MG_REGS_GIGE_TX_FIFO_SEL_GIGE_TX_FIFO_SEL_BM                     0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GIGE_TX_FIFO_SEL_U
{
    struct
    {
        ag_mg_regs_register
            gige_tx_fifo_sel : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gige_tx_fifo_sel_u;
#endif


/*
 * SRIO_TX_FIFO_SEL (sRIO Transmit FIFO Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_TX_FIFO_SEL_RO                                      0x00000460
#define AG_MG_REGS_SRIO_TX_FIFO_SEL_RM                                      0x00000001

#define AG_MG_REGS_SRIO_TX_FIFO_SEL_SRIO_TX_FIFO_SEL_BO                     0
#define AG_MG_REGS_SRIO_TX_FIFO_SEL_SRIO_TX_FIFO_SEL_BM                     0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_TX_FIFO_SEL_U
{
    struct
    {
        ag_mg_regs_register
            srio_tx_fifo_sel : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_tx_fifo_sel_u;
#endif


/*
 * SERDES_TRIMDONE (SERDES Trim Done Status Bit)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SERDES_TRIMDONE_RO                                       0x00000464
#define AG_MG_REGS_SERDES_TRIMDONE_RM                                       0x00000001

#define AG_MG_REGS_SERDES_TRIMDONE_SERDES_TRIMDONE_BO                       0
#define AG_MG_REGS_SERDES_TRIMDONE_SERDES_TRIMDONE_BM                       0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SERDES_TRIMDONE_U
{
    struct
    {
        ag_mg_regs_register
            serdes_trimdone : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_serdes_trimdone_u;
#endif


/*
 * UHD_DELAY (UHD Read Margin)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UHD_DELAY_RO                                             0x00000468
#define AG_MG_REGS_UHD_DELAY_RM                                             0x0000003F

#define AG_MG_REGS_UHD_DELAY_DBM_UHD_DELAY_BO                               0
#define AG_MG_REGS_UHD_DELAY_DBM_UHD_DELAY_BM                               0x00000003

#define AG_MG_REGS_UHD_DELAY_DSS_UHD_DELAY_BO                               2
#define AG_MG_REGS_UHD_DELAY_DSS_UHD_DELAY_BM                               0x0000000C

#define AG_MG_REGS_UHD_DELAY_PPB_UHD_DELAY_BO                               4
#define AG_MG_REGS_UHD_DELAY_PPB_UHD_DELAY_BM                               0x00000030

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UHD_DELAY_U
{
    struct
    {
        ag_mg_regs_register
            dbm_uhd_delay : 2,
            dss_uhd_delay : 2,
            ppb_uhd_delay : 2,
            fill0 : 26;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_uhd_delay_u;
#endif


/*
 * DSS_L2CACHE_EN (DSS L2 Cache Enable Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DSS_L2CACHE_EN_RO                                        0x0000046C
#define AG_MG_REGS_DSS_L2CACHE_EN_RM                                        0x0000000F

#define AG_MG_REGS_DSS_L2CACHE_EN_DSS0_L2CACHE_EN_BO                        0
#define AG_MG_REGS_DSS_L2CACHE_EN_DSS0_L2CACHE_EN_BM                        0x00000001

#define AG_MG_REGS_DSS_L2CACHE_EN_DSS1_L2CACHE_EN_BO                        1
#define AG_MG_REGS_DSS_L2CACHE_EN_DSS1_L2CACHE_EN_BM                        0x00000002

#define AG_MG_REGS_DSS_L2CACHE_EN_DSS2_L2CACHE_EN_BO                        2
#define AG_MG_REGS_DSS_L2CACHE_EN_DSS2_L2CACHE_EN_BM                        0x00000004

#define AG_MG_REGS_DSS_L2CACHE_EN_DSS3_L2CACHE_EN_BO                        3
#define AG_MG_REGS_DSS_L2CACHE_EN_DSS3_L2CACHE_EN_BM                        0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DSS_L2CACHE_EN_U
{
    struct
    {
        ag_mg_regs_register
            dss0_l2cache_en : 1,
            dss1_l2cache_en : 1,
            dss2_l2cache_en : 1,
            dss3_l2cache_en : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dss_l2cache_en_u;
#endif


/*
 * PEI_INT_GEN (PEI Response Interrupt Generation Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PEI_INT_GEN_RO                                           0x00000470
#define AG_MG_REGS_PEI_INT_GEN_RM                                           0x00000001

#define AG_MG_REGS_PEI_INT_GEN_PEI_INT_GEN_BO                               0
#define AG_MG_REGS_PEI_INT_GEN_PEI_INT_GEN_BM                               0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PEI_INT_GEN_U
{
    struct
    {
        ag_mg_regs_register
            pei_int_gen : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pei_int_gen_u;
#endif


/*
 * PEI_INT_MASK (PEI Response Interrupt Mask Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PEI_INT_MASK_RO                                          0x00000474
#define AG_MG_REGS_PEI_INT_MASK_RM                                          0x00000001

#define AG_MG_REGS_PEI_INT_MASK_PEI_INT_MASK_BO                             0
#define AG_MG_REGS_PEI_INT_MASK_PEI_INT_MASK_BM                             0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PEI_INT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            pei_int_mask : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pei_int_mask_u;
#endif


/*
 * PEI_INT_CLEAR (PEI Response Interrupt Clear Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PEI_INT_CLEAR_RO                                         0x00000478
#define AG_MG_REGS_PEI_INT_CLEAR_RM                                         0x00000001

#define AG_MG_REGS_PEI_INT_CLEAR_PEI_INT_CLEAR_BO                           0
#define AG_MG_REGS_PEI_INT_CLEAR_PEI_INT_CLEAR_BM                           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PEI_INT_CLEAR_U
{
    struct
    {
        ag_mg_regs_register
            pei_int_clear : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pei_int_clear_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GIGE_SERDES_PD_RO                                        0x00000480
#define AG_MG_REGS_GIGE_SERDES_PD_RM                                        0x00000003

#define AG_MG_REGS_GIGE_SERDES_PD_GIGE_PD_0_BO                              0
#define AG_MG_REGS_GIGE_SERDES_PD_GIGE_PD_0_BM                              0x00000001

#define AG_MG_REGS_GIGE_SERDES_PD_GIGE_PD_1_BO                              1
#define AG_MG_REGS_GIGE_SERDES_PD_GIGE_PD_1_BM                              0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GIGE_SERDES_PD_U
{
    struct
    {
        ag_mg_regs_register
            gige_pd_0 : 1,
            gige_pd_1 : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gige_serdes_pd_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GIGE_RX_LOS_SEL_RO                                       0x00000484
#define AG_MG_REGS_GIGE_RX_LOS_SEL_RM                                       0x00000003

#define AG_MG_REGS_GIGE_RX_LOS_SEL_GIGE_RX_LOS_SEL_BO                       0
#define AG_MG_REGS_GIGE_RX_LOS_SEL_GIGE_RX_LOS_SEL_BM                       0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GIGE_RX_LOS_SEL_U
{
    struct
    {
        ag_mg_regs_register
            gige_rx_los_sel : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gige_rx_los_sel_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SRIO_RX_LOS_SEL_RO                                       0x00000488
#define AG_MG_REGS_SRIO_RX_LOS_SEL_RM                                       0x00000003

#define AG_MG_REGS_SRIO_RX_LOS_SEL_SRIO_RX_LOS_SEL_BO                       0
#define AG_MG_REGS_SRIO_RX_LOS_SEL_SRIO_RX_LOS_SEL_BM                       0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SRIO_RX_LOS_SEL_U
{
    struct
    {
        ag_mg_regs_register
            srio_rx_los_sel : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_srio_rx_los_sel_u;
#endif



/*
* Physical register addresses (for ARM accessing DBM CAR)
*/
/*
 * Physical register addresses (for ARM or DSS accessing car)
 */
#define AG_MG_REGS_CAR_BASE		0x98012000
#define AG_MG_REGS_CAR_REG(ro)		(AG_MG_REGS_CAR_BASE+(ro))

#define AG_MG_REGS_CAR_PLL1CTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PLL1CTL_RO)
#define AG_MG_REGS_CAR_PLL1SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PLL1SEL_RO)
#define AG_MG_REGS_CAR_PLL2CTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PLL2CTL_RO)
#define AG_MG_REGS_CAR_CLKCTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CLKCTL_RO)
#define AG_MG_REGS_CAR_CLKOUT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CLKOUT_RO)
#define AG_MG_REGS_CAR_DSSEN_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DSSEN_RO)
#define AG_MG_REGS_CAR_CHIP_WAKE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CHIP_WAKE_RO)
#define AG_MG_REGS_CAR_TDM0IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM0IO_RO)
#define AG_MG_REGS_CAR_TDM1IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM1IO_RO)
#define AG_MG_REGS_CAR_TDM2IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM2IO_RO)
#define AG_MG_REGS_CAR_TDM3IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM3IO_RO)
#define AG_MG_REGS_CAR_TDM4IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM4IO_RO)
#define AG_MG_REGS_CAR_TDM5IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TDM5IO_RO)
#define AG_MG_REGS_CAR_SSSMII0IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SSSMII0IO_RO)
#define AG_MG_REGS_CAR_SSSMII1IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SSSMII1IO_RO)
#define AG_MG_REGS_CAR_SSPIO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SSPIO_RO)
#define AG_MG_REGS_CAR_GPIOIO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_GPIOIO_RO)
#define AG_MG_REGS_CAR_CLOCKSIO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CLOCKSIO_RO)
#define AG_MG_REGS_CAR_DDR3IO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3IO_RO)
#define AG_MG_REGS_CAR_FLASHIO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_FLASHIO_RO)
#define AG_MG_REGS_CAR_DDR3_SWAP_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_SWAP_RO)
#define AG_MG_REGS_CAR_DDR3_KILL_FORCE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_KILL_FORCE_RO)
#define AG_MG_REGS_CAR_SERDESIO_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SERDESIO_RO)
#define AG_MG_REGS_CAR_RSTPROT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RSTPROT_RO)
#define AG_MG_REGS_CAR_RESSTAT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RESSTAT_RO)
#define AG_MG_REGS_CAR_RSTCTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RSTCTL_RO)
#define AG_MG_REGS_CAR_CHIPID_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CHIPID_RO)
#define AG_MG_REGS_CAR_DEVICEID_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DEVICEID_RO)
#define AG_MG_REGS_CAR_HOST_RESET_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_HOST_RESET_RO)
#define AG_MG_REGS_CAR_ARMRSTCTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_ARMRSTCTL_RO)
#define AG_MG_REGS_CAR_CHIP_RESET_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_CHIP_RESET_RO)
#define AG_MG_REGS_CAR_PFUSE_RA(p)	AG_MG_REGS_CAR_REG(AG_MG_REGS_PFUSE0_RO+((p)* 4))
#define AG_MG_REGS_CAR_DSS_RPR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DSS_RPR_RO)
#define AG_MG_REGS_CAR_ARM_RPR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_ARM_RPR_RO)
#define AG_MG_REGS_CAR_IROM_RPR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_IROM_RPR_RO)
#define AG_MG_REGS_CAR_SYS_RPR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SYS_RPR_RO)
#define AG_MG_REGS_CAR_PPB_RPR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PPB_RPR_RO)
#define AG_MG_REGS_CAR_RPR_RST_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RPR_RST_RO)
#define AG_MG_REGS_CAR_DEBUG_TRIG_CTRL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DEBUG_TRIG_CTRL_RO)
#define AG_MG_REGS_CAR_DDR3_STAT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_STAT_RO)
#define AG_MG_REGS_CAR_DDR3_REFRESH_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_REFRESH_RO)
#define AG_MG_REGS_CAR_FEED_FORWARD_STAT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_FEED_FORWARD_STAT_RO)
#define AG_MG_REGS_CAR_FEED_FORWARD_KEY_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_FEED_FORWARD_KEY_RO)
#define AG_MG_REGS_CAR_MBIST_DSS_STAT0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_DSS_STAT0_RO)
#define AG_MG_REGS_CAR_MBIST_DSS_STAT1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_DSS_STAT1_RO)
#define AG_MG_REGS_CAR_MBIST_ARM_STAT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_ARM_STAT_RO)
#define AG_MG_REGS_CAR_MBIST_SYS_STAT0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_SYS_STAT0_RO)
#define AG_MG_REGS_CAR_MBIST_SYS_STAT1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_SYS_STAT1_RO)
#define AG_MG_REGS_CAR_MBIST_PPB_STAT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MBIST_PPB_STAT_RO)
#define AG_MG_REGS_CAR_SPEEDSTART_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDSTART_RO)
#define AG_MG_REGS_CAR_SPEEDDUR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDUR_RO)
#define AG_MG_REGS_CAR_SPEEDDATA0H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA0H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA0S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA0S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA0L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA0L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA1H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA1H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA1S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA1S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA1L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA1L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA2H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA2H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA2S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA2S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA2L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA2L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA3H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA3H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA3S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA3S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA3L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA3L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA4H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA4H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA4S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA4S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA4L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA4L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA5H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA5H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA5S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA5S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA5L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA5L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA6H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA6H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA6S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA6S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA6L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA6L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA7H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA7H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA7S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA7S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA7L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA7L_RO)
#define AG_MG_REGS_CAR_SPEEDDATA8H_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA8H_RO)
#define AG_MG_REGS_CAR_SPEEDDATA8S_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA8S_RO)
#define AG_MG_REGS_CAR_SPEEDDATA8L_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SPEEDDATA8L_RO)
#define AG_MG_REGS_CAR_TEMPDATA0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA0_RO)
#define AG_MG_REGS_CAR_TEMPDATA1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA1_RO)
#define AG_MG_REGS_CAR_TEMPDATA2_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA2_RO)
#define AG_MG_REGS_CAR_TEMPDATA3_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA3_RO)
#define AG_MG_REGS_CAR_TEMPDATA4_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA4_RO)
#define AG_MG_REGS_CAR_TEMPDATA5_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA5_RO)
#define AG_MG_REGS_CAR_TEMPDATA6_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA6_RO)
#define AG_MG_REGS_CAR_TEMPDATA7_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA7_RO)
#define AG_MG_REGS_CAR_TEMPDATA8_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TEMPDATA8_RO)
#define AG_MG_REGS_CAR_VOLTDATA0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA0_RO)
#define AG_MG_REGS_CAR_VOLTDATA1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA1_RO)
#define AG_MG_REGS_CAR_VOLTDATA2_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA2_RO)
#define AG_MG_REGS_CAR_VOLTDATA3_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA3_RO)
#define AG_MG_REGS_CAR_VOLTDATA4_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA4_RO)
#define AG_MG_REGS_CAR_VOLTDATA5_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA5_RO)
#define AG_MG_REGS_CAR_VOLTDATA6_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA6_RO)
#define AG_MG_REGS_CAR_VOLTDATA7_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA7_RO)
#define AG_MG_REGS_CAR_VOLTDATA8_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_VOLTDATA8_RO)
#define AG_MG_REGS_CAR_RTC0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RTC0_RO)
#define AG_MG_REGS_CAR_RTC1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RTC1_RO)
#define AG_MG_REGS_CAR_RTCCTL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_RTCCTL_RO)
#define AG_MG_REGS_CAR_TURBOMODE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TURBOMODE_RO)
#define AG_MG_REGS_CAR_TURBOSELECT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TURBOSELECT_RO)
#define AG_MG_REGS_CAR_FBCNT_END_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_FBCNT_END_RO)
#define AG_MG_REGS_CAR_TURBOFORCE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_TURBOFORCE_RO)
#define AG_MG_REGS_CAR_PCIE_RC_EP_N_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PCIE_RC_EP_N_RO)
#define AG_MG_REGS_CAR_SRIO_LANE_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_LANE_SEL_RO)
#define AG_MG_REGS_CAR_PCIE_TX_FIFOEN_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PCIE_TX_FIFOEN_RO)
#define AG_MG_REGS_CAR_SRIO0_MODE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO0_MODE_RO)
#define AG_MG_REGS_CAR_SRIO_PEFCAR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_PEFCAR_RO)
#define AG_MG_REGS_CAR_SRIO1_MODE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO1_MODE_RO)
#define AG_MG_REGS_CAR_SRIO_AI_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_AI_RO)
#define AG_MG_REGS_CAR_SRIO_AVI_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_AVI_RO)
#define AG_MG_REGS_CAR_SRIO_AR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_AR_RO)
#define AG_MG_REGS_CAR_SRIO0_PORT_NUM_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO0_PORT_NUM_RO)
#define AG_MG_REGS_CAR_SRIO1_PORT_NUM_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO1_PORT_NUM_RO)
#define AG_MG_REGS_CAR_MATRIX_SECURE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_MATRIX_SECURE_RO)
#define AG_MG_REGS_CAR_GIGE_SERDES_UPDATE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_GIGE_SERDES_UPDATE_RO)
#define AG_MG_REGS_CAR_SRIO_SERDES_UPDATE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_SERDES_UPDATE_RO)
#define AG_MG_REGS_CAR_GIGE_TX_FIFO_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_GIGE_TX_FIFO_SEL_RO)
#define AG_MG_REGS_CAR_SRIO_TX_FIFO_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_TX_FIFO_SEL_RO)
#define AG_MG_REGS_CAR_SERDES_TRIMDONE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SERDES_TRIMDONE_RO)
#define AG_MG_REGS_CAR_UHD_DELAY_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_UHD_DELAY_RO)
#define AG_MG_REGS_CAR_DSS_L2CACHE_EN_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DSS_L2CACHE_EN_RO)
#define AG_MG_REGS_CAR_PEI_INT_GEN_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PEI_INT_GEN_RO)
#define AG_MG_REGS_CAR_PEI_INT_MASK_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PEI_INT_MASK_RO)
#define AG_MG_REGS_CAR_PEI_INT_CLEAR_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_PEI_INT_CLEAR_RO)
#define AG_MG_REGS_CAR_GIGE_SERDES_PD_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_GIGE_SERDES_PD_RO)
#define AG_MG_REGS_CAR_GIGE_RX_LOS_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_GIGE_RX_LOS_SEL_RO)
#define AG_MG_REGS_CAR_SRIO_RX_LOS_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_SRIO_RX_LOS_SEL_RO)
#define AG_MG_REGS_CAR_DDR3_RUN_BIST_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_RUN_BIST_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_CLK_GATE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_CLK_GATE_RO)
#define AG_MG_REGS_CAR_DDR3_BIT_PULSE_WIDTH_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIT_PULSE_WIDTH_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_CAPTURE_WAIT_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_CAPTURE_WAIT_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_HM_SEL_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_HM_SEL_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_DONE_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_DONE_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_HM_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_HM_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_DP_BIT0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_DP_BIT0_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_DP_BIT1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_DP_BIT1_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_DP_BIT2_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_DP_BIT2_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_ADR_BIT0_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_ADR_BIT0_RO)
#define AG_MG_REGS_CAR_DDR3_BIST_PASSED_ADR_BIT1_RA	AG_MG_REGS_CAR_REG(AG_MG_REGS_DDR3_BIST_PASSED_ADR_BIT1_RO)

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_CAR_REGS_S
{
	ag_mg_regs_pll1ctl_u	pll1ctl ;
	ag_mg_regs_pll1sel_u	pll1sel ;
	ag_mg_regs_pll2ctl_u	pll2ctl ;
	ag_mg_regs_register	FILL0 ;
	ag_mg_regs_clkctl_u	clkctl ;
	ag_mg_regs_clkout_u	clkout ;
	ag_mg_regs_dssen_u	dssen ;
	ag_mg_regs_chip_wake_u	chip_wake ;
	ag_mg_regs_tdm0io_u	tdm0io ;
	ag_mg_regs_tdm1io_u	tdm1io ;
	ag_mg_regs_tdm2io_u	tdm2io ;
	ag_mg_regs_tdm3io_u	tdm3io ;
	ag_mg_regs_tdm4io_u	tdm4io ;
	ag_mg_regs_tdm5io_u	tdm5io ;
	ag_mg_regs_sssmii0io_u	sssmii0io ;
	ag_mg_regs_sssmii1io_u	sssmii1io ;
	ag_mg_regs_sspio_u	sspio ;
	ag_mg_regs_gpioio_u	gpioio ;
	ag_mg_regs_clocksio_u	clocksio ;
	ag_mg_regs_ddr3io_u	ddr3io ;
	ag_mg_regs_flashio_u	flashio ;
	ag_mg_regs_serdesio_u	serdesio ;
	ag_mg_regs_ddr3_swap_u 	ddr3_swap;
	ag_mg_regs_ddr3_kill_force_u ddr3_kill_force;
	ag_mg_regs_rstprot_u	rstprot ;
	ag_mg_regs_resstat_u	resstat ;
	ag_mg_regs_rstctl_u	rstctl ;
	ag_mg_regs_chipid_u	chipid ;
	ag_mg_regs_deviceid_u	deviceid ;
	ag_mg_regs_host_reset_u	host_reset ;
	ag_mg_regs_armrstctl_u	armrstctl ;
	ag_mg_regs_chip_reset_u	chip_reset ;
	ag_mg_regs_pfuse0_u	pfuse[128] ;
	ag_mg_regs_register	FILL2[32] ;
	ag_mg_regs_speedstart_u	speedstart ;
	ag_mg_regs_speeddur_u	speeddur ;
	ag_mg_regs_register	FILL3[2] ;
	ag_mg_regs_speeddata0h_u	speeddata0h ;
	ag_mg_regs_speeddata0s_u	speeddata0s ;
	ag_mg_regs_speeddata0l_u	speeddata0l ;
	ag_mg_regs_speeddata1h_u	speeddata1h ;
	ag_mg_regs_speeddata1s_u	speeddata1s ;
	ag_mg_regs_speeddata1l_u	speeddata1l ;
	ag_mg_regs_speeddata2h_u	speeddata2h ;
	ag_mg_regs_speeddata2s_u	speeddata2s ;
	ag_mg_regs_speeddata2l_u	speeddata2l ;
	ag_mg_regs_speeddata3h_u	speeddata3h ;
	ag_mg_regs_speeddata3s_u	speeddata3s ;
	ag_mg_regs_speeddata3l_u	speeddata3l ;
	ag_mg_regs_speeddata4h_u	speeddata4h ;
	ag_mg_regs_speeddata4s_u	speeddata4s ;
	ag_mg_regs_speeddata4l_u	speeddata4l ;
	ag_mg_regs_speeddata5h_u	speeddata5h ;
	ag_mg_regs_speeddata5s_u	speeddata5s ;
	ag_mg_regs_speeddata5l_u	speeddata5l ;
	ag_mg_regs_speeddata6h_u	speeddata6h ;
	ag_mg_regs_speeddata6s_u	speeddata6s ;
	ag_mg_regs_speeddata6l_u	speeddata6l ;
	ag_mg_regs_speeddata7h_u	speeddata7h ;
	ag_mg_regs_speeddata7s_u	speeddata7s ;
	ag_mg_regs_speeddata7l_u	speeddata7l ;
	ag_mg_regs_speeddata8h_u	speeddata8h ;
	ag_mg_regs_speeddata8s_u	speeddata8s ;
	ag_mg_regs_speeddata8l_u	speeddata8l ;
	ag_mg_regs_tempdata0_u	tempdata0 ;
	ag_mg_regs_tempdata1_u	tempdata1 ;
	ag_mg_regs_tempdata2_u	tempdata2 ;
	ag_mg_regs_tempdata3_u	tempdata3 ;
	ag_mg_regs_tempdata4_u	tempdata4 ;
	ag_mg_regs_tempdata5_u	tempdata5 ;
	ag_mg_regs_tempdata6_u	tempdata6 ;
	ag_mg_regs_tempdata7_u	tempdata7 ;
	ag_mg_regs_tempdata8_u	tempdata8 ;
	ag_mg_regs_voltdata0_u	voltdata0 ;
	ag_mg_regs_voltdata1_u	voltdata1 ;
	ag_mg_regs_voltdata2_u	voltdata2 ;
	ag_mg_regs_voltdata3_u	voltdata3 ;
	ag_mg_regs_voltdata4_u	voltdata4 ;
	ag_mg_regs_voltdata5_u	voltdata5 ;
	ag_mg_regs_voltdata6_u	voltdata6 ;
	ag_mg_regs_voltdata7_u	voltdata7 ;
	ag_mg_regs_voltdata8_u	voltdata8 ;
	ag_mg_regs_register	FILL4[15] ;
	ag_mg_regs_rtc0_u	rtc0 ;
	ag_mg_regs_rtc1_u	rtc1 ;
	ag_mg_regs_rtcctl_u	rtcctl ;
	ag_mg_regs_register	FILL5 ;
	ag_mg_regs_turbomode_u	turbomode ;
	ag_mg_regs_turboselect_u	turboselect ;
	ag_mg_regs_fbcnt_end_u	fbcnt_end ;
	ag_mg_regs_turboforce_u	turboforce ;
	ag_mg_regs_pcie_rc_ep_n_u	pcie_rc_ep_n ;
	ag_mg_regs_srio_lane_sel_u	srio_lane_sel ;
	ag_mg_regs_pcie_tx_fifoen_u	pcie_tx_fifoen ;
	ag_mg_regs_register FILL6 ;
	ag_mg_regs_srio0_mode_u	srio0_mode ;
	ag_mg_regs_srio_pefcar_u	srio_pefcar ;
	ag_mg_regs_srio1_mode_u	srio1_mode ;
	ag_mg_regs_srio_ai_u	srio_ai ;
	ag_mg_regs_srio_avi_u	srio_avi ;
	ag_mg_regs_srio_ar_u	srio_ar ;
	ag_mg_regs_srio0_port_num_u	srio0_port_num ;
	ag_mg_regs_srio1_port_num_u	srio1_port_num ;
	ag_mg_regs_matrix_secure_u	matrix_secure ;
	ag_mg_regs_gige_serdes_update_u	gige_serdes_update ;
	ag_mg_regs_srio_serdes_update_u	srio_serdes_update ;
	ag_mg_regs_gige_tx_fifo_sel_u	gige_tx_fifo_sel ;
	ag_mg_regs_srio_tx_fifo_sel_u	srio_tx_fifo_sel ;
	ag_mg_regs_serdes_trimdone_u	serdes_trimdone ;
	ag_mg_regs_uhd_delay_u	uhd_delay ;
	ag_mg_regs_dss_l2cache_en_u	dss_l2cache_en ;
	ag_mg_regs_pei_int_gen_u	pei_int_gen ;
	ag_mg_regs_pei_int_mask_u	pei_int_mask ;
	ag_mg_regs_pei_int_clear_u	pei_int_clear ;
	ag_mg_regs_register		FILL7 ;
	ag_mg_regs_gige_serdes_pd_u	gige_serdes_pd ;
	ag_mg_regs_gige_rx_los_sel_u	gige_rx_los_sel ;
	ag_mg_regs_srio_rx_los_sel_u	srio_rx_los_sel ;
	ag_mg_regs_register		FILL8[29] ;
	ag_mg_regs_dss_rpr_u	dss_rpr ;
	ag_mg_regs_arm_rpr_u	arm_rpr ;
	ag_mg_regs_irom_rpr_u	irom_rpr ;
	ag_mg_regs_sys_rpr_u	sys_rpr ;
	ag_mg_regs_ppb_rpr_u	ppb_rpr ;
	ag_mg_regs_rpr_rst_u	rpr_rst ;
	ag_mg_regs_debug_trig_ctrl_u	debug_trig_ctrl ;
	ag_mg_regs_ddr3_stat_u	ddr3_stat ;
	ag_mg_regs_ddr3_refresh_u	ddr3_refresh ;
	ag_mg_regs_register		FILL9 ;
	ag_mg_regs_feed_forward_stat_u	feed_forward_stat ;
	ag_mg_regs_feed_forward_key_u	feed_forward_key ;
	ag_mg_regs_mbist_dss_stat0_u	mbist_dss_stat0 ;
	ag_mg_regs_mbist_dss_stat1_u	mbist_dss_stat1 ;
	ag_mg_regs_mbist_arm_stat_u	mbist_arm_stat ;
	ag_mg_regs_mbist_sys_stat0_u	mbist_sys_stat0 ;
	ag_mg_regs_mbist_sys_stat1_u	mbist_sys_stat1 ;
	ag_mg_regs_mbist_ppb_stat_u	mbist_ppb_stat ;
} ag_mg_regs_car_reg_s ;

/*
* Recommended C syntax for typical usage :
*   volatile ag_mg_regs_car_reg_s *car_regs =
*       (volatile ag_mg_regs_car_reg_s *)(AG_MG_REGS_CAR_BASE);
*/
#endif

#endif

/******** History ********
$Log: ag_mg_regs_car.h,v $
Revision 1.1  2012/04/18 18:08:25  srane
Initial checkin


$Endlog$
*/

