/* $Id: ag_mg_regs.h,v 1.2 2017/07/28 07:58:33 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg_regs.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/******************************************************************************
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>    NOTIFICATION    <<<<<<<<<<<<<<<<<<<<<<<<<<<<
 *
 * Copyright (c) 2010 LSI Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 * ----------------------------------------------------------------------------
 *
 * Author:	 BAS July 2009
 * Content:	 DSS memory-mapped IO peripheral declarations
 *
 * $History: $
 *
 ******************************************************************************/

#ifndef	LSI_MG_DSSREGS_H_
#define	LSI_MG_DSSREGS_H_

/* WARNING: Using C Language Structures to access memory-mapped registers provides
 * 		a method to easily view data but writing registers using structures can lead
 *      to two types of problems:
 *
 * 	1. Accesses to bit-fields use 'read-modify-write' operations that can cause
 *     side-effects. For example setting a single bit in a R/COW1 status register
 * 	   can clear other status bits that are set at the time.
 *
 * 	   Another example is a dual write to flush DSS data cache:
 *			DCC_REGS->flush_cntl.fields.scope = 3;
 *			DCC_REGS->flush_cntl.fields.type = 3;
 *     This doesn't work, since DCC expects both fields to be written at once so
 *     it should be done as a single assignment:
 * 			DCC_REGS->flush_cntl.reg = AG_MG_REGS_DCC_FLUSH_CNTL_SCOPE_BM |
 * 				AG_MG_REGS_DCC_FLUSH_CNTL_TYPE_BM;
 *
 *  2. The APB and AHB buses in the SP2704 are only set up to decode 32-bit aligned
 *     accesses (the lower 2-bits of the address are ignored.) This works fine for
 *     the StarCore Compiler (scc) but the GNUARM compiler (arm-elf-gcc) uses
 *     byte-accesses to manipulate the bit-fields and this can cause data corruption.
 * */
#define AG_MG_REGS_USE_C_STRUCTURES

/******************************************************************************
*                                   Include Files
*******************************************************************************/
#include <stdint.h>
#include "ag_mg_regs_regops.h"

#define AG_MG_REGS_BUILT_FOR_DSS 1

#include "ag_mg_regs_cpf.h"
#include "ag_mg_regs_dcc.h"
#include "ag_mg_regs_icu.h"
#include "ag_mg_regs_ltc.h"
#include "ag_mg_regs_mpu.h"
#include "ag_mg_regs_oce.h"
#include "ag_mg_regs_pcc.h"
#include "ag_mg_regs_armctl.h"
#include "ag_mg_regs_car.h"
#include "ag_mg_regs_dbm.h"
#include "ag_mg_regs_dmac31.h"
#include "ag_mg_regs_dss.h"
#include "ag_mg_regs_gpio.h"
#include "ag_mg_regs_mac.h"
#include "ag_mg_regs_pce.h"
#include "ag_mg_regs_txd.h"
#include "ag_mg_regs_mdio.h"
#include "ag_mg_regs_pcie.h"
#include "ag_mg_regs_serdes.h"
#include "ag_mg_regs_tdm.h"
#include "ag_mg_regs_timer.h"
#include "ag_mg_regs_gttimer.h"
#include "ag_mg_regs_nand_flash.h"
#include "ag_mg_regs_wdog.h"
#include "ag_mg_regs_uart.h"
#include "ag_mg_regs_ssp.h"
#include "ag_mg_regs_ddr3.h"
#include "ag_mg_regs_ddr3phy.h"
#include "ag_mg_regs_trng.h"
#include "ag_mg_regs_wdog.h"
#include "ag_mg_regs_srio_grio.h"
#include "ag_mg_regs_srio_rab.h"
#include "ag_mg_regs_pcie.h"

extern volatile ag_mg_regs_armctrl_reg_s	ARMCTL_REG[1];
extern volatile ag_mg_regs_car_reg_s CAR_REG[1];
extern volatile ag_mg_regs_dbm_reg_s DBM_REG[1];

extern volatile ag_mg_regs_cpf_reg_s CPF_REG[1];
extern volatile ag_mg_regs_ddr3_reg_s DDR3_REG[1];
extern volatile ag_mg_regs_ddr3phy_dp_reg_s DDR3PHYDP_REG[1];
extern volatile ag_mg_regs_ddr3phy_adr_reg_s DDR3PHYADR_REG[1];
extern volatile ag_mg_regs_ddr3phy_phy_top_reg_s DDR3PHYTOP_REG[1];
extern volatile ag_mg_regs_ddr3phy_dp_external_reg_s DDR3PHYDPEXTERNAL_REG[1];

