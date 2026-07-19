/* $Id: platform_xfi.h,v 1.2 2018/05/18 09:24:58 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_xfi.h,v $
 *------------------------------------------------------------------
 * Filename: platform_xaui.h
 *
 * Description: XAUI interface Common Header Files
 * Author: Mecca Ho
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
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
#define SEL_PORT_XFI "xfi"

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
 * Revision 1.2  2018/05/18 09:24:58  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.9  2017/04/10 05:27:25  meho
 * Integrated BCM82752/82757 API.
 *
 * Revision 1.1.2.8  2017/01/09 08:46:19  meho
 * rename eth to xfi for 10G PHY.
 *
 * Revision 1.1.2.7  2016/11/28 03:43:55  meho
 * 1. Fixed GE phy Mac/Int/Ext loopback test bugs.
 * 2. Added 10G FW download.
 *
 * Revision 1.1.2.6  2016/08/18 06:57:49  meho
 * Code clean up.
 *
 * Revision 1.1.2.5  2016/08/12 10:12:19  meho
 * Clean up code.
 *
 * Revision 1.1.2.4  2016/07/26 10:13:45  meho
 * clean up code.
 *
 * Revision 1.1.2.3  2016/07/20 01:45:00  meho
 * Added GE PHY loopback debug utilities.
 *
 * Revision 1.1.2.2  2016/07/14 09:17:41  meho
 * Added internal/SFP-external loopback for BCM82752.
 *
 * Revision 1.1.2.1  2016/07/07 09:04:31  meho
 * 1. Added BCM54194 RDB register r/w utility.
 * 2. Added GE PHY internal/external loopback skeleton.
 * 3. Added 10GE PHY internal/external loopback skeleton.
 *
 *
 * $Endlog$
 */
