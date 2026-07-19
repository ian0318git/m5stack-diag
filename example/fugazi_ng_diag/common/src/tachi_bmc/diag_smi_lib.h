/* $Id: diag_smi_lib.h,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_smi_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_smi_lib.h - smi Library
 *
 * June 2015, Ben Chen
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_SMI_LIB_H__
#define __DIAG_SMI_LIB_H__

/** Prototypes. **/
extern int diag_smi_reg_read(char *, ulong , ulong, ulong *);
extern int diag_smi_reg_write(char *, ulong , ulong, ulong);
extern int diag_smi_6320_reg_read(char *, int, int, int, int *);
extern int diag_smi_6320_reg_write(char *, int, int, int, int);

typedef enum {
    SMI_CMD_REG = 0,
    SMI_DATA_REG,
} dev_mrvl_multichip_addr_mode;

#define PHY_DEVICE_NAME           "eth0"
#define PILOT_III_ETH_PORT_B           "eth1"
#define MRV88E6320_SWITCH_ID      (0x8)
#define MRV88E6320_PORT_NUM       (6)
#define MRVL6320_SMI_BUSY_MODE    (0x9000)
#define MRVL6320_SMI_READ         (0xA00)
#define MRVL6320_SMI_WRITE        (0x600)

#define MRVL6320_DEV_ADDR(x)  x<<5
#define MRVL6320_REG_ADDR(x)  x

#endif /* __DIAG_SMI_LIB_H__ */

/*---------------------------------------------------------------
$Log: diag_smi_lib.h,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/04 02:26:08  hondwang
add port b define

Revision 1.1.2.1  2015/07/31 07:22:30  hondwang
smi lib

$Endlog$
*/
