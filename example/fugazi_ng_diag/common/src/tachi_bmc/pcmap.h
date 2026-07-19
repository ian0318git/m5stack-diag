/* $Id: pcmap.h,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines for Informers.
 *
 * June 2015, Times Huang adapted from O2
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PCMAP_H_
#define _PCMAP_H_

/*
 * Macro to map DRAM virtual space to physical space and vice versa. 
 */
#define PHYSICAL_ADDR_START     0

/*
 * Physical DRAM definitions.
 */
#define ADRSPC_RAM      PHYSICAL_ADDR_START     /* start of RAM */
#define PHY_ADRSPC_RAM      PHYSICAL_ADDR_START /* Start of RAM */
#define ADRSPC_RAM_END      (PHYSICAL_ADDR_START + 0x80000000) 
#define ADRSPC_RAM_SIZE     (ADRSPC_RAM_END - ADRSPC_RAM)

#endif /* _PCMAP_H_ */


/******** History ******** 
$Log: pcmap.h,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project


$Endlog$
*/
