/* $Id: sm_canis.h,v 1.8 2013/05/14 23:32:47 shhuang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/sm_canis.h,v $
 *******************************************************************************
 * File Name: sm_canis.h
 *
 * Description: Canis SM main header file
 *
 * Author: Khalid Sabzwari
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef SM_CANIS_H
#define SM_CANIS_H

#include "ngio.h"

#define ETHER_PACKET_LEN_MAX    1514

/* from Hari (hardware team) suggestion */
#define BMC_RDY_BIT_MINS        5
#define BMC_RDY_BIT_SECS        (BMC_RDY_BIT_MINS * 60)

#define PRE_BMC_RDY_MINS        4
#define PRE_BMC_RDY_SECS        (PRE_BMC_RDY_MINS * 60)
#define BMC_RDY_MINS            1
#define BMC_RDY_SECS            (BMC_RDY_MINS * 60)
#define POST_BMC_RDY_MINS       5
#define POST_BMC_RDY_SECS       (POST_BMC_RDY_MINS * 60)

#define WAIT_ONE_SEC()          msleep(1000)

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

/* LTC4215 GPIO3 pin defines to support IOS-XE features */
#define LTC4215_GPIO3_OUT       0x80
#define LTC4215_GPIO3_IN        0x04

typedef struct canis_ds {
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
    struct  ngio_intf_t *canis_sm_iface;
} canis_ds_t;

#endif /* SM_CANIS_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_canis.h,v $
 * Revision 1.8  2013/05/14 23:32:47  shhuang
 * Changed post BMC ready wait to 5 minutes per O2/Canis RDT meeting.
 *
 * Revision 1.7  2013/05/02 21:35:57  shhuang
 * Add Canis BMC ready bit set too early work-around function. (CSCug49316)
 * Add utility to switch to the work-around function.
 *
 * Revision 1.6  2013/03/14 17:19:40  shhuang
 * Extend BMC Linux boot up ready time check for Canis phase-2. (CSCuf28643)
 *
 * Revision 1.5  2013/02/21 19:20:43  shhuang
 * Set LTC4215 GPIO3 pin for Canis BMC to power up Intel side.
 *
 * Revision 1.4  2012/07/10 22:46:54  shhuang
 * Added missing wrapper #ifndef.
 *
 * Revision 1.3  2012/07/09 19:26:56  shhuang
 * Added i2c io expander defines per NGIO spec (EDCS-1108257).
 *
 * Revision 1.2  2012/06/27 09:39:46  hondwang
 * revise canis source code for proper indent
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

