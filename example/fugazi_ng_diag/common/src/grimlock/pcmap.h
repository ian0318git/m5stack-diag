/* $Id: pcmap.h,v 1.2 2020/03/13 12:06:54 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/pcmap.h,v $
 *------------------------------------------------------------------
 * pcmap.h
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef _PCMAP_H_
#define _PCMAP_H_

#define DRAM_VIR_TO_PHY(x)	(x)
#define DRAM_PHY_TO_VIR(x)	(x)

/*
 * Macro to map DRAM virtual space to physical space and vice versa. 
 */
#define PHYSICAL_ADDR_START      0

/*
 * Physical DRAM definitions.
 */
#define ADRSPC_RAM	 	 PHYSICAL_ADDR_START     /* start of RAM */
#define PHY_ADRSPC_RAM		 PHYSICAL_ADDR_START	/* Start of RAM */
#define ADRSPC_RAM_END		 (PHYSICAL_ADDR_START + 0x8000000) 
#define ADRSPC_RAM_SIZE		 (ADRSPC_RAM_END - ADRSPC_RAM)

#define NPU_RIF_BASE             0x1B000000
#define NPU_CS1_BASE             0x1D000000
#define NPU_CS2_BASE             0x1D100000

#define FPGA_BASE                NPU_CS2_BASE
#define FRAMER_BASE              NPU_CS1_BASE
#define FPGA_OFFSET              (FPGA_BASE - NPU_RIF_BASE)
#define FRAMER_OFFSET            (FRAMER_BASE - NPU_RIF_BASE)

#define WP3_MMAP_LEN             (NPU_CS2_BASE + 0x10000 - NPU_RIF_BASE) 

/* The following definitions for IRQ is from Linux kernel(need to confirm) */
#define NPU_INT1                 28           /* from FPGA */
#define NPU_INT2                 29           /* from Framer */

/* The first 8 interrupts are reserved for internal use for MIPs core. */
#define WINPATH_IRQ_BASE         8

#define UNCACHE_ADDR_MSK         0xA0000000  /* KSEG1 region */

//-----------------------------------------------
//        block = I2C
//-----------------------------------------------
#ifndef MAP_CHCIIMT
#define MAP_CHCIIMT  0
#endif

#define MAP_I2C_OFFSET            (MAP_CHCIIMT + 0x10E80)
#define MAP_I2C_SIZE              0x80

#define MAP_I2C_CFG               (MAP_CHCIIMT + 0x10e80)   
#define MAP_I2C_CLKDIV            (MAP_CHCIIMT + 0x10e84)  
#define MAP_I2C_DEV_ADDR          (MAP_CHCIIMT + 0x10e88)   
#define MAP_I2C_ADDR              (MAP_CHCIIMT + 0x10e8c)   
#define MAP_I2C_DATA_OUT          (MAP_CHCIIMT + 0x10e90)   
#define MAP_I2C_DATA_IN           (MAP_CHCIIMT + 0x10e94)   
#define MAP_I2C_STATUS            (MAP_CHCIIMT + 0x10e98)   
#define MAP_I2C_SDEN              (MAP_CHCIIMT + 0x10e9c)   
#define MAP_I2C_BYTCNT            (MAP_CHCIIMT + 0x10ea0)  

extern unsigned long get_npu_rif_base(void);
extern unsigned long get_fpga_base(void);
extern unsigned long get_framer_base(void);

#endif /* _PCMAP_H_ */

/******** History ********
$Log: pcmap.h,v $
Revision 1.2  2020/03/13 12:06:54  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.2  2020/01/15 03:30:12  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