extern volatile ag_mg_regs_dcc_reg_s DCC_REG[1];
extern volatile ag_mg_regs_icu_reg_s ICU_REG[1];
extern volatile ag_mg_regs_ltc_reg_s LTC_REG[1];
extern volatile ag_mg_regs_mpu_reg_s MPU_REG[1];
extern volatile ag_mg_regs_oce_reg_s OCE_REG[1];
extern volatile ag_mg_regs_pcc_reg_s PCC_REG[1];

extern volatile ag_mg_regs_dmac31_reg_s	DMAC_REG[1];
extern volatile ag_mg_regs_dmac31_reg_s	DMAC_G0_REG[1];
extern volatile ag_mg_regs_dmac31_reg_s	DMAC_G1_REG[1];

extern volatile ag_mg_regs_dss_reg_s DSS_REG[1];
extern volatile ag_mg_regs_gpio_reg_s GPIO_REG[1];
extern volatile ag_mg_regs_gpio_reg_s PPB_GPIO_REG[1];
extern volatile ag_mg_regs_mac_reg_s MAC0_REG[1];
extern volatile ag_mg_regs_mac_reg_s MAC1_REG[1];
extern volatile ag_mg_regs_txd_reg_s TXD0_REG[1];
extern volatile ag_mg_regs_txd_reg_s TXD1_REG[1];
extern volatile ag_mg_regs_pce_reg_s PCE0_REG[1];
extern volatile ag_mg_regs_pce_reg_s PCE1_REG[1];
extern volatile ag_mg_regs_timer_reg_s TIMER_REG[1];
extern volatile ag_mg_regs_gttimer_reg_s GTTIMER_REG[1];
extern volatile ag_mg_regs_tdm_reg_s TDM_REG[1];
extern volatile ag_mg_regs_tdm_swtu_ptr_reg_s		TDM_SWTU_PTR_REG[1];
extern volatile ag_mg_regs_uart_reg_s UART_REG[1];
extern volatile ag_mg_regs_ssp_reg_s SSP_REG[1];
extern volatile ag_mg_regs_trng_reg_s TRNG_REG[1];
extern volatile ag_mg_regs_nand_flash_reg_s NAND_FLASH_REG[1];
extern volatile ag_mg_regs_mdio_reg_s MDIO_REG[1];
extern volatile ag_mg_regs_wdog_reg_s WDOG_REG[1];
extern volatile ag_mg_regs_srio_phys_reg_s SRIO0_REG[1];
extern volatile ag_mg_regs_srio_phys_reg_s SRIO1_REG[1];
extern volatile ag_mg_regs_pcie_reg_s PCIE_REG[1];

extern volatile ag_mg_regs_gige_serdes_reg_s GIGE_SERDES_REG[1];
extern volatile ag_mg_regs_pcie_serdes_reg_s PCIE_SERDES_REG[1];
extern volatile ag_mg_regs_srio_serdes_reg_s SRIO_SERDES_REG[1];


/******************************************************************************/



/* SYSCONFIG additions */
#define	AG_MG_REGS_DSS_SYSCONFG_Div2048		7
#define	AG_MG_REGS_DSS_SYSCONFG_Div1024		6
#define	AG_MG_REGS_DSS_SYSCONFG_Div512		5
#define	AG_MG_REGS_DSS_SYSCONFG_Div256		4
#define	AG_MG_REGS_DSS_SYSCONFG_Div128		3
#define	AG_MG_REGS_DSS_SYSCONFG_Div32		2
#define	AG_MG_REGS_DSS_SYSCONFG_Div8		1
#define	AG_MG_REGS_DSS_SYSCONFG_Div2		0


#define TIMER_ENABLE (1<<AG_MG_REGS_TIMER_CTRL_EN_BO)
#define TIMER_DISABLE 0

#define TIMER_PERIODIC_MOD (1<<AG_MG_REGS_TIMER_CTRL_MOD_BO)
#define TIMER_FREERUN_MOD 0

#define TIMER_INT_ENABLE (1<<AG_MG_REGS_TIMER_CTRL_INTEN_BO)
#define TIMER_INT_DISABLE 0

#define TIMERPRE_DIV1 0
#define TIMERPRE_DIV16 (1 << AG_MG_REGS_TIMER_CTRL_PRE_BO)
#define TIMERPRE_DIV256 (2 << AG_MG_REGS_TIMER_CTRL_PRE_BO)

#define TIMER_32_COUNTER (1<<AG_MG_REGS_TIMER_CTRL_SIZE_BO)
#define TIMER_16_COUNTER 0

