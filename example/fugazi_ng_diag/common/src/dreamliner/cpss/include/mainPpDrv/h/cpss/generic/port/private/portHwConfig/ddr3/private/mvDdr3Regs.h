/* $Id: mvDdr3Regs.h,v 1.1 2015/02/13 11:34:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private/mvDdr3Regs.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell 
* International Ltd. and/or its affiliates ("Marvell") under the following 
* alternative licensing terms.  
* If you received this File from Marvell, you may opt to use, redistribute 
* and/or modify this File under the following licensing terms. 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer. 
*  -   Redistributions in binary form must reproduce the above copyright 
*       notice, this list of conditions and the following disclaimer in the 
*       documentation and/or other materials provided with the distribution. 
*  -    Neither the name of Marvell nor the names of its contributors may be 
*       used to endorse or promote products derived from this software without 
*       specific prior written permission. 
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
* mvDdr3Regs.h
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __mvDdr3Regs_H
#define __mvDdr3Regs_H

/* DFX Server regs */
#define DFX_SRV_SPARE_CONTROL                   0x44F8264

/* EMC controler offsets */
#define EMC_GLOBAL_CONTROL_0                    0x0
#define EMC_GLOBAL_CONTROL_1                    0x4
#define DDR_RESET_CONTROL                       0x8
#define DOUBLE_DUNIT_MODE_SELECTOR              0xC
#define CLIENTS_BASE_ADDRESS_MAP_0              0x100
#define CLIENTS_BASE_ADDRESS_MAP_1              0x104
#define CLIENTS_BASE_ADDRESSN                   0x108
#define WRITE_ARBITER_CLIENTS_PRIORITIES        0x128
#define WRITE_ARBITER_CLIENTS_PRIORITIES1       0x12c
#define FIFO_THRESHOLD_CONFIGURATION            0x130
#define EMC_ECC_ERROR_LOG0                      0x218
#define EMC_ECC_ERROR_LOG1                      0x21c
#define MG_ACCESS_CONTROL_1                     0x300
#define MG_ACCESS_CONTROL_2                     0x304
#define MG_ACCESS_CONTROL_3                     0x308
#define MG_ACCESS_LEVELING                      0x30c
#define MG_ACCESS_LEVELING_1                    0x310
#define DRAM_INITIALIZATION_CONTROL_AND_STATUS  0x400

/* DUNIT offsets */
#define SDRAM_CONFIGURATION                     0x1400
#define DUNIT_DDR_CONTROLLER_CONTROL_LOW        0x1404
#define DUNIT_SDRAM_TIMING_LOW                  0x1408
#define DUNIT_SDRAM_TIMING_HIGH                 0x140C
#define DUNIT_SDRAM_ADRESS_CONTROL              0x1410
#define SDRAM_OPEN_PAGES_CONTROL                0x1414
#define DUNIT_SDRAM_OPERATION                   0x1418
#define DDR_CONTROLLER_CONTROL_HIGH             0x1424
#define DDR_ODT_TIMING_LOW                      0x1428
#define DDR_DDR3_TIMING                         0x142C
#define DDR_ODT_TIMING_HIGH                     0x147C
#define SDRAM_INIT_CONTROL                      0x1480
#define DDR_CONTROLLER_ODT_CONTROL              0x149C
#define DRAM_Data_DQS_Driving_Strength          0x14C4
#define DRAM_MAIN_PADS_CALIB_M_CONTROL          0x14CC
#define DYNAMIC_POWER_SAVE                      0x1520
#define DUNIT_SDRAM_DDR_IO                      0x1524
#define DUNIT_SDRAM_DFS                         0x1528
#define DUNIT_READ_DATA_SAMPLE_DELAYS           0x1538
#define DUNIT_READ_DATA_READY_DELAYS            0x153C
#define DUNIT_TRAINING                          0x15B0
#define DUNIT_TRAINING_SW_2                     0x15B8
#define DUNIT_TRAINING_PATTERNS_BASE_ADDR       0x15BC
#define DDR3_MR0                                0x15D0
#define DDR3_MR1                                0x15D4
#define DDR3_MR2                                0x15D8
#define DDR3_MR3                                0x15DC
#define DDR3_RANK_CONTROL                       0x15E0
#define DDR3_ZQC_CONFIG                         0x15E4
#define DUNIT_DRAM_PHY_CFG                      0x15EC
#define DUNIT_ODPG_DATA_CONTROL                 0x1630
#define DUNIT_ODPG_PHASE_LENGTH                 0x1634
#define DRAM_PHY_LOCK_STATUS                    0x1674
#define PHY_REG_FILE_ACCESS                     0x16A0
#define DUNIT_ODPG_DATA_WR_ADDR                 0x16B0
#define DUNIT_ODPG_DATA_WR_HIGH                 0x16B4
#define DUNIT_ODPG_DATA_WR_LOW                  0x16B8
#define DUNIT_ODPG_DATA_WR_ERROR                0x16CC
#define DUNIT_DDR3_REG_DRAM_CONTROL             0x16D0
#define DUAL_DRAM_CONTROLLER_CFG                0x16D8

/* Forward EMC controler */
#define LU_EMC_BASE 0x03000000
#define FW_EMC_BASE 0x03200000
#define LU_DUNIT_NUMS 14
#define FWD_DUNIT_NUMS 8

#define LU_MEMORY_BASE 0x40000000
#define FW_MEMORY_BASE 0x50000000

#endif /* mvDdr3Regs_H */
/*
 *------------------------------------------------------------------
 * $Log: mvDdr3Regs.h,v $
 * Revision 1.1  2015/02/13 11:34:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
