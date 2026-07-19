/* $Id: ag_mg_regs_ddr3.h,v 1.2 2017/07/28 07:58:34 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg_regs_ddr3.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_ddr3.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_ddr3.h
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
 * Definitions for accessing the registers of the DDR3 Memory Controller.
 * NOTE: the DDR3 PLL (i.e. PLL2) must be running before accessing these registers.
 *
 * Typically, initial values for these registers are provided by LSI based on the
 * DDR3 devices connected to the SP2700 family processor and the board layout.
 * Please contact your LSI representative for more information.
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *  DENALI_CTL_00
 *  DENALI_CTL_01
 *  DENALI_CTL_02
 *  DENALI_CTL_03
 *  DENALI_CTL_04
 *  DENALI_CTL_05
 *  DENALI_CTL_06
 *  DENALI_CTL_07
 *  DENALI_CTL_08
 *  DENALI_CTL_09
 *  DENALI_CTL_10
 *  DENALI_CTL_11
 *  DENALI_CTL_12
 *  DENALI_CTL_13
 *  DENALI_CTL_14
 *  DENALI_CTL_15
 *  DENALI_CTL_16
 *  DENALI_CTL_17
 *  DENALI_CTL_18
 *  DENALI_CTL_19
 *  DENALI_CTL_20
 *  DENALI_CTL_21
 *  DENALI_CTL_22
 *  DENALI_CTL_23
 *  DENALI_CTL_24
 *  DENALI_CTL_25
 *  DENALI_CTL_26
 *  DENALI_CTL_27
 *  DENALI_CTL_28
 *  DENALI_CTL_29
 *  DENALI_CTL_30
 *  DENALI_CTL_31
 *  DENALI_CTL_32
 *  DENALI_CTL_33
 *  DENALI_CTL_34
 *  DENALI_CTL_35
 *  DENALI_CTL_36
 *  DENALI_CTL_37
 *  DENALI_CTL_38
 *  DENALI_CTL_39
 *  DENALI_CTL_40
 *  DENALI_CTL_41
 *  DENALI_CTL_42
 *  DENALI_CTL_43
 *  DENALI_CTL_44
 *  DENALI_CTL_45
 *  DENALI_CTL_46
 *  DENALI_CTL_47
 *  DENALI_CTL_48
 *  DENALI_CTL_49
 *  DENALI_CTL_50
 *  DENALI_CTL_51
 *  DENALI_CTL_52
 *  DENALI_CTL_53
 *  DENALI_CTL_54
 *  DENALI_CTL_55
 *  DENALI_CTL_56
 *  DENALI_CTL_57
 *  DENALI_CTL_58
 *  DENALI_CTL_59
 *  DENALI_CTL_60
 *  DENALI_CTL_61
 *  DENALI_CTL_62
 *  DENALI_CTL_63
 *  DENALI_CTL_64
 *  DENALI_CTL_65
 *  DENALI_CTL_66
 *  DENALI_CTL_67
 *  DENALI_CTL_68
 *  DENALI_CTL_69
 *  DENALI_CTL_70
 *  DENALI_CTL_71
 *  DENALI_CTL_72
 *  DENALI_CTL_73
 *  DENALI_CTL_74
 *  DENALI_CTL_75
 *  DENALI_CTL_76
 *  DENALI_CTL_77
 *  DENALI_CTL_78
 *  DENALI_CTL_79
 *  DENALI_CTL_80
 *  DENALI_CTL_81
 *  DENALI_CTL_82
 *  DENALI_CTL_83
 *  DENALI_CTL_84
 *  DENALI_CTL_85
 *  DENALI_CTL_86
 *  DENALI_CTL_87
 *  DENALI_CTL_88
 *  DENALI_CTL_89
 *  DENALI_CTL_90
 *  DENALI_CTL_91
 *  DENALI_CTL_92
 *  DENALI_CTL_93
 *  DENALI_CTL_94
 *  DENALI_CTL_95
 *  DENALI_CTL_96
 *  DENALI_CTL_97
 *  DENALI_CTL_98
 *  DENALI_CTL_99
 *  DENALI_CTL_100
 *  DENALI_CTL_101
 *  DENALI_CTL_102
 *  DENALI_CTL_103
 *  DENALI_CTL_104
 *  DENALI_CTL_105
 *  DENALI_CTL_106
 *  DENALI_CTL_107
 *  DENALI_CTL_108
 *  DENALI_CTL_109
 *  DENALI_CTL_110
 *  DENALI_CTL_111
 *  DENALI_CTL_112
 *  DENALI_CTL_113
 *  DENALI_CTL_114
 *  DENALI_CTL_115
 *  DENALI_CTL_116
 *  DENALI_CTL_117
 *  DENALI_CTL_118
 *  DENALI_CTL_119
 *  DENALI_CTL_120
 *  DENALI_CTL_121
 *  DENALI_CTL_122
 *  DENALI_CTL_123
 *  DENALI_CTL_124
 *  DENALI_CTL_125
 *  DENALI_CTL_126
 *  DENALI_CTL_127
 *  DENALI_CTL_128
 *  DENALI_CTL_129
 *  DENALI_CTL_130
 *  DENALI_CTL_131
 *  DENALI_CTL_132
 *  DENALI_CTL_133
 *  DENALI_CTL_134
 *  DENALI_CTL_135
 *  DENALI_CTL_136
 *  DENALI_CTL_137
 *  DENALI_CTL_138
 *  DENALI_CTL_139
 *  DENALI_CTL_140
 *  DENALI_CTL_141
 *  DENALI_CTL_142
 *  DENALI_CTL_143
 *  DENALI_CTL_144
 *  DENALI_CTL_145
 *  DENALI_CTL_146
 *  DENALI_CTL_147
 *  DENALI_CTL_148
 *  DENALI_CTL_149
 *  DENALI_CTL_150
 *  DENALI_CTL_151
 *  DENALI_CTL_152
 *  DENALI_CTL_153
 *  DENALI_CTL_154
 *  DENALI_CTL_155
 *  DENALI_CTL_156
 *  DENALI_CTL_157
 *  DENALI_CTL_158
 *  DENALI_CTL_159
 *  DENALI_CTL_160
 *  DENALI_CTL_161
 *  DENALI_CTL_162
 *  DENALI_CTL_163
 *  DENALI_CTL_164
 *  DENALI_CTL_165
 *  DENALI_CTL_166
 *  DENALI_CTL_167
 *  DENALI_CTL_168
 *  DENALI_CTL_169
 *  DENALI_CTL_170
 *  DENALI_CTL_171
 *  DENALI_CTL_172
 *  DENALI_CTL_173
 *  DENALI_CTL_174
 *  DENALI_CTL_175
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

#ifndef AG_MG_REGS_DDR3_REGISTERS_H
#define AG_MG_REGS_DDR3_REGISTERS_H

#include "ag_mg_regs_regops.h"


/* 
 * Generated by HSI Designer release 2.3.5.
 */





/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_00_RO                                 0x00000000
#define AG_MG_REGS_DENALI_CTL_00_RM                                 0x010101FF

#define AG_MG_REGS_DENALI_CTL_00_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_00_OBSOLETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_00_ADDR_CMP_EN_BO                     8
#define AG_MG_REGS_DENALI_CTL_00_ADDR_CMP_EN_BM                     0x00000100

#define AG_MG_REGS_DENALI_CTL_00_AP_BO                              16
#define AG_MG_REGS_DENALI_CTL_00_AP_BM                              0x00010000

#define AG_MG_REGS_DENALI_CTL_00_AREFRESH_BO                        24
#define AG_MG_REGS_DENALI_CTL_00_AREFRESH_BM                        0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_00_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 8,
            addr_cmp_en : 1,
            fill2 : 7,
            ap : 1,
            fill1 : 7,
            arefresh : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_00_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_01_RO                                 0x00000004
#define AG_MG_REGS_DENALI_CTL_01_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_01_AUTO_REFRESH_MODE_BO               0
#define AG_MG_REGS_DENALI_CTL_01_AUTO_REFRESH_MODE_BM               0x00000001

#define AG_MG_REGS_DENALI_CTL_01_AXI_ALIGNED_STROBE_DISABLE_BO      8
#define AG_MG_REGS_DENALI_CTL_01_AXI_ALIGNED_STROBE_DISABLE_BM      0x00000100

#define AG_MG_REGS_DENALI_CTL_01_BANK_SPLIT_EN_BO                   16
#define AG_MG_REGS_DENALI_CTL_01_BANK_SPLIT_EN_BM                   0x00010000

