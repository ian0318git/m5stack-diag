/* $Id: sm_woodlawn.h,v 1.6 2018/05/18 09:25:02 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn.h,v $
 *******************************************************************************
 * File Name: sm_woodlawn.h
 *
 * Description: Woodlawn SM main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef SM_WOODLAWN_H
#define SM_WOODLAWN_H

#include "ngio.h"

#define WOODLAWN_GE_BP_PACKET_NO    (10)

#define WOODLAWN_CVMX_VEND_ID       (0x177D)
#define WOODLAWN_CVMX_DEV_ID        (0x0091)

#define WOODLAWN_POWER_UP_DELAY     (1000)
#define WOODLAWN_POWER_DOWN_DELAY   (1000) 

#define WOODLAWN_PING_TOUT          (10)  /* 10 secs */
#define WOODLAWN_BL_PROMPT_TOUT     (600)  /* 600 secs */
#define WOODLAWN_DIAG_PROMPT_TOUT   (15)  /* 120 secs */
#define WOODLAWN_DIAG_IP_ADDR_SUBNET "192.123.123"
#define WOODLAWN_DIAG_IP_ADDR_BASE  (100)
#define PCI_DEVICE_FILENAME         "/proc/bus/pci/devices"

#define WOODLAWN_DIAG_IMG_FILENAME  "/tftpboot/sm_woodlawn_diag.img"
#define WOODLAWN_DEST_DIAG_IMG      "/firmware/sm_woodlawn_diag.img"
#define WOODLAWN_SRC_DIAG_IMG       "sm_woodlawn_diag.img"

#define WOODLAWN_BL_PROMPT          "SM-X-"
#define WOODLAWN_DIAG_PROMPT        "Menu item >"
#define WOODLAWN_CR_STRING          "\012"
#define WOODLAWN_ESC_CR_STRING      "\033\012"
#define WOODLAWN_CTRL_C_STRING      "\03"
#define WOODLAWN_PING_ALIVE         "is alive"
#define WOODLAWN_REMOVE_MEM_ENV     "setenv octeon_reserved_mem_linux_size\012"
#define WOODLAWN_SAVE_ENV           "saveenv\012"
#define WOODLAWN_RESET_UBOOT        "reset\012"
#define WOODLAWN_SET_IPADDR         "setenv ipaddr 192.123.123.166\012"
#define WOODLAWN_SET_NETMASK        "setenv netmask 255.255.255.0\012"
#define WOODLAWN_SET_SERVERIP       "setenv serverip 192.123.123.1\012"
#define WOODLAWN_PING_SERVERIP      "ping 192.123.123.1\012"
#define WOODLAWN_SET_ETHACT         "setenv ethact octeth4\012"
#define WOODLAWN_SET_FILENAME       "setenv l_filename /firmware/sm_woodlawn_diag.img"
#define WOODLAWN_SET_LOAD_LINUX     "setenv load_linux 'tftpboot $(loadaddr) $(l_filename);bootoctlinux $(loadaddr) coremask=1 mem=0'"
#define WOODLAWN_BOOT_UP_CMD        "run load_linux"
#define WOODLAWN_RUN_DIAG           "woodlawnnet\012"
#define WOODLAWN_TURN_GE_LPBK       "woodlawnnet -l\012"
#define WOODLAWN_TURN_ON_GE1_LPBK      "woodlawnnet -m\012"
#define WOODLAWN_TURN_OFF_GE1_LPBK      "woodlawnnet -c\012"
#define WOODLAWN_TURN_OFF_GE0_LPBK      "woodlawnnet -z\012"

#define ETHER_PACKET_LEN_MAX    1514
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

typedef struct woodlawn_ds {
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
    struct  ngio_intf_t *woodlawn_sm_iface;
} woodlawn_ds_t;

extern int set_gesw_line_loopback(int , int);
extern int get_gesw_line_loopback(int);
extern void woodlawn_get_sm_ip_addr(char *);

#endif /* SM_WOODLAWN_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_woodlawn.h,v $
 * Revision 1.6  2018/05/18 09:25:02  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.5.20.1  2017/06/13 09:28:19  leschen
 * Fix for UART kernel panic on Neptune system. This problem doesn't be found on USD or O2 systems before, will verify the changes on these platforms afterwards.
 *
 * Revision 1.5  2015/03/31 07:35:23  leschen
 * Fix for get prompt.
 *
 * Revision 1.4  2014/11/12 05:59:20  leschen
 * Add flag to turn off tlk10232 lpbk bit
 *
 * Revision 1.3  2014/10/17 07:39:53  leschen
 * Support Greyhound switch
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.3  2013/08/28 10:34:24  tirawan
 * Change boot loader keyword to adapt with latest bootloader
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.10  2013/05/30 08:43:37  leschen
 * Add macro for firmware download
 *
 * Revision 1.1.2.9  2013/05/17 03:42:11  leschen
 * Modify the seraching name of Uboot prompt when doing firmware download
 *
 * Revision 1.1.2.8  2013/04/25 07:11:13  kodko
 * Add power down delay macro
 *
 * Revision 1.1.2.7  2013/04/24 11:12:52  tirawan
 * Fix GE BP Loopback issue which sets up loopback bit through UART
 *
 * Revision 1.1.2.6  2013/04/24 07:27:38  tirawan
 * Fix intermittent boot up issue
 *
 * Revision 1.1.2.5  2013/04/18 02:30:14  tirawan
 * Hit CTRL-C to interrupt Uboot instead of using CR
 *
 * Revision 1.1.2.3  2013/04/03 05:46:40  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 * Revision 1.1.2.2  2013/03/07 12:55:02  tirawan
 * Update FPGA I2C Address, and add power on sequence for Woodlawn SM
 *
 * Revision 1.1.2.1  2013/02/06 03:04:55  tirawan
 * Woodlawn Support on O2
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

