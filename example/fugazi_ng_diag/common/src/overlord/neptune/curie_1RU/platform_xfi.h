/* $Id: platform_xfi.h,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_xfi.h,v $
 *------------------------------------------------------------------
 * Filename: platform_xaui.h
 *
 * Description: XAUI interface Common Header Files
 * Author: Mecca Ho
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef PLATFORM_XFI_LIB_H_
#define PLATFORM_XFI_LIB_H_

#define WOODLAWN_DIAG_IP_ADDR_SUBNET "192.123.123"
#define WOODLAWN_DIAG_IP_ADDR_BASE  (100)

#define INFO_LEN               (10000)
#define EN_XFI_EXT_LPBK        (1)
#define DIS_XFI_EXT_LPBK       (0)

#define SPD_10000MBPS   10000
#define SEL_PORT_XFI "eth"

enum xfi_pnum {
  XFI0 = 0,
  XFI1,
  XFI2,
  XFI3,
};

extern int xfi_mapping_qlm_num[];

extern void set_xfi_int_lpbk (int, boolean);
extern boolean cavium_is_xfi_linkup (int);

#endif /* PLATFORM_XAUI_LIB_H_ */

/*
 * $Log: platform_xfi.h,v $
 * Revision 1.2  2019/08/06 06:56:14  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.1.2.1  2019/03/12 07:41:52  leschen
 * Initial check in to support BCM82752
 *
 *
 * $Endlog$
 */