#define AG_MG_REGS_DENALI_CTL_01_BIST_ADDR_CHECK_BO                 24
#define AG_MG_REGS_DENALI_CTL_01_BIST_ADDR_CHECK_BM                 0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_01_U
{
    struct
    {
        ag_mg_regs_register
            auto_refresh_mode : 1,
            fill3 : 7,
            axi_aligned_strobe_disable : 1,
            fill2 : 7,
            bank_split_en : 1,
            fill1 : 7,
            bist_addr_check : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_01_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_02_RO                                 0x00000008
#define AG_MG_REGS_DENALI_CTL_02_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_02_BIST_DATA_CHECK_BO                 0
#define AG_MG_REGS_DENALI_CTL_02_BIST_DATA_CHECK_BM                 0x00000001

#define AG_MG_REGS_DENALI_CTL_02_BIST_GO_BO                         8
#define AG_MG_REGS_DENALI_CTL_02_BIST_GO_BM                         0x00000100

#define AG_MG_REGS_DENALI_CTL_02_CONCURRENTAP_BO                    16
#define AG_MG_REGS_DENALI_CTL_02_CONCURRENTAP_BM                    0x00010000

#define AG_MG_REGS_DENALI_CTL_02_DLLLOCKREGV1_0_BO                  24
#define AG_MG_REGS_DENALI_CTL_02_DLLLOCKREGV1_0_BM                  0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_02_U
{
    struct
    {
        ag_mg_regs_register
            bist_data_check : 1,
            fill3 : 7,
            bist_go : 1,
            fill2 : 7,
            concurrentap : 1,
            fill1 : 7,
            dlllockregV1_0 : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_02_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_03_RO                                 0x0000000C
#define AG_MG_REGS_DENALI_CTL_03_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_03_DLL_BYPASS_MODEV1_0_BO             0
#define AG_MG_REGS_DENALI_CTL_03_DLL_BYPASS_MODEV1_0_BM             0x00000001

#define AG_MG_REGS_DENALI_CTL_03_DQS_N_ENV1_0_BO                    8
#define AG_MG_REGS_DENALI_CTL_03_DQS_N_ENV1_0_BM                    0x00000100

#define AG_MG_REGS_DENALI_CTL_03_DRIVE_DQ_DQSV1_0_BO                16
#define AG_MG_REGS_DENALI_CTL_03_DRIVE_DQ_DQSV1_0_BM                0x00010000

#define AG_MG_REGS_DENALI_CTL_03_ECC_DISABLE_W_UC_ERR_BO            24
#define AG_MG_REGS_DENALI_CTL_03_ECC_DISABLE_W_UC_ERR_BM            0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_03_U
{
    struct
    {
        ag_mg_regs_register
            dll_bypass_modeV1_0 : 1,
            fill3 : 7,
            dqs_n_enV1_0 : 1,
            fill2 : 7,
            drive_dq_dqsV1_0 : 1,
            fill1 : 7,
            ecc_disable_w_uc_err : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_03_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_04_RO                                 0x00000010
#define AG_MG_REGS_DENALI_CTL_04_RM                                 0x01FF0101

#define AG_MG_REGS_DENALI_CTL_04_EIGHT_BANK_MODE_BO                 0
#define AG_MG_REGS_DENALI_CTL_04_EIGHT_BANK_MODE_BM                 0x00000001

#define AG_MG_REGS_DENALI_CTL_04_ENABLE_QUICK_SREFRESH_BO           8
#define AG_MG_REGS_DENALI_CTL_04_ENABLE_QUICK_SREFRESH_BM           0x00000100

#define AG_MG_REGS_DENALI_CTL_04_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_04_OBSOLETE_BM                        0x00FF0000

#define AG_MG_REGS_DENALI_CTL_04_FWC_BO                             24
#define AG_MG_REGS_DENALI_CTL_04_FWC_BM                             0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_04_U
{
    struct
    {
        ag_mg_regs_register
            eight_bank_mode : 1,
            fill2 : 7,
            enable_quick_srefresh : 1,
            fill1 : 7,
            obsolete : 8,
            fwc : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_04_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_05_RO                                 0x00000014
#define AG_MG_REGS_DENALI_CTL_05_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_05_INTRPTAPBURSTV1_0_BO                   0
#define AG_MG_REGS_DENALI_CTL_05_INTRPTAPBURSTV1_0_BM                   0x00000001

#define AG_MG_REGS_DENALI_CTL_05_INTRPTREADAV1_0_BO                     8
#define AG_MG_REGS_DENALI_CTL_05_INTRPTREADAV1_0_BM                     0x00000100

#define AG_MG_REGS_DENALI_CTL_05_INTRPTWRITEAV1_0_BO                    16
#define AG_MG_REGS_DENALI_CTL_05_INTRPTWRITEAV1_0_BM                    0x00010000

#define AG_MG_REGS_DENALI_CTL_05_NO_CMD_INIT_BO                     24
#define AG_MG_REGS_DENALI_CTL_05_NO_CMD_INIT_BM                     0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_05_U
{
    struct
    {
        ag_mg_regs_register
            intrptapburstV1_0 : 1,
            fill3 : 7,
           intrptreadaV1_0 : 1,
            fill2 : 7,
            intrptwriteaV1_0 : 1,
            no_cmd_init : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_05_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_06_RO                                 0x00000018
#define AG_MG_REGS_DENALI_CTL_06_RM                                 0x010101FF

#define AG_MG_REGS_DENALI_CTL_06_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_06_OBSOLETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_06_ODT_ALT_EN_BO                      8
#define AG_MG_REGS_DENALI_CTL_06_ODT_ALT_EN_BM                      0x00000100

#define AG_MG_REGS_DENALI_CTL_06_PLACEMENT_EN_BO                    16
#define AG_MG_REGS_DENALI_CTL_06_PLACEMENT_EN_BM                    0x00010000

#define AG_MG_REGS_DENALI_CTL_06_POWER_DOWN_BO                      24
#define AG_MG_REGS_DENALI_CTL_06_POWER_DOWN_BM                      0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_06_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 8,
            odt_alt_en : 1,
            fill2 : 7,
            placement_en : 1,
            fill1 : 7,
            power_down : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_06_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_07_RO                                 0x0000001C
#define AG_MG_REGS_DENALI_CTL_07_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_07_PRIORITY_EN_BO                     0
#define AG_MG_REGS_DENALI_CTL_07_PRIORITY_EN_BM                     0x00000001

#define AG_MG_REGS_DENALI_CTL_07_PWRUP_SREFRESH_EXIT_BO             8
#define AG_MG_REGS_DENALI_CTL_07_PWRUP_SREFRESH_EXIT_BM             0x00000100

#define AG_MG_REGS_DENALI_CTL_07_RDLVL_BEGIN_DELAY_EN_BO            16
#define AG_MG_REGS_DENALI_CTL_07_RDLVL_BEGIN_DELAY_EN_BM            0x00010000

#define AG_MG_REGS_DENALI_CTL_07_RDLVL_EDGE_BO                      24
#define AG_MG_REGS_DENALI_CTL_07_RDLVL_EDGE_BM                      0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_07_U
{
    struct
    {
        ag_mg_regs_register
            priority_en : 1,
            fill3 : 7,
            pwrup_srefresh_exit : 1,
            fill2 : 7,
            rdlvl_begin_delay_en : 1,
            fill1 : 7,
            rdlvl_edge : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_07_u;
#endif


/* 
 * Initialization value: 0x02000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_08_RO                                 0x00000020
#define AG_MG_REGS_DENALI_CTL_08_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_08_RDLVL_EN_BO                        0
#define AG_MG_REGS_DENALI_CTL_08_RDLVL_EN_BM                        0x00000001

#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_EN_BO                   8
#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_EN_BM                   0x00000100

#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_PREAMBLE_CHECK_EN_BO    16
#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_PREAMBLE_CHECK_EN_BM    0x00010000

#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_REQ_BO                  24
#define AG_MG_REGS_DENALI_CTL_08_RDLVL_GATE_REQ_BM                  0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_08_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_en : 1,
            fill3 : 7,
            rdlvl_gate_en : 1,
            fill2 : 7,
            rdlvl_gate_preamble_check_en : 1,
            fill1 : 7,
            rdlvl_gate_req : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_08_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_09_RO                                 0x00000024
#define AG_MG_REGS_DENALI_CTL_09_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_0_BO              0
#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_0_BM              0x00000001

#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_1_BO              8
#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_1_BM              0x00000100

#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_2_BO              16
#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_2_BM              0x00010000

#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_3_BO              24
#define AG_MG_REGS_DENALI_CTL_09_RDLVL_OFFSET_DIR_3_BM              0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_09_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_offset_dir_0 : 1,
            fill3 : 7,
            rdlvl_offset_dir_1 : 1,
            fill2 : 7,
            rdlvl_offset_dir_2 : 1,
            fill1 : 7,
            rdlvl_offset_dir_3 : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_09_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_10_RO                                 0x00000028
#define AG_MG_REGS_DENALI_CTL_10_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_10_RDLVL_OFFSET_DIR_4_BO              0
#define AG_MG_REGS_DENALI_CTL_10_RDLVL_OFFSET_DIR_4_BM              0x00000001

#define AG_MG_REGS_DENALI_CTL_10_RDLVL_REQ_BO                       8
#define AG_MG_REGS_DENALI_CTL_10_RDLVL_REQ_BM                       0x00000100

#define AG_MG_REGS_DENALI_CTL_10_REDUC_BO                           16
#define AG_MG_REGS_DENALI_CTL_10_REDUC_BM                           0x00010000

#define AG_MG_REGS_DENALI_CTL_10_REG_DIMM_ENABLE_BO                 24
#define AG_MG_REGS_DENALI_CTL_10_REG_DIMM_ENABLE_BM                 0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_10_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_offset_dir_4 : 1,
            fill3 : 7,
            rdlvl_req : 1,
            fill2 : 7,
            reduc : 1,
            fill1 : 7,
            reg_dimm_enable : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_10_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_11_RO                                 0x0000002C
#define AG_MG_REGS_DENALI_CTL_11_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_11_RW_SAME_EN_BO                      0
#define AG_MG_REGS_DENALI_CTL_11_RW_SAME_EN_BM                      0x00000001

#define AG_MG_REGS_DENALI_CTL_11_SREFRESH_BO                        8
#define AG_MG_REGS_DENALI_CTL_11_SREFRESH_BM                        0x00000100

#define AG_MG_REGS_DENALI_CTL_11_START_BO                           16
#define AG_MG_REGS_DENALI_CTL_11_START_BM                           0x00010000

#define AG_MG_REGS_DENALI_CTL_11_SWAP_EN_BO                         24
#define AG_MG_REGS_DENALI_CTL_11_SWAP_EN_BM                         0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_11_U
{
    struct
    {
        ag_mg_regs_register
            rw_same_en : 1,
            fill3 : 7,
            srefresh : 1,
            fill2 : 7,
            start : 1,
            fill1 : 7,
            swap_en : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_11_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_12_RO                                 0x00000030
#define AG_MG_REGS_DENALI_CTL_12_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_12_SWAP_PORT_RW_SAME_EN_BO            0
#define AG_MG_REGS_DENALI_CTL_12_SWAP_PORT_RW_SAME_EN_BM            0x00000001

#define AG_MG_REGS_DENALI_CTL_12_SWLVL_EXIT_BO                      8
#define AG_MG_REGS_DENALI_CTL_12_SWLVL_EXIT_BM                      0x00000100

#define AG_MG_REGS_DENALI_CTL_12_SWLVL_LOAD_BO                      16
#define AG_MG_REGS_DENALI_CTL_12_SWLVL_LOAD_BM                      0x00010000

#define AG_MG_REGS_DENALI_CTL_12_SWLVL_OP_DONE_BO                   24
#define AG_MG_REGS_DENALI_CTL_12_SWLVL_OP_DONE_BM                   0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_12_U
{
    struct
    {
        ag_mg_regs_register
            swap_port_rw_same_en : 1,
            fill3 : 7,
            swlvl_exit : 1,
            fill2 : 7,
            swlvl_load : 1,
            fill1 : 7,
            swlvl_op_done : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_12_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_13_RO                                 0x00000034
#define AG_MG_REGS_DENALI_CTL_13_RM                                 0x01010101

#define AG_MG_REGS_DENALI_CTL_13_SWLVL_START_BO                     0
#define AG_MG_REGS_DENALI_CTL_13_SWLVL_START_BM                     0x00000001

#define AG_MG_REGS_DENALI_CTL_13_TRAS_LOCKOUT_BO                    8
#define AG_MG_REGS_DENALI_CTL_13_TRAS_LOCKOUT_BM                    0x00000100

#define AG_MG_REGS_DENALI_CTL_13_TREF_ENABLE_BO                     16
#define AG_MG_REGS_DENALI_CTL_13_TREF_ENABLE_BM                     0x00010000

#define AG_MG_REGS_DENALI_CTL_13_WRITEINTERP_BO                     24
#define AG_MG_REGS_DENALI_CTL_13_WRITEINTERP_BM                     0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_13_U
{
    struct
    {
        ag_mg_regs_register
            swlvl_start : 1,
            fill3 : 7,
            tras_lockout : 1,
            fill2 : 7,
            tref_enable : 1,
            fill1 : 7,
            writeinterp : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_13_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_14_RO                                 0x00000038
#define AG_MG_REGS_DENALI_CTL_14_RM                                 0x0301FF01

#define AG_MG_REGS_DENALI_CTL_14_WRITE_MODEREGV1_0_BO               0
#define AG_MG_REGS_DENALI_CTL_14_WRITE_MODEREGV1_0_BM               0x00000001

#define AG_MG_REGS_DENALI_CTL_14_OBSOLETE_BO                        8
#define AG_MG_REGS_DENALI_CTL_14_OBSOLETE_BM                        0x0000FF00

#define AG_MG_REGS_DENALI_CTL_14_WRLVL_REQ_BO                       16
#define AG_MG_REGS_DENALI_CTL_14_WRLVL_REQ_BM                       0x00010000

#define AG_MG_REGS_DENALI_CTL_14_AXI0_R_PRIORITY_BO                 24
#define AG_MG_REGS_DENALI_CTL_14_AXI0_R_PRIORITY_BM                 0x03000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_14_U
{
    struct
    {
        ag_mg_regs_register
            write_moderegV1_0 : 1,
            fill2 : 15,
            wrlvl_req : 1,
            fill1 : 7,
            axi0_r_priority : 2,
            fill0 : 6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_14_u;
#endif


/* 
 * Initialization value: 0x0C0C0000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_15_RO                                 0x0000003C
#define AG_MG_REGS_DENALI_CTL_15_RM                                 0x01030303

#define AG_MG_REGS_DENALI_CTL_15_AXI0_W_PRIORITY_BO                 0
#define AG_MG_REGS_DENALI_CTL_15_AXI0_W_PRIORITY_BM                 0x00000003

#define AG_MG_REGS_DENALI_CTL_15_BIST_RESULT_BO                     8
#define AG_MG_REGS_DENALI_CTL_15_BIST_RESULT_BM                     0x00000300

#define AG_MG_REGS_DENALI_CTL_15_CTRL_RAW_BO                        16
#define AG_MG_REGS_DENALI_CTL_15_CTRL_RAW_BM                        0x00030000

#define AG_MG_REGS_DENALI_CTL_15_RDLVL_CS_BO                        24
#define AG_MG_REGS_DENALI_CTL_15_RDLVL_CS_BM                        0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_15_U
{
    struct
    {
        ag_mg_regs_register
            axi0_w_priority : 2,
            fill3 : 6,
            bist_result : 2,
            fill2 : 6,
            ctrl_raw : 2,
            fill1 : 6,
            rdlvl_cs : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_15_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_16_RO                                 0x00000040
#define AG_MG_REGS_DENALI_CTL_16_RM                                 0x030103FF

#define AG_MG_REGS_DENALI_CTL_16_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_16_OBSOLETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_16_SW_LEVELING_MODE_BO                8
#define AG_MG_REGS_DENALI_CTL_16_SW_LEVELING_MODE_BM                0x00000300

#define AG_MG_REGS_DENALI_CTL_16_WRLVL_CS_BO                        16
#define AG_MG_REGS_DENALI_CTL_16_WRLVL_CS_BM                        0x00010000

#define AG_MG_REGS_DENALI_CTL_16_ZQ_ON_SREF_EXIT_BO                 24
#define AG_MG_REGS_DENALI_CTL_16_ZQ_ON_SREF_EXIT_BM                 0x03000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_16_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 8,
            sw_leveling_mode : 2,
            fill2 : 6,
            wrlvl_cs : 1,
            fill1 : 7,
            zq_on_sref_exit : 2,
            fill0 : 6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_16_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_17_RO                                 0x00000044
#define AG_MG_REGS_DENALI_CTL_17_RM                                 0x07070703

#define AG_MG_REGS_DENALI_CTL_17_ZQ_REQ_BO                          0
#define AG_MG_REGS_DENALI_CTL_17_ZQ_REQ_BM                          0x00000003

#define AG_MG_REGS_DENALI_CTL_17_ADDR_PINS_BO                       8
#define AG_MG_REGS_DENALI_CTL_17_ADDR_PINS_BM                       0x00000700

#define AG_MG_REGS_DENALI_CTL_17_BSTLEN_BO                          16
#define AG_MG_REGS_DENALI_CTL_17_BSTLEN_BM                          0x00070000

#define AG_MG_REGS_DENALI_CTL_17_CKE_DELAY_BO                       24
#define AG_MG_REGS_DENALI_CTL_17_CKE_DELAY_BM                       0x07000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_17_U
{
    struct
    {
        ag_mg_regs_register
            zq_req : 2,
            fill3 : 6,
            addr_pins : 3,
            fill2 : 5,
            bstlen : 3,
            fill1 : 5,
            cke_delay : 3,
            fill0 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_17_u;
#endif


/* 
 * Initialization value: 0x00060000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_18_RO                                 0x00000048
#define AG_MG_REGS_DENALI_CTL_18_RM                                 0x07030707

#define AG_MG_REGS_DENALI_CTL_18_COLUMN_SIZE_BO                     0
#define AG_MG_REGS_DENALI_CTL_18_COLUMN_SIZE_BM                     0x00000007

#define AG_MG_REGS_DENALI_CTL_18_LVL_STATUS_BO                      8
#define AG_MG_REGS_DENALI_CTL_18_LVL_STATUS_BM                      0x00000700

#define AG_MG_REGS_DENALI_CTL_18_MAX_CS_REG_BO                      16
#define AG_MG_REGS_DENALI_CTL_18_MAX_CS_REG_BM                      0x00030000

#define AG_MG_REGS_DENALI_CTL_18_PORT_DATA_ERROR_TYPE_BO            24
#define AG_MG_REGS_DENALI_CTL_18_PORT_DATA_ERROR_TYPE_BM            0x07000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_18_U
{
    struct
    {
        ag_mg_regs_register
            column_size : 3,
            fill3 : 5,
            lvl_status : 3,
            fill2 : 5,
            max_cs_reg : 2,
            fill1 : 6,
            port_data_error_type : 3,
            fill0 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_18_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_19_RO                                 0x0000004C
#define AG_MG_REGS_DENALI_CTL_19_RM                                 0x070FFF07

#define AG_MG_REGS_DENALI_CTL_19_TCKE_BO                            0
#define AG_MG_REGS_DENALI_CTL_19_TCKE_BM                            0x00000007

#define AG_MG_REGS_DENALI_CTL_19_TRRD_BO                            8
#define AG_MG_REGS_DENALI_CTL_19_TRRD_BM                            0x0000FF00

#define AG_MG_REGS_DENALI_CTL_19_TRTP_BO                            16
#define AG_MG_REGS_DENALI_CTL_19_TRTP_BM                            0x000F0000

#define AG_MG_REGS_DENALI_CTL_19_W2R_DIFFCS_DLY_BO                  24
#define AG_MG_REGS_DENALI_CTL_19_W2R_DIFFCS_DLY_BM                  0x07000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_19_U
{
    struct
    {
        ag_mg_regs_register
            tcke : 3,
            fill3 : 5,
            trrd : 8,
            trtp : 4,
            fill1 : 4,
            w2r_diffcs_dly : 3,
            fill0 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_19_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_20_RO                                 0x00000050
#define AG_MG_REGS_DENALI_CTL_20_RM                                 0x0F0F0307

#define AG_MG_REGS_DENALI_CTL_20_W2R_SAMECS_DLY_BO                  0
#define AG_MG_REGS_DENALI_CTL_20_W2R_SAMECS_DLY_BM                  0x00000007

#define AG_MG_REGS_DENALI_CTL_20_ADDRESS_MIRRORING_BO               8
#define AG_MG_REGS_DENALI_CTL_20_ADDRESS_MIRRORING_BM               0x00000300

#define AG_MG_REGS_DENALI_CTL_20_AGE_COUNT_BO                       16
#define AG_MG_REGS_DENALI_CTL_20_AGE_COUNT_BM                       0x000F0000

#define AG_MG_REGS_DENALI_CTL_20_APREBIT_BO                         24
#define AG_MG_REGS_DENALI_CTL_20_APREBIT_BM                         0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_20_U
{
    struct
    {
        ag_mg_regs_register
            w2r_samecs_dly : 3,
            fill3 : 5,
            address_mirroring : 2,
            fill2 : 6,
            age_count : 4,
            fill1 : 4,
            aprebit : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_20_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_21_RO                                 0x00000054
#define AG_MG_REGS_DENALI_CTL_21_RM                                 0x030F000F

#define AG_MG_REGS_DENALI_CTL_21_BURST_ON_FLY_BIT_BO                0
#define AG_MG_REGS_DENALI_CTL_21_BURST_ON_FLY_BIT_BM                0x0000000F

#define AG_MG_REGS_DENALI_CTL_21_CASLATV1_0_BO                      8
#define AG_MG_REGS_DENALI_CTL_21_CASLATV1_0_BM                      0x00000F00

#define AG_MG_REGS_DENALI_CTL_21_COMMAND_AGE_COUNT_BO               16
#define AG_MG_REGS_DENALI_CTL_21_COMMAND_AGE_COUNT_BM               0x000F0000

#define AG_MG_REGS_DENALI_CTL_21_CS_MAP_BO                          24
#define AG_MG_REGS_DENALI_CTL_21_CS_MAP_BM                          0x03000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_21_U
{
    struct
    {
        ag_mg_regs_register
            burst_on_fly_bit : 4,
            fill3 : 4,
            caslatV1_0 : 4,
            fill2 : 4,
            command_age_count : 4,
            fill1 : 4,
            cs_map : 2,
            fill0 : 6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_21_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_22_RO                                 0x00000058
#define AG_MG_REGS_DENALI_CTL_22_RM                                 0x0F0F030F

#define AG_MG_REGS_DENALI_CTL_22_DRAM_CLASS_BO                      0
#define AG_MG_REGS_DENALI_CTL_22_DRAM_CLASS_BM                      0x0000000F

#define AG_MG_REGS_DENALI_CTL_22_DRAM_CLK_DISABLE_BO                8
#define AG_MG_REGS_DENALI_CTL_22_DRAM_CLK_DISABLE_BM                0x00000300

#define AG_MG_REGS_DENALI_CTL_22_INITAREF_BO                        16
#define AG_MG_REGS_DENALI_CTL_22_INITAREF_BM                        0x000F0000

#define AG_MG_REGS_DENALI_CTL_22_MAX_COL_REG_BO                     24
#define AG_MG_REGS_DENALI_CTL_22_MAX_COL_REG_BM                     0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_22_U
{
    struct
    {
        ag_mg_regs_register
            dram_class : 4,
            fill3 : 4,
            dram_clk_disable : 2,
            fill2 : 6,
            initaref : 4,
            fill1 : 4,
            max_col_reg : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_22_u;
#endif


/* 
 * Initialization value: 0x0000000E  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_23_RO                                 0x0000005C
#define AG_MG_REGS_DENALI_CTL_23_RM                                 0xFF03030F

#define AG_MG_REGS_DENALI_CTL_23_MAX_ROW_REG_BO                     0
#define AG_MG_REGS_DENALI_CTL_23_MAX_ROW_REG_BM                     0x0000000F

#define AG_MG_REGS_DENALI_CTL_23_ODT_RD_MAP_CS0_BO                  8
#define AG_MG_REGS_DENALI_CTL_23_ODT_RD_MAP_CS0_BM                  0x00000300

#define AG_MG_REGS_DENALI_CTL_23_ODT_RD_MAP_CS1_BO                  16
#define AG_MG_REGS_DENALI_CTL_23_ODT_RD_MAP_CS1_BM                  0x00030000

#define AG_MG_REGS_DENALI_CTL_23_OBSOLETE_BO                        24
#define AG_MG_REGS_DENALI_CTL_23_OBSOLETE_BM                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_23_U
{
    struct
    {
        ag_mg_regs_register
            max_row_reg : 4,
            fill2 : 4,
            odt_rd_map_cs0 : 2,
            fill1 : 6,
            odt_rd_map_cs1 : 2,
            fill0 : 6,
            obsolete : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_23_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_24_RO                                 0x00000060
#define AG_MG_REGS_DENALI_CTL_24_RM                                 0xFF0303FF

#define AG_MG_REGS_DENALI_CTL_24_OBSELETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_24_OBSELETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_24_ODT_WR_MAP_CS0_BO                  8
#define AG_MG_REGS_DENALI_CTL_24_ODT_WR_MAP_CS0_BM                  0x00000300

#define AG_MG_REGS_DENALI_CTL_24_ODT_WR_MAP_CS1_BO                  16
#define AG_MG_REGS_DENALI_CTL_24_ODT_WR_MAP_CS1_BM                  0x00030000

#define AG_MG_REGS_DENALI_CTL_24_OBSOLETE_BO                        24
#define AG_MG_REGS_DENALI_CTL_24_OBSOLETE_BM                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_24_U
{
    struct
    {
        ag_mg_regs_register
            obselete : 8,
            odt_wr_map_cs0 : 2,
            fill1 : 6,
            odt_wr_map_cs1 : 2,
            fill0 : 6,
            obsolete : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_24_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_25_RO                                 0x00000064
#define AG_MG_REGS_DENALI_CTL_25_RM                                 0x0F0F0FFF

#define AG_MG_REGS_DENALI_CTL_25_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_25_OBSOLETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_25_PORT_CMD_ERROR_TYPE_BO             8
#define AG_MG_REGS_DENALI_CTL_25_PORT_CMD_ERROR_TYPE_BM             0x00000F00

#define AG_MG_REGS_DENALI_CTL_25_Q_FULLNESS_BO                      16
#define AG_MG_REGS_DENALI_CTL_25_Q_FULLNESS_BM                      0x000F0000

#define AG_MG_REGS_DENALI_CTL_25_RDLVL_DQ_ZERO_COUNT_BO             24
#define AG_MG_REGS_DENALI_CTL_25_RDLVL_DQ_ZERO_COUNT_BM             0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_25_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 8,
            port_cmd_error_type : 4,
            fill2 : 4,
            q_fullness : 4,
            fill1 : 4,
            rdlvl_dq_zero_count : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_25_u;
#endif


/* 
 * Initialization value: 0x00000400  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_26_RO                                 0x00000068
#define AG_MG_REGS_DENALI_CTL_26_RM                                 0x001F0F0F

#define AG_MG_REGS_DENALI_CTL_26_RDLVL_GATE_DQ_ZERO_COUNT_BO        0
#define AG_MG_REGS_DENALI_CTL_26_RDLVL_GATE_DQ_ZERO_COUNT_BM        0x0000000F

#define AG_MG_REGS_DENALI_CTL_26_TDFI_CTRLUPD_MIN_BO                8
#define AG_MG_REGS_DENALI_CTL_26_TDFI_CTRLUPD_MIN_BM                0x00000F00

#define AG_MG_REGS_DENALI_CTL_26_TDFI_PHY_WRLAT_BO                  16
#define AG_MG_REGS_DENALI_CTL_26_TDFI_PHY_WRLAT_BM                  0x001F0000

#define AG_MG_REGS_DENALI_CTL_26_TDFI_PHY_WRLAT_BASEV1_0_BM             0x0F000000
#define AG_MG_REGS_DENALI_CTL_26_TDFI_PHY_WRLAT_BASEV1_0_BO             24

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_26_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_dq_zero_count : 4,
            fill3 : 4,
            tdfi_ctrlupd_min : 4,
            fill2 : 4,
            tdfi_phy_wrlat : 5,
            fill1 : 3,
            tdfi_phy_wrlat_baseV1_0 : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_26_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_27_RO                                 0x0000006C
#define AG_MG_REGS_DENALI_CTL_27_RM                                 0x0F0FFFFF

#define AG_MG_REGS_DENALI_CTL_27_TDFI_RDLVL_EN_BO                   0
#define AG_MG_REGS_DENALI_CTL_27_TDFI_RDLVL_EN_BM                   0x000000FF

#define AG_MG_REGS_DENALI_CTL_27_TDFI_WRLVL_EN_BO                   8
#define AG_MG_REGS_DENALI_CTL_27_TDFI_WRLVL_EN_BM                   0x0000FF00

#define AG_MG_REGS_DENALI_CTL_27_TRP_BO                             16
#define AG_MG_REGS_DENALI_CTL_27_TRP_BM                             0x000F0000

#define AG_MG_REGS_DENALI_CTL_27_TWTR_BO                            24
#define AG_MG_REGS_DENALI_CTL_27_TWTR_BM                            0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_27_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_rdlvl_en : 8,
            tdfi_wrlvl_en : 8,
            trp : 4,
            fill1 : 4,
            twtr : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_27_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_28_RO                                 0x00000070
#define AG_MG_REGS_DENALI_CTL_28_RM                                 0x3F001F1F

#define AG_MG_REGS_DENALI_CTL_28_WRLAT_BO                           0
#define AG_MG_REGS_DENALI_CTL_28_WRLAT_BM                           0x0000001F

#define AG_MG_REGS_DENALI_CTL_28_WRLAT_ADJ_BO                       8
#define AG_MG_REGS_DENALI_CTL_28_WRLAT_ADJ_BM                       0x00001F00

#define AG_MG_REGS_DENALI_CTL_28_CASLAT_LIN_BO                      24
#define AG_MG_REGS_DENALI_CTL_28_CASLAT_LIN_BM                      0x3F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_28_U
{
    struct
    {
        ag_mg_regs_register
            wrlat : 5,
            fill2 : 3,
            wrlat_adj : 5,
            fill1 : 11,
            caslat_lin : 6,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_28_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_29_RO                                 0x00000074
#define AG_MG_REGS_DENALI_CTL_29_RM                                 0x3F1F1F00

#define AG_MG_REGS_DENALI_CTL_29_CASLAT_LIN_GATEV1_0_BO                 0
#define AG_MG_REGS_DENALI_CTL_29_CASLAT_LIN_GATEV1_0_BM                 0x0000001F

#define AG_MG_REGS_DENALI_CTL_29_OCD_ADJUST_PDN_CS_0_BO             8
#define AG_MG_REGS_DENALI_CTL_29_OCD_ADJUST_PDN_CS_0_BM             0x00001F00

#define AG_MG_REGS_DENALI_CTL_29_OCD_ADJUST_PUP_CS_0_BO             16
#define AG_MG_REGS_DENALI_CTL_29_OCD_ADJUST_PUP_CS_0_BM             0x001F0000

#define AG_MG_REGS_DENALI_CTL_29_OUT_OF_RANGE_TYPE_BO               24
#define AG_MG_REGS_DENALI_CTL_29_OUT_OF_RANGE_TYPE_BM               0x3F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_29_U
{
    struct
    {
        ag_mg_regs_register
            caslat_lin_gateV1_0 : 5,
            fill3 : 3,
            ocd_adjust_pdn_cs_0 : 5,
            fill2 : 3,
            ocd_adjust_pup_cs_0 : 5,
            fill1 : 3,
            out_of_range_type : 6,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_29_u;
#endif


/* 
 * Initialization value: 0x20400000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_30_RO                                 0x00000078
#define AG_MG_REGS_DENALI_CTL_30_RM                                 0x3F3F1F3F

#define AG_MG_REGS_DENALI_CTL_30_RDLAT_ADJ_BO                       0
#define AG_MG_REGS_DENALI_CTL_30_RDLAT_ADJ_BM                       0x0000003F

#define AG_MG_REGS_DENALI_CTL_30_TDAL_BO                            8
#define AG_MG_REGS_DENALI_CTL_30_TDAL_BM                            0x00001F00

#define AG_MG_REGS_DENALI_CTL_30_TDFI_PHY_RDLAT_BO                  16
#define AG_MG_REGS_DENALI_CTL_30_TDFI_PHY_RDLAT_BM                  0x003F0000

#define AG_MG_REGS_DENALI_CTL_30_TDFI_RDDATA_EN_BO                  24
#define AG_MG_REGS_DENALI_CTL_30_TDFI_RDDATA_EN_BM                  0x3F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_30_U
{
    struct
    {
        ag_mg_regs_register
            rdlat_adj : 6,
            fill3 : 2,
            tdal : 5,
            fill2 : 3,
            tdfi_phy_rdlat : 6,
            fill1 : 2,
            tdfi_rddata_en : 6,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_30_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_31_RO                                 0x0000007C
#define AG_MG_REGS_DENALI_CTL_31_RM                                 0xFF1F3F00

#define AG_MG_REGS_DENALI_CTL_31_TDFI_RDDATA_EN_BASEV1_0_BM         0x0000001F
#define AG_MG_REGS_DENALI_CTL_31_TDFI_RDDATA_EN_BASEV1_0_BO         0

#define AG_MG_REGS_DENALI_CTL_31_TFAW_BO                            8
#define AG_MG_REGS_DENALI_CTL_31_TFAW_BM                            0x00003F00

#define AG_MG_REGS_DENALI_CTL_31_TMRD_BO                            16
#define AG_MG_REGS_DENALI_CTL_31_TMRD_BM                            0x001F0000

#define AG_MG_REGS_DENALI_CTL_31_TRC_BO                             24
#define AG_MG_REGS_DENALI_CTL_31_TRC_BM                             0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_31_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_rddata_en_baseV1_0 : 5,
            fill3 : 3,
            tfaw : 6,
            fill2 : 2,
            tmrd : 5,
            fill1 : 3,
            trc : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_31_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_32_RO                                 0x00000080
#define AG_MG_REGS_DENALI_CTL_32_RM                                 0x3F3F3F1F

#define AG_MG_REGS_DENALI_CTL_32_TWR_INT_BO                         0
#define AG_MG_REGS_DENALI_CTL_32_TWR_INT_BM                         0x0000001F

#define AG_MG_REGS_DENALI_CTL_32_ADDR_SPACE_BO                      8
#define AG_MG_REGS_DENALI_CTL_32_ADDR_SPACE_BM                      0x00003F00

#define AG_MG_REGS_DENALI_CTL_32_WLDQSEN_BO                         16
#define AG_MG_REGS_DENALI_CTL_32_WLDQSEN_BM                         0x003F0000

#define AG_MG_REGS_DENALI_CTL_32_WLMRD_BO                           24
#define AG_MG_REGS_DENALI_CTL_32_WLMRD_BM                           0x3F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_32_U
{
    struct
    {
        ag_mg_regs_register
            twr_int : 5,
            fill3 : 3,
            addr_space : 6,
            fill2 : 2,
            wldqsen : 6,
            fill1 : 2,
            wlmrd : 6,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_32_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_33_RO                                 0x00000084
#define AG_MG_REGS_DENALI_CTL_33_RM                                 0xFF7F7F00

#define AG_MG_REGS_DENALI_CTL_33_DLL_LOCKV1_0_BO                        0
#define AG_MG_REGS_DENALI_CTL_33_DLL_LOCKV1_0_BM                        0x0000007F

#define AG_MG_REGS_DENALI_CTL_33_ECC_C_SYND_BO                      8
#define AG_MG_REGS_DENALI_CTL_33_ECC_C_SYND_BM                      0x00007F00

#define AG_MG_REGS_DENALI_CTL_33_ECC_U_SYND_BO                      16
#define AG_MG_REGS_DENALI_CTL_33_ECC_U_SYND_BM                      0x007F0000

#define AG_MG_REGS_DENALI_CTL_33_DLL_RST_ADJ_DLY_BO                 24
#define AG_MG_REGS_DENALI_CTL_33_DLL_RST_ADJ_DLY_BM                 0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_33_U
{
    struct
    {
        ag_mg_regs_register
            dll_lockV1_0 : 7,
            fill2 : 1,
            ecc_c_synd : 7,
            fill1 : 1,
            ecc_u_synd : 7,
            fill0 : 1,
            dll_rst_adj_dly : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_33_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_34_RO                                 0x00000088
#define AG_MG_REGS_DENALI_CTL_34_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_34_OUT_OF_RANGE_LENGTH_BO             0
#define AG_MG_REGS_DENALI_CTL_34_OUT_OF_RANGE_LENGTH_BM             0x000000FF

#define AG_MG_REGS_DENALI_CTL_34_OBSELETE_BO                        8
#define AG_MG_REGS_DENALI_CTL_34_OBSELETE_BM                        0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_34_U
{
    struct
    {
        ag_mg_regs_register
            out_of_range_length : 8,
            obselete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_34_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_35_RO                                 0x0000008C
#define AG_MG_REGS_DENALI_CTL_35_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_35_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_35_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_35_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_35_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_36_RO                                 0x00000090
#define AG_MG_REGS_DENALI_CTL_36_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_36_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_36_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_36_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_36_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_37_RO                                 0x00000094
#define AG_MG_REGS_DENALI_CTL_37_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_37_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_37_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_37_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_37_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_38_RO                                 0x00000098
#define AG_MG_REGS_DENALI_CTL_38_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_38_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_38_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_38_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_38_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_39_RO                                 0x0000009C
#define AG_MG_REGS_DENALI_CTL_39_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_39_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_39_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_39_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_39_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_40_RO                                 0x000000A0
#define AG_MG_REGS_DENALI_CTL_40_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_40_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_40_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_40_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_40_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_41_RO                                 0x000000A4
#define AG_MG_REGS_DENALI_CTL_41_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_41_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_41_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_41_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_41_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_42_RO                                 0x000000A8
#define AG_MG_REGS_DENALI_CTL_42_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_42_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_42_OBSOLETE_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_42_REFRESH_PER_ZQ_BO                  8
#define AG_MG_REGS_DENALI_CTL_42_REFRESH_PER_ZQ_BM                  0x0000FF00

#define AG_MG_REGS_DENALI_CTL_42_SWLVL_RESP_0_BO                    16
#define AG_MG_REGS_DENALI_CTL_42_SWLVL_RESP_0_BM                    0x00FF0000

#define AG_MG_REGS_DENALI_CTL_42_SWLVL_RESP_1_BO                    24
#define AG_MG_REGS_DENALI_CTL_42_SWLVL_RESP_1_BM                    0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_42_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 8,
            refresh_per_zq : 8,
            swlvl_resp_0 : 8,
            swlvl_resp_1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_42_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_43_RO                                 0x000000AC
#define AG_MG_REGS_DENALI_CTL_43_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_2_BO                    0
#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_2_BM                    0x000000FF

#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_3_BO                    8
#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_3_BM                    0x0000FF00

#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_4_BO                    16
#define AG_MG_REGS_DENALI_CTL_43_SWLVL_RESP_4_BM                    0x00FF0000

#define AG_MG_REGS_DENALI_CTL_43_TDFI_RDLVL_DLL_BO                  24
#define AG_MG_REGS_DENALI_CTL_43_TDFI_RDLVL_DLL_BM                  0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_43_U
{
    struct
    {
        ag_mg_regs_register
            swlvl_resp_2 : 8,
            swlvl_resp_3 : 8,
            swlvl_resp_4 : 8,
            tdfi_rdlvl_dll : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_43_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_44_RO                                 0x000000B0
#define AG_MG_REGS_DENALI_CTL_44_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_44_OBSOLETE1_BO                       0
#define AG_MG_REGS_DENALI_CTL_44_OBSOLETE1_BM                       0x000000FF

#define AG_MG_REGS_DENALI_CTL_44_TDFI_RDLVL_RESPLAT_BO              8
#define AG_MG_REGS_DENALI_CTL_44_TDFI_RDLVL_RESPLAT_BM              0x0000FF00

#define AG_MG_REGS_DENALI_CTL_44_OBSOLETE2_BO                       16
#define AG_MG_REGS_DENALI_CTL_44_OBSOLETE2_BM                       0x00FF0000

#define AG_MG_REGS_DENALI_CTL_44_TDFI_WRLVL_DLL_BO                  24
#define AG_MG_REGS_DENALI_CTL_44_TDFI_WRLVL_DLL_BM                  0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_44_U
{
    struct
    {
        ag_mg_regs_register
            obsolete1 : 8,
            tdfi_rdlvl_resplat : 8,
            obsolete2 : 8,
            tdfi_wrlvl_dll : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_44_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_45_RO                                 0x000000B4
#define AG_MG_REGS_DENALI_CTL_45_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_45_OBSOLETE1_BO                       0
#define AG_MG_REGS_DENALI_CTL_45_OBSOLETE1_BM                       0x000000FF

#define AG_MG_REGS_DENALI_CTL_45_TDFI_WRLVL_RESPLAT_BO              8
#define AG_MG_REGS_DENALI_CTL_45_TDFI_WRLVL_RESPLAT_BM              0x0000FF00

#define AG_MG_REGS_DENALI_CTL_45_OBSOLETE2_BO                       16
#define AG_MG_REGS_DENALI_CTL_45_OBSOLETE2_BM                       0x00FF0000

#define AG_MG_REGS_DENALI_CTL_45_TMOD_BO                            24
#define AG_MG_REGS_DENALI_CTL_45_TMOD_BM                            0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_45_U
{
    struct
    {
        ag_mg_regs_register
            obsolete1 : 8,
            tdfi_wrlvl_resplat : 8,
            obsolete2 : 8,
            tmod : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_45_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_46_RO                                 0x000000B8
#define AG_MG_REGS_DENALI_CTL_46_RM                                 0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_46_TRAS_MIN_BO                        0
#define AG_MG_REGS_DENALI_CTL_46_TRAS_MIN_BM                        0x000000FF

#define AG_MG_REGS_DENALI_CTL_46_TRCD_INT_BO                        8
#define AG_MG_REGS_DENALI_CTL_46_TRCD_INT_BM                        0x0000FF00

#define AG_MG_REGS_DENALI_CTL_46_TRFC_BO                            16
#define AG_MG_REGS_DENALI_CTL_46_TRFC_BM                            0x00FF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_46_U
{
    struct
    {
        ag_mg_regs_register
            tras_min : 8,
            trcd_int : 8,
            trfcV1_0 : 8,
            fill : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_46_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_47_RO                                 0x000000BC
#define AG_MG_REGS_DENALI_CTL_47_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_47_ECC_C_ID_BO                        0
#define AG_MG_REGS_DENALI_CTL_47_ECC_C_ID_BM                        0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_47_ECC_U_ID_BO                        16
#define AG_MG_REGS_DENALI_CTL_47_ECC_U_ID_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_47_U
{
    struct
    {
        ag_mg_regs_register
            ecc_c_id : 16,
            ecc_u_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_47_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_48_RO                                 0x000000C0
#define AG_MG_REGS_DENALI_CTL_48_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_48_OUT_OF_RANGE_SOURCE_ID_BO          0
#define AG_MG_REGS_DENALI_CTL_48_OUT_OF_RANGE_SOURCE_ID_BM          0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_48_PORT_CMD_ERROR_ID_BO               16
#define AG_MG_REGS_DENALI_CTL_48_PORT_CMD_ERROR_ID_BM               0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_48_U
{
    struct
    {
        ag_mg_regs_register
            out_of_range_source_id : 16,
            port_cmd_error_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_48_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_49_RO                                 0x000000C4
#define AG_MG_REGS_DENALI_CTL_49_RM                                 0x0FFFFFFF

#define AG_MG_REGS_DENALI_CTL_49_PORT_DATA_ERROR_ID_BO              0
#define AG_MG_REGS_DENALI_CTL_49_PORT_DATA_ERROR_ID_BM              0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_49_WRLVL_STATUS_BO                    16
#define AG_MG_REGS_DENALI_CTL_49_WRLVL_STATUS_BM                    0x0FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_49_U
{
    struct
    {
        ag_mg_regs_register
            port_data_error_id : 16,
            wrlvl_status : 12,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_49_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_50_RO                                 0x000000C8
#define AG_MG_REGS_DENALI_CTL_50_RM                                 0xFFFF0FFF

#define AG_MG_REGS_DENALI_CTL_50_ZQCS_BO                            0
#define AG_MG_REGS_DENALI_CTL_50_ZQCS_BM                            0x00000FFF

#define AG_MG_REGS_DENALI_CTL_50_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_50_OBSOLETE_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_50_U
{
    struct
    {
        ag_mg_regs_register
            zqcs : 12,
            fill0 : 4,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_50_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_51_RO                                 0x000000CC
#define AG_MG_REGS_DENALI_CTL_51_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_51_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_51_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_51_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_51_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_52_RO                                 0x000000D0
#define AG_MG_REGS_DENALI_CTL_52_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_52_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_52_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_52_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_52_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_53_RO                                 0x000000D4
#define AG_MG_REGS_DENALI_CTL_53_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_53_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_53_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_53_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_53_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_54_RO                                 0x000000D8
#define AG_MG_REGS_DENALI_CTL_54_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_54_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_54_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_54_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_54_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_55_RO                                 0x000000DC
#define AG_MG_REGS_DENALI_CTL_55_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_55_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_55_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_55_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_55_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_56_RO                                 0x000000E0
#define AG_MG_REGS_DENALI_CTL_56_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_56_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_56_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_56_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_56_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_57_RO                                 0x000000E4
#define AG_MG_REGS_DENALI_CTL_57_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_57_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_57_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_57_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_57_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_58_RO                                 0x000000E8
#define AG_MG_REGS_DENALI_CTL_58_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_58_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_58_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_58_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_58_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_59_RO                                 0x000000EC
#define AG_MG_REGS_DENALI_CTL_59_RM                                 0x3FFF3FFF

#define AG_MG_REGS_DENALI_CTL_59_TDFI_CTRLUPD_MAX_BO                0
#define AG_MG_REGS_DENALI_CTL_59_TDFI_CTRLUPD_MAX_BM                0x00003FFF

#define AG_MG_REGS_DENALI_CTL_59_TDFI_PHYUPD_RESP_BO                16
#define AG_MG_REGS_DENALI_CTL_59_TDFI_PHYUPD_RESP_BM                0x3FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_59_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_ctrlupd_max : 14,
            fill1 : 2,
            tdfi_phyupd_resp : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_59_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_60_RO                                 0x000000F0
#define AG_MG_REGS_DENALI_CTL_60_RM                                 0x3FFFFFFF

#define AG_MG_REGS_DENALI_CTL_60_TDFI_PHYUPD_TYPE0_BO               0
#define AG_MG_REGS_DENALI_CTL_60_TDFI_PHYUPD_TYPE0_BM               0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_60_TREF_BO                            16
#define AG_MG_REGS_DENALI_CTL_60_TREF_BM                            0x3FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_60_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_phyupd_type0 : 16,
            tref : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_60_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_61_RO                                 0x000000F4
#define AG_MG_REGS_DENALI_CTL_61_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_61_AXI0_EN_SIZE_LT_WIDTH_INSTR_BO     0
#define AG_MG_REGS_DENALI_CTL_61_AXI0_EN_SIZE_LT_WIDTH_INSTR_BM     0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_61_DLL_RST_DELAY_BO                   16
#define AG_MG_REGS_DENALI_CTL_61_DLL_RST_DELAY_BM                   0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_61_U
{
    struct
    {
        ag_mg_regs_register
            axi0_en_size_lt_width_instr : 16,
            dll_rst_delay : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_61_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_62_RO                                 0x000000F8
#define AG_MG_REGS_DENALI_CTL_62_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_62_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_62_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_62_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_62_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_63_RO                                 0x000000FC
#define AG_MG_REGS_DENALI_CTL_63_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_63_TCPD_BO                            0
#define AG_MG_REGS_DENALI_CTL_63_TCPD_BM                            0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_63_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_63_OBSOLETE_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_63_U
{
    struct
    {
        ag_mg_regs_register
            tcpd : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_63_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_64_RO                                 0x00000100
#define AG_MG_REGS_DENALI_CTL_64_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_64_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_64_OBSOLETE_BM                        0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_64_TDLL_BO                            16
#define AG_MG_REGS_DENALI_CTL_64_TDLL_BM                            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_64_U
{
    struct
    {
        ag_mg_regs_register
            obsolete : 16,
            tdll : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_64_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_65_RO                                 0x00000104
#define AG_MG_REGS_DENALI_CTL_65_RM                                 0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_65_TPDEX_BO                           0
#define AG_MG_REGS_DENALI_CTL_65_TPDEX_BM                           0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_65_TRAS_MAXV1_0_BO                        16
#define AG_MG_REGS_DENALI_CTL_65_TRAS_MAXV1_0_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_65_U
{
    struct
    {
        ag_mg_regs_register
            tpdex : 16,
            tras_maxV1_0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_65_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_66_RO                                 0x00000108
#define AG_MG_REGS_DENALI_CTL_66_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_66_TXPDLL_BO                          0
#define AG_MG_REGS_DENALI_CTL_66_TXPDLL_BM                          0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_66_TXSNR_BO                           16
#define AG_MG_REGS_DENALI_CTL_66_TXSNR_BM                           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_66_U
{
    struct
    {
        ag_mg_regs_register
            txpdll : 16,
            txsnr : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_66_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_67_RO                                 0x0000010C
#define AG_MG_REGS_DENALI_CTL_67_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_67_TXSR_BO                            0
#define AG_MG_REGS_DENALI_CTL_67_TXSR_BM                            0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_67_VERSION_BO                         16
#define AG_MG_REGS_DENALI_CTL_67_VERSION_BM                         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_67_U
{
    struct
    {
        ag_mg_regs_register
            txsr : 16,
            version : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_67_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_68_RO                                 0x00000110
#define AG_MG_REGS_DENALI_CTL_68_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_68_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_68_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_68_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_68_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_69_RO                                 0x00000114
#define AG_MG_REGS_DENALI_CTL_69_RM                                 0x003FFFFF

#define AG_MG_REGS_DENALI_CTL_69_INT_ACK_BO                         0
#define AG_MG_REGS_DENALI_CTL_69_INT_ACK_BM                         0x003FFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_69_U
{
    struct
    {
        ag_mg_regs_register
            int_ack : 22,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_69_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_70_RO                                 0x00000118
#define AG_MG_REGS_DENALI_CTL_70_RM                                 0x007FFFFF

#define AG_MG_REGS_DENALI_CTL_70_INT_MASK_BO                        0
#define AG_MG_REGS_DENALI_CTL_70_INT_MASK_BM                        0x007FFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_70_U
{
    struct
    {
        ag_mg_regs_register
            int_mask : 23,
            fill0 : 9;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_70_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_71_RO                                 0x0000011C
#define AG_MG_REGS_DENALI_CTL_71_RM                                 0x007FFFFF

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_ACCESS_OUTV1_0_BO               0
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_ACCESS_OUTV1_0_BM               0x00000001

#define AG_MG_REGS_DENALI_CTL_71_MULT_ACCESS_OUTV1_0_BO                 1
#define AG_MG_REGS_DENALI_CTL_71_MULT_ACCESS_OUTV1_0_BM                 0x00000002

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_CORR_ECCV1_0_BO                 2
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_CORR_ECCV1_0_BM                 0x00000004

#define AG_MG_REGS_DENALI_CTL_71_MULT_CORR_ECCV1_0_BO                   3
#define AG_MG_REGS_DENALI_CTL_71_MULT_CORR_ECCV1_0_BM                   0x00000008

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_UNCORR_ECCV1_0_BO               4
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_UNCORR_ECCV1_0_BM               0x00000010

#define AG_MG_REGS_DENALI_CTL_71_MULT_UNCORR_ECCV1_0_BO                 5
#define AG_MG_REGS_DENALI_CTL_71_MULT_UNCORR_ECCV1_0_BM                 0x00000020

#define AG_MG_REGS_DENALI_CTL_71_CMD_CHAN_ERRV1_0_BO                    6
#define AG_MG_REGS_DENALI_CTL_71_CMD_CHAN_ERRV1_0_BM                    0x00000040

#define AG_MG_REGS_DENALI_CTL_71_CMD_DATA_CHAN_ERRV1_0_BO               7
#define AG_MG_REGS_DENALI_CTL_71_CMD_DATA_CHAN_ERRV1_0_BM               0x00000080

#define AG_MG_REGS_DENALI_CTL_71_DRAM_INIT_COMPLETEV1_0_BO              8
#define AG_MG_REGS_DENALI_CTL_71_DRAM_INIT_COMPLETEV1_0_BM              0x00000100

#define AG_MG_REGS_DENALI_CTL_71_BIST_COMPLETEV1_0_BO                   9
#define AG_MG_REGS_DENALI_CTL_71_BIST_COMPLETEV1_0_BM                   0x00000200

#define AG_MG_REGS_DENALI_CTL_71_ODT_CAS_LAT_3_ERRV1_0_BO               10
#define AG_MG_REGS_DENALI_CTL_71_ODT_CAS_LAT_3_ERRV1_0_BM               0x00000400

#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_ERRV1_0_BO                    11
#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_ERRV1_0_BM                    0x00000800

#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_GATE_TRN_ERRV1_0_BO           12
#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_GATE_TRN_ERRV1_0_BM           0x00001000

#define AG_MG_REGS_DENALI_CTL_71_WRITE_LVL_ERRV1_0_BO                   13
#define AG_MG_REGS_DENALI_CTL_71_WRITE_LVL_ERRV1_0_BM                   0x00002000

#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_REQV1_0_BO                      14
#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_REQV1_0_BM                      0x00004000

#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_COMPLETEV1_0_BO                 15
#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_COMPLETEV1_0_BM                 0x00008000

#define AG_MG_REGS_DENALI_CTL_71_DLL_LOCK_STATE_CHANGEV1_0_BO           16
#define AG_MG_REGS_DENALI_CTL_71_DLL_LOCK_STATE_CHANGEV1_0_BM           0x00010000

#define AG_MG_REGS_DENALI_CTL_71_USER_DLL_RESYNCE_DONEV1_0_BO           17
#define AG_MG_REGS_DENALI_CTL_71_USER_DLL_RESYNCE_DONEV1_0_BM           0x00020000

#define AG_MG_REGS_DENALI_CTL_71_OR_ALL_BITSV1_0_BO                     18
#define AG_MG_REGS_DENALI_CTL_71_OR_ALL_BITSV1_0_BM                     0x00040000


#define AG_MG_REGS_DENALI_CTL_71_MEM_RESET_VALID_ON_DFI_BO          0
#define AG_MG_REGS_DENALI_CTL_71_MEM_RESET_VALID_ON_DFI_BM          0x00000001

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_ACCESS_OUT_BO               1
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_ACCESS_OUT_BM               0x00000002

#define AG_MG_REGS_DENALI_CTL_71_MULT_ACCESS_OUT_BO                 2
#define AG_MG_REGS_DENALI_CTL_71_MULT_ACCESS_OUT_BM                 0x00000004

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_CORR_ECC_BO                 3
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_CORR_ECC_BM                 0x00000008

#define AG_MG_REGS_DENALI_CTL_71_MULT_CORR_ECC_BO                   4
#define AG_MG_REGS_DENALI_CTL_71_MULT_CORR_ECC_BM                   0x00000010

#define AG_MG_REGS_DENALI_CTL_71_SINGLE_UNCORR_ECC_BO               5
#define AG_MG_REGS_DENALI_CTL_71_SINGLE_UNCORR_ECC_BM               0x00000020

#define AG_MG_REGS_DENALI_CTL_71_MULT_UNCORR_ECC_BO                 6
#define AG_MG_REGS_DENALI_CTL_71_MULT_UNCORR_ECC_BM                 0x00000040

#define AG_MG_REGS_DENALI_CTL_71_CMD_CHAN_ERR_BO                    7
#define AG_MG_REGS_DENALI_CTL_71_CMD_CHAN_ERR_BM                    0x00000080

#define AG_MG_REGS_DENALI_CTL_71_CMD_DATA_CHAN_ERR_BO               8
#define AG_MG_REGS_DENALI_CTL_71_CMD_DATA_CHAN_ERR_BM               0x00000100

#define AG_MG_REGS_DENALI_CTL_71_DRAM_INIT_COMPLETE_BO              9
#define AG_MG_REGS_DENALI_CTL_71_DRAM_INIT_COMPLETE_BM              0x00000200

#define AG_MG_REGS_DENALI_CTL_71_BIST_COMPLETE_BO                   10
#define AG_MG_REGS_DENALI_CTL_71_BIST_COMPLETE_BM                   0x00000400

#define AG_MG_REGS_DENALI_CTL_71_ODT_CAS_LAT_3_ERR_BO               11
#define AG_MG_REGS_DENALI_CTL_71_ODT_CAS_LAT_3_ERR_BM               0x00000800

#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_ERR_BO                    12
#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_ERR_BM                    0x00001000

#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_GATE_TRN_ERR_BO           13
#define AG_MG_REGS_DENALI_CTL_71_READ_LVL_GATE_TRN_ERR_BM           0x00002000

#define AG_MG_REGS_DENALI_CTL_71_WRITE_LVL_ERR_BO                   14
#define AG_MG_REGS_DENALI_CTL_71_WRITE_LVL_ERR_BM                   0x00004000

#define AG_MG_REGS_DENALI_CTL_71_DFI_UPDATE_ERR_BO                  15
#define AG_MG_REGS_DENALI_CTL_71_DFI_UPDATE_ERR_BM                  0x00008000

#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_REQ_BO                      16        
#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_REQ_BM                      0x00010000

#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_COMPLETE_BO                 17
#define AG_MG_REGS_DENALI_CTL_71_LVL_OP_COMPLETE_BM                 0x00020000  

#define AG_MG_REGS_DENALI_CTL_71_INTF_MODE_WRITE_COMPLETE_BO        18        
#define AG_MG_REGS_DENALI_CTL_71_INTF_MODE_WRITE_COMPLETE_BM        0x00040000

#define AG_MG_REGS_DENALI_CTL_71_INHIBIT_DRAM_BO                    19        
#define AG_MG_REGS_DENALI_CTL_71_INHIBIT_DRAM_BM                    0x00080000

#define AG_MG_REGS_DENALI_CTL_71_DFI_INIT_COMPLETE_STATE_CHANGE_BO  20        
#define AG_MG_REGS_DENALI_CTL_71_DFI_INIT_COMPLETE_STATE_CHANGE_BM  0x00100000

#define AG_MG_REGS_DENALI_CTL_71_USER_DLL_RESYNC_DONE_BO            21
#define AG_MG_REGS_DENALI_CTL_71_USER_DLL_RESYNC_DONE_BM            0x00200000

#define AG_MG_REGS_DENALI_CTL_71_OR_ALL_BITS_BO                     22
#define AG_MG_REGS_DENALI_CTL_71_OR_ALL_BITS_BM                     0x00400000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_71_U
{
    struct
    {
        ag_mg_regs_register
	    single_access_out : 1,
	    mult_access_out : 1,
	    single_corr_ecc : 1,
	    mult_corr_ecc : 1,
	    single_uncorr_ecc : 1,
	    mult_uncorr_ecc : 1,
	    cmd_chan_err : 1,
	    cmd_data_chan_err : 1,
	    dram_init_complete : 1,
	    bist_complete : 1,
	    odt_cas_lat_3_err : 1,
	    read_lvl_err : 1,
	    read_lvl_gate_training_err : 1,
	    write_lvl_err : 1,
	    lvl_op_req : 1,
	    lvl_op_complete : 1,
	    dll_lock_state_change : 1,
	    user_dll_resync_done : 1,
	    or_all_bits : 1,
        fill0 : 13;
    } fieldsV10;
    struct
    {
        ag_mg_regs_register
        mem_reset_valid_on_dfi : 1,
	    single_access_out : 1,
	    mult_access_out : 1,
	    single_corr_ecc : 1,
	    mult_corr_ecc : 1,
	    single_uncorr_ecc : 1,
	    mult_uncorr_ecc : 1,
	    cmd_chan_err : 1,
	    cmd_data_chan_err : 1,
	    dram_init_complete : 1,
	    bist_complete : 1,
	    odt_cas_lat_3_err : 1,
	    read_lvl_err : 1,
	    read_lvl_gate_training_err : 1,
	    write_lvl_err : 1,
	    dfi_update_err : 1,
	    lvl_op_req : 1,
	    lvl_op_complete : 1,
	    intf_mode_write_complete : 1,
        inhibit_dram : 1,
        dfi_init_complete_state_change : 1,
        user_dll_resync_done : 1,
	    or_all_bits : 1,
        fill0 : 9;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_71_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_72_RO                                 0x00000120
#define AG_MG_REGS_DENALI_CTL_72_RM                                 0xFF3FFFFF

#define AG_MG_REGS_DENALI_CTL_72_RDLVL_ERROR_STATUS_BO              0
#define AG_MG_REGS_DENALI_CTL_72_RDLVL_ERROR_STATUS_BM              0x003FFFFF

#define AG_MG_REGS_DENALI_CTL_72_OBSOLETE_BO                        24
#define AG_MG_REGS_DENALI_CTL_72_OBSOLETE_BM                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_72_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_error_status : 22,
            fill0 : 2,
            obsolete : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_72_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_73_RO                                 0x00000124
#define AG_MG_REGS_DENALI_CTL_73_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_73_TINIT_BO                           0
#define AG_MG_REGS_DENALI_CTL_73_TINIT_BM                           0x00FFFFFF

#define AG_MG_REGS_DENALI_CTL_73_OBSOLETE_BO                        24
#define AG_MG_REGS_DENALI_CTL_73_OBSOLETE_BM                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_73_U
{
    struct
    {
        ag_mg_regs_register
            tinit : 24,
            obsolete : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_73_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_74_RO                                 0x00000128
#define AG_MG_REGS_DENALI_CTL_74_RM                                 0x0FFFFFFF

#define AG_MG_REGS_DENALI_CTL_74_XOR_CHECK_BITS_BO                  0
#define AG_MG_REGS_DENALI_CTL_74_XOR_CHECK_BITS_BM                  0x0FFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_74_U
{
    struct
    {
        ag_mg_regs_register
            xor_check_bits : 28,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_74_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_75_RO                                 0x0000012C
#define AG_MG_REGS_DENALI_CTL_75_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_75_CKE_INACTIVE_BO                    0
#define AG_MG_REGS_DENALI_CTL_75_CKE_INACTIVE_BM                    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_75_U
{
    struct
    {
        ag_mg_regs_register
            cke_inactive;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_75_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_76_RO                                 0x00000130
#define AG_MG_REGS_DENALI_CTL_76_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_76_DFI_WRLVL_MAX_DELAY_BO             0
#define AG_MG_REGS_DENALI_CTL_76_DFI_WRLVL_MAX_DELAY_BM             0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_76_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_76_OBSOLETE_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_76_U
{
    struct
    {
        ag_mg_regs_register
            dfi_wrlvl_max_delay : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_76_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_77_RO                                 0x00000134
#define AG_MG_REGS_DENALI_CTL_77_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_77_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_77_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_77_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_77_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_78_RO                                 0x00000138
#define AG_MG_REGS_DENALI_CTL_78_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_78_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_78_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_78_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_78_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_79_RO                                 0x0000013C
#define AG_MG_REGS_DENALI_CTL_79_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_79_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_79_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_79_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_79_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_80_RO                                 0x00000140
#define AG_MG_REGS_DENALI_CTL_80_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_80_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_80_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_80_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_80_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_81_RO                                 0x00000144
#define AG_MG_REGS_DENALI_CTL_81_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_81_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_81_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_81_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_81_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_82_RO                                 0x00000148
#define AG_MG_REGS_DENALI_CTL_82_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_82_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_82_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_82_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_82_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_83_RO                                 0x0000014C
#define AG_MG_REGS_DENALI_CTL_83_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_83_ECC_C_DATA_BO                      0
#define AG_MG_REGS_DENALI_CTL_83_ECC_C_DATA_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_83_U
{
    struct
    {
        ag_mg_regs_register
            ecc_c_data;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_83_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_84_RO                                 0x00000150
#define AG_MG_REGS_DENALI_CTL_84_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_84_ECC_U_DATA_BO                      0
#define AG_MG_REGS_DENALI_CTL_84_ECC_U_DATA_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_84_U
{
    struct
    {
        ag_mg_regs_register
            ecc_u_data;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_84_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_85_RO                                 0x00000154
#define AG_MG_REGS_DENALI_CTL_85_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_85_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_85_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_85_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_85_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_86_RO                                 0x00000158
#define AG_MG_REGS_DENALI_CTL_86_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_86_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_86_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_86_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_86_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_87_RO                                 0x0000015C
#define AG_MG_REGS_DENALI_CTL_87_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_87_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_87_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_87_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_87_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_88_RO                                 0x00000160
#define AG_MG_REGS_DENALI_CTL_88_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_88_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_88_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_88_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_88_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_89_RO                                 0x00000164
#define AG_MG_REGS_DENALI_CTL_89_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_89_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_89_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_89_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_89_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_90_RO                                 0x00000168
#define AG_MG_REGS_DENALI_CTL_90_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_90_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_90_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_90_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_90_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_91_RO                                 0x0000016C
#define AG_MG_REGS_DENALI_CTL_91_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_91_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_91_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_91_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_91_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_92_RO                                 0x00000170
#define AG_MG_REGS_DENALI_CTL_92_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_92_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_92_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_92_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_92_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_93_RO                                 0x00000174
#define AG_MG_REGS_DENALI_CTL_93_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_93_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_93_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_93_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_93_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_94_RO                                 0x00000178
#define AG_MG_REGS_DENALI_CTL_94_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_94_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_94_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_94_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_94_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_95_RO                                 0x0000017C
#define AG_MG_REGS_DENALI_CTL_95_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_95_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_95_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_95_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_95_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_96_RO                                 0x00000180
#define AG_MG_REGS_DENALI_CTL_96_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_96_OBSOLETE_BO                        0
#define AG_MG_REGS_DENALI_CTL_96_OBSOLETE_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_96_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_96_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_97_RO                                 0x00000184
#define AG_MG_REGS_DENALI_CTL_97_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_97_TRST_PWRON_BO                      0
#define AG_MG_REGS_DENALI_CTL_97_TRST_PWRON_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_97_U
{
    struct
    {
        ag_mg_regs_register
            trst_pwron;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_97_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_98_RO                                 0x00000188
#define AG_MG_REGS_DENALI_CTL_98_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_98_WRLVL_DELAY_0_BO                   0
#define AG_MG_REGS_DENALI_CTL_98_WRLVL_DELAY_0_BM                   0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_98_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_98_OBSOLETE_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_98_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_delay_0 : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_98_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_99_RO                                 0x0000018C
#define AG_MG_REGS_DENALI_CTL_99_RM                                 0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_99_WRLVL_DELAY_1_BO                   0
#define AG_MG_REGS_DENALI_CTL_99_WRLVL_DELAY_1_BM                   0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_99_OBSOLETE_BO                        16
#define AG_MG_REGS_DENALI_CTL_99_OBSOLETE_BM                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_99_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_delay_1 : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_99_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_100_RO                                0x00000190
#define AG_MG_REGS_DENALI_CTL_100_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_100_WRLVL_DELAY_2_BO                  0
#define AG_MG_REGS_DENALI_CTL_100_WRLVL_DELAY_2_BM                  0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_100_OBSOLETE_BO                       16
#define AG_MG_REGS_DENALI_CTL_100_OBSOLETE_BM                       0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_100_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_delay_2 : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_100_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_101_RO                                0x00000194
#define AG_MG_REGS_DENALI_CTL_101_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_101_WRLVL_DELAY_3_BO                  0
#define AG_MG_REGS_DENALI_CTL_101_WRLVL_DELAY_3_BM                  0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_101_OBSOLETE_BO                       16
#define AG_MG_REGS_DENALI_CTL_101_OBSOLETE_BM                       0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_101_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_delay_3 : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_101_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_102_RO                                0x00000198
#define AG_MG_REGS_DENALI_CTL_102_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_102_WRLVL_DELAY_4_BO                  0
#define AG_MG_REGS_DENALI_CTL_102_WRLVL_DELAY_4_BM                  0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_102_OBSOLETE_BO                       16
#define AG_MG_REGS_DENALI_CTL_102_OBSOLETE_BM                       0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_102_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_delay_4 : 16,
            obsolete : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_102_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_103_RO                                0x0000019C
#define AG_MG_REGS_DENALI_CTL_103_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_103_BIST_FAIL_ADDRESS_31_0_BO         0
#define AG_MG_REGS_DENALI_CTL_103_BIST_FAIL_ADDRESS_31_0_BM         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_103_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_address_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_103_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_104_RO                                0x000001A0
#define AG_MG_REGS_DENALI_CTL_104_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_104_BIST_FAIL_ADDRESS_32_BO           0
#define AG_MG_REGS_DENALI_CTL_104_BIST_FAIL_ADDRESS_32_BM           0x00000001

#define AG_MG_REGS_DENALI_CTL_104_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_104_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_104_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_address_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_104_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_105_RO                                0x000001A4
#define AG_MG_REGS_DENALI_CTL_105_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_105_BIST_START_ADDRESS_31_0_BO        0
#define AG_MG_REGS_DENALI_CTL_105_BIST_START_ADDRESS_31_0_BM        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_105_U
{
    struct
    {
        ag_mg_regs_register
            bist_start_address_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_105_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_106_RO                                0x000001A8
#define AG_MG_REGS_DENALI_CTL_106_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_106_BIST_START_ADDRESS_32_BO          0
#define AG_MG_REGS_DENALI_CTL_106_BIST_START_ADDRESS_32_BM          0x00000001

#define AG_MG_REGS_DENALI_CTL_106_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_106_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_106_U
{
    struct
    {
        ag_mg_regs_register
            bist_start_address_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_106_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_107_RO                                0x000001AC
#define AG_MG_REGS_DENALI_CTL_107_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_107_ECC_C_ADDR_31_0_BO                0
#define AG_MG_REGS_DENALI_CTL_107_ECC_C_ADDR_31_0_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_107_U
{
    struct
    {
        ag_mg_regs_register
            ecc_c_addr_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_107_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_108_RO                                0x000001B0
#define AG_MG_REGS_DENALI_CTL_108_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_108_ECC_C_ADDR_32_BO                  0
#define AG_MG_REGS_DENALI_CTL_108_ECC_C_ADDR_32_BM                  0x00000001

#define AG_MG_REGS_DENALI_CTL_108_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_108_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_108_U
{
    struct
    {
        ag_mg_regs_register
            ecc_c_addr_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_108_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_109_RO                                0x000001B4
#define AG_MG_REGS_DENALI_CTL_109_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_109_ECC_U_ADDR_31_0_BO                0
#define AG_MG_REGS_DENALI_CTL_109_ECC_U_ADDR_31_0_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_109_U
{
    struct
    {
        ag_mg_regs_register
            ecc_u_addr_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_109_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_110_RO                                0x000001B8
#define AG_MG_REGS_DENALI_CTL_110_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_110_ECC_U_ADDR_32_BO                  0
#define AG_MG_REGS_DENALI_CTL_110_ECC_U_ADDR_32_BM                  0x00000001

#define AG_MG_REGS_DENALI_CTL_110_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_110_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_110_U
{
    struct
    {
        ag_mg_regs_register
            ecc_u_addr_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_110_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_111_RO                                0x000001BC
#define AG_MG_REGS_DENALI_CTL_111_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_111_OUT_OF_RANGE_ADDR_31_0_BO         0
#define AG_MG_REGS_DENALI_CTL_111_OUT_OF_RANGE_ADDR_31_0_BM         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_111_U
{
    struct
    {
        ag_mg_regs_register
            out_of_range_addr_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_111_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_112_RO                                0x000001C0
#define AG_MG_REGS_DENALI_CTL_112_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_112_OUT_OF_RANGE_ADDR_32_BO           0
#define AG_MG_REGS_DENALI_CTL_112_OUT_OF_RANGE_ADDR_32_BM           0x00000001

#define AG_MG_REGS_DENALI_CTL_112_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_112_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_112_U
{
    struct
    {
        ag_mg_regs_register
            out_of_range_addr_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_112_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_113_RO                                0x000001C4
#define AG_MG_REGS_DENALI_CTL_113_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_113_PORT_CMD_ERROR_ADDR_31_0_BO       0
#define AG_MG_REGS_DENALI_CTL_113_PORT_CMD_ERROR_ADDR_31_0_BM       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_113_U
{
    struct
    {
        ag_mg_regs_register
            port_cmd_error_addr_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_113_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_114_RO                                0x000001C8
#define AG_MG_REGS_DENALI_CTL_114_RM                                0xFFFFFF01

#define AG_MG_REGS_DENALI_CTL_114_PORT_CMD_ERROR_ADDR_32_BO         0
#define AG_MG_REGS_DENALI_CTL_114_PORT_CMD_ERROR_ADDR_32_BM         0x00000001

#define AG_MG_REGS_DENALI_CTL_114_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_114_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_114_U
{
    struct
    {
        ag_mg_regs_register
            port_cmd_error_addr_32 : 1,
            fill0 : 7,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_114_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_115_RO                                0x000001CC
#define AG_MG_REGS_DENALI_CTL_115_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_115_RDLVL_GATE_RESP_MASK_31_0_BO      0
#define AG_MG_REGS_DENALI_CTL_115_RDLVL_GATE_RESP_MASK_31_0_BM      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_115_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_resp_mask_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_115_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_116_RO                                0x000001D0
#define AG_MG_REGS_DENALI_CTL_116_RM                                0xFFFFFF7F

#define AG_MG_REGS_DENALI_CTL_116_RDLVL_GATE_RESP_MASK_38_32_BO     0
#define AG_MG_REGS_DENALI_CTL_116_RDLVL_GATE_RESP_MASK_38_32_BM     0x0000007F

#define AG_MG_REGS_DENALI_CTL_116_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_116_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_116_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_resp_mask_38_32 : 7,
            fill0 : 1,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_116_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_117_RO                                0x000001D4
#define AG_MG_REGS_DENALI_CTL_117_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_117_RDLVL_RESP_MASK_31_0_BO           0
#define AG_MG_REGS_DENALI_CTL_117_RDLVL_RESP_MASK_31_0_BM           0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_117_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_resp_mask_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_117_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_118_RO                                0x000001D8
#define AG_MG_REGS_DENALI_CTL_118_RM                                0xFFFFFF7F

#define AG_MG_REGS_DENALI_CTL_118_RDLVL_RESP_MASK_38_32_BO          0
#define AG_MG_REGS_DENALI_CTL_118_RDLVL_RESP_MASK_38_32_BM          0x0000007F

#define AG_MG_REGS_DENALI_CTL_118_OBSOLETE_BO                       8
#define AG_MG_REGS_DENALI_CTL_118_OBSOLETE_BM                       0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_118_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_resp_mask_38_32 : 7,
            fill0 : 1,
            obsolete : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_118_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_119_RO                                0x000001DC
#define AG_MG_REGS_DENALI_CTL_119_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_119_BIST_DATA_MASK_31_0_BO            0
#define AG_MG_REGS_DENALI_CTL_119_BIST_DATA_MASK_31_0_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_119_U
{
    struct
    {
        ag_mg_regs_register
            bist_data_mask_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_119_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_120_RO                                0x000001E0
#define AG_MG_REGS_DENALI_CTL_120_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_120_BIST_DATA_MASK_63_32_BO           0
#define AG_MG_REGS_DENALI_CTL_120_BIST_DATA_MASK_63_32_BM           0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_120_U
{
    struct
    {
        ag_mg_regs_register
            bist_data_mask_63_32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_120_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_121_RO                                0x000001E4
#define AG_MG_REGS_DENALI_CTL_121_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_121_BIST_EXP_DATA_31_0_BO             0
#define AG_MG_REGS_DENALI_CTL_121_BIST_EXP_DATA_31_0_BM             0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_121_U
{
    struct
    {
        ag_mg_regs_register
            bist_exp_data_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_121_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_122_RO                                0x000001E8
#define AG_MG_REGS_DENALI_CTL_122_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_122_BIST_EXP_DATA_63_32_BO            0
#define AG_MG_REGS_DENALI_CTL_122_BIST_EXP_DATA_63_32_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_122_U
{
    struct
    {
        ag_mg_regs_register
            bist_exp_data_63_32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_122_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_123_RO                                0x000001EC
#define AG_MG_REGS_DENALI_CTL_123_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_123_BIST_EXP_DATA_95_64_BO            0
#define AG_MG_REGS_DENALI_CTL_123_BIST_EXP_DATA_95_64_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_123_U
{
    struct
    {
        ag_mg_regs_register
            bist_exp_data_95_64;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_123_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_124_RO                                0x000001F0
#define AG_MG_REGS_DENALI_CTL_124_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_124_BIST_EXP_DATA_127_96_BO           0
#define AG_MG_REGS_DENALI_CTL_124_BIST_EXP_DATA_127_96_BM           0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_124_U
{
    struct
    {
        ag_mg_regs_register
            bist_exp_data_127_96;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_124_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_125_RO                                0x000001F4
#define AG_MG_REGS_DENALI_CTL_125_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_125_BIST_FAIL_DATA_31_0_BO            0
#define AG_MG_REGS_DENALI_CTL_125_BIST_FAIL_DATA_31_0_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_125_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_data_31_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_125_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_126_RO                                0x000001F8
#define AG_MG_REGS_DENALI_CTL_126_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_126_BIST_FAIL_DATA_63_32_BO           0
#define AG_MG_REGS_DENALI_CTL_126_BIST_FAIL_DATA_63_32_BM           0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_126_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_data_63_32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_126_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_127_RO                                0x000001FC
#define AG_MG_REGS_DENALI_CTL_127_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_127_BIST_FAIL_DATA_95_64_BO           0
#define AG_MG_REGS_DENALI_CTL_127_BIST_FAIL_DATA_95_64_BM           0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_127_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_data_95_64;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_127_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_128_RO                                0x00000200
#define AG_MG_REGS_DENALI_CTL_128_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_128_BIST_FAIL_DATA_127_96_BO          0
#define AG_MG_REGS_DENALI_CTL_128_BIST_FAIL_DATA_127_96_BM          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_128_U
{
    struct
    {
        ag_mg_regs_register
            bist_fail_data_127_96;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_128_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_129_RO                                0x00000204
#define AG_MG_REGS_DENALI_CTL_129_RM                                0x01010100

#define AG_MG_REGS_DENALI_CTL_129_CONCURRENTAP_WR_ONLYV1_0_BO           0
#define AG_MG_REGS_DENALI_CTL_129_CONCURRENTAP_WR_ONLYV1_0_BM           0x00000001

#define AG_MG_REGS_DENALI_CTL_129_RDLVL_GATE_REG_EN_BO              8
#define AG_MG_REGS_DENALI_CTL_129_RDLVL_GATE_REG_EN_BM              0x00000100

#define AG_MG_REGS_DENALI_CTL_129_RDLVL_REG_EN_BO                   16
#define AG_MG_REGS_DENALI_CTL_129_RDLVL_REG_EN_BM                   0x00010000

#define AG_MG_REGS_DENALI_CTL_129_RESYNC_DLL_BO                     24
#define AG_MG_REGS_DENALI_CTL_129_RESYNC_DLL_BM                     0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_129_U
{
    struct
    {
        ag_mg_regs_register
            concurrentap_wr_onlyV1_0 : 1,
            fill3 : 7,
            rdlvl_gate_reg_en : 1,
            fill2 : 7,
            rdlvl_reg_en : 1,
            fill1 : 7,
            resync_dll : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_129_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_130_RO                                0x00000208
#define AG_MG_REGS_DENALI_CTL_130_RM                                0x01010101

#define AG_MG_REGS_DENALI_CTL_130_RESYNC_DLL_PER_AREF_EN_BO         0
#define AG_MG_REGS_DENALI_CTL_130_RESYNC_DLL_PER_AREF_EN_BM         0x00000001

#define AG_MG_REGS_DENALI_CTL_130_WRLVL_REG_EN_BO                   8
#define AG_MG_REGS_DENALI_CTL_130_WRLVL_REG_EN_BM                   0x00000100

#define AG_MG_REGS_DENALI_CTL_130_ZQCS_ROTATE_BO                    16
#define AG_MG_REGS_DENALI_CTL_130_ZQCS_ROTATE_BM                    0x00010000

#define AG_MG_REGS_DENALI_CTL_130_ZQ_IN_PROGRESS_BO                 24
#define AG_MG_REGS_DENALI_CTL_130_ZQ_IN_PROGRESS_BM                 0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_130_U
{
    struct
    {
        ag_mg_regs_register
            resync_dll_per_aref_en : 1,
            fill3 : 7,
            wrlvl_reg_en : 1,
            fill2 : 7,
            zqcs_rotate : 1,
            fill1 : 7,
            zq_in_progress : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_130_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_131_RO                                0x0000020C
#define AG_MG_REGS_DENALI_CTL_131_RM                                0x07070707

#define AG_MG_REGS_DENALI_CTL_131_R2R_DIFFCS_DLY_BO                 0
#define AG_MG_REGS_DENALI_CTL_131_R2R_DIFFCS_DLY_BM                 0x00000007

#define AG_MG_REGS_DENALI_CTL_131_R2R_SAMECS_DLY_BO                 8
#define AG_MG_REGS_DENALI_CTL_131_R2R_SAMECS_DLY_BM                 0x00000700

#define AG_MG_REGS_DENALI_CTL_131_R2W_DIFFCS_DLY_BO                 16
#define AG_MG_REGS_DENALI_CTL_131_R2W_DIFFCS_DLY_BM                 0x00070000

#define AG_MG_REGS_DENALI_CTL_131_R2W_SAMECS_DLY_BO                 24
#define AG_MG_REGS_DENALI_CTL_131_R2W_SAMECS_DLY_BM                 0x07000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_131_U
{
    struct
    {
        ag_mg_regs_register
            r2r_diffcs_dly : 3,
            fill3 : 5,
            r2r_samecs_dly : 3,
            fill2 : 5,
            r2w_diffcs_dly : 3,
            fill1 : 5,
            r2w_samecs_dly : 3,
            fill0 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_131_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_132_RO                                0x00000210
#define AG_MG_REGS_DENALI_CTL_132_RM                                0x0007071F

#define AG_MG_REGS_DENALI_CTL_132_TCCD_BO                           0
#define AG_MG_REGS_DENALI_CTL_132_TCCD_BM                           0x0000001F

#define AG_MG_REGS_DENALI_CTL_132_W2W_DIFFCS_DLY_BO                 8
#define AG_MG_REGS_DENALI_CTL_132_W2W_DIFFCS_DLY_BM                 0x00000700

#define AG_MG_REGS_DENALI_CTL_132_W2W_SAMECS_DLY_BO                 16
#define AG_MG_REGS_DENALI_CTL_132_W2W_SAMECS_DLY_BM                 0x00070000

#define AG_MG_REGS_DENALI_CTL_132_ADD_ODT_CLK_DIFFTYPE_SAMECSV1_0_BO    24
#define AG_MG_REGS_DENALI_CTL_132_ADD_ODT_CLK_DIFFTYPE_SAMECSV1_0_BM    0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_132_U
{
    struct
    {
        ag_mg_regs_register
            tccd : 5,
            fill3 : 3,
            w2w_diffcs_dly : 3,
            fill2 : 5,
            w2w_samecs_dly : 3,
            fill1 : 5,
            add_odt_clk_difftype_samecsV1_0 : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_132_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_133_RO                                0x00000214
#define AG_MG_REGS_DENALI_CTL_133_RM                                0x003F0F0F

#define AG_MG_REGS_DENALI_CTL_133_ADD_ODT_CLK_SAMETYPE_DIFFCS_BO    0
#define AG_MG_REGS_DENALI_CTL_133_ADD_ODT_CLK_SAMETYPE_DIFFCS_BM    0x0000000F

#define AG_MG_REGS_DENALI_CTL_133_TRP_AB_BO                         8
#define AG_MG_REGS_DENALI_CTL_133_TRP_AB_BM                         0x00000F00

#define AG_MG_REGS_DENALI_CTL_133_ADD_ODT_CLK_DIFFTYPE_DIFFCS_BO    16
#define AG_MG_REGS_DENALI_CTL_133_ADD_ODT_CLK_DIFFTYPE_DIFFCS_BM    0x003F0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_133_U
{
    struct
    {
        ag_mg_regs_register
            add_odt_clk_sametype_diffcs : 4,
            fill2 : 4,
            trp_ab : 4,
            fill1 : 4,
            add_odt_clk_difftype_diffcs : 6,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_133_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_134_RO                                0x00000218
#define AG_MG_REGS_DENALI_CTL_134_RM                                0x03FF03FF

#define AG_MG_REGS_DENALI_CTL_134_TDFI_RDLVL_RR_BO                  0
#define AG_MG_REGS_DENALI_CTL_134_TDFI_RDLVL_RR_BM                  0x000003FF

#define AG_MG_REGS_DENALI_CTL_134_TDFI_WRLVL_WW_BO                  16
#define AG_MG_REGS_DENALI_CTL_134_TDFI_WRLVL_WW_BM                  0x03FF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_134_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_rdlvl_rr : 10,
            fill1 : 6,
            tdfi_wrlvl_ww : 10,
            fill0 : 6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_134_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_135_RO                                0x0000021C
#define AG_MG_REGS_DENALI_CTL_135_RM                                0x0FFF0FFF

#define AG_MG_REGS_DENALI_CTL_135_ZQCL_BO                           0
#define AG_MG_REGS_DENALI_CTL_135_ZQCL_BM                           0x00000FFF

#define AG_MG_REGS_DENALI_CTL_135_ZQINIT_BO                         16
#define AG_MG_REGS_DENALI_CTL_135_ZQINIT_BM                         0x0FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_135_U
{
    struct
    {
        ag_mg_regs_register
            zqcl : 12,
            fill1 : 4,
            zqinit : 12,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_135_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_136_RO                                0x00000220
#define AG_MG_REGS_DENALI_CTL_136_RM                                0x3FFF3FFF

#define AG_MG_REGS_DENALI_CTL_136_MR0_DATA_0_BO                     0
#define AG_MG_REGS_DENALI_CTL_136_MR0_DATA_0_BM                     0x00003FFF

#define AG_MG_REGS_DENALI_CTL_136_MR0_DATA_1_BO                     16
#define AG_MG_REGS_DENALI_CTL_136_MR0_DATA_1_BM                     0x3FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_136_U
{
    struct
    {
        ag_mg_regs_register
            mr0_data_0 : 14,
            fill1 : 2,
            mr0_data_1 : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_136_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_137_RO                                0x00000224
#define AG_MG_REGS_DENALI_CTL_137_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_137_OBSOLETE_BO                       0
#define AG_MG_REGS_DENALI_CTL_137_OBSOLETE_BM                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_137_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_137_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_138_RO                                0x00000228
#define AG_MG_REGS_DENALI_CTL_138_RM                                0x1FFF1FFF

#define AG_MG_REGS_DENALI_CTL_138_MR1_DATA_0_BO                     0
#define AG_MG_REGS_DENALI_CTL_138_MR1_DATA_0_BM                     0x00001FFF

#define AG_MG_REGS_DENALI_CTL_138_MR1_DATA_1_BO                     16
#define AG_MG_REGS_DENALI_CTL_138_MR1_DATA_1_BM                     0x1FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_138_U
{
    struct
    {
        ag_mg_regs_register
            mr1_data_0 : 14,
            fill1 : 2,
            mr1_data_1 : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_138_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_139_RO                                0x0000022C
#define AG_MG_REGS_DENALI_CTL_139_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_139_OBSOLETE_BO                       0
#define AG_MG_REGS_DENALI_CTL_139_OBSOLETE_BM                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_139_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_139_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_140_RO                                0x00000230
#define AG_MG_REGS_DENALI_CTL_140_RM                                0x3FFF3FFF

#define AG_MG_REGS_DENALI_CTL_140_MR2_DATA_0_BO                     0
#define AG_MG_REGS_DENALI_CTL_140_MR2_DATA_0_BM                     0x00003FFF

#define AG_MG_REGS_DENALI_CTL_140_MR2_DATA_1_BO                     16
#define AG_MG_REGS_DENALI_CTL_140_MR2_DATA_1_BM                     0x3FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_140_U
{
    struct
    {
        ag_mg_regs_register
            mr2_data_0 : 14,
            fill1 : 2,
            mr2_data_1 : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_140_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_141_RO                                0x00000234
#define AG_MG_REGS_DENALI_CTL_141_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_141_OBSOLETE_BO                       0
#define AG_MG_REGS_DENALI_CTL_141_OBSOLETE_BM                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_141_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_141_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_142_RO                                0x00000238
#define AG_MG_REGS_DENALI_CTL_142_RM                                0x3FFF3FFF

#define AG_MG_REGS_DENALI_CTL_142_MR3_DATA_0_BO                     0
#define AG_MG_REGS_DENALI_CTL_142_MR3_DATA_0_BM                     0x00003FFF

#define AG_MG_REGS_DENALI_CTL_142_MR3_DATA_1_BO                     16
#define AG_MG_REGS_DENALI_CTL_142_MR3_DATA_1_BM                     0x3FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_142_U
{
    struct
    {
        ag_mg_regs_register
            mr3_data_0 : 14,
            fill1 : 2,
            mr3_data_1 : 14,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_142_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_143_RO                                0x0000023C
#define AG_MG_REGS_DENALI_CTL_143_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_143_OBSOLETE_BO                       0
#define AG_MG_REGS_DENALI_CTL_143_OBSOLETE_BM                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_143_U
{
    struct
    {
        ag_mg_regs_register
            obsolete;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_143_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_144_RO                                0x00000240
#define AG_MG_REGS_DENALI_CTL_144_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_144_RDLVL_BEGIN_DELAY_0_BO            0
#define AG_MG_REGS_DENALI_CTL_144_RDLVL_BEGIN_DELAY_0_BM            0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_144_RDLVL_BEGIN_DELAY_1_BO            16
#define AG_MG_REGS_DENALI_CTL_144_RDLVL_BEGIN_DELAY_1_BM            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_144_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_begin_delay_0 : 16,
            rdlvl_begin_delay_1 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_144_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_145_RO                                0x00000244
#define AG_MG_REGS_DENALI_CTL_145_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_145_RDLVL_BEGIN_DELAY_2_BO            0
#define AG_MG_REGS_DENALI_CTL_145_RDLVL_BEGIN_DELAY_2_BM            0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_145_RDLVL_BEGIN_DELAY_3_BO            16
#define AG_MG_REGS_DENALI_CTL_145_RDLVL_BEGIN_DELAY_3_BM            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_145_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_begin_delay_2 : 16,
            rdlvl_begin_delay_3 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_145_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_146_RO                                0x00000248
#define AG_MG_REGS_DENALI_CTL_146_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_146_RDLVL_BEGIN_DELAY_4_BO            0
#define AG_MG_REGS_DENALI_CTL_146_RDLVL_BEGIN_DELAY_4_BM            0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_146_RDLVL_DELAY_0_BO                  16
#define AG_MG_REGS_DENALI_CTL_146_RDLVL_DELAY_0_BM                  0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_146_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_begin_delay_4 : 16,
            rdlvl_delay_0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_146_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_147_RO                                0x0000024C
#define AG_MG_REGS_DENALI_CTL_147_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_147_RDLVL_DELAY_1_BO                  0
#define AG_MG_REGS_DENALI_CTL_147_RDLVL_DELAY_1_BM                  0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_147_RDLVL_DELAY_2_BO                  16
#define AG_MG_REGS_DENALI_CTL_147_RDLVL_DELAY_2_BM                  0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_147_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_delay_1 : 16,
            rdlvl_delay_2 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_147_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_148_RO                                0x00000250
#define AG_MG_REGS_DENALI_CTL_148_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_148_RDLVL_DELAY_3_BO                  0
#define AG_MG_REGS_DENALI_CTL_148_RDLVL_DELAY_3_BM                  0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_148_RDLVL_DELAY_4_BO                  16
#define AG_MG_REGS_DENALI_CTL_148_RDLVL_DELAY_4_BM                  0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_148_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_delay_3 : 16,
            rdlvl_delay_4 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_148_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_149_RO                                0x00000254
#define AG_MG_REGS_DENALI_CTL_149_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_149_RDLVL_END_DELAY_0_BO              0
#define AG_MG_REGS_DENALI_CTL_149_RDLVL_END_DELAY_0_BM              0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_149_RDLVL_END_DELAY_1_BO              16
#define AG_MG_REGS_DENALI_CTL_149_RDLVL_END_DELAY_1_BM              0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_149_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_end_delay_0 : 16,
            rdlvl_end_delay_1 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_149_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_150_RO                                0x00000258
#define AG_MG_REGS_DENALI_CTL_150_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_150_RDLVL_END_DELAY_2_BO              0
#define AG_MG_REGS_DENALI_CTL_150_RDLVL_END_DELAY_2_BM              0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_150_RDLVL_END_DELAY_3_BO              16
#define AG_MG_REGS_DENALI_CTL_150_RDLVL_END_DELAY_3_BM              0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_150_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_end_delay_2 : 16,
            rdlvl_end_delay_3 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_150_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_151_RO                                0x0000025C
#define AG_MG_REGS_DENALI_CTL_151_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_151_RDLVL_END_DELAY_4_BO              0
#define AG_MG_REGS_DENALI_CTL_151_RDLVL_END_DELAY_4_BM              0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_151_RDLVL_GATE_DELAY_0_BO             16
#define AG_MG_REGS_DENALI_CTL_151_RDLVL_GATE_DELAY_0_BM             0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_151_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_end_delay_4 : 16,
            rdlvl_gate_delay_0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_151_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_152_RO                                0x00000260
#define AG_MG_REGS_DENALI_CTL_152_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_152_RDLVL_GATE_DELAY_1_BO             0
#define AG_MG_REGS_DENALI_CTL_152_RDLVL_GATE_DELAY_1_BM             0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_152_RDLVL_GATE_DELAY_2_BO             16
#define AG_MG_REGS_DENALI_CTL_152_RDLVL_GATE_DELAY_2_BM             0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_152_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_delay_1 : 16,
            rdlvl_gate_delay_2 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_152_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_153_RO                                0x00000264
#define AG_MG_REGS_DENALI_CTL_153_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_153_RDLVL_GATE_DELAY_3_BO             0
#define AG_MG_REGS_DENALI_CTL_153_RDLVL_GATE_DELAY_3_BM             0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_153_RDLVL_GATE_DELAY_4_BO             16
#define AG_MG_REGS_DENALI_CTL_153_RDLVL_GATE_DELAY_4_BM             0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_153_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_delay_3 : 16,
            rdlvl_gate_delay_4 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_153_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_154_RO                                0x00000268
#define AG_MG_REGS_DENALI_CTL_154_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_154_RDLVL_GATE_MAX_DELAY_BO           0
#define AG_MG_REGS_DENALI_CTL_154_RDLVL_GATE_MAX_DELAY_BM           0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_154_RDLVL_MAX_DELAY_BO                16
#define AG_MG_REGS_DENALI_CTL_154_RDLVL_MAX_DELAY_BM                0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_154_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_max_delay : 16,
            rdlvl_max_delay : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_154_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_155_RO                                0x0000026C
#define AG_MG_REGS_DENALI_CTL_155_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_155_RDLVL_MIDPOINT_DELAY_0_BO         0
#define AG_MG_REGS_DENALI_CTL_155_RDLVL_MIDPOINT_DELAY_0_BM         0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_155_RDLVL_MIDPOINT_DELAY_1_BO         16
#define AG_MG_REGS_DENALI_CTL_155_RDLVL_MIDPOINT_DELAY_1_BM         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_155_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_midpoint_delay_0 : 16,
            rdlvl_midpoint_delay_1 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_155_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_156_RO                                0x00000270
#define AG_MG_REGS_DENALI_CTL_156_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_156_RDLVL_MIDPOINT_DELAY_2_BO         0
#define AG_MG_REGS_DENALI_CTL_156_RDLVL_MIDPOINT_DELAY_2_BM         0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_156_RDLVL_MIDPOINT_DELAY_3_BO         16
#define AG_MG_REGS_DENALI_CTL_156_RDLVL_MIDPOINT_DELAY_3_BM         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_156_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_midpoint_delay_2 : 16,
            rdlvl_midpoint_delay_3 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_156_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_157_RO                                0x00000274
#define AG_MG_REGS_DENALI_CTL_157_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_157_RDLVL_MIDPOINT_DELAY_4_BO         0
#define AG_MG_REGS_DENALI_CTL_157_RDLVL_MIDPOINT_DELAY_4_BM         0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_157_RDLVL_OFFSET_DELAY_0_BO           16
#define AG_MG_REGS_DENALI_CTL_157_RDLVL_OFFSET_DELAY_0_BM           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_157_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_midpoint_delay_4 : 16,
            rdlvl_offset_delay_0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_157_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_158_RO                                0x00000278
#define AG_MG_REGS_DENALI_CTL_158_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_158_RDLVL_OFFSET_DELAY_1_BO           0
#define AG_MG_REGS_DENALI_CTL_158_RDLVL_OFFSET_DELAY_1_BM           0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_158_RDLVL_OFFSET_DELAY_2_BO           16
#define AG_MG_REGS_DENALI_CTL_158_RDLVL_OFFSET_DELAY_2_BM           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_158_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_offset_delay_1 : 16,
            rdlvl_offset_delay_2 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_158_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_159_RO                                0x0000027C
#define AG_MG_REGS_DENALI_CTL_159_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_159_RDLVL_OFFSET_DELAY_3_BO           0
#define AG_MG_REGS_DENALI_CTL_159_RDLVL_OFFSET_DELAY_3_BM           0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_159_RDLVL_OFFSET_DELAY_4_BO           16
#define AG_MG_REGS_DENALI_CTL_159_RDLVL_OFFSET_DELAY_4_BM           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_159_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_offset_delay_3 : 16,
            rdlvl_offset_delay_4 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_159_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_160_RO                                0x00000280
#define AG_MG_REGS_DENALI_CTL_160_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_160_TDFI_RDLVL_MAX_BO                 0
#define AG_MG_REGS_DENALI_CTL_160_TDFI_RDLVL_MAX_BM                 0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_160_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_rdlvl_max;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_160_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_161_RO                                0x00000284
#define AG_MG_REGS_DENALI_CTL_161_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_161_TDFI_RDLVL_RESP_BO                0
#define AG_MG_REGS_DENALI_CTL_161_TDFI_RDLVL_RESP_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_161_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_rdlvl_resp;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_161_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_162_RO                                0x00000288
#define AG_MG_REGS_DENALI_CTL_162_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_162_TDFI_WRLVL_MAX_BO                 0
#define AG_MG_REGS_DENALI_CTL_162_TDFI_WRLVL_MAX_BM                 0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_162_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_wrlvl_max;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_162_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DENALI_CTL_163_RO                                0x0000028C
#define AG_MG_REGS_DENALI_CTL_163_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_163_TDFI_WRLVL_RESP_BO                0
#define AG_MG_REGS_DENALI_CTL_163_TDFI_WRLVL_RESP_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_163_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_wrlvl_resp;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_163_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_164_RO                                0x00000290
#define AG_MG_REGS_DENALI_CTL_164_RM                                0xFFFF1F07

#define AG_MG_REGS_DENALI_CTL_164_TBST_INT_INTERVAL_BO              0
#define AG_MG_REGS_DENALI_CTL_164_TBST_INT_INTERVAL_BM              0x00000007

#define AG_MG_REGS_DENALI_CTL_164_TCKESR_BO                         8
#define AG_MG_REGS_DENALI_CTL_164_TCKESR_BM                         0x00001F00

#define AG_MG_REGS_DENALI_CTL_164_TDFI_PHYUPD_TYPE1_BO              16
#define AG_MG_REGS_DENALI_CTL_164_TDFI_PHYUPD_TYPE1_BM              0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_164_U
{
    struct
    {
        ag_mg_regs_register
            tbst_int_interval : 3,
            fill2 : 5,
            tckesr : 5,
            fill1 : 3,
            tdfi_phyupd_type1 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_164_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_165_RO                                0x00000294
#define AG_MG_REGS_DENALI_CTL_165_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_165_TDFI_PHYUPD_TYPE2_BO              0
#define AG_MG_REGS_DENALI_CTL_165_TDFI_PHYUPD_TYPE2_BM              0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_165_TDFI_PHYUPD_TYPE3_BO              16
#define AG_MG_REGS_DENALI_CTL_165_TDFI_PHYUPD_TYPE3_BM              0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_165_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_phyupd_type2 : 16,
            tdfi_phyupd_type3 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_165_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_166_RO                                0x00000298
#define AG_MG_REGS_DENALI_CTL_166_RM                                0xFFFF0101

#define AG_MG_REGS_DENALI_CTL_166_CKE_STATUS_BO                     0
#define AG_MG_REGS_DENALI_CTL_166_CKE_STATUS_BM                     0x00000001

#define AG_MG_REGS_DENALI_CTL_166_WRLVL_EN_BO                       8
#define AG_MG_REGS_DENALI_CTL_166_WRLVL_EN_BM                       0x00000100

#define AG_MG_REGS_DENALI_CTL_166_TDFI_RDLVL_LOAD_BO                16
#define AG_MG_REGS_DENALI_CTL_166_TDFI_RDLVL_LOAD_BM                0x00FF0000

#define AG_MG_REGS_DENALI_CTL_166_TDFI_WRLVL_LOAD_BO                24
#define AG_MG_REGS_DENALI_CTL_166_TDFI_WRLVL_LOAD_BM                0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_166_U
{
    struct
    {
        ag_mg_regs_register
            cke_status : 1,
            fill1 : 7,
            wrlvl_en : 1,
            fill0 : 7,
            tdfi_rdlvl_load : 8,
            tdfi_wrlvl_load : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_166_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_167_RO                                0x0000029C
#define AG_MG_REGS_DENALI_CTL_167_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_167_RDLVL_GATE_REFRESH_INTERVAL_BO    0
#define AG_MG_REGS_DENALI_CTL_167_RDLVL_GATE_REFRESH_INTERVAL_BM    0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_167_RDLVL_REFRESH_INTERVAL_BO         16
#define AG_MG_REGS_DENALI_CTL_167_RDLVL_REFRESH_INTERVAL_BM         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_167_U
{
    struct
    {
        ag_mg_regs_register
            rdlvl_gate_refresh_interval : 16,
            rdlvl_refresh_interval : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_167_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_168_RO                                0x000002A0
#define AG_MG_REGS_DENALI_CTL_168_RM                                0x0000FFFF

#define AG_MG_REGS_DENALI_CTL_168_WRLVL_REFRESH_INTERVAL_BO         0
#define AG_MG_REGS_DENALI_CTL_168_WRLVL_REFRESH_INTERVAL_BM         0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_168_U
{
    struct
    {
        ag_mg_regs_register
            wrlvl_refresh_interval : 16,
            fill : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_168_u;
#endif


/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_169_RO                                0x000002A4
#define AG_MG_REGS_DENALI_CTL_169_RM                                0x0001FFFF

#define AG_MG_REGS_DENALI_CTL_169_TRAS_MAX_BO                       0
#define AG_MG_REGS_DENALI_CTL_169_TRAS_MAX_BM                       0x0001FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_169_U
{
    struct
    {
        ag_mg_regs_register
            tras_max : 17,
            fill : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_169_u;
#endif

/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_170_RO                                0x000002A8
#define AG_MG_REGS_DENALI_CTL_170_RM                                0x000103FF

#define AG_MG_REGS_DENALI_CTL_170_TRFC_BO                            0
#define AG_MG_REGS_DENALI_CTL_170_TRFC_BM                            0x000003FF

#define AG_MG_REGS_DENALI_CTL_170_SREFRESH_EXIT_NO_REFRESH_BO        16
#define AG_MG_REGS_DENALI_CTL_170_SREFRESH_EXIT_NO_REFRESH_BM        0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_170_U
{
    struct
    {
        ag_mg_regs_register
            trfc : 10,
            fill1 : 6,
            srefresh_exit_no_refresh  : 1,
            fill2 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_170_u;
#endif

/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_171_RO                                0x000002AC
#define AG_MG_REGS_DENALI_CTL_171_RM                                0x03FFFFFF

#define AG_MG_REGS_DENALI_CTL_171_WRITE_MODEREG_BO                  0
#define AG_MG_REGS_DENALI_CTL_171_WRITE_MODEREG_BM                  0x03FFFFFF


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_171_U
{
    struct
    {
        ag_mg_regs_register
            write_modereg  : 26,
            fill : 6;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_171_u;
#endif


/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_172_RO                                0x000002B0
#define AG_MG_REGS_DENALI_CTL_172_RM                                0x003FFFFF

#define AG_MG_REGS_DENALI_CTL_172_MRW_STATUS_BO                     0
#define AG_MG_REGS_DENALI_CTL_172_MRW_STATUS_BM                     0x000000FF

#define AG_MG_REGS_DENALI_CTL_172_MRSINGLE_DATA_0_BO                8
#define AG_MG_REGS_DENALI_CTL_172_MRSINGLE_DATA_0_BM                0x003FFF00


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_172_U
{
    struct
    {
        ag_mg_regs_register
            mrw_status  : 8,
            mrsingle_data_0  : 14,
            fill : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_172_u;
#endif

/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_173_RO                                0x000002B4
#define AG_MG_REGS_DENALI_CTL_173_RM                                0x0F013FFF

#define AG_MG_REGS_DENALI_CTL_173_MRSINGLE_DATA_1_BO                0
#define AG_MG_REGS_DENALI_CTL_173_MRSINGLE_DATA_1_BM                0x00003FFF

#define AG_MG_REGS_DENALI_CTL_173_INHIBIT_DRAM_CMD_BO               16
#define AG_MG_REGS_DENALI_CTL_173_INHIBIT_DRAM_CMD_BM               0x00010000

#define AG_MG_REGS_DENALI_CTL_173_ADD_ODT_CLK_R2W_SAMECS_BO         24
#define AG_MG_REGS_DENALI_CTL_173_ADD_ODT_CLK_R2W_SAMECS_BM         0x0F000000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_173_U
{
    struct
    {
        ag_mg_regs_register
            mrsingle_data_1  : 14,
            fill : 2,
            inhibit_dram_cmd : 1,
            fill1 : 7,
            add_odt_clk_r2w_samecs : 4,
            fill2 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_173_u;
#endif

/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_174_RO                                0x000002B8
#define AG_MG_REGS_DENALI_CTL_174_RM                                0x003F010F

#define AG_MG_REGS_DENALI_CTL_174_ADD_ODT_CLK_W2R_SAMECS_BO         0
#define AG_MG_REGS_DENALI_CTL_174_ADD_ODT_CLK_W2R_SAMECS_BM         0x0000000F

#define AG_MG_REGS_DENALI_CTL_174_MEM_RST_VALID_BO                  8
#define AG_MG_REGS_DENALI_CTL_174_MEM_RST_VALID_BM                  0x00000100

#define AG_MG_REGS_DENALI_CTL_174_UPDATE_ERROR_STATUS_BO            16
#define AG_MG_REGS_DENALI_CTL_174_UPDATE_ERROR_STATUS_BM            0x003F0000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_174_U
{
    struct
    {
        ag_mg_regs_register
            add_odt_clk_w2r_samecs  : 4,
            fill : 4,
            mem_rst_valid : 1,
            fill1 : 7,
            update_error_status : 7,
            fill2 : 9;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_174_u;
#endif

/* V1.1 only
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DENALI_CTL_175_RO                                0x000002BC
#define AG_MG_REGS_DENALI_CTL_175_RM                                0xFFFFFFFF

#define AG_MG_REGS_DENALI_CTL_175_TDFI_CTRL_UPD_INTERVAL_BO         0
#define AG_MG_REGS_DENALI_CTL_175_TDFI_CTRL_UPD_INTERVAL_BM         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DENALI_CTL_175_U
{
    struct
    {
        ag_mg_regs_register
            tdfi_ctrl_upd_interval  : 32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_denali_ctl_175_u;
#endif


/*
 * Physical register addresses (for arm accessing ddr3)
 */
#define AG_MG_REGS_DDR3_BASE		0x9C000000
#define AG_MG_REGS_DDR3_REG(ro)		(AG_MG_REGS_DDR3_BASE+(ro))
 
#define AG_MG_REGS_DDR3_DENALI_CTL_00_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_00_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_01_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_01_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_02_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_02_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_03_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_03_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_04_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_04_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_05_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_05_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_06_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_06_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_07_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_07_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_08_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_08_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_09_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_09_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_10_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_10_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_11_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_11_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_12_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_12_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_13_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_13_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_14_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_14_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_15_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_15_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_16_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_16_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_17_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_17_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_18_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_18_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_19_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_19_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_20_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_20_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_21_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_21_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_22_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_22_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_23_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_23_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_24_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_24_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_25_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_25_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_26_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_26_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_27_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_27_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_28_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_28_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_29_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_29_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_30_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_30_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_31_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_31_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_32_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_32_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_33_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_33_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_34_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_34_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_35_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_35_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_36_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_36_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_37_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_37_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_38_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_38_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_39_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_39_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_40_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_40_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_41_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_41_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_42_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_42_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_43_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_43_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_44_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_44_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_45_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_45_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_46_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_46_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_47_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_47_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_48_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_48_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_49_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_49_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_50_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_50_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_51_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_51_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_52_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_52_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_53_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_53_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_54_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_54_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_55_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_55_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_56_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_56_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_57_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_57_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_58_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_58_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_59_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_59_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_60_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_60_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_61_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_61_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_62_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_62_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_63_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_63_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_64_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_64_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_65_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_65_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_66_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_66_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_67_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_67_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_68_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_68_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_69_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_69_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_70_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_70_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_71_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_71_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_72_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_72_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_73_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_73_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_74_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_74_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_75_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_75_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_76_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_76_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_77_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_77_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_78_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_78_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_79_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_79_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_80_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_80_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_81_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_81_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_82_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_82_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_83_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_83_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_84_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_84_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_85_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_85_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_86_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_86_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_87_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_87_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_88_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_88_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_89_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_89_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_90_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_90_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_91_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_91_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_92_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_92_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_93_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_93_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_94_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_94_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_95_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_95_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_96_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_96_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_97_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_97_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_98_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_98_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_99_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_99_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_100_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_100_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_101_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_101_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_102_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_102_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_103_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_103_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_104_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_104_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_105_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_105_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_106_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_106_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_107_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_107_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_108_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_108_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_109_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_109_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_110_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_110_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_111_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_111_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_112_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_112_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_113_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_113_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_114_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_114_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_115_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_115_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_116_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_116_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_117_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_117_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_118_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_118_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_119_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_119_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_120_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_120_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_121_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_121_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_122_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_122_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_123_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_123_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_124_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_124_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_125_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_125_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_126_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_126_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_127_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_127_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_128_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_128_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_129_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_129_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_130_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_130_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_131_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_131_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_132_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_132_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_133_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_133_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_134_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_134_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_135_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_135_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_136_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_136_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_137_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_137_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_138_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_138_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_139_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_139_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_140_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_140_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_141_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_141_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_142_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_142_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_143_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_143_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_144_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_144_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_145_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_145_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_146_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_146_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_147_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_147_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_148_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_148_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_149_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_149_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_150_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_150_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_151_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_151_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_152_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_152_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_153_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_153_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_154_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_154_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_155_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_155_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_156_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_156_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_157_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_157_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_158_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_158_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_159_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_159_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_160_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_160_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_161_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_161_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_162_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_162_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_163_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_163_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_164_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_164_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_165_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_165_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_166_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_166_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_167_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_167_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_168_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_168_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_169_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_169_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_170_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_170_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_171_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_171_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_172_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_172_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_173_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_173_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_174_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_174_RO)
#define AG_MG_REGS_DDR3_DENALI_CTL_175_RA	AG_MG_REGS_DDR3_REG(AG_MG_REGS_DENALI_CTL_175_RO)

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_DDR3_REGS_S
{
	ag_mg_regs_denali_ctl_00_u	denali_ctl_00 ;
	ag_mg_regs_denali_ctl_01_u	denali_ctl_01 ;
	ag_mg_regs_denali_ctl_02_u	denali_ctl_02 ;
	ag_mg_regs_denali_ctl_03_u	denali_ctl_03 ;
	ag_mg_regs_denali_ctl_04_u	denali_ctl_04 ;
	ag_mg_regs_denali_ctl_05_u	denali_ctl_05 ;
	ag_mg_regs_denali_ctl_06_u	denali_ctl_06 ;
	ag_mg_regs_denali_ctl_07_u	denali_ctl_07 ;
	ag_mg_regs_denali_ctl_08_u	denali_ctl_08 ;
	ag_mg_regs_denali_ctl_09_u	denali_ctl_09 ;
	ag_mg_regs_denali_ctl_10_u	denali_ctl_10 ;
	ag_mg_regs_denali_ctl_11_u	denali_ctl_11 ;
	ag_mg_regs_denali_ctl_12_u	denali_ctl_12 ;
	ag_mg_regs_denali_ctl_13_u	denali_ctl_13 ;
	ag_mg_regs_denali_ctl_14_u	denali_ctl_14 ;
	ag_mg_regs_denali_ctl_15_u	denali_ctl_15 ;
	ag_mg_regs_denali_ctl_16_u	denali_ctl_16 ;
	ag_mg_regs_denali_ctl_17_u	denali_ctl_17 ;
	ag_mg_regs_denali_ctl_18_u	denali_ctl_18 ;
	ag_mg_regs_denali_ctl_19_u	denali_ctl_19 ;
	ag_mg_regs_denali_ctl_20_u	denali_ctl_20 ;
	ag_mg_regs_denali_ctl_21_u	denali_ctl_21 ;
	ag_mg_regs_denali_ctl_22_u	denali_ctl_22 ;
	ag_mg_regs_denali_ctl_23_u	denali_ctl_23 ;
	ag_mg_regs_denali_ctl_24_u	denali_ctl_24 ;
	ag_mg_regs_denali_ctl_25_u	denali_ctl_25 ;
	ag_mg_regs_denali_ctl_26_u	denali_ctl_26 ;
	ag_mg_regs_denali_ctl_27_u	denali_ctl_27 ;
	ag_mg_regs_denali_ctl_28_u	denali_ctl_28 ;
	ag_mg_regs_denali_ctl_29_u	denali_ctl_29 ;
	ag_mg_regs_denali_ctl_30_u	denali_ctl_30 ;
	ag_mg_regs_denali_ctl_31_u	denali_ctl_31 ;
	ag_mg_regs_denali_ctl_32_u	denali_ctl_32 ;
	ag_mg_regs_denali_ctl_33_u	denali_ctl_33 ;
	ag_mg_regs_denali_ctl_34_u	denali_ctl_34 ;
	ag_mg_regs_denali_ctl_35_u	denali_ctl_35 ;
	ag_mg_regs_denali_ctl_36_u	denali_ctl_36 ;
	ag_mg_regs_denali_ctl_37_u	denali_ctl_37 ;
	ag_mg_regs_denali_ctl_38_u	denali_ctl_38 ;
	ag_mg_regs_denali_ctl_39_u	denali_ctl_39 ;
	ag_mg_regs_denali_ctl_40_u	denali_ctl_40 ;
	ag_mg_regs_denali_ctl_41_u	denali_ctl_41 ;
	ag_mg_regs_denali_ctl_42_u	denali_ctl_42 ;
	ag_mg_regs_denali_ctl_43_u	denali_ctl_43 ;
	ag_mg_regs_denali_ctl_44_u	denali_ctl_44 ;
	ag_mg_regs_denali_ctl_45_u	denali_ctl_45 ;
	ag_mg_regs_denali_ctl_46_u	denali_ctl_46 ;
	ag_mg_regs_denali_ctl_47_u	denali_ctl_47 ;
	ag_mg_regs_denali_ctl_48_u	denali_ctl_48 ;
	ag_mg_regs_denali_ctl_49_u	denali_ctl_49 ;
	ag_mg_regs_denali_ctl_50_u	denali_ctl_50 ;
	ag_mg_regs_denali_ctl_51_u	denali_ctl_51 ;
	ag_mg_regs_denali_ctl_52_u	denali_ctl_52 ;
	ag_mg_regs_denali_ctl_53_u	denali_ctl_53 ;
	ag_mg_regs_denali_ctl_54_u	denali_ctl_54 ;
	ag_mg_regs_denali_ctl_55_u	denali_ctl_55 ;
	ag_mg_regs_denali_ctl_56_u	denali_ctl_56 ;
	ag_mg_regs_denali_ctl_57_u	denali_ctl_57 ;
	ag_mg_regs_denali_ctl_58_u	denali_ctl_58 ;
	ag_mg_regs_denali_ctl_59_u	denali_ctl_59 ;
	ag_mg_regs_denali_ctl_60_u	denali_ctl_60 ;
	ag_mg_regs_denali_ctl_61_u	denali_ctl_61 ;
	ag_mg_regs_denali_ctl_62_u	denali_ctl_62 ;
	ag_mg_regs_denali_ctl_63_u	denali_ctl_63 ;
	ag_mg_regs_denali_ctl_64_u	denali_ctl_64 ;
	ag_mg_regs_denali_ctl_65_u	denali_ctl_65 ;
	ag_mg_regs_denali_ctl_66_u	denali_ctl_66 ;
	ag_mg_regs_denali_ctl_67_u	denali_ctl_67 ;
	ag_mg_regs_denali_ctl_68_u	denali_ctl_68 ;
	ag_mg_regs_denali_ctl_69_u	denali_ctl_69 ;
	ag_mg_regs_denali_ctl_70_u	denali_ctl_70 ;
	ag_mg_regs_denali_ctl_71_u	denali_ctl_71 ;
	ag_mg_regs_denali_ctl_72_u	denali_ctl_72 ;
	ag_mg_regs_denali_ctl_73_u	denali_ctl_73 ;
	ag_mg_regs_denali_ctl_74_u	denali_ctl_74 ;
	ag_mg_regs_denali_ctl_75_u	denali_ctl_75 ;
	ag_mg_regs_denali_ctl_76_u	denali_ctl_76 ;
	ag_mg_regs_denali_ctl_77_u	denali_ctl_77 ;
	ag_mg_regs_denali_ctl_78_u	denali_ctl_78 ;
	ag_mg_regs_denali_ctl_79_u	denali_ctl_79 ;
	ag_mg_regs_denali_ctl_80_u	denali_ctl_80 ;
	ag_mg_regs_denali_ctl_81_u	denali_ctl_81 ;
	ag_mg_regs_denali_ctl_82_u	denali_ctl_82 ;
	ag_mg_regs_denali_ctl_83_u	denali_ctl_83 ;
	ag_mg_regs_denali_ctl_84_u	denali_ctl_84 ;
	ag_mg_regs_denali_ctl_85_u	denali_ctl_85 ;
	ag_mg_regs_denali_ctl_86_u	denali_ctl_86 ;
	ag_mg_regs_denali_ctl_87_u	denali_ctl_87 ;
	ag_mg_regs_denali_ctl_88_u	denali_ctl_88 ;
	ag_mg_regs_denali_ctl_89_u	denali_ctl_89 ;
	ag_mg_regs_denali_ctl_90_u	denali_ctl_90 ;
	ag_mg_regs_denali_ctl_91_u	denali_ctl_91 ;
	ag_mg_regs_denali_ctl_92_u	denali_ctl_92 ;
	ag_mg_regs_denali_ctl_93_u	denali_ctl_93 ;
	ag_mg_regs_denali_ctl_94_u	denali_ctl_94 ;
	ag_mg_regs_denali_ctl_95_u	denali_ctl_95 ;
	ag_mg_regs_denali_ctl_96_u	denali_ctl_96 ;
	ag_mg_regs_denali_ctl_97_u	denali_ctl_97 ;
	ag_mg_regs_denali_ctl_98_u	denali_ctl_98 ;
	ag_mg_regs_denali_ctl_99_u	denali_ctl_99 ;
	ag_mg_regs_denali_ctl_100_u	denali_ctl_100 ;
	ag_mg_regs_denali_ctl_101_u	denali_ctl_101 ;
	ag_mg_regs_denali_ctl_102_u	denali_ctl_102 ;
	ag_mg_regs_denali_ctl_103_u	denali_ctl_103 ;
	ag_mg_regs_denali_ctl_104_u	denali_ctl_104 ;
	ag_mg_regs_denali_ctl_105_u	denali_ctl_105 ;
	ag_mg_regs_denali_ctl_106_u	denali_ctl_106 ;
	ag_mg_regs_denali_ctl_107_u	denali_ctl_107 ;
	ag_mg_regs_denali_ctl_108_u	denali_ctl_108 ;
	ag_mg_regs_denali_ctl_109_u	denali_ctl_109 ;
	ag_mg_regs_denali_ctl_110_u	denali_ctl_110 ;
	ag_mg_regs_denali_ctl_111_u	denali_ctl_111 ;
	ag_mg_regs_denali_ctl_112_u	denali_ctl_112 ;
	ag_mg_regs_denali_ctl_113_u	denali_ctl_113 ;
	ag_mg_regs_denali_ctl_114_u	denali_ctl_114 ;
	ag_mg_regs_denali_ctl_115_u	denali_ctl_115 ;
	ag_mg_regs_denali_ctl_116_u	denali_ctl_116 ;
	ag_mg_regs_denali_ctl_117_u	denali_ctl_117 ;
	ag_mg_regs_denali_ctl_118_u	denali_ctl_118 ;
	ag_mg_regs_denali_ctl_119_u	denali_ctl_119 ;
	ag_mg_regs_denali_ctl_120_u	denali_ctl_120 ;
	ag_mg_regs_denali_ctl_121_u	denali_ctl_121 ;
	ag_mg_regs_denali_ctl_122_u	denali_ctl_122 ;
	ag_mg_regs_denali_ctl_123_u	denali_ctl_123 ;
	ag_mg_regs_denali_ctl_124_u	denali_ctl_124 ;
	ag_mg_regs_denali_ctl_125_u	denali_ctl_125 ;
	ag_mg_regs_denali_ctl_126_u	denali_ctl_126 ;
	ag_mg_regs_denali_ctl_127_u	denali_ctl_127 ;
	ag_mg_regs_denali_ctl_128_u	denali_ctl_128 ;
	ag_mg_regs_denali_ctl_129_u	denali_ctl_129 ;
	ag_mg_regs_denali_ctl_130_u	denali_ctl_130 ;
	ag_mg_regs_denali_ctl_131_u	denali_ctl_131 ;
	ag_mg_regs_denali_ctl_132_u	denali_ctl_132 ;
	ag_mg_regs_denali_ctl_133_u	denali_ctl_133 ;
	ag_mg_regs_denali_ctl_134_u	denali_ctl_134 ;
	ag_mg_regs_denali_ctl_135_u	denali_ctl_135 ;
	ag_mg_regs_denali_ctl_136_u	denali_ctl_136 ;
	ag_mg_regs_denali_ctl_137_u	denali_ctl_137 ;
	ag_mg_regs_denali_ctl_138_u	denali_ctl_138 ;
	ag_mg_regs_denali_ctl_139_u	denali_ctl_139 ;
	ag_mg_regs_denali_ctl_140_u	denali_ctl_140 ;
	ag_mg_regs_denali_ctl_141_u	denali_ctl_141 ;
	ag_mg_regs_denali_ctl_142_u	denali_ctl_142 ;
	ag_mg_regs_denali_ctl_143_u	denali_ctl_143 ;
	ag_mg_regs_denali_ctl_144_u	denali_ctl_144 ;
	ag_mg_regs_denali_ctl_145_u	denali_ctl_145 ;
	ag_mg_regs_denali_ctl_146_u	denali_ctl_146 ;
	ag_mg_regs_denali_ctl_147_u	denali_ctl_147 ;
	ag_mg_regs_denali_ctl_148_u	denali_ctl_148 ;
	ag_mg_regs_denali_ctl_149_u	denali_ctl_149 ;
	ag_mg_regs_denali_ctl_150_u	denali_ctl_150 ;
	ag_mg_regs_denali_ctl_151_u	denali_ctl_151 ;
	ag_mg_regs_denali_ctl_152_u	denali_ctl_152 ;
	ag_mg_regs_denali_ctl_153_u	denali_ctl_153 ;
	ag_mg_regs_denali_ctl_154_u	denali_ctl_154 ;
	ag_mg_regs_denali_ctl_155_u	denali_ctl_155 ;
	ag_mg_regs_denali_ctl_156_u	denali_ctl_156 ;
	ag_mg_regs_denali_ctl_157_u	denali_ctl_157 ;
	ag_mg_regs_denali_ctl_158_u	denali_ctl_158 ;
	ag_mg_regs_denali_ctl_159_u	denali_ctl_159 ;
	ag_mg_regs_denali_ctl_160_u	denali_ctl_160 ;
	ag_mg_regs_denali_ctl_161_u	denali_ctl_161 ;
	ag_mg_regs_denali_ctl_162_u	denali_ctl_162 ;
	ag_mg_regs_denali_ctl_163_u	denali_ctl_163 ;
	ag_mg_regs_denali_ctl_164_u	denali_ctl_164 ;
	ag_mg_regs_denali_ctl_165_u	denali_ctl_165 ;
	ag_mg_regs_denali_ctl_166_u	denali_ctl_166 ;
	ag_mg_regs_denali_ctl_167_u	denali_ctl_167 ;
	ag_mg_regs_denali_ctl_168_u	denali_ctl_168 ;
	ag_mg_regs_denali_ctl_169_u	denali_ctl_169 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_170_u	denali_ctl_170 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_171_u	denali_ctl_171 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_172_u	denali_ctl_172 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_173_u	denali_ctl_173 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_174_u	denali_ctl_174 ;	/* V1.1 only */
	ag_mg_regs_denali_ctl_175_u	denali_ctl_175 ;	/* V1.1 only */
} ag_mg_regs_ddr3_reg_s ;

/*
* Recommended C syntax for typical usage :
*   volatile ag_mg_regs_ddr3_reg_s *ddr3_regs =
*       (volatile ag_mg_regs_ddr3_reg_s *)AG_MG_REGS_DDR3_BASE;
*/
#endif

#endif

/******** History ********
$Log: ag_mg_regs_ddr3.h,v $
Revision 1.2  2017/07/28 07:58:34  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:25  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 18:08:25  srane
Initial checkin


$Endlog$
*/

