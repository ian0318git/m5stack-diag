/* $Id: ngwic_shedir.h,v 1.4 2015/07/14 08:58:33 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shedir/ngwic_shedir.h,v $
 *------------------------------------------------------------------------------
 *
 * ngwic_shedir.h: Shedir NIM main header file
 *
 * May  2015 - Honda Wang
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#ifndef NGWIC_SHEDIR_H
#define NGWIC_SHEDIR_H

#include "ngio.h"

#define SHEDIR_GE_BP_PACKET_NO    (10)
#define SHEDIR_OK_POWERUP         (0x00)

#define SHEDIR_CVMX_VEND_ID       (0x177D)
#define SHEDIR_CVMX_DEV_ID        (0x0091)

#define SHEDIR_POWER_UP_DELAY     (1000)
#define SHEDIR_POWER_DOWN_DELAY   (1000) 

#define SHEDIR_PING_TOUT          (10)    /* 10 secs */
#define SHEDIR_BL_PROMPT_TOUT     (30)    /* 30 secs */
#define SHEDIR_DIAG_PROMPT_TOUT   (120)   /* 120 secs */
#define SHEDIR_DIAG_IP_ADDR_SUBNET "192.123.123"
#define SHEDIR_DIAG_IP_ADDR_BASE  (100)
#define PCI_DEVICE_FILENAME             "/proc/bus/pci/devices"

#define SHEDIR_DIAG_IMG_FILENAME  "/tftpboot/ngwic_shedir_diag.img"
#define SHEDIR_DEST_DIAG_IMG      "/firmware/ngwic_shedir_diag.img"
#define SHEDIR_SRC_DIAG_IMG       "ngwic_shedir_diag.img"

#define ETHER_PACKET_LEN_MAX            1514

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
#define SHOW_LTC4215_DEBUG_MESSAGE      (0)

/* NIM slot baud rate */
#define SHEDIR_NIM_SLOT1          1
#define SHEDIR_NIM_SLOT2          2
#define SHEDIR_NIM_SLOT3          3

#define WAIT_ONE_SEC() msleep(1000)
#define POST_BMC_RDY_MINS       5
#define POST_BMC_RDY_SECS       (POST_BMC_RDY_MINS * 60)

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

#define SHEDIR_B115200            0   /* Baudrate (0-115200, 1-9600)*/
#define SHEDIR_B9600              1   /* Baudrate (0-115200, 1-9600)*/ 

extern void shedir_get_wic_ip_addr(char *);

typedef struct shedir_ds {
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
    struct  ngio_intf_t *shedir_wic_iface;
} shedir_ds_t;

#endif /* NGWIC_SHEDIR_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngwic_shedir.h,v $
 * Revision 1.4  2015/07/14 08:58:33  hondwang
 * for solving shedir nim rdt issue
 *
 * Revision 1.3  2015/05/25 03:58:21  steja
 * Fix merge conflict issue
 *
 * Revision 1.2.2.2  2015/05/22 15:42:31  steja
 * Sync skye-branch2 with Maintrunk
 *
 * Revision 1.2  2015/05/14 05:31:42  hondwang
 * Merge Shedir NIM to maintrunk
 *
 * Revision 1.1.2.1  2014/08/29 03:13:21  hondwang
 * shedir project
 *
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