#define TIMER_ONESHOT_MODE (1<<AG_MG_REGS_TIMER_CTRL_ONESH_BO)
#define TIMER_WRAPPING_MODE 0

#define AG_MG_REGS_CAR_RSTPROT_KEY 0x50F1C917

#define AG_MG_REGS_IS_V10_DEVICE()	 ((HW_REG_ACCESS(AG_MG_REGS_CAR_CHIPID_RA) & AG_MG_REGS_CHIPID_REV_ID_BM) == 0)

/******************************************************************************/
/* Memory Arrays as seen by DSS */

/* 512 KByte PPB Memory starts at 0 */
#define LSI_MG_DSS_PPBMEM_BASE		0xC1000000
#define LSI_MG_DSS_PPBMEM_BSIZE 	0x80000

/* 256 KByte DSS local memories as seen by the DSS */
#define LSI_MG_DSS_LM_BASE		0x40000000
#define LSI_MG_DSS_LM_BSIZE		0x40000

/******************************************************************************/
/*  memory base addresses */

/* Memory Arrays as seen from PPB */

/* 512 KByte PPB Memory starts at 0 */
#define LSI_MG_PPB_MEM_BASE		0
#define LSI_MG_PPB_MEM_BSIZE 	0x80000

/* 256 KByte DSS local memories as seen by the DSS */
#define LSI_MG_DSS_LM_BASE		0x40000000
#define LSI_MG_DSS_LM_BSIZE		0x40000

/* DSS local memories as seen on DBM */
/* 256 KByte DSS0 Local Memory starts at 0x8000000 */
#define LSI_MG_PPB_DSS0LM_BASE	0x80000000

/* 256 KByte DSS1 Local Memory starts at 0x8200000 */
#define LSI_MG_PPB_DSS1LM_BASE	0x82000000

/* 256 KByte DSS2 Local Memory starts at 0x8400000 */
#define LSI_MG_PPB_DSS2LM_BASE	0x84000000

/* 256 KByte DSS2 Local Memory starts at 0x8600000 */
#define LSI_MG_PPB_DSS3LM_BASE	0x86000000

/* SRIO space starts at 0xA0000000 */
#define LSI_MG_SRIOMEM_BASE			0xA0000000
#define LSI_MG_SRIOMEM_BSIZE		0x10000000
#define LSI_MG_SRIO_0MEM_BASE		0xA0000000
#define LSI_MG_SRIO_1MEM_BASE		0xB0000000
#define LSI_MG_SRIOMEM_BSIZE		0x10000000

/* 6 MByte System Memory starts at 0xC0000000 */
#define LSI_MG_PPB_SMEM_BASE	0xC0000000
#define LSI_MG_PPB_SMEM_BSIZE	0x600000

/* SMEM Banks are 512 KBytes */
#define LSI_MG_PPB_SMEM_BANKSIZE	0x80000

/* 512 KByte System Memory Page 12 Starts at 0xC0600000 */
#define LSI_MG_PPB_SMEM12_BASE	0xC0600000

/* 512 KByte System Memory Page 1 Starts at 0xC0040000 */
#define LSI_MG_PPB_SMEM13_BASE	0xC0680000

/* 512 KByte System Memory Page 2 Starts at 0xC0080000 */
#define LSI_MG_PPB_SMEM14_BASE	0xC0700000

/* 512 KByte System Memory Page 3 Starts at 0xC00C0000 */
#define LSI_MG_PPB_SMEM15_BASE	0xC0780000

/* PCIe space starts at 0xD0000000 */
#define LSI_MG_PCIEMEM_BASE		0xD0000000
#define LSI_MG_PCIEMEM_BSIZE	0x10000000

/* DDR3 space starts at 0xE0000000 */
#define LSI_MG_DDR3MEM_BASE		0xE0000000
#define LSI_MG_DDR3MEM_BSIZE	0x20000000

/* 64 KByte Internal ROM starts at 0xFFFF0000 */
#define LSI_MG_PPB_IROM_BASE	0xFFFF0000
#define LSI_MG_PPB_IROM_BSIZE	0x10000

/* locations cached memory spaces (DSS-only) */
#define LSI_MG_SYSMEM_CACHED_BASE		0x0
#define LSI_MG_PCIEMEM_CACHED_BASE		0x10000000
#define LSI_MG_DDR3MEM_CACHED_BASE		0x20000000


/******************************************************************************/
#endif

/* 
 * $Log: ag_mg_regs.h,v $
 * Revision 1.2  2017/07/28 07:58:33  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:24  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2012/04/18 18:08:26  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

