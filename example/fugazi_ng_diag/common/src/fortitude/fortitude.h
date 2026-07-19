/* $Id: fortitude.h,v 1.6 2012/11/29 17:41:08 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude.h,v $
 *------------------------------------------------------------------
 *
 * Filename:  fortitude.h
 *
 * Christine Wen - Oct. 2011.
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 */

#ifndef __FORTITUDE_H__
#define __FORTITUDE_H__

#include "wp_enet.h"

/*
 * Main menu test flag defines
 */
#define MM_1	(MF_CONTINUOUS | MF_SHOW_ERRCOUNT)
#define MM_2	(MM_1 | MF_DOALL)

typedef enum {
    CLK_MASTER = 0,
    CLK_SLAVE,
} frmr_clk_mode;

typedef enum {
    FRMR_DIG_LPBK_SLAVE = 0,
    FRMR_DIG_LPBK_MASTER,
    FRMR_EXT_LPBK_SLAVE,
    FRMR_EXT_LPBK_MASTER,
    FRMR_ALOOP_LPBK_MASTER,
    FRMR_ALOOP,
} frmr_lpbk_mode;

typedef enum {
    NPU_BP_ENET_LPBK = 0,
    NPU_BP_ENET_PASSTHRU,
    NPU_DC_ENET_LPBK,
    NPU_TDI_LPBK,
    NPU_TDI_PASSTHRU_MASTER,
    NPU_TDI_PASSTHRU_SLAVE,
} npu_lpbk_mode;

/* we use 1 SGMII port from WinNet6 - WP_PORT_ENET11 
          1 SGMII port from WinNet5 - WP_PORT_ENET9
*/
#define SGMII_INTERFACE_1        WP_PORT_ENET11  /* connect to backplane */
#define SGMII_INTERFACE_2        WP_PORT_ENET9   /* connect to daughter card */

#define NUM_ENET                 2
#define NUM_TDI                  8

#define GPIO6                    0x40
#define GPIO7                    0x80
#define GPIO8                    0x100
#define GPIO9                    0x200
#define GPIO15                   0x8000

#define FPGA_PROG_L              GPIO6
#define FPGA_CCLK                GPIO7
#define FPGA_DIN                 GPIO8
#define FPGA_DONE                GPIO9
#define FPGA_INIT                GPIO15

#define NPU_GPDR_OFFSET          0x10E00
#define NPU_GPVR_OFFSET          0x10E04

#define READY_BIT                0x00001000 /* GPIO pin 12, NGIO ready bit */

#define NPU_SERDES_CTRL_OFFSET   0x82a8

#define LINE_LPBK_EN             0x100

extern int get_num_ports();
extern boolean is_hw_rev_new();

#endif  /*  __FORTITUDE_H__ */

/* ----------------- End of File -----------------*/
/* ------ History ------------ 
$Log: fortitude.h,v $
Revision 1.6  2012/11/29 17:41:08  ywen
Add support for Nightster platform.

Revision 1.5  2012/11/01 21:37:17  ywen
Add code to set primary interface ready bit before bringing up the main menu.

Revision 1.4  2012/10/04 21:39:40  ywen
- Add code to get board HW revision information.
- Add support for different HW revision.

Revision 1.3  2012/08/29 22:45:44  ywen
Add framer analog loopback test utility.

Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
