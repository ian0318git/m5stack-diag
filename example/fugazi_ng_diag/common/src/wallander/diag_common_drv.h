/* $Id: diag_common_drv.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_common_drv.h,v $
 *------------------------------------------------------------------
 * Filename: diag_common_drv.h
 *
 * Description: Device Driver Common Header Files
 * Apr 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "dev_object.h"

#ifndef DIAG_COMMON_DRV_H_
#define DIAG_COMMON_DRV_H_

#define SMI_BUS_0 (0x0);
#define SMI_BUS_1 (0x1);

#define SMI_ACCESS_RETRY_TIME       (5)

/* SMI Register Start Addr */
#define SMI_START_ADDR           (0x1180000001800ULL)
#define SMI_LENGTH               (0x0200)

/* SMI Registers */
#define SMI_CMD(bus_id)         ((bus_id) * 0x100)
#define SMI_WR_DAT(bus_id)      (0x0008 + (bus_id) * 0x100)
#define SMI_RD_DAT(bus_id)      (0x0010 + (bus_id) * 0x100)
#define SMI_CLK(bus_id)         (0x0018 + (bus_id) * 0x100)
#define SMI_EN(bus_id)          (0x0020 + (bus_id) * 0x100)
#define SMI_DRV_CTL             (0x0028)

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

extern void disable_pcs_interrupt(int, uint64_t);
extern int get_num_ports(void);

extern int wallander_phy_reg_page_rd(int, int, int, int, ushort *);
extern int wallander_phy_reg_page_wr(int, int, int, int, ushort);
extern int wallander_phy_reg_rd(int, int, int, int *);
extern int wallander_phy_reg_wr(int, int, int, int);

extern int wallander_phy_reg_page_spi_rd(int, int, int, ushort *);
extern int wallander_phy_reg_page_spi_wr(int, int, int, ushort);
extern int wallander_phy_reg_spi_rd(int, int, int *);
extern int wallander_phy_reg_spi_wr(int, int, int);

#endif /* DIAG_COMMON_DRV_H_ */


/*------------------------------------------------------------------
 * $Log: diag_common_drv.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
