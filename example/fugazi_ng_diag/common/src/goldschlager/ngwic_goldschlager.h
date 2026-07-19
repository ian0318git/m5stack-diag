/* $Id: ngwic_goldschlager.h,v 1.3 2015/02/12 13:39:41 jamlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/ngwic_goldschlager.h,v $
 *------------------------------------------------------------------------------
 *
 * ngwic_goldschlager.h: Goldschlager NIM main header file
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#ifndef NGWIC_GOLDSCHLAGER_H
#define NGWIC_GOLDSCHLAGER_H

#include "ngio.h"

#define GOLDSCHLAGER_GE_BP_PACKET_NO    (10)

#define GOLDSCHLAGER_CVMX_VEND_ID       (0x177D)
#define GOLDSCHLAGER_CVMX_DEV_ID        (0x0091)

#define GOLDSCHLAGER_POWER_UP_DELAY     (1000)
#define GOLDSCHLAGER_POWER_DOWN_DELAY   (1000) 

#define GOLDSCHLAGER_PING_TOUT          (10)    /* 10 secs */
#define GOLDSCHLAGER_BL_PROMPT_TOUT     (30)    /* 30 secs */
#define GOLDSCHLAGER_DIAG_PROMPT_TOUT   (120)   /* 120 secs */
#define GOLDSCHLAGER_DIAG_IP_ADDR_SUBNET "192.123.123"
#define GOLDSCHLAGER_DIAG_IP_ADDR_BASE  (100)
#define PCI_DEVICE_FILENAME             "/proc/bus/pci/devices"

#define GOLDSCHLAGER_SRC_DIAG_A_IMG        "nim_gs_a_diag.img"
#define GOLDSCHLAGER_SRC_DIAG_B_IMG        "nim_gs_b_diag.img"
#define GOLDSCHLAGER_SRC_DIAG_M_IMG        "nim_gs_m_diag.img"

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

/* NIM slot baud rate */
#define GOLDSCHLAGER_NIM_SLOT1          1
#define GOLDSCHLAGER_NIM_SLOT2          2
#define GOLDSCHLAGER_NIM_SLOT3          3

#define GOLDSCHLAGER_B115200            0   /* Baudrate (0-115200, 1-9600)*/
#define GOLDSCHLAGER_B9600              1   /* Baudrate (0-115200, 1-9600)*/ 

extern void goldschlager_get_wic_ip_addr(char *);

/* CFE parameters */
#define GS_CFE_WAIT_TIME_30S            30000  /* 30 second */
#define GS_CFE_TOUT_100MS               100
#define GS_CFE_TOUT_1S                  1000
#define GS_CFE_RETRY_30T                30
#define WAIT_FOR_FLASH_WRITE            2000     

#endif /* NGWIC_GOLDSCHLAGER_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngwic_goldschlager.h,v $
 * Revision 1.3  2015/02/12 13:39:41  jamlin
 * CFE parameter set enhancement and bug fix.
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:57  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.4  2014/06/12 07:05:33  jamlin
 * Change diag name to nim_gs_x_diag.img
 *
 * Revision 1.1.4.3  2014/03/11 09:48:03  jamlin
 * added check NG-Module image exist in firmware directory function
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.1  2013/11/02 13:39:51  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
