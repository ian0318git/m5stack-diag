/* $Id: vm_timingcard.h,v 1.3 2015/02/18 06:08:26 bowang3 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard.h,v $
 *******************************************************************************
 * File Name: vm_timingcard.h
 *
 * Description: Timing Card NGVM main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMINGCARD_H
#define VM_TIMINGCARD_H

#include "ngio.h"
#include "i2c_api.h"

/*
 * I2C IO Expnader pins per NGIO spec (EDCS-1108257)
 *
 * GPIO bit   Direction       Signal        Applied on Canis
 * --------   ---------   ---------------   ----------------
 *    0         IN        db_present_l      always set to 1
 *    1         OUT       boot_select       always set to 1
 *    2         OUT       db_reset_l        connect to BMC for future use
 *    3         IN        primary_if_ready  ready from BMC CPU
 *    4         OUT       uart_mux_select   N/A
 *    5         OUT       reset_config_l    N/A
 *    6         -         -                 N/A
 *    7         -         -                 N/A
 */

/* IOE bit defines */
#define IOE_DB_PRESENT_L        0x01    /* bit 0: In    */
#define IOE_BOOT_SEL            0x02    /* bit 1: Out   */
#define IOE_DB_RESET_L          0x04    /* bi2 2: Out   */
#define IOE_PRIM_IF_READY       0x08    /* bit 3: In    */
#define IOE_UART_MUX_SEL        0x10    /* bit 4: Out   */
#define IOE_RESET_CFG_L         0x20    /* bit 5: Out   */
#define IOE_BIT6                0x40    /* bit 6: Undef */
#define IOE_BIT7                0x80    /* bit 7: Undef */

/* IOE direction */
#define IOE_IN  1
#define IOE_OUT 0
#define IOE_ALL_IN              0xff    /* all bits as input */

#define ETHER_PACKET_LEN_MAX    1514

typedef struct timingcard_ds {
    ushort  board_id;
    uchar   slot;
    uchar   uart;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   patriot_ds_addr;
    uchar   b_name[30];
    int     ge_in_port;
    int     ge_out_port;
    uchar   tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar   rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr;
    struct  ngio_intf_t *timingcard_vm_iface;
} timingcard_ds_t;

extern int timingcard_vm_test(void *);
extern int timingcard_init_seq(void);
extern int get_timingcard_sku_id(void);
extern n2g_i2c_if_t *get_timingcard_i2c_device(void);
extern void clear_timingcard_init_flag(void);
extern void set_timingcard_i2c_addr(void);

#endif /* VM_TIMINGCARD_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard.h,v $
 * Revision 1.3  2015/02/18 06:08:26  bowang3
 * Support Wallander NIM 1588 test with timing card
 *
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.3  2014/04/22 06:06:02  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.2  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

