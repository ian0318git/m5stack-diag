/* $Id: curie2ru_xhci.h,v 1.1 2020/01/09 01:01:58 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_xhci.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru_xhci.c - Curie2ru XHCI interfaces.
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_XHCI_H__
#define __CURIE2RU_XHCI_H__

enum {
    C2RU_USB_PORT_FRONT_A    = 6,
    C2RU_USB_PORT_FRONT_C    = 7,
    C2RU_USB_PORT_PIM        = 9,
};

enum {
    C2RU_USB_PORT_MASK_FRONT_A  = (1 << 6),
    C2RU_USB_PORT_MASK_FRONT_C  = (1 << 7),
    C2RU_USB_PORT_MASK_PIM      = (1 << 9),
};


extern int c2ru_disable_usb3_ss(int disable);
extern int __c2ru_disable_usb3_ss(int disable, unsigned int mask);

extern int c2ru_usb_2p0_mode_set(unsigned int port_mask);
extern int c2ru_usb_3p0_mode_set(unsigned int port_mask);

#endif

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_xhci.h,v $
Revision 1.1  2020/01/09 01:01:58  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
