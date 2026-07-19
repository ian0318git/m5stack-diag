/* $Id: plat_defs.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/plat_defs.h,v $
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

#define PHOENIX_FPGA_REG_WIDTH    4

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

/* WIC and SM slot number starts from 1
 */
#define NGWIC1_SLOT		1
#define NGWIC2_SLOT		2
#define NGWIC3_SLOT		3

#define NGSM1_SLOT		1
#define NGSM2_SLOT		2
#define NGSM3_SLOT		3
#define NGSM4_SLOT		4 /* Neptune has a pseudo SM4 which
				     needed to supoort double wide SM3 */

#define PHOENIX_NIM_MAX_ETH_PORT   1

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

/* 
 * Host CPU GE ports defines
 */
#define CPU_SGMII_PORT0         0
#define CPU_SGMII_PORT1         1
#define CPU_SGMII_PORT2         2
#define CPU_SGMII_PORT3         3
#define CPU_SGMII_PORT4         4
#define CPU_SGMII_PORT5         5
#define CPU_SGMII_PORT6         6
#define CPU_SGMII_PORT7         7
#define CPU_SGMII_PORT8         8

int skip_init_seq;

/* Extern */
extern int usb_dump_x(int);
extern int get_i2c_fd(int);
extern int diag_extend_feature(boolean);
extern int phoenix_show_cpuinfo(void);
extern void phoenix_show_meminfo(void);
extern uint host_ngio_10gkr_capability (uint, uint);

#endif                          /* _PLAT_DEFS_H_ */
