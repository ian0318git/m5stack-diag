 /* $Id: plat_defs.h,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Tabei platform defines.
 *
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_

#include "types.h"

#define TABEI_FPGA_REG_WIDTH    4

/* define sleep seconds */
#define LENGTH100             100
#define LENGTH1000            1000
#define LENGTH1024            1024

#define LENGTH32               32
#define LENGTH64               64 

/* define usb register */
#define USB_PORT_CTRL			"0480"
#define USB_PORT_CTRL_TEST		"0484"

/* GESW use these macro
 *  */
#define TGT_DEV_CPU     0 /* Control Plane CPU */
#define TGT_DEV_NGSM    1
#define TGT_DEV_NGWIC   2
#define TGT_DEV_NGVM    3
#define TGT_DEV_DP      4 /* Data Plane cpu */

/* PLX device id and vendor id (Dummy) */
#define PLX_PCIE_SW_VID       0x10b5  /* vendor id */
#define PLX_PCIE_SW_DID_8618  0x8618  /* device id Juno, Utah*/
#define PLX_PCIE_SW_DID_8617  0x8617  /* device id Sword */
#define PLX_PCIE_SW_DID_8604  0x8604  /* device id dagger */

/* IDT device id and vendor id */
#define IDT_PCIE_SW_VID     0x111d  /* vendor id */
#define IDT_PCIE_SW_DID     0x8090  /* device id */


/* NGIO module local GE port numbering used in the port mapping
 * table in bcm_gesw_api.c
 * NGIO module local GE port bit mask defines
 */
enum ngio_port_num {
  NGIO_GE0 = 0,
  NGIO_GE1,
};
#define NGIO_GE0_BITMASK     0x1
#define NGIO_GE1_BITMASK     0x2

#define TABEI_NIM_MAX_ETH_PORT   1

#define QUICK_MODE                1

/* WIC and SM slot number starts from 1
 *  */
#define NGWIC1_SLOT             1
#define NGWIC2_SLOT             2
#define NGWIC3_SLOT             3

#define NGSM1_SLOT              1
#define NGSM2_SLOT              2
#define NGSM3_SLOT              3
#define NGSM4_SLOT              4 /* Neptune has a pseudo SM4 which
                                     needed to supoort double wide SM3 */


int skip_init_seq;

/* Extern */
extern int usb_dump_x(int);
extern int get_i2c_fd(int);
extern int diag_extend_feature(boolean);
extern int tabei_show_cpuinfo(void);
extern void tabei_show_meminfo(void);
extern uint host_ngio_10gkr_capability (uint, uint);

#endif                          /* _PLAT_DEFS_H_ */
/*-------------------------------------------------
 * $Log: plat_defs.h,v $
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.9  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.4.8  2019/03/07 05:53:15  olin2
 * Clean up code
 *
 * Revision 1.1.4.7  2019/01/18 02:30:16  olin2
 * Clean up code
 *
 * Revision 1.1.4.6  2018/11/16 05:42:12  olin2
 * Clean up code
 *
 * Revision 1.1.4.5  2018/10/23 11:34:26  olin2
 * Support Testcard test
 *
 * Revision 1.1.4.4  2018/10/15 11:48:29  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.4.3  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
