/* $Id: ngsm_aquila.h,v 1.3 2019/01/25 06:46:28 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/aquila/ngsm_aquila.h,v $
 *------------------------------------------------------------------------------
 *
 * ngsm_aquila.h: Aquila NGSM Header Files
 *
 * May  2015 - Times Huang
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#ifndef NGSM_AQUILA_H
#define NGSM_AQUILA_H

#include "ngio.h"

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

/* PCA9557 Definition */
#define PCA9557_IN_PORT_REG             0x00
#define PCA9557_OUT_PORT_REG            0x01
#define PCA9557_POLAR_INV_P_REG         0x02
#define PCA9557_CFG_PORT_REG            0x03

#define PCA9557_PORT_MASK               0xFF
#define PCA9557_PORT_INIT               0x00

#define PCA9557_IO_INPUT                0x1
#define PCA9557_IO_OUTPUT               0x0
#define PCA9557_IO_HIGH                 0x1
#define PCA9557_IO_LOW                  0x0

/* NGSM slot baud rate */
#define AQUILA_NGSM_SLOT1          1
#define AQUILA_NGSM_SLOT2          2
#define AQUILA_NGSM_SLOT3          3


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

#define AQUILA_B115200            0   /* Baudrate (0-115200, 1-9600)*/
#define AQUILA_B9600              1   /* Baudrate (0-115200, 1-9600)*/ 

#define ETHER_PACKET_LEN_MAX            1514

typedef struct aquila_ds {
    ushort  board_id;
    uchar   slot;
    uchar   uart;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   aquila_ds_addr;
    uchar   b_name[30];
    int     ge_in_port;
    int     ge_out_port;
    uchar   tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar   rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr;
    struct  ngio_intf_t *aquila_ngsm_iface;
} aquila_ds_t;

extern int aquila_rbcp_bmc_con_switch(void);
extern int aquila_rbcp_intel_con_switch(void);
extern void set_aquila_bp_loopback(int, int, int);

#endif /* NGSM_AQUILA_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngsm_aquila.h,v $
 * Revision 1.3  2019/01/25 06:46:28  alpeng
 * CSCvo11021 - fixed bug aquila and shedir will skip slot3
 *
 * Revision 1.2  2017/03/21 08:41:55  olin2
 * Collapse Aquila-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2016/04/12 06:30:12  olin2
 * Initial commit code for Aquila
 *
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
