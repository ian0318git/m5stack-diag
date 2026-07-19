/* $Id: diag_ge_phy_88E1112C_lib.h,v 1.3 2014/11/12 06:19:41 leschen Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1112C_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1112C_lib.h 
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_88E1112C_LIB_H__
#define __DIAG_GE_PHY_88E1112C_LIB_H__

#include "smi_api.h"
#include "dev_object.h"

extern uint32_t woodlawn_88e1112c_smi_read(smi_if_t *);
extern uint32_t woodlawn_88e1112c_smi_write(smi_if_t *);
extern uint32_t woodlawn_88e1112c_smi_open(smi_if_t *);
extern uint32_t woodlawn_88e1112c_smi_close(smi_if_t *);
extern void woodlawn_88e1112c_sfp_operation(int, int);
extern dev_object_t *diag_get_88e11112c_obj(int);
extern int ge_phy_reset(boolean);
extern int setting_1112_lpbk_bit(int);

#define WOODLAWN_88E1112C_SMI_BUS   (2)
#define WOODLAWN_88E1112C_PHY_ID    (0x18)

#define MRVL_1112C_PHY_PAGE2  2
#define MRVL_1112C_PHY_PAGE22  22
#define SET_1112C_REMOTE_LPBK_VAL 0x5040
#define CLEAR_1112C_REMOTE_LPBK_VAL 0x1040
#define SET_1112C_LPBK_BIT 1
#define CLEAR_1112C_LPBK_BIT 0 

#define MAC_CTRL_REG0  0

#define GE_PHY_SUBMENU      0x80000000    /* Submenu */

#endif
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1112C_lib.h,v $
 * Revision 1.3  2014/11/12 06:19:41  leschen
 * Support turn on/off 1112 lpbk bit
 *
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.2  2013/02/19 00:49:28  leslie
 * Add and fix 88e1112c lib header file
 *
 * Revision 1.1  2013/01/16 02:35:09  leslie
 * Add Woodlawn PHY 88E1112C lib header file.
 *
 * $Endlog$
 *-------------------------------------------------
 */
