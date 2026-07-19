/* $Id: pwr_seq_diag.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/pwr_seq_diag.h,v $
 *------------------------------------------------------------------
 * pwr_seq_diag.h: Head file for Power Sequencer Diag definitions.
 *
 * May 20, 2014 - Paul Lin(palin2) created.
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PWR_SEQ_DIAG_H__
#define __PWR_SEQ_DIAG_H__

/* Common Definition */
#define PS_REG_SZ      256
#define PLAT_VOLT_SZ   7
#define PS_BUF_SZ      32

/* Definition of Page number for Voltage Rail */
#define VOLT_3P3V_PAGE        0x00
#define VOLT_2P5V_PAGE        0x01
#define VOLT_1P2V_PAGE        0x02
#define VOLT_1P8V_PAGE        0x03
#define VOLT_1P0V_PAGE        0x04
#define VOLT_1P35V_GX0_PAGE   0x05
#define VOLT_1P35V_GX1_PAGE   0x0A
#define VOLT_ALL_PAGE         0xFF

/* Power Sequencer: TI UCD90120 */
/* Definition of Command code */
#define PS_PAGE               0x00
#define PS_OPERATION          0x01

/* Definition of Operation command */
#define VOLT_NORMAL           0x80
#define VOLT_MARGIN_LOW       0x94
#define VOLT_MARGIN_HIGH      0xA4

/* Definition of Voltage margin setup info structure */
typedef struct vm_setup_info {
    char    *name;
    uchar   value;
} vm_setup_info_t;

/*
 * Externs
 */


#endif /* __PWR_SEQ_DIAG_H__ */

/******** History ********/
/*
 * $Log: pwr_seq_diag.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:27  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:38  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */

