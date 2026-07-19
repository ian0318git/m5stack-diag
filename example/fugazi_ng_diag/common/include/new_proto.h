/* $Id: new_proto.h,v 1.3 2013/05/09 19:25:15 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/new_proto.h,v $
 *------------------------------------------------------------------
 * new_proto.c - Contains function definitions to help in clean compiles..
 * 
 * Nov. 2007, Alan O'Sullivan
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __NEW_PROTO_H__
#define __NEW_PROTO_H__

extern void init_interrupt_vector (void);
extern void turn_off_bev_bit (void);
extern void init_netboot (void);
extern void reset_octeon (void);
extern int  check_octeon_bist_results(void);
extern int  config_octeon_cache(void);

extern uchar get_msi_data_value (int sm_slot);
extern ulong get_msi_addr_lo (void);
extern int get_real_sm_slot (int slot);
extern void reset_sm_module (int slot, boolean enable);
extern boolean is_sm_present (int slot);
extern void * get_io_base_address (uint slot);
extern void platform_config_pcie_ism(void);
#endif

/* ------ End of Module ------ */


/******** History ******** 
$Log: new_proto.h,v $
Revision 1.3  2013/05/09 19:25:15  mcharon
remove unused header files. fixed dependancy compile problem

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
