/* $Id: platform_pcie_clk.h,v 1.3 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pcie_clk.h,v $
 *------------------------------------------------------------------
 * platform_pcie_clk.h - Overlord PCIe CLK definitions
 *
 * May 2011, Alan Peng
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PCIE_CLK_H__
#define __PLATFORM_PCIE_CLK_H__

/* Define PCIe buffer size */
#define PCIE_CLK_BUF_SIZE   8
#define MB_I2C_ADDR_CLK_BUFFER       (0xD6 >> 1)    /* 0x6E (after shifted) */


extern int write_pcie_clk_reg(int, unsigned char);
extern int read_pcie_clk_reg(unsigned char *);
#endif /* __PLATFORM_PCIE_CLK_H__ */

/*
*--------------------------------------------------
$Log: platform_pcie_clk.h,v $
Revision 1.3  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.2.32.1  2016/10/10 17:02:41  alpeng
support NIM module for neptune

Revision 1.2  2014/08/25 23:15:11  mcharon
disable/eanble pci clock during init

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
