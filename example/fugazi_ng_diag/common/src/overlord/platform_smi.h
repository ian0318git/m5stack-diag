/* $Id: platform_smi.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_smi.h,v $
 *------------------------------------------------------------------
 * Filename: platform_smi.h
 *
 * Description: Informers SMI/MDIO header file.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SMI_H__
#define __PLATFORM_SMI_H__

/* Common defines */
#define ONE		1

/* Motherboard SMI/MDIO controllers enumberation */
typedef enum {
    MB_FPGA_SMI0 = 0,	/* Motherboard FPGA SMI controller 0 */
    MB_FPGA_SMI1,	/* Motherboard FPGA SMI controller 1 */
    MB_SMI_CTL_INVALID,	/* Invalid SMI controller */
} MB_SMI_CONTROLLER;

/* Motherboard SMI/MDIO devices enumeration */
typedef enum {
    /* IOFPGA SMI Controller */
    MB_SMI_GE_SW = 0,	/* GE switch */
    MB_SMI_PVDM_S0_0,	/* PVDM Slot 0 Address 0 */
    MB_SMI_PVDM_S0_1,	/* PVDM Slot 0 Address 1 */
    MB_SMI_PVDM_S0_2,	/* PVDM Slot 0 Address 2 */
    MB_SMI_PVDM_S0_3,	/* PVDM Slot 0 Address 3 */
    MB_SMI_PVDM_S1_0,	/* PVDM Slot 1 Address 0 */
    MB_SMI_PVDM_S1_1,	/* PVDM Slot 1 Address 1 */
    MB_SMI_PVDM_S1_2,	/* PVDM Slot 1 Address 2 */
    MB_SMI_PVDM_S1_3,	/* PVDM Slot 1 Address 3 */
    MB_SMI_PVDM_S2_0,	/* PVDM Slot 2 Address 0 */
    MB_SMI_PVDM_S2_1,	/* PVDM Slot 2 Address 1 */
    MB_SMI_PVDM_S2_2,	/* PVDM Slot 2 Address 2 */
    MB_SMI_PVDM_S2_3,	/* PVDM Slot 2 Address 3 */
    MB_SMI_INVALID,	/* Invalid SMI */
} MB_SMI_DEVICE;

/* SMI Devices defines */
/* PVDM SMI addresses. Refer to EDCS-521833 Rev 7.0, Table 7 */
#define MB_SMI_ADDR_PVDM_S0_0	0x00	/* Slot 0 Address 0 */
#define MB_SMI_ADDR_PVDM_S0_1	0x01	/* Slot 0 Address 1 */
#define MB_SMI_ADDR_PVDM_S0_2	0x02	/* Slot 0 Address 2 */
#define MB_SMI_ADDR_PVDM_S0_3	0x03	/* Slot 0 Address 3 */
#define MB_SMI_ADDR_PVDM_S1_0	0x04	/* Slot 1 Address 0 */
#define MB_SMI_ADDR_PVDM_S1_1	0x05	/* Slot 1 Address 1 */
#define MB_SMI_ADDR_PVDM_S1_2	0x06	/* Slot 1 Address 2 */
#define MB_SMI_ADDR_PVDM_S1_3	0x07	/* Slot 1 Address 3 */
#define MB_SMI_ADDR_PVDM_S2_0	0x08	/* Slot 2 Address 0 */
#define MB_SMI_ADDR_PVDM_S2_1	0x09	/* Slot 2 Address 1 */
#define MB_SMI_ADDR_PVDM_S2_2	0x0A	/* Slot 2 Address 2 */
#define MB_SMI_ADDR_PVDM_S2_3	0x0B	/* Slot 2 Address 3 */
#define MB_SMI_ADDR_GE_SW	0x10	/* GE switch */

/* Refer to 88E1112/4 datasheet. T_RESET (Minimum reset pulse width during
 * normal operation) - 10 ms
 */
/* Also refer to Marvell GE switch errata -
 *	Clock stabilization after reset
 *	Type: Functional Erratum
 *	Description: After power-up, a 100 ms time lapse is required to obtain
 *		phase lock in the internal clock multipliers (PLLs). Therefore, 
 *		RESETn must be asserted for a minimal duration of 100 ms.
 *		However, due to this erratum, the internal clock generator,
 *		a PLL, does not lock after every reset and the device does not
 *		function.
 *	Functional Impact: None
 *	Workaround: An additional reset of 100 ms is needed after the
 *		initial reset.
 */
#define PHY_RESET_TIME		110	/* 100 ms + 10% for margin */

/* The following defines should be in the device header files. Temporary use
 * this file as the holder.
 */
#define FE_PHY_MII_CTL		0	/* MII Control Register for FE PHY */
#define FE_PHY_RESET		0x8000	/* PHY reset. self clear */

#endif /* __PLATFORM_SMI_H__ */

/*------------------------------------------------------------------
$Log: platform_smi.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
