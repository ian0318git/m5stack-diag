/* $Id: mon_plat_defs.h,v 1.2 2016/04/20 11:25:28 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/mon_plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * mon_plat_defs.h
 *
 * June 2015, Times Huang from O2
 * 
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MON_PLAT_DEFS_H_
#define _MON_PLAT_DEFS_H_

#define MAX_WASTETIME_VALUE     80000000

/*
 * act2 using the definition
 */
#define BOARD_TYPE_OVERLORD_REV0    0  /* Overlord P0 */
#define BOARD_TYPE_OVERLORD_REV1    1  /* Overlord P1 */
#define BOARD_TYPE_OVERLORD_REV2    2  /* Overlord P2 */

/*
 * uart oscillator frequency
 */
#define UART_OSC_FREQ		1843200
#define MSEC2USEC(msec)		(msec * 1000)
#define USEC2MSEC(usec)		(usec / 1000)

#ifndef ASMINCLUDE

/*
 * Slot Numbering.
 */  
#define EHWIC0_SLOT		0
#define EHWIC1_SLOT		1
#define EHWIC2_SLOT		2
#define EHWIC3_SLOT		3
#define SM1_SLOT		1
#define SM2_SLOT		2
#define SM3_SLOT		3
#define SM4_SLOT		4
#define MAX_NUM_PVDM_SLOTS	3
#define MAX_NUM_ISM_SLOTS	0
#define MAX_NUM_EHWIC_SLOTS	3 /* see WARNING below re NUM_HWIC_SLOTS */
#define NUM_SM_TYPE_SLOTS	5
#define MAX_NUM_DEVS_PER_SM	4

/*
 * Compact Flash
 */
#define NUM_CF			2

#define  SPI_ECAN_EEPROM	0
#define  SPI_INACTIVE		1
#define  SPI_DOWNLOAD_FPGA	2
#define  SPI_HWIC_EEPROM	3

/*
 * WARNING: Informers support up to 3 hwic slots,
 * but if defines here as 3, will get redefined error,
 * since NUM_HWIC_SLOTS also defined in /hwic_slot.h as 4
 */
#define NUM_HWIC_SLOTS		3
#define MAX_VWIC_SLOTS		NUM_HWIC_SLOTS

/*
 * cavium xformer has an max of 1 PCI NM Slot.
 */
#define MAX_PA_BAYS		MAX_NUM_SM_SLOTS
#define MAX_GOOFY_DS_PORTS	4

/*
 * Controller ID values used by Serial WIC interface Cards.
 */
#define MARVELL_VENDOR_ID	0x963411ab   /* Fellowship */

/*
 * Fellowship platform Specific I/O defines.
 */
#define MAX_MB_MOD_IDS		9
#define MAX_AIM_SLOTS		0

/*
 * The platform-specific function get_real_slot is called from both NM
 * & AIM diags.  The parameter value in ENM & VNM entries is
 * incremented by NUM_NM_TYPE_SLOTS for submenus; in AIM entries by
 * MAX_AIM_SLOTS.
 *
 * get_real_slot() distinguishes calls from * AIM diags by testing the
 * arg against AIM_SLOT_BASE.
 */
#define SM_SLOT0		0
#define SM_SLOT1		1
#define SM_SLOT2		2
#define SM_SLOT3		3
#define SM_SLOT4		4
#define SM_SLOT5		5

#define ENM_SLOT		1
#define VNM_SLOT		2
#define AIM_SLOT_BASE		10
#define AIM0_SLOT		(AIM_SLOT_BASE + 0)
#define AIM1_SLOT		(AIM_SLOT_BASE + 1)
#define SAFENET_SLOT		9 /* Its a dummy value */

/*
 * Externs needs in this file.
 */
extern uchar max_numslot;
extern void *malloc_nm(unsigned long nbytes);
extern int   get_aim_slot(int slot_index);
extern void  (*should_pause_diag)(void);
extern void  read_real_time(void);
extern int   mb_board_type(void);

#define PCI_DEFAULT		0x00 /* Hardware */
#define PCI_25MHZ		0x01
#define PCI_33MHZ		0x02
#define PCI_50MHZ		0x03
#define PCI_66MHZ		0x04

/******************************* NOTE *********************************
 * THIS IS VMCR REG DEFINE. THE REASON IT IS HERE IS THAT SYSTEMS USING
 * GT96K HAVE TO SWAP CLK SIGNALS FOR PORTS 4 AND 5 (SLOT-3)
 * MARVELL DOES NOT NEED TO DO THAT. HENCE THIS BIT IS ZERO 
 ******************************** NOTE *******************************/ 
#define SWAP_LAB_P4_P5		0x00000000

#endif /* ASMINCLUDE */

/*
 * Boot parameter
 */
#define SPD_DIMM_VACANT		0xFF
#define SPD_DIMM_PRESENT	0x00

/* for Volant NM, Chaucer AIM */
#define I82559FE_BUF_ADDR(x)	((ulong)x)
#define I82559FE_PHY_ADDR(x)	((ulong)x)

#endif  /* _MON_PLAT_DEFS_H_ */


/******** History ******** 
$Log: mon_plat_defs.h,v $
Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
