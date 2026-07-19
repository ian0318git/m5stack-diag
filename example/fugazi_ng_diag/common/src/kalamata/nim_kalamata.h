/* $Id: nim_kalamata.h,v 1.3 2018/05/18 09:24:50 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/kalamata/nim_kalamata.h,v $
 *------------------------------------------------------------------------------
 *
 * nim_kalamata.h: Kalamata NIM Header Files
 *
 * June 2018 - Kody Ko
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#ifndef NIM_AQUILA_H
#define NIM_AQUILA_H

#include "ngio.h"

/* from Hari (hardware team) suggestion */
#define BMC_RDY_BIT_MINS                5
#define BMC_RDY_BIT_SECS                (BMC_RDY_BIT_MINS * 60)

#define PRE_BMC_RDY_MINS                4
#define PRE_BMC_RDY_SECS                (PRE_BMC_RDY_MINS * 60)
#define BMC_RDY_MINS                    1
#define BMC_RDY_SECS                    (BMC_RDY_MINS * 60)
#define POST_BMC_RDY_MINS               5
#define POST_BMC_RDY_SECS               (POST_BMC_RDY_MINS * 60)

#define WAIT_ONE_SEC()                  msleep(1000)
#define WHITE_SPACE_NUM                 9 /* P1A2 MFG pads PID with 9 white spaces. */
#define ROUND                           10


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

/* NIM slot baud rate */
#define KALAMATA_NIM_SLOT1              1
#define KALAMATA_NIM_SLOT2              2
#define KALAMATA_NIM_SLOT3              3

/* IOE bit defines */
#define IOE_DB_PRESENT_L                0x01    /* bit 0: In    */
#define IOE_BOOT_SEL                    0x02    /* bit 1: Out   */
#define IOE_DB_RESET_L                  0x04    /* bi2 2: Out   */
#define IOE_PRIM_IF_READY               0x08    /* bit 3: In    */
#define IOE_UART_MUX_SEL                0x10    /* bit 4: Out   */
#define IOE_RESET_CFG_L                 0x20    /* bit 5: Out   */
#define IOE_BIT6                        0x40    /* bit 6: Undef */
#define IOE_BIT7                        0x80    /* bit 7: Undef */


/* IOE direction */
#define IOE_IN                          1
#define IOE_OUT                         0
#define IOE_ALL_IN                      0xff    /* all bits as input */

#define KALAMATA_B115200                0   /* Baudrate (0-115200, 1-9600)*/
#define KALAMATA_B9600                  1   /* Baudrate (0-115200, 1-9600)*/ 

#define ETHER_PACKET_LEN_MAX            1514

typedef struct kalamata_ds {
    ushort  board_id;
    uchar   slot;
    uchar   uart;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   kalamata_ds_addr;
    uchar   b_name[30];
    int     ge_in_port;
    int     ge_out_port;
    uchar   tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar   rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr;
    struct  ngio_intf_t *kalamata_nim_iface;
} kalamata_ds_t;

extern void set_kalamata_bp_loopback(int, int, int);
extern long kalamata_rbcp_picocom_switch(void);
extern int tftp_get (unsigned char *dir, unsigned char *file,
     unsigned char *server_ip, unsigned char *dest, unsigned int check);

#endif /* NIM_KALAMATA_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: nim_kalamata.h,v $
 * Revision 1.3  2018/05/18 09:24:50  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.2.14.2  2018/04/20 08:45:41  alpeng
 * support kalamata on Neptune
 *
 * Revision 1.2  2018/02/24 07:36:25  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.1.4.3  2017/09/21 01:38:02  kodko
 * Support ISR4K platform slot2 and slot3.
 *
 * Revision 1.1.4.2  2017/06/16 07:17:03  kodko
 * Initial platform code commit for Kalamata project.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
