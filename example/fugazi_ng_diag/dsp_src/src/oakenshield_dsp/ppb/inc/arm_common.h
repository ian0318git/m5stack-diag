/* $Id: arm_common.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/arm_common.h,v $
 *------------------------------------------------------------------
 * arm_common.h
 *      Oakenshield project ARM diagnostic 
 *              Location to pick up MAC address, etc. Needed due to bootloader.
 *
 * Apr. 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------
 * arm_common.h  Shared common area for bootloader to arm software
 *               communication
 *
 * March 2008, Jim Muir
 *
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __ARM_COMMON_H
#define __ARM_COMMON_H

#include "section_table.h"

typedef struct {
    section_table_t section_table;
    uint8_t mac_da[6];    /* Destination mac address */
    uint8_t mac_sa[6];    /* Source (our) mac address */
    uint16_t vlan_tag;    /* VLAN tag for outgoing messages */
    uint16_t device_id;   /* ID of this device */
} arm_common_t;

#endif

/*
 * $Log: arm_common.h,v $
 * Revision 1.2  2017/07/28 07:58:37  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:31  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

