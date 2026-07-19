/* $Id: nutella_comm.h,v 1.5 2020/03/06 07:42:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/nutella_comm.h,v $
 *------------------------------------------------------------------
 * 
 * nutella_comm.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _NUTELLA_COMM_H_
#define _NUTELLA_COMM_H_

#define BOOTLOADER_IS_ROMMON_FLAG    "/lib/modules/4.14.3/rommon.txt" 

extern int  nutella_mem_read32(uint, uint *);
extern int  nutella_mem_write32(uint, uint);
extern int is_bootloader_rommon(void);


#endif /* _NUTELLA_COMM_H_ */

/*-------------------------------------------------
$Log: nutella_comm.h,v $
Revision 1.5  2020/03/06 07:42:32  alicehua
CSCvt28948:
1. Modify codes for FPGA register default value changing issue.
   With new ROMMON (17.3(03d)), FPGA register (0x010) will get 0x59,
   so we just check bit 7:0, ignore bit 0.
2. Add a function to distinguish bootloader is BIOS or ROMMON,
   so that we can hide IRQ test items if bootloader is BIOS.

Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
